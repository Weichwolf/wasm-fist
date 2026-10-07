"""Complete original PATH/PINF chunk returns with declared DOS read/seek boundaries.

Only DOS file I/O is supplied by the host. Original loader instructions, near
returns, phase callbacks and input bytes remain unchanged. This supplies the
previously absent mission-order input before recovering living command phases.
"""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle

PLATOONS = 8
PATH_RECORD = 268
PATH_HEADER = 12
WAYPOINTS = 32
DESCRIPTOR_RECORD = 22


class OriginalMissionOrdersOracle(OriginalVehicleStartOracle):
    def __init__(self):
        super().__init__()
        self.entries = ((b'PATH', 0xd87f, 0x7d40, PLATOONS * PATH_RECORD),
                        (b'PINF', 0xd8a9, 0x85b6, PLATOONS * DESCRIPTOR_RECORD))
        dispatch = {self.image[DGROUP + offset:DGROUP + offset + 4]:
                    struct.unpack_from('<H', self.image, DGROUP + offset + 4)[0]
                    for offset in range(0xe9e6, 0xe9e6 + 7 * 6, 6)}
        for tag, entry, _, _ in self.entries:
            if dispatch[tag] != entry:
                raise RuntimeError('Original mission-order chunk dispatch differs')
        for table, base, stride in ((0x7d2a, 0x7d40, PATH_RECORD),
                                    (0x85a0, 0x85b6, DESCRIPTOR_RECORD)):
            pointers = struct.unpack_from('<8H', self.image, DGROUP + table)
            if pointers != tuple(base + index * stride for index in range(PLATOONS)):
                raise RuntimeError('Original platoon-order record layout differs')
        if PATH_HEADER + WAYPOINTS * 8 != PATH_RECORD:
            raise RuntimeError('Waypoint storage does not fill its original record')

    def load(self, machine, paths, descriptors, *, mode=1):
        from unicorn import UC_HOOK_INTR
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
                                      UC_X86_REG_DS, UC_X86_REG_DX, UC_X86_REG_EFLAGS)
        if len(paths) != self.entries[0][3] or len(descriptors) != self.entries[1][3] or not 0 <= mode <= 255:
            raise ValueError('The declared loader boundary requires complete original order blocks')
        requests = []
        for (tag, entry, destination, size), body in zip(self.entries, (paths, descriptors)):
            machine.mem_write(DGROUP + 0xe975, bytes([mode]))
            machine.mem_write(DGROUP + 0xe970, struct.pack('<H', 0x42))
            machine.mem_write(DGROUP + 0xe97a, struct.pack('<H', size))
            machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            before = bytes(machine.mem_read(DGROUP, 65536))
            calls = []

            def dos_read_or_seek(uc, interrupt, context):
                ax, bx, cx, dx = (uc.reg_read(register) for register in
                                  (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX))
                if interrupt != 0x21 or bx != 0x42 or uc.reg_read(UC_X86_REG_DS) != 0x1c00:
                    raise RuntimeError('Unexpected original mission-order DOS boundary')
                if mode == 1:
                    if ax >> 8 != 0x3f or (cx, dx) != (size, destination):
                        raise RuntimeError('Original mission-order read range differs')
                    uc.mem_write(DGROUP + dx, body)
                    operation = 'read'
                else:
                    if ax != 0x4201 or (cx, dx) != (0, size):
                        raise RuntimeError('Original mission-order skip range differs')
                    uc.reg_write(UC_X86_REG_DX, 0)
                    operation = 'seek'
                uc.reg_write(UC_X86_REG_AX, size)
                uc.reg_write(UC_X86_REG_EFLAGS, uc.reg_read(UC_X86_REG_EFLAGS) & ~1)
                calls.append((tag.decode(), operation, size, destination))

            handle = machine.hook_add(UC_HOOK_INTR, dos_read_or_seek)
            try:
                self.call(machine, entry)
            finally:
                machine.hook_del(handle)
            if len(calls) != 1 or machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise RuntimeError('Incomplete original mission-order chunk return')
            expected = bytearray(before)
            if mode == 1:
                expected[destination:destination + size] = body
            if bytes(machine.mem_read(DGROUP, 65536)) != bytes(expected):
                raise RuntimeError('Original mission-order loader changed unrelated DGROUP bytes')
            requests.extend(calls)
        return requests

    def blocks(self, machine):
        return tuple(bytes(machine.mem_read(DGROUP + destination, size))
                     for _, _, destination, size in self.entries)

    def append_boundaries(self):
        """Actual editor admission/address prefix, before XY copy/UI calls.

        This deliberately observes a bounded fragment, not a complete editor
        action. Both branches stop at their real next instruction unchanged.
        """
        from unicorn.x86_const import UC_X86_REG_BX, UC_X86_REG_SI, UC_X86_REG_SP
        if self.image[0x4df0:0x4df5] != bytes.fromhex('80 3c 20 73 13'):
            raise RuntimeError('Original editor route-count comparison/branch differs')
        machine = self.machine((1, 2, 32768, 65535), 0)
        boundaries = []
        for platoon in range(PLATOONS):
            route = 0x7d40 + platoon * PATH_RECORD
            for count in range(256):
                machine.mem_write(DGROUP + 0x7000 + 0x1b, bytes([platoon]))
                machine.mem_write(DGROUP + route, bytes([count]))
                machine.reg_write(UC_X86_REG_SI, 0x7000)
                machine.reg_write(UC_X86_REG_SP, 0x9000)
                before = bytes(machine.mem_read(DGROUP, 65536))
                admitted = count < WAYPOINTS
                self.execute(machine, 0x4de6, 0x4dff if admitted else 0x4e08)
                expected = bytearray(before)
                if admitted:
                    expected[0x8ffe:0x9000] = struct.pack('<H', route)
                if bytes(machine.mem_read(DGROUP, 65536)) != bytes(expected):
                    raise RuntimeError('Original editor admission mutated unrelated bytes')
                registers = (machine.reg_read(UC_X86_REG_SI), machine.reg_read(UC_X86_REG_SP),
                             machine.reg_read(UC_X86_REG_BX))
                wanted = (route + PATH_HEADER + count * 8, 0x8ffe, count * 8) if admitted else (
                    route, 0x9000, platoon * 2)
                if registers != wanted:
                    raise RuntimeError('Original editor route admission/address differs')
                boundaries.append((platoon, count, admitted))
        return boundaries
