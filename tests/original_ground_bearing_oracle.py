"""Complete unchanged ab88 near returns, real geometry and exact state writes."""
import struct

from ground_bearing_contract import DISPATCH, RETREAT_DURATIONS
from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_unit_oracle import DGROUP


class OriginalGroundBearingOracle(OriginalMissionReadyOracle):
    def __init__(self):
        super().__init__()
        if struct.unpack_from('<8H', self.image, DGROUP + 0x9810) != DISPATCH:
            raise AssertionError('Original complete bearing bank differs')
        if self.image[DGROUP + 0x994e:DGROUP + 0x9952] != bytes(RETREAT_DURATIONS):
            raise AssertionError('Original retreat duration bytes differ')
        if self.image[0xac72:0xac75] != b'\xc3' * 3:
            raise AssertionError('Original three inactive bearing entries differ')

    def direction(self, machine, pointer, platoon):
        from unicorn.x86_const import UC_X86_REG_DI
        machine.mem_write(DGROUP + 0x9796, struct.pack('<H', 0x85b6 + platoon * 22))
        machine.reg_write(UC_X86_REG_DI, pointer)
        before = bytes(machine.mem_read(DGROUP, 65536))
        self.call(machine, 0xab88)
        if machine.reg_read(UC_X86_REG_DI) != pointer:
            raise AssertionError('Original bearing changed actor identity')
        after = bytes(machine.mem_read(DGROUP, 65536))
        # Numeric scratch and bounded stack are owned by the already recovered
        # complete 0541 helper. Actor writes are checked individually by model.
        ranges = sorted(((0x2034, 0x203e), (0x8fc0, 0x9002),
                         (pointer + 0x30, pointer + 0x32),
                         (pointer + 0x40, pointer + 0x42),
                         (pointer + 0x44, pointer + 0x45),
                         (pointer + 0x53, pointer + 0x55)))
        offset = 0
        for begin, end in (*ranges, (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original bearing changed unrelated actor/target/orders/RNG')
            offset = end
        return after[pointer:pointer + 251]
