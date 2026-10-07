"""Complete unchanged ac75 returns with real route/formation/rotation inputs."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_motion_oracle import OriginalVehicleMotionOracle


class OriginalGroundGoalOracle(OriginalVehicleMotionOracle):
    def __init__(self):
        super().__init__()
        self.dispatch = struct.unpack_from('<8H', self.image, DGROUP + 0x9800)
        if self.dispatch != (0xac7e, 0xac9e, 0xad03, 0xad02, 0xad04, 0xad05, 0xad06, 0xad07):
            raise RuntimeError('Original complete goal dispatch table differs')
        if self.image[0xad02:0xad08] != b'\xc3' * 6:
            raise RuntimeError('Original six goal-mode returns differ')
        pointers = struct.unpack_from('<6H', self.image, DGROUP + 0x9830)
        if pointers != tuple(0x983c + index * 16 for index in range(6)):
            raise RuntimeError('Original four-member formation table extent differs')
        self.formations = tuple(struct.unpack_from('<8H', self.image, DGROUP + p) for p in pointers)

    def assign(self, machine, pointer, platoon):
        from unicorn.x86_const import UC_X86_REG_DI
        machine.mem_write(DGROUP + 0x9796, struct.pack('<HH', 0x85b6 + platoon * 22,
                                                     0x7d40 + platoon * 268))
        if bytes(machine.mem_read(DGROUP + 0x2040, 1)) != b'\0':
            raise AssertionError('Declared complete goal boundary requires fine rotation')
        before = bytes(machine.mem_read(DGROUP, 65536))
        random = self.random_state(machine)
        machine.reg_write(UC_X86_REG_DI, pointer)
        self.call(machine, 0xac75)
        if machine.reg_read(UC_X86_REG_DI) != pointer or self.random_state(machine) != random:
            raise AssertionError('Original goal changed actor identity or RNG')
        after = bytes(machine.mem_read(DGROUP, 65536))
        ranges = sorted(((pointer, pointer + 251), (0x8fc0, 0x9002)))
        offset = 0
        for begin, end in (*ranges, (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original goal changed unrelated DGROUP/leader/orders')
            offset = end
        return bytes(machine.mem_read(DGROUP + pointer, 251))

    def cases(self, cases):
        machine = self.machine()
        results = []
        for raw, leader, presence, descriptor, route, seeds, cursor in cases:
            platoon = raw[0x1b]
            machine.mem_write(DGROUP + 0x7000, raw)
            if presence in (1, 2):
                machine.mem_write(DGROUP + 0x7100, leader)
            pointer = 0 if presence == 0 else (0x7000 if presence == 3 else 0x7100)
            machine.mem_write(DGROUP + 0x6d3c + platoon * 8, struct.pack('<H', pointer))
            machine.mem_write(DGROUP + 0x85b6 + platoon * 22, struct.pack('<11H', *descriptor))
            machine.mem_write(DGROUP + 0x7d40 + platoon * 268, route)
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                              0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
            results.append(self.assign(machine, 0x7000, platoon))
        return results
