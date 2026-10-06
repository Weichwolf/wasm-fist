"""Original default ground-model dispatch, node construction and facing rounding.

The host supplies the original two-part model count and an output node at the
render-list boundary. Original instructions remain unmodified; this does not
execute gameplay initialization, view-bearing recovery or sprite rasterization.
"""
import hashlib
import struct

from original_model_oracle import OriginalModelOracle
from original_unit_oracle import DGROUP, ROOT

MGA_SHA256 = '54bb7b7b61371c63d8109ac658ee9cd4ffb2cb1c3ab09fae04673a0f3b09c037'
MGA = 0x80000
MGA_DATA = MGA + 0x46a0
UNIT = 0x7000
OUTPUT = 0x7200


class OriginalVehicleOracle:
    def __init__(self):
        self.model = OriginalModelOracle()
        self.mga = (ROOT / 're_out/fist_mga_image.bin').read_bytes()
        if hashlib.sha256(self.mga).hexdigest() != MGA_SHA256:
            raise RuntimeError('Frozen original MGA image does not match its pin')
        self.names = self.catalog()

    def machine(self):
        from unicorn.x86_const import UC_X86_REG_SS, UC_X86_REG_SP
        machine = self.model.machine()
        machine.mem_map(0x70000, 0x30000)
        machine.mem_write(MGA, self.mga)
        machine.mem_write(DGROUP + 0x70a, struct.pack('<H', MGA_DATA // 16))
        machine.reg_write(UC_X86_REG_SS, 0x1c00)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        return machine

    def catalog(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DS
        machine = self.machine()
        names = {}
        # STR's first complete loader list, after its 8-byte sentinel/header.
        cursor = 0x2d740 + 0x80a
        while True:
            lod, name_index, code, method = struct.unpack_from('<4H', self.model.original.image, cursor)
            cursor += 8
            if lod == 65535:
                break
            if method not in (0x183f, 0x1684, 0x1735) or lod not in (8, 16, 32):
                raise RuntimeError('Original complete model-loader list changed')
            machine.reg_write(UC_X86_REG_DS, 0x1c00)
            machine.reg_write(UC_X86_REG_AX, name_index)
            self.model.original.execute(machine, MGA + 0x197, MGA + 0x207, 0x8000)
            filename = bytes(machine.mem_read(DGROUP + 0x740, 13))
            if filename[8:] != b'.MXX\0':
                raise RuntimeError('Original filename service did not emit a model basename')
            name = filename[:8].rstrip(b' ').decode('ascii')
            if code in names and names[code] != name:
                raise RuntimeError('Original model code has conflicting filenames')
            names[code] = name
        if sorted(names) != list(range(0, 68, 2)):
            raise RuntimeError('Original complete catalog is absent/incomplete')
        return names

    def facing(self, heading, bearing):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DX
        # Inputs immediately after 059a has recovered the bearing and popped the
        # heading into DX. Execute actual subtraction and all rounding/wrap ops.
        machine = self.machine()
        machine.reg_write(UC_X86_REG_AX, bearing)
        machine.reg_write(UC_X86_REG_DX, heading)
        self.model.original.execute(machine, 0x5a9, 0x5b6)
        return machine.reg_read(UC_X86_REG_AX) // 2

    def all_facings(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DX
        machine = self.machine()
        facings = []
        for heading in range(65536):
            machine.reg_write(UC_X86_REG_AX, 0)
            machine.reg_write(UC_X86_REG_DX, heading)
            self.model.original.execute(machine, 0x5a9, 0x5b6)
            facings.append(machine.reg_read(UC_X86_REG_AX) // 2)
        return facings

    def visual(self, state, bearing):
        from unicorn.x86_const import (UC_X86_REG_SI, UC_X86_REG_DI, UC_X86_REG_ES,
                                      UC_X86_REG_DX, UC_X86_REG_BP)
        kind, = struct.unpack_from('<H', state)
        if kind >= 4 or len(state) != 251:
            raise RuntimeError('Ground-vehicle oracle needs a complete supported snapshot')
        machine = self.machine()
        machine.mem_write(DGROUP + UNIT, state)
        machine.reg_write(UC_X86_REG_SI, UNIT)
        self.model.original.execute(machine, 0xc4fb, 0xc507)
        code = machine.reg_read(UC_X86_REG_DX)
        machine.mem_write(DGROUP + 0x276c + code, struct.pack('<H', 2))
        machine.reg_write(UC_X86_REG_ES, 0x1c00)
        machine.reg_write(UC_X86_REG_DI, OUTPUT)
        machine.reg_write(UC_X86_REG_BP, 0)
        self.model.original.execute(machine, 0xc5fc, 0xc60e)
        node = bytes(machine.mem_read(DGROUP + OUTPUT, 54))
        if bytes(machine.mem_read(DGROUP + UNIT, len(state))) != state:
            raise RuntimeError('Original node builder did not restore the complete source')
        if node[4:6] != bytes((30, code)) or machine.reg_read(UC_X86_REG_DI) != OUTPUT + 54:
            raise RuntimeError('Original builder did not emit a complete two-part vehicle node')
        scale, = struct.unpack_from('<H', node, 6)
        primary, = struct.unpack_from('<H', node, 8)
        secondary, = struct.unpack_from('<H', node, 52)
        parts = list(node[50:52])
        headings = (primary, secondary)
        poses = []
        for part, flags in enumerate(parts):
            # Actual renderer's per-part direction-selection branch. Descriptors
            # need not be populated: stop before ES is fetched/dereferenced.
            machine.mem_write(DGROUP + 0x6ad8, struct.pack('<H', self.facing(primary, bearing) * 2))
            machine.mem_write(DGROUP + 0x6b6a, struct.pack('<H', self.facing(secondary, bearing) * 2))
            machine.reg_write(UC_X86_REG_DI, OUTPUT + 50 + part)
            self.model.original.execute(machine, 0x2fa0, 0x2fb5)
            from unicorn.x86_const import UC_X86_REG_BX
            facing = machine.reg_read(UC_X86_REG_BX) // 2
            # Actual BL doubling at 2fc5..2fcc removes the selector bit.
            self.model.original.execute(machine, 0x2fc5, 0x2fcc)
            variant = machine.reg_read(UC_X86_REG_BX) // 2
            poses += [facing, part, variant]
        return [code, self.names[code], scale, *headings, *parts, *poses]
