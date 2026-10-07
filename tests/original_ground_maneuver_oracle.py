"""Complete unchanged maneuver/idle/obstacle returns and genuine parent entries.

Finite searches retain every nested geometry, registry, collision and RNG call.
Instruction/timeout limits fail incomplete runs; no hooks or replacements.
"""
import struct

from ground_maneuver_contract import DISPATCH, OFFSETS, idle_turret, maneuver, motion_obstacle, parent
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE, OriginalTargetAcquisitionOracle
from original_unit_oracle import DGROUP


class OriginalGroundManeuverOracle(OriginalTargetAcquisitionOracle):
    def __init__(self):
        super().__init__()
        if struct.unpack_from('<4H', self.image, 0xae73) != DISPATCH:
            raise AssertionError('Complete original masked maneuver dispatch differs')
        if struct.unpack_from('<32h', self.image, DGROUP + 0x97aa) != OFFSETS:
            raise AssertionError('Complete original search/selected turn offsets differ')
        if self.image[0xb059:0xb05d] != bytes.fromhex('0e e8 b5 00'):
            raise AssertionError('Original motion obstacle producer differs')

    @staticmethod
    def execute(machine, start, stop, code_segment=0):
        from unicorn.x86_const import UC_X86_REG_CS, UC_X86_REG_IP
        boundary = start, stop, code_segment
        if getattr(machine, '_fist_maneuver_boundary', None) != boundary:
            machine.ctl_remove_cache(0, 0x60000)
            machine._fist_maneuver_boundary = boundary
        machine.reg_write(UC_X86_REG_CS, code_segment)
        # Fifteen searches can each visit 182 bodies and sample 24 positions.
        machine.emu_start(start, stop, timeout=10_000_000, count=20_000_000)
        reached = machine.reg_read(UC_X86_REG_CS) * 16 + machine.reg_read(UC_X86_REG_IP)
        if reached != stop:
            raise RuntimeError(f'Incomplete original maneuver at {reached:#x}, wanted {stop:#x}')

    def observe(self, machine, actor, *, operation='maneuver', candidate=0):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SI, UC_X86_REG_SP
        models = {'maneuver': (0xae66, maneuver), 'idle_turret': (0xb017, idle_turret),
                  'parent': (0xab03, parent), 'motion_obstacle': (0xb059, motion_obstacle)}
        entry, model = models[operation]
        far = operation == 'motion_obstacle'
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SI, candidate)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if far else struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        expected, evidence = model(before, actor, candidate) if far else model(before, actor)
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        self.execute(machine, entry, 0xeff0)
        registers = self.machine_registers()
        actual_registers = tuple(machine.reg_read(reg) for reg in registers)
        if actual_registers != (actor, 0x1c00, 0x9004 if far else 0x9002, 0x1c00):
            raise AssertionError('Complete maneuver actor/segments/stack return differs')
        actual = bytes(machine.mem_read(DGROUP, 65536))
        expected[0x8fc0:0x9000] = actual[0x8fc0:0x9000]
        if actual != bytes(expected):
            differences = [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
            raise AssertionError('Complete maneuver DGROUP differs: ' + str(differences[:16]))
        if bytes(machine.mem_read(0, DGROUP)) != code or bytes(machine.mem_read(TEXT_BASE, 65536)) != text:
            raise AssertionError('Maneuver modified immutable code/text')
        if bytes(machine.mem_read(MAILBOX, 4096)) != mailbox:
            raise AssertionError('Maneuver modified unrelated device mailbox')
        return actual, evidence
