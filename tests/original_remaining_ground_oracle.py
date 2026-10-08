"""Complete unchanged ae5c/b0be and genuine ab03 entries nine/twelve.

All gameplay calls and returns execute. Only the already proved DOS/PM ABI
transports actual kernel height/audio registers; full state is checked.
"""
import struct

from audio_request_contract import request
from original_support_audio_oracle import OriginalSupportAudioOracle
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE
from original_unit_oracle import DGROUP
from remaining_ground_contract import (CUE_TABLES, NORMAL_TABLES, RELOAD_TABLES,
                                      VARIANT_TABLES, preferences, remaining)
from test_ground import contact
from test_weapon_control import COUNTS, LOAD_CUES, TIMES


class OriginalRemainingGroundOracle(OriginalSupportAudioOracle):
    def __init__(self, audio):
        super().__init__(audio)
        for index, target in ((9, 0xae5c), (12, 0xb0be)):
            if struct.unpack_from('<H', self.image, DGROUP + 0x98dc + index * 2)[0] != target:
                raise AssertionError('Actual automatic parent entry differs')
            if struct.unpack_from('<H', self.image, DGROUP + 0x98fc + index * 2)[0] != 0xb111:
                raise AssertionError('Actual controlled parent is not the genuine RET')
        for kind in range(4):
            for target in range(28):
                pointer = struct.unpack_from('<H', self.image, TEXT_BASE + NORMAL_TABLES[kind] + target * 2)[0]
                if target == 26:
                    if pointer != 65535:
                        raise AssertionError('Original variant sentinel differs')
                elif tuple(self.image[TEXT_BASE + pointer:TEXT_BASE + pointer + 3]) != preferences(kind, target):
                    raise AssertionError('Literal original station preference differs')
            for variant in range(4):
                pointer = struct.unpack_from('<H', self.image, TEXT_BASE + VARIANT_TABLES[kind] + variant * 2)[0]
                if tuple(self.image[TEXT_BASE + pointer:TEXT_BASE + pointer + 3]) != preferences(kind, 26, variant):
                    raise AssertionError('Literal original variant preference differs')
            for table, expected in ((RELOAD_TABLES[kind], TIMES[kind]), (CUE_TABLES[kind], LOAD_CUES[kind])):
                if self.image[DGROUP + table:DGROUP + table + COUNTS[kind]] != bytes(expected):
                    raise AssertionError('Original setter table differs')
        self.install_height(2, bytes([17]) * 4)

    def install_height(self, side, pixels):
        from unicorn import UC_PROT_READ
        from original_ground_oracle import BUFFER
        self.height_cache.clear()
        self.plane_side, self.plane = side, bytes(pixels)
        self.height_machine = self.ground.prepare(side, self.plane)
        self.height_machine.mem_protect(BUFFER, (len(pixels) + 4095) & ~4095, UC_PROT_READ)
        self.height_cache[(side, self.plane)] = self.height_machine

    def map_height(self, position, raw):
        from original_ground_oracle import DGROUP as KD, RETURN, ROSTER, STACK
        machine = self.height_machine
        machine.mem_write(KD + position, raw)
        guards = {address: bytes(machine.mem_read(address, size)) for address, size in
                  ((0, (len(self.ground.image) + 4095) & ~4095), (KD, 65536),
                   (ROSTER, 4096), (RETURN, 4096))}
        stack = bytes(machine.mem_read(STACK, 4096))
        value = self.height(position, raw, self.plane, side=self.plane_side)
        for address, guard in guards.items():
            if bytes(machine.mem_read(address, len(guard))) != guard:
                raise AssertionError('Actual original height changed immutable kernel/maps/guards')
        after = bytes(machine.mem_read(STACK, 4096))
        if after[:0xfe4] != stack[:0xfe4] or after[0xff4:] != stack[0xff4:]:
            raise AssertionError('Actual original height escaped bounded stack')
        return value

    def observe(self, machine, actor, audio_fixture, *, entry=0xae5c):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EAX,
                                      UC_X86_REG_ECX, UC_X86_REG_DX, UC_X86_REG_SP,
                                      UC_X86_REG_CS, UC_X86_REG_EBX)
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        _, predicted_audio, _ = request(*audio_fixture, self.audio.banks)
        expected, effect = remaining(before, actor, text, entry=entry, height=0,
                                     audio_return=predicted_audio['eax'])
        if effect['operation'] == 'station' and effect['audio']:
            effect['mailbox_ebx'] |= machine.reg_read(UC_X86_REG_EBX) & 0xffff0000
        if effect['height_position'] is not None:
            x, y = struct.unpack('<2i', effect['height_position'])
            height = contact(self.plane_side, self.plane, (x, y, 0))[0]
            expected, effect = remaining(before, actor, text, entry=entry, height=height,
                                         audio_return=predicted_audio['eax'])
            self.execute(machine, entry, 0xe1eb)
            position = effect['allocation'][0] + 4
            raw = bytes(machine.mem_read(DGROUP + position, 8))
            if (machine.reg_read(UC_X86_REG_DI) != position or raw != effect['height_position']
                    or bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x54\0'):
                raise AssertionError('Original constructor height request differs')
            actual_height = self.map_height(position, raw)
            if actual_height != height:
                raise AssertionError('Independent height prediction differs')
            machine.reg_write(UC_X86_REG_AX, actual_height)
            entry = 0xe1ee
        if effect['audio']:
            self.execute(machine, entry, 0xe2db)
            actual_request = (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX),
                              machine.reg_read(UC_X86_REG_ECX))
            # c047 only replaces DL; its DX high byte follows the constructor's
            # actual rotation. bf3c supplies the complete documented clock word.
            predicted_request = effect['request']
            if (actual_request[0] != predicted_request[0] or actual_request[2] != 0
                    or (actual_request[1] if effect['operation'] == 'station'
                        else actual_request[1] & 255) != predicted_request[1]
                    or bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x64\0'):
                raise AssertionError('Original complete audio request differs')
            registers = {name: machine.reg_read(register) for name, register in self.audio.registers.items()}
            registers['ebx'] = struct.unpack('<I', machine.mem_read(MAILBOX + 0x3f2, 4))[0]
            if registers['ebx'] != effect['mailbox_ebx']:
                raise AssertionError('Original complete audio mailbox differs')
            for name, value in registers.items():
                self.audio.machine.reg_write(self.audio.registers[name], value)
            _, actual_audio, kernel_effect = self.audio.observe(audio_fixture[0], registers)
            machine.reg_write(UC_X86_REG_EAX, actual_audio['eax'])
            effect['kernel_branch'] = kernel_effect['branch']
            entry = 0xe2de
        if effect['consumed_al'] is not None:
            comparison = 0xb10b if before[actor + 0x43] == 4 else 0xb0e7
            self.execute(machine, entry, comparison)
            if machine.reg_read(UC_X86_REG_AX) & 255 != effect['consumed_al']:
                raise AssertionError('Actual support consumed another complete smoke return')
            entry = comparison
        self.execute(machine, entry, 0xeff0)
        registers = tuple(machine.reg_read(reg) for reg in self.machine_registers())
        if registers != (actor, 0x1c00, 0x9002, 0x1c00) or machine.reg_read(UC_X86_REG_CS) != 0:
            raise AssertionError('Original complete actor/segment/stack/return differs')
        after = bytes(machine.mem_read(DGROUP, 65536))
        expected[0x8fc0:0x9000] = after[0x8fc0:0x9000]
        if after != expected:
            differences = [hex(i) for i, (a, b) in enumerate(zip(after, expected)) if a != b]
            raise AssertionError('Complete remaining-ground DGROUP differs: ' + str(differences[:24]))
        expected_mailbox = bytearray(mailbox)
        if effect['mailbox_ebx'] is not None:
            struct.pack_into('<I', expected_mailbox, 0x3f2, effect['mailbox_ebx'])
        if bytes(machine.mem_read(MAILBOX, 4096)) != expected_mailbox:
            raise AssertionError('Original remaining-ground changed another mailbox byte')
        if bytes(machine.mem_read(0, DGROUP)) != code or bytes(machine.mem_read(TEXT_BASE, 65536)) != text:
            raise AssertionError('Original remaining-ground changed immutable code/text')
        return after, effect
