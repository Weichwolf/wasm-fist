"""Complete unchanged ab82 return, including the nested original RNG call."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle


class OriginalGroundCommandOracle(OriginalVehicleStartOracle):
    def __init__(self):
        super().__init__()
        if self.image[0xab82:0xab88] != bytes.fromhex('9a ef b4 69 0f c3'):
            raise RuntimeError('Original complete command wrapper differs')
        if self.image[0xb26a:0xb26e] != bytes.fromhex('e8 24 50 cb'):
            raise RuntimeError('Original nested random wrapper differs')
        if self.image[DGROUP + 0x9946:DGROUP + 0x994a] != bytes((100, 10, 200, 0)):
            raise RuntimeError('Original damage-choice table differs')
        if self.image[DGROUP + 0x9956:DGROUP + 0x995a] != bytes((0, 50, 255, 0)):
            raise RuntimeError('Original target-choice table differs')

    def select(self, machine, pointer, descriptor, phase_random):
        from unicorn.x86_const import UC_X86_REG_DI
        machine.mem_write(DGROUP + 0x9796, struct.pack('<H', descriptor))
        machine.mem_write(DGROUP + 0x978c, struct.pack('<H', phase_random))
        machine.reg_write(UC_X86_REG_DI, pointer)
        before = bytes(machine.mem_read(DGROUP, 65536))
        self.call(machine, 0xab82)
        if machine.reg_read(UC_X86_REG_DI) != pointer:
            raise AssertionError('Original command selection changed the actor pointer')
        after = bytes(machine.mem_read(DGROUP, 65536))
        # Complete actor comparison happens in the caller. These are the actual
        # random cursor/words/result and call-stack writes, not substituted code.
        ranges = sorted(((pointer, pointer + 251), (0x1f82, 0x1f8c),
                         (0x0342, 0x0344), (0x8ff0, 0x9002)))
        offset = 0
        for begin, end in (*ranges, (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original selection changed unrelated DGROUP state')
            offset = end
        return bytes(machine.mem_read(DGROUP + pointer, 251)), self.random_state(machine)

    def cases(self, cases):
        machine = self.machine((0, 0, 0, 0), 0)
        observed = []
        for raw, descriptor, phase_random, seeds, cursor in cases:
            machine.mem_write(DGROUP + 0x7000, raw)
            machine.mem_write(DGROUP + 0x85b6, struct.pack('<11H', *descriptor))
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                              0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
            observed.append(self.select(machine, 0x7000, 0x85b6, phase_random))
        return observed

    def choice_cycles(self):
        """Execute complete original increment/decrement UI tails for both words."""
        return self._choice_cycles(((0, 0x620c, 0x621c, 4), (2, 0x626f, 0x6282, 4)))

    def formation_cycles(self):
        return self._choice_cycles(((4, 0x62cf, 0x62e2, 6),))

    def throttle_cycles(self):
        return self._choice_cycles(((6, 0x632f, 0x6342, 4),))

    def _choice_cycles(self, choices):
        from unicorn.x86_const import UC_X86_REG_BX
        machine = self.machine((0, 0, 0, 0), 0)
        results = []
        for platoon in range(8):
            pointer = 0x85b6 + platoon * 22
            for offset, up, down, count in choices:
                for value in range(count):
                    for entry, delta in ((up, 1), (down, -1)):
                        raw = bytearray(range(22))
                        struct.pack_into('<H', raw, offset, value)
                        machine.mem_write(DGROUP + pointer, bytes(raw))
                        machine.reg_write(UC_X86_REG_BX, pointer)
                        self.call(machine, entry)
                        struct.pack_into('<H', raw, offset, (value + delta) % count)
                        actual = bytes(machine.mem_read(DGROUP + pointer, 22))
                        if actual != bytes(raw):
                            raise AssertionError('Original four-choice UI cycle differs')
                        results.append((platoon, offset, value, delta))
        return results
