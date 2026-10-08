"""Complete unchanged class motion/turret calls with full unrelated-world guards."""
import struct

from original_target_acquisition_oracle import OriginalTargetAcquisitionOracle, TEXT_BASE
from original_unit_oracle import DGROUP
from original_vehicle_motion_oracle import MOTION, TURRET


class OriginalGroundTargetMotionOracle(OriginalTargetAcquisitionOracle):
    def step(self, machine, actor, *, move=False, coarse=0):
        from unicorn.x86_const import UC_X86_REG_DI
        kind = struct.unpack('<H', machine.mem_read(DGROUP + actor, 2))[0]
        if kind >= 4 or (move and coarse):
            raise ValueError('This ordered motion scope requires a ground class and normal detail')
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        machine.mem_write(DGROUP + 0x8e48, b'\0')
        machine.reg_write(UC_X86_REG_DI, actor)
        before = bytes(machine.mem_read(DGROUP, 65536))
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        random = self.random_state(machine)
        if move:
            self.call(machine, MOTION[kind])
        self.call(machine, TURRET[kind])
        after = bytes(machine.mem_read(DGROUP, 65536))
        spans = ((actor, actor + 251), (0x2034, 0x203e), (0x9684, 0x969c),
                 (0x8e48, 0x8e49), (0x8fc0, 0x9002))
        self.unchanged(before, after, spans, 'Target motion changed unrelated DGROUP/world')
        if (machine.reg_read(UC_X86_REG_DI) != actor or self.random_state(machine) != random or
                bytes(machine.mem_read(0, DGROUP)) != code or
                bytes(machine.mem_read(TEXT_BASE, 65536)) != text):
            raise AssertionError('Target motion changed actor identity, RNG or immutable instructions/text')
        return self.raw(machine, actor), after[0x8e48]
