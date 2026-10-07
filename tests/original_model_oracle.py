"""Execute original model facing, part-list and texel-reference instruction blocks.

Buffered input and uploaded-record addresses are provided at the original I/O
boundaries. No instruction hooks replace code. DOS/heap/LOD/rasterization are
outside this model-data oracle; all original lookups use real segment registers.
"""
import struct

from original_unit_oracle import DGROUP, OriginalUnitOracle

RECORD = 0x40000
BASE = 0x50000
TABLE = 0x6000
NODE = 0x7000


class OriginalModelOracle:
    def __init__(self):
        self.original = OriginalUnitOracle()  # Shared DOS-image/version pin and 16-bit executor.
        selectors = struct.unpack_from('<32H', self.original.image, DGROUP + 0x3a8c)
        if selectors != (8,) * 16 + (6,) * 16:
            raise RuntimeError('Original direction-to-part-table selectors changed')

    def machine(self):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_16
        from unicorn.x86_const import UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP
        machine = Uc(UC_ARCH_X86, UC_MODE_16)
        machine.mem_map(0, 0x70000)
        machine.mem_write(0, self.original.image)
        machine.reg_write(UC_X86_REG_DS, 0x1c00)
        machine.reg_write(UC_X86_REG_SS, 0x6000)
        machine.reg_write(UC_X86_REG_SP, 0xff0)
        machine.mem_write(DGROUP + 0x2674, b'\0\0')
        machine.mem_write(DGROUP + 0x27f4, struct.pack('<H', TABLE))
        return machine

    def facing(self, record):
        machine = self.machine()
        machine.mem_write(DGROUP + 0x267c, record[:16])
        self.original.execute(machine, 0x1a91, 0x1ab6)
        a, b = struct.unpack('<HH', machine.mem_read(DGROUP + 0x267e, 4))
        return (a, b) if a == 65534 else (a // 2, b // 2)

    def family_faces(self, files):
        machine = self.machine()
        slots = {}
        for file_index, records in enumerate(files):
            for ordinal, record in enumerate(records):
                machine.mem_write(DGROUP + 0x267c, record[:16])
                self.original.execute(machine, 0x1a91, 0x1ad9)
                descriptor, = struct.unpack('<H', machine.mem_read(DGROUP + 0x3ad4, 2))
                slots[descriptor] = (file_index, ordinal)
                self.original.execute(machine, 0x1ae7, 0x1afe)
            if file_index == 0:
                self.original.execute(machine, 0x197b, 0x1995)
            elif file_index == 1:
                self.original.execute(machine, 0x1996, 0x19b8)
            elif file_index == 2:
                self.original.execute(machine, 0x19b9, 0x19d0)
        return [slots[value] for value in struct.unpack('<32H', machine.mem_read(DGROUP + TABLE, 64))]

    def sprites(self, record, count):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_ES, UC_X86_REG_ESI
        machine = self.machine()
        machine.mem_write(RECORD, record)
        machine.mem_write(RECORD + 2, struct.pack('<I', RECORD))
        machine.reg_write(UC_X86_REG_ES, RECORD // 16)
        self.original.execute(machine, 0x1b91, 0x1b99)
        first_texel = machine.reg_read(UC_X86_REG_AX)
        atlas, = struct.unpack_from('<H', record, 14)
        if first_texel != atlas + (count + 1) * 4:
            raise RuntimeError('Original metadata extent differs from complete sprite directory')
        result = []
        for index in range(count):
            machine.reg_write(UC_X86_REG_BX, index * 2)
            self.original.execute(machine, 0x302d, 0x3045)
            dimensions = machine.reg_read(UC_X86_REG_CX)
            width, height = dimensions & 255, dimensions >> 8
            address = machine.reg_read(UC_X86_REG_ESI)
            result.append((address - RECORD, width, height,
                           bytes(machine.mem_read(address, width * height))))
        return result

    def record(self, record, base, tables):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_DI,
                                      UC_X86_REG_ES, UC_X86_REG_ESI, UC_X86_REG_CX,
                                      UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_SP)
        machine = self.machine()
        machine.mem_write(RECORD, record)
        machine.mem_write(RECORD + 2, struct.pack('<I', RECORD))  # Original 1b87 upload address.
        machine.mem_write(BASE, base)
        machine.mem_write(BASE + 2, struct.pack('<I', BASE))
        machine.mem_write(DGROUP + 0x4b9e, struct.pack('<H', NODE))
        machine.mem_write(DGROUP + NODE + 5, b'\0')
        machine.mem_write(DGROUP + TABLE - 2, struct.pack('<H', 0x6200))
        machine.mem_write(DGROUP + 0x6200, struct.pack('<H', BASE // 16))
        machine.reg_write(UC_X86_REG_ES, RECORD // 16)
        self.original.execute(machine, 0x1b35, 0x1b49)
        count, = struct.unpack('<H', machine.mem_read(DGROUP + 0x276c, 2))
        if count != len(tables[0]):
            raise RuntimeError('Original part count differs from complete parsed table')
        result = []
        for orientation, parts in enumerate(tables):
            for part, variants in enumerate(parts):
                for variant_index, _ in enumerate(variants):
                    if variant_index >= 128:
                        raise RuntimeError('Original per-part variant byte cannot address this table')
                    machine.reg_write(UC_X86_REG_ES, RECORD // 16)
                    machine.reg_write(UC_X86_REG_BX, 32 if orientation == 0 else 0)
                    machine.mem_write(DGROUP + 0x6b5e, struct.pack('<H', part * 2))
                    machine.mem_write(DGROUP + NODE + 0x32, bytes((variant_index,)))
                    machine.reg_write(UC_X86_REG_DI, NODE + 0x32)
                    self.original.execute(machine, 0x2fb7, 0x2fcf)
                    pointer = machine.reg_read(UC_X86_REG_SI)
                    self.original.execute(machine, 0x2fd2, 0x2fd4)
                    key = machine.reg_read(UC_X86_REG_AX)
                    selected = []
                    for piece in range(key & 255):
                        machine.reg_write(UC_X86_REG_ES, RECORD // 16)
                        machine.reg_write(UC_X86_REG_SI, pointer + 2 + piece * 4)
                        machine.reg_write(UC_X86_REG_SP, 0xff0)
                        self.original.execute(machine, 0x301d, 0x3045)
                        width_height = machine.reg_read(UC_X86_REG_CX)
                        placement = machine.reg_read(UC_X86_REG_DX)
                        mirrored = machine.reg_read(UC_X86_REG_DI) != 0
                        address = machine.reg_read(UC_X86_REG_ESI)
                        width, height = width_height & 255, width_height >> 8
                        x, y = struct.unpack('<bb', struct.pack('<H', placement))
                        pixels = bytes(machine.mem_read(address, width * height))
                        selected.append((address - (BASE if address >= BASE else RECORD),
                                         address >= BASE, mirrored, x, y, width, height, pixels))
                    result.append((orientation, part, variant_index, key >> 8, selected))
        return result
