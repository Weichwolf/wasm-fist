"""Complete unchanged throttle, gear and class/display setter returns."""
import struct

from ground_throttle_contract import DISPATCH, DISPLAY_OFFSETS, LEADER_THROTTLES, PROFILE_COMPONENTS, SETTERS
from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_unit_oracle import DGROUP


class OriginalGroundThrottleOracle(OriginalMissionReadyOracle):
    def __init__(self):
        super().__init__()
        if struct.unpack_from('<8H', self.image, DGROUP + 0x97f0) != DISPATCH:
            raise AssertionError('Original complete throttle bank differs')
        if struct.unpack_from('<4H', self.image, DGROUP + 0x992c) != LEADER_THROTTLES:
            raise AssertionError('Original leader throttle choices differ')
        if struct.unpack_from('<4H', self.image, DGROUP + 0x965e) != SETTERS:
            raise AssertionError('Original class profile setter bank differs')
        if self.image[0xae06] != 0xc3 or self.image[0xae25] != 0xc3:
            raise AssertionError('Original inactive throttle entries are not returns')
        expected = b''.join(b'\xc6\x06' + struct.pack('<H', offset) + b'\x03'
                            for offset in DISPLAY_OFFSETS) + b'\xcb'
        if self.image[0x170eb:0x17100] != expected:
            raise AssertionError('Complete original display refresh instructions differ')

    def set_profile(self, machine, value):
        from unicorn.x86_const import UC_X86_REG_AX
        _, ds, sp, ss = self.machine_registers()
        machine.reg_write(UC_X86_REG_AX, value)
        machine.reg_write(sp, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        self.execute(machine, 0xa19e, 0xeff0)
        if (machine.reg_read(sp), machine.reg_read(ds), machine.reg_read(ss)) != (
                0x9004, 0x1c00, 0x1c00):
            raise AssertionError('Complete original profile setter return/segment contract differs')

    def command(self, machine, pointer, platoon, operation=0):
        from unicorn.x86_const import UC_X86_REG_DI
        machine.mem_write(DGROUP + 0x9796, struct.pack('<H', 0x85b6 + platoon * 22))
        machine.reg_write(UC_X86_REG_DI, pointer)
        before = bytes(machine.mem_read(DGROUP, 65536))
        if operation >= 2:
            self.set_profile(machine, operation - 2)
        else:
            self.call(machine, 0xad2f if operation == 0 else 0xad3b)
        if machine.reg_read(UC_X86_REG_DI) != pointer:
            raise AssertionError('Original complete throttle/setter changed actor identity')
        after = bytes(machine.mem_read(DGROUP, 65536))
        component = PROFILE_COMPONENTS[int.from_bytes(before[pointer:pointer + 2], 'little')]
        ranges = sorted(((pointer + 0x57, pointer + 0x59), (pointer + 0x90, pointer + 0x91),
                         (pointer + component, pointer + component + 1),
                         (0x8fc0, 0x9004), *((offset, offset + 1) for offset in DISPLAY_OFFSETS)))
        offset = 0
        for begin, end in (*ranges, (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original throttle changed unrelated DGROUP/target/orders/RNG')
            offset = end
        dirty = tuple(after[offset] for offset in DISPLAY_OFFSETS)
        return after[pointer:pointer + 251], dirty
