"""Complete unchanged automatic fire, missile readiness/launch and parent returns.

Only an admitted unconsumed op-64 audio request has a device boundary. The real
allocator, constructor, readiness, display and all near/far returns execute.
"""
import struct

from automatic_fire_contract import FIRE_THRESHOLDS, SHOT_PACKET, automatic_fire, parent
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE, OriginalTargetAcquisitionOracle
from original_unit_oracle import DGROUP


class OriginalAutomaticFireOracle(OriginalTargetAcquisitionOracle):
    def __init__(self):
        super().__init__()
        if self.image[0xaf97:0xafa2] != bytes.fromhex('833d017406833d037401c3'):
            raise AssertionError('Original af97 class gate differs')
        if self.image[DGROUP + 0x9952:DGROUP + 0x9956] != bytes(FIRE_THRESHOLDS):
            raise AssertionError('Original automatic-fire probabilities differ')
        if self.image[DGROUP + 0x9ff9:DGROUP + 0x9ffc] != SHOT_PACKET:
            raise AssertionError('Original missile sound packet differs')
        expected = (0xaf97, 0xafa2, 0xaf97)
        if tuple(struct.unpack_from('<H', self.image, DGROUP + 0x98dc + 2 * i)[0]
                 for i in (5, 10, 14)) != expected:
            raise AssertionError('Original complete automatic fire bank differs')
        if any(struct.unpack_from('<H', self.image, DGROUP + 0x98fc + 2 * i)[0] != 0xb111
               for i in (5, 10, 14)) or self.image[0xb111] != 0xc3:
            raise AssertionError('Original controlled fire entries are not genuine RETs')

    def observe(self, machine, actor, *, entry=0xaf97):
        from unicorn.x86_const import (UC_X86_REG_DI, UC_X86_REG_SP, UC_X86_REG_CS,
                                      UC_X86_REG_EBX, UC_X86_REG_EDX, UC_X86_REG_ECX,
                                      UC_X86_REG_AX)
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.reg_write(UC_X86_REG_EBX, 0)
        machine.reg_write(UC_X86_REG_EDX, 0xa512)
        machine.reg_write(UC_X86_REG_ECX, 0x12345678)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        expected, effect = parent(before, actor) if entry == 0xab03 else automatic_fire(before, actor, entry=entry)
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        expected_mailbox = bytearray(mailbox)
        if effect['request'] is not None:
            self.execute(machine, entry, 0xe2db)
            request = (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_EDX),
                       machine.reg_read(UC_X86_REG_ECX))
            if request != (7, 0xa500, 0) or word(machine, 0xea10) != 100:
                raise AssertionError('Complete missile audio request differs')
            struct.pack_into('<I', expected_mailbox, 0x3f2, effect['request']['ebx'])
            # The device call has no consumed result. Resume its actual caller;
            # do not replace a nested gameplay service or return instruction.
            self.execute(machine, 0xe2de, 0xeff0)
        else:
            self.execute(machine, entry, 0xeff0)
        if machine.reg_read(UC_X86_REG_CS) != 0:
            raise AssertionError('Complete automatic-fire code segment differs')
        registers = tuple(machine.reg_read(reg) for reg in self.machine_registers())
        if registers != (actor, 0x1c00, 0x9002, 0x1c00):
            raise AssertionError('Complete automatic-fire actor/stack/segments differ')
        after = bytes(machine.mem_read(DGROUP, 65536))
        expected[0x8fc0:0x9000] = after[0x8fc0:0x9000]
        if after != bytes(expected):
            changed = [hex(i) for i, (actual, wanted) in enumerate(zip(after, expected)) if actual != wanted]
            raise AssertionError('Complete automatic-fire DGROUP differs: ' + str(changed[:24]))
        if bytes(machine.mem_read(MAILBOX, 4096)) != bytes(expected_mailbox):
            raise AssertionError('Complete automatic-fire mailbox differs')
        if bytes(machine.mem_read(0, DGROUP)) != code or bytes(machine.mem_read(TEXT_BASE, 65536)) != text:
            raise AssertionError('Automatic fire modified original code/text')
        return after, effect


def word(machine, offset):
    return struct.unpack('<H', machine.mem_read(DGROUP + offset, 2))[0]
