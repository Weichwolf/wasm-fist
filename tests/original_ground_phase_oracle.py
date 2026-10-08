"""Complete unchanged ab03 with checked prefix, real child and diagnostic.

The actual parent executes its indirect CALL and every nested gameplay call.
A separately executed complete child checks composition against the previously
proved child boundary. Only genuine PM height, visibility and audio operations
cross the declared DOS/kernel ABI. No instruction hooks or patches are used.
"""
import struct

from audio_request_contract import request
from automatic_fire_contract import automatic_fire
from ground_command_phase_contract import AUTOMATIC, CONTROLLED, prefix
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_remaining_ground_oracle import OriginalRemainingGroundOracle
from original_selected_diagnostic_oracle import OriginalSelectedDiagnosticOracle
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE
from original_unit_oracle import DGROUP
from remaining_ground_contract import remaining
from roster_promotion_contract import store, word
from selected_diagnostic_contract import panel
from target_acquisition_contract import acquisition
from target_discovery_contract import discovery
from test_ground import contact

# Existing complete children guard8fc0..9004. The parent adds its two-byte
# indirect return below their standalone frame; preserve the bounded union.
STACK_BEGIN, STACK_END = 0x8fbe, 0x9004


class OriginalGroundPhaseOracle(OriginalRemainingGroundOracle):
    execute = staticmethod(OriginalGroundManeuverOracle.execute)

    def __init__(self, audio):
        super().__init__(audio)
        self.diagnostic = OriginalSelectedDiagnosticOracle()
        self.shadow = self.machine()
        for offset, expected in ((0x98dc, AUTOMATIC), (0x98fc, CONTROLLED)):
            if struct.unpack_from('<16H', self.image, DGROUP + offset) != expected:
                raise AssertionError('Complete original parent callback bank differs')

    def prepared_machine(self, data):
        machine = self.diagnostic.relocated(data)
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, MAILBOX // 16))
        return machine

    def install_height(self, side, pixels):
        super().install_height(side, pixels)
        self.visibility_machine = self.visibility.prepare(side, pixels)

    def plan(self, data, actor, entry, audio_options):
        plan = {'transfers': [], 'requests': [], 'height': None,
                'expected_data': None, 'expected_actor': None}
        if entry not in (0xb011, 0xae32, 0xaf97, 0xafa2, 0xae5c, 0xb0be):
            return plan
        raw = data[actor:actor + 251]
        objects = {pointer: self.raw_from_data(data, pointer)
                   for pointer in set(struct.unpack_from('<364H', data, 0xdfbc)[::2]) if pointer}
        for pointer in (word(raw, 0x97), word(raw, 0x9d)):
            if pointer:
                objects[pointer] = self.raw_from_data(data, pointer)
        if entry == 0xb011:
            effect = discovery(self, actor, raw, struct.unpack_from('<364H', data, 0xdfbc)[::2],
                               objects, data[0x6dae], self.plane_side, self.plane,
                               coarse=data[0x2040], gate=word(data, 0x6da2),
                               selected=word(data, 0x6d34), clock=word(data, 0x452),
                               last_voice=word(data, 0x9fca))
            plan.update(transfers=effect['transfers'], requests=effect['requests'],
                        expected_actor=effect['actor'])
        elif entry == 0xae32:
            cursor = ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4
            seeds = list(struct.unpack_from('<4H', data, 0x1f84))
            effect = acquisition(self, actor, raw, objects, (seeds, cursor),
                                 word(data, word(data, 0x9796)), self.plane_side, self.plane,
                                 automatic=True, candidate=word(raw, 0x9d),
                                 selected=word(data, 0x6d34), gate=word(data, 0x6da2),
                                 clock=word(data, 0x452), last_voice=word(data, 0x9fca),
                                 display=struct.unpack_from('<2H', data, 0x969e))
            plan.update(transfers=[effect['transfer']] if effect['transfer'] else [],
                        requests=effect['requests'], expected_actor=effect['actor'])
        elif entry in (0xaf97, 0xafa2):
            expected, effect = automatic_fire(data, actor, entry=entry)
            packet = effect['request']
            plan.update(expected_data=expected,
                        requests=[(packet['ax'], packet['dl'], packet['ecx'])] if packet else [])
        elif entry in (0xae5c, 0xb0be):
            _, registers = self.audio.fixture(**audio_options)
            _, response, _ = request(bytes(self.audio.machine.mem_read(0, len(self.audio.baseline))),
                                     registers, self.audio.banks)
            expected, effect = remaining(data, actor, self.image[TEXT_BASE:TEXT_BASE + 65536],
                                         entry=entry, height=0, audio_return=response['eax'])
            if effect['height_position'] is not None:
                position = struct.unpack('<2i', effect['height_position'])
                height = contact(self.plane_side, self.plane, (*position, 0))[0]
                expected, effect = remaining(data, actor, self.image[TEXT_BASE:TEXT_BASE + 65536],
                                             entry=entry, height=height, audio_return=response['eax'])
                plan['height'] = (effect['allocation'][0] + 4, effect['height_position'], height)
            plan.update(expected_data=expected,
                        requests=[effect['request']] if effect['audio'] else [])
        return plan

    @staticmethod
    def raw_from_data(data, pointer):
        return data[pointer:pointer + (251 if word(data, pointer) in (0, 1, 2, 3, 19) else 55)]

    def run_child(self, machine, start, stop, plan, audio_options):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_DX,
                                      UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX)
        before = bytes(machine.mem_read(0, 0x60000))
        expected_mailbox = bytearray(before[MAILBOX:MAILBOX + 4096])
        transfers, requests = [], []
        if plan['height'] is not None:
            pointer, position, expected = plan['height']
            self.execute(machine, start, 0xe1eb)
            if (machine.reg_read(UC_X86_REG_DI) != pointer or
                    bytes(machine.mem_read(DGROUP + pointer, 8)) != position or
                    word(bytes(machine.mem_read(DGROUP, 65536)), 0xea10) != 0x54):
                raise AssertionError('Actual parent/child constructor height request differs')
            actual = self.map_height(pointer, position)
            if actual != expected:
                raise AssertionError('Independent and genuine kernel height differ')
            machine.reg_write(UC_X86_REG_AX, actual)
            start = 0xe1ee
        for transfer in plan['transfers']:
            self.execute(machine, start, 0xe286)
            # e291 transports EBX before e299 replaces its low word with58.
            mailbox_ebx = machine.reg_read(UC_X86_REG_EBX)
            self.execute(machine, 0xe286, 0xe2a0)
            source, target = struct.unpack('<3i', machine.mem_read(MAILBOX + 0xd2, 12)), struct.unpack(
                '<3i', machine.mem_read(MAILBOX + 0xde, 12))
            if ((source, target) != (transfer['source'], transfer['target']) or
                    bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x58\0'):
                raise AssertionError('Actual complete parent visibility operands differ')
            visible = self.visibility.visible(self.visibility_machine, [(source, target)])[0]
            if visible != transfer['visible']:
                raise AssertionError('Independent and actual kernel visibility differ')
            machine.reg_write(UC_X86_REG_EAX, self.visibility_machine.reg_read(UC_X86_REG_EAX))
            struct.pack_into('<6i', expected_mailbox, 0xd2, *source, *target)
            struct.pack_into('<I', expected_mailbox, 0x3f2, mailbox_ebx)
            transfers.append((source, target, visible))
            start = 0xe2a3
        for packet in plan['requests']:
            self.execute(machine, start, 0xe2c2)
            # e2cc transports EBX before e2d4 replaces its low word with64.
            mailbox_ebx = machine.reg_read(UC_X86_REG_EBX)
            self.execute(machine, 0xe2c2, 0xe2db)
            actual = (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX),
                      machine.reg_read(UC_X86_REG_ECX))
            if ((actual[0], actual[1] & 255, actual[2]) != (packet[0], packet[1] & 255, packet[2]) or
                    bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x64\0'):
                raise AssertionError('Actual complete parent audio request differs')
            if packet[0] >= 0x280 and actual[1] != packet[1]:
                raise AssertionError('Actual parent voice lost its complete clock operand')
            audio_before, _ = self.audio.fixture(**audio_options)
            registers = {name: machine.reg_read(reg) for name, reg in self.audio.registers.items()}
            registers['ebx'] = struct.unpack('<I', machine.mem_read(MAILBOX + 0x3f2, 4))[0]
            if registers['ebx'] != mailbox_ebx:
                raise AssertionError(f"Original op-64 EBX transfer differs: {registers['ebx']:#x} / "
                                     f"{mailbox_ebx:#x}")
            struct.pack_into('<I', expected_mailbox, 0x3f2, registers['ebx'])
            for name, value in registers.items():
                self.audio.machine.reg_write(self.audio.registers[name], value)
            kernel, response, effect = self.audio.observe(audio_before, registers)
            machine.reg_write(UC_X86_REG_EAX, response['eax'])
            requests.append((actual, registers['ebx'], response['eax'], effect['branch'], kernel))
            start = 0xe2de
        self.execute(machine, start, stop)
        after = bytes(machine.mem_read(0, 0x60000))
        external = bytearray(before)
        external[DGROUP:DGROUP + 65536] = after[DGROUP:DGROUP + 65536]
        external[MAILBOX:MAILBOX + 4096] = expected_mailbox
        if after != external:
            changes = [hex(i) for i, (a, b) in enumerate(zip(after, external)) if a != b]
            raise AssertionError('Original child changed code/text/external/mailbox bytes: ' + str(changes[:16]))
        return transfers, requests

    @staticmethod
    def compare_memory(actual, expected, description):
        normalized = bytearray(actual)
        normalized[DGROUP + STACK_BEGIN:DGROUP + STACK_END] = expected[
            DGROUP + STACK_BEGIN:DGROUP + STACK_END]
        if normalized != expected:
            changes = [hex(i) for i, (a, b) in enumerate(zip(normalized, expected)) if a != b]
            raise AssertionError(description + ': ' + str(changes[:16]))

    def observe(self, machine, actor, **audio_options):
        from unicorn.x86_const import (UC_X86_REG_CS, UC_X86_REG_DI, UC_X86_REG_DS,
                                      UC_X86_REG_ES, UC_X86_REG_FS, UC_X86_REG_GS,
                                      UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_EAX,
                                      UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                                      UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
                                      UC_X86_REG_EFLAGS)
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(0, 0x60000))
        data, effect = prefix(before[DGROUP:DGROUP + 65536], actor)
        boundary = 0xab78 if effect['entry'] is None else 0xab6e if effect['automatic'] else 0xab74
        self.execute(machine, 0xab03, boundary)
        expected = bytearray(before)
        expected[DGROUP:DGROUP + 65536] = data
        current = bytes(machine.mem_read(0, 0x60000))
        self.compare_memory(current, expected, 'Complete independently predicted parent prefix differs')
        transcript = ([], [])
        if effect['entry'] is not None:
            plan = self.plan(data, actor, effect['entry'], audio_options)
            shadow = self.shadow
            shadow.mem_write(0, current)
            for register in (UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_ES, UC_X86_REG_FS,
                             UC_X86_REG_GS, UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                             UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP,
                             UC_X86_REG_EFLAGS):
                shadow.reg_write(register, machine.reg_read(register))
            shadow.reg_write(UC_X86_REG_SP, 0x8ffe)
            shadow.mem_write(DGROUP + 0x8ffe, struct.pack('<H', 0xeff0))
            wanted_transcript = self.run_child(shadow, effect['entry'], 0xeff0, plan, audio_options)
            if tuple(shadow.reg_read(reg) for reg in (UC_X86_REG_DI, UC_X86_REG_DS,
                    UC_X86_REG_SP, UC_X86_REG_SS, UC_X86_REG_CS)) != (actor, 0x1c00, 0x9000, 0x1c00, 0):
                raise AssertionError('Complete separately executed original child ABI differs')
            transcript = self.run_child(machine, boundary, 0xab78, plan, audio_options)
            if transcript != wanted_transcript:
                raise AssertionError('Original complete child/parent PM transcripts differ')
            expected = bytes(shadow.mem_read(0, 0x60000))
            current = bytes(machine.mem_read(0, 0x60000))
            self.compare_memory(current, expected, 'Complete child/parent composition differs')
            if plan['expected_actor'] is not None and current[DGROUP + actor:DGROUP + actor + 251] != plan['expected_actor']:
                raise AssertionError('Independent complete discovery/acquisition actor differs')
            if plan['expected_data'] is not None:
                wanted = bytearray(current)
                wanted[DGROUP:DGROUP + 65536] = plan['expected_data']
                self.compare_memory(current, wanted, 'Independent complete fire/station/support state differs')
        expected = bytearray(current)
        screen_address = self.diagnostic.screen_address(machine)
        effect['diagnostic'] = word(data, 0x7ae0) == actor
        if effect['diagnostic']:
            final, screen = panel(current[DGROUP:DGROUP + 65536], actor,
                                  current[screen_address:screen_address + 4000])
            expected[DGROUP:DGROUP + 65536] = final
            expected[screen_address:screen_address + 4000] = screen
        self.execute(machine, 0xab78, 0xeff0)
        after = bytes(machine.mem_read(0, 0x60000))
        self.compare_memory(after, expected, 'Complete independently predicted parent diagnostic tail differs')
        if tuple(machine.reg_read(reg) for reg in (UC_X86_REG_DI, UC_X86_REG_DS,
                UC_X86_REG_SP, UC_X86_REG_SS, UC_X86_REG_CS)) != (actor, 0x1c00, 0x9002, 0x1c00, 0):
            raise AssertionError('Complete original parent ABI/return differs')
        effect.update(visibility_returns=len(transcript[0]), audio_returns=len(transcript[1]),
                      height_returns=int(effect['entry'] is not None and plan['height'] is not None))
        return after[DGROUP:DGROUP + 65536], effect
