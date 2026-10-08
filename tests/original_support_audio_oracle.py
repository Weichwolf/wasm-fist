"""Complete unchanged b0be with actual height and admitted op-64 response.

The existing DOS/PM ABI adapters transfer returned height/sample registers;
all DOS allocation, constructors, support queues, notices and returns execute.
Whole-state predictions precede execution. No gameplay call is replaced.
"""
import struct

from audio_request_contract import request
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE, OriginalTargetAcquisitionOracle
from original_unit_oracle import DGROUP
from roster_promotion_contract import store
from support_audio_contract import SMOKE_STOCK, support
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_units import snapshot
from test_vehicle_motion import start


class OriginalSupportAudioOracle(OriginalTargetAcquisitionOracle):
    def __init__(self, audio):
        super().__init__()
        self.audio = audio
        self.fixtures = {}
        self.fixture_counts = {}
        if self.image[DGROUP + 0x9fe1 + 39:DGROUP + 0x9fe1 + 42] != b'\x0b\0\0':
            raise AssertionError('Original smoke request packet differs')
        if self.text(0x3193) != 'ARTILLERY REQUEST RECEIVED':
            raise AssertionError('Original global artillery message differs')

    def fixture(self, kind=0, *, stock=2, full=False, selected=True, source=True,
                draw=0, mode=4, clock=1800, prior=0, flags=8, target=True, distance=20,
                air_count=1, air_delay=0, air_age=480, air_used=0,
                artillery_count=1, ammunition=2, artillery_age=480, artillery_busy=0,
                artillery_used=0, notice_context=0, heading=0, velocity=(0, 0),
                pose=(0, 0), target_pose=(123456789, -123456789), coarse=0):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS,
                                      UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                      UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                      UC_X86_REG_EBP)
        key = kind, full
        if key not in self.fixtures or self.fixture_counts[key] == 256:
            previous = self.fixtures.get(key)
            actor_raw = bytearray(start(kind))
            actor_raw[22] = 0
            machine, objects = self.prepare_saved([
                (0, 1, bytes(actor_raw)), (1, 2, snapshot(5, flags=0)), (2, 3, snapshot(27, flags=0))],
                SEEDS, 0, 0, (bytes(2144), bytes(176)))
            if full:
                for _ in range(148):
                    machine.reg_write(UC_X86_REG_AX, 8)
                    self.far_call(machine, 0x1b1df)
                    if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                        raise AssertionError('Actual short-pool capacity fixture failed')
            baseline = bytes(machine.mem_read(0, 0x60000))
            if previous is not None and (baseline, objects) != (previous[1], previous[2]):
                raise AssertionError('Fresh complete original world preparation differs')
            self.fixtures[key] = machine, baseline, objects
            self.fixture_counts[key] = 0
        self.fixture_counts[key] += 1
        machine, baseline, objects = self.fixtures[key]
        machine.mem_write(0, baseline)
        actor, target_pointer, artillery = (objects[index][2] for index in (150, 0, 1))
        raw = bytearray(machine.mem_read(DGROUP + actor, 251))
        raw[22], raw[0x43] = flags, mode
        struct.pack_into('<2i', raw, 4, *pose)
        struct.pack_into('<2h', raw, 0x59, *velocity)
        for offset, value in ((0x26, heading), (0x97, target_pointer if target else 0), (0x99, distance)):
            store(raw, offset, value)
        stock_offset = SMOKE_STOCK[kind]
        if kind == 2:
            store(raw, stock_offset, stock)
        elif stock_offset is not None:
            raw[stock_offset] = stock
        machine.mem_write(DGROUP + actor, bytes(raw))
        machine.mem_write(DGROUP + target_pointer + 4, struct.pack('<2i', *target_pose))
        words = list(SEEDS)
        words[0] = seed_for_draw(draw)
        machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H', 0x1f8a, *words))
        for offset, value in ((0x6d34, actor if selected else 0), (0x9fdf, actor if source else 0),
                              (0x6da2, 65535), (0x6cde, clock), (0x9794, prior), (0x452, 12345),
                              (0x9452, air_count), (0x9456, air_delay), (0x9462, clock - air_age),
                              (0x945a, 0x9524), (0x9ccd, artillery_count), (0x9cd7, artillery),
                              (0x9f19, clock - artillery_age), (0x9ce5, artillery_busy),
                              (artillery + 0x1f, ammunition)):
            machine.mem_write(DGROUP + offset, struct.pack('<H', value & 65535))
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        machine.mem_write(DGROUP + 0x6ce6, bytes([notice_context]))
        machine.mem_write(DGROUP + 0x9524, b''.join(struct.pack('<6H', actor if i < air_used else 0, 17, 18, 19, 20, 21) for i in range(16)))
        machine.mem_write(DGROUP + 0x9dc7, b''.join(struct.pack('<7H', 1 if i < artillery_used else 0, 31, 32, 33, 34, 35, 36) for i in range(16)))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, MAILBOX // 16))
        for register in (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                         UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP):
            machine.reg_write(register, 0)
        machine.reg_write(UC_X86_REG_DI, actor)
        return machine, actor

    def observe(self, machine, actor, audio_fixture, *, height=17):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EAX,
                                      UC_X86_REG_ECX, UC_X86_REG_DX, UC_X86_REG_SP)
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        predicted_kernel, predicted_registers, _ = request(*audio_fixture, self.audio.banks)
        expected, evidence = support(before, actor, height=height, audio_return=predicted_registers['eax'])
        entry = 0xb0be
        if evidence['allocation'] is not None:
            self.execute(machine, entry, 0xe1eb)
            marker = evidence['allocation'][0]
            if machine.reg_read(UC_X86_REG_DI) != marker + 4:
                raise AssertionError('Actual smoke constructor height operand differs')
            position = bytes(machine.mem_read(DGROUP + marker + 4, 8))
            if position != evidence['height_position']:
                raise AssertionError('Actual constructor rotation/velocity/position differs')
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x54\0':
                raise AssertionError('Actual constructor selected another height operation')
            actual_height = self.checked_height(marker + 4, position, height)
            if actual_height != height:
                raise AssertionError('Actual original height transfer differs')
            machine.reg_write(UC_X86_REG_AX, actual_height)
            entry = 0xe1ee
        if evidence['audio']:
            self.execute(machine, entry, 0xe2db)
            if (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX) & 255,
                    machine.reg_read(UC_X86_REG_ECX)) != (11, 0, 0):
                raise AssertionError('Actual c047 admitted another audio request')
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x64\0':
                raise AssertionError('Actual c047 selected another PM operation')
            # e2c2 transports EBX in the actual mailbox before setting BX=0x64.
            # Pass the original caller's full request registers to the kernel.
            request_registers = {name: machine.reg_read(register)
                                 for name, register in self.audio.registers.items()}
            request_registers['ebx'] = struct.unpack('<I', machine.mem_read(MAILBOX + 0x3f2, 4))[0]
            for name, value in request_registers.items():
                self.audio.machine.reg_write(self.audio.registers[name], value)
            actual_kernel, actual_registers, _ = self.audio.observe(audio_fixture[0], request_registers)
            if actual_kernel != predicted_kernel:
                raise AssertionError('Coupled original kernel prediction differs')
            # Transfer the actual complete kernel EAX response, never a supplied
            # logical event or invented sample address, through the declared ABI.
            machine.reg_write(UC_X86_REG_EAX, actual_registers['eax'])
            entry = 0xe2de
        if evidence['consumed_al'] is not None:
            comparison = 0xb10b if before[actor + 0x43] == 4 else 0xb0e7
            self.execute(machine, entry, comparison)
            if machine.reg_read(UC_X86_REG_AX) & 255 != evidence['consumed_al']:
                raise AssertionError('Actual complete smoke return AL at its consumer differs')
            entry = comparison
        self.execute(machine, entry, 0xeff0)
        actual_registers = tuple(machine.reg_read(reg) for reg in self.machine_registers())
        if actual_registers != (actor, 0x1c00, 0x9002, 0x1c00):
            raise AssertionError('Complete support actor/segment/stack return differs')
        after = bytes(machine.mem_read(DGROUP, 65536))
        expected[0x8fc0:0x9000] = after[0x8fc0:0x9000]
        if after != expected:
            differences = [hex(i) for i, (a, b) in enumerate(zip(after, expected)) if a != b]
            raise AssertionError('Complete support DGROUP differs: ' + str(differences[:16]))
        expected_mailbox = bytearray(mailbox)
        if evidence['mailbox_ebx'] is not None:
            struct.pack_into('<I', expected_mailbox, 0x3f2, evidence['mailbox_ebx'])
        if bytes(machine.mem_read(MAILBOX, 4096)) != expected_mailbox:
            raise AssertionError('Actual support device mailbox differs')
        if bytes(machine.mem_read(0, DGROUP)) != code or bytes(machine.mem_read(TEXT_BASE, 65536)) != text:
            raise AssertionError('Actual support modified immutable code/text')
        return after, evidence

    def checked_height(self, near_position, raw, height):
        """Guard the complete existing original height transfer, including maps."""
        from original_ground_oracle import BUFFER, DGROUP as KERNEL_DGROUP, RETURN, ROSTER, STACK
        pixels = bytes([height]) * 4
        key = (2, pixels)
        if key not in self.height_cache:
            self.height_cache[key] = self.ground.prepare(2, pixels)
        machine = self.height_cache[key]
        machine.mem_write(KERNEL_DGROUP + near_position, raw)
        guards = {address: bytes(machine.mem_read(address, size)) for address, size in
                  ((0, (len(self.ground.image) + 4095) & ~4095), (BUFFER, 4096),
                   (KERNEL_DGROUP, 65536), (ROSTER, 4096), (RETURN, 4096))}
        stack = bytes(machine.mem_read(STACK, 4096))
        actual = self.height(near_position, raw, pixels)
        for address, guard in guards.items():
            if bytes(machine.mem_read(address, len(guard))) != guard:
                raise AssertionError('Actual height transfer modified kernel/map/DGROUP/guards')
        after = bytes(machine.mem_read(STACK, 4096))
        if after[:0xfe4] != stack[:0xfe4] or after[0xff4:] != stack[0xff4:]:
            raise AssertionError('Actual height transfer escaped bounded stack scratch')
        return actual
