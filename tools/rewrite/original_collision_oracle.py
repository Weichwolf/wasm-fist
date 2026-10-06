"""Complete actual bb1b/0ea9/c14f queries and b65c hit-aspect instructions."""
import struct

from original_object_pool_oracle import REGISTRY, SHORT_BASE, OriginalObjectPoolOracle
from original_unit_oracle import DGROUP


class OriginalCollisionOracle(OriginalObjectPoolOracle):
    def queries(self, bodies, queries, seeds, cursor):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS, UC_X86_REG_SI
        machine = self.machine(seeds, cursor)
        self.far_call(machine, 0x1b176)
        pointers = []
        for kind, index, value, x, y, altitude, heading, scale, flags, mode in bodies:
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, value)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise AssertionError('Actual original body import failed')
            pointer = machine.reg_read(UC_X86_REG_DI)
            pointers.append(pointer)
            machine.mem_write(DGROUP + pointer + 4, struct.pack('<3iH', x, y, altitude, heading))
            machine.mem_write(DGROUP + pointer + 0x14, struct.pack('<H', scale))
            machine.mem_write(DGROUP + pointer + 0x16, bytes([flags]))
            machine.mem_write(DGROUP + pointer + 0x19, bytes([mode]))
        before = bytes(machine.mem_read(DGROUP + SHORT_BASE, REGISTRY - SHORT_BASE))
        metadata = self.state(machine)
        result = ''
        for ordinal in queries:
            pointer = pointers[ordinal]
            machine.reg_write(UC_X86_REG_DI, pointer)
            self.call(machine, 0xbb1b)
            carry = machine.reg_read(UC_X86_REG_EFLAGS) & 1
            hit = (65535, 65535, 0, 0)
            if carry:
                target, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9a25, 2))
                if machine.reg_read(UC_X86_REG_SI) != target:
                    raise AssertionError('Actual bb1b result and SI disagree')
                entry = next((index, value) for index in range(182)
                             for address, value in [struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))]
                             if address == target)
                # Exact following flight-owner aspect arithmetic, before damage dispatch.
                self.execute(machine, 0xb65c, 0xb66b)
                aspect, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9bd7, 2))
                hit = self.slot(target), *entry, aspect
            words, next_stream = self.random_state(machine)
            result += 'hit ' + ' '.join(map(str, hit)) + '\nrandom ' + ' '.join(map(str, [next_stream, *words])) + '\n'
        if bytes(machine.mem_read(DGROUP + SHORT_BASE, REGISTRY - SHORT_BASE)) != before or self.state(machine) != metadata:
            raise AssertionError('Original query mutated body or pool state')
        return result
