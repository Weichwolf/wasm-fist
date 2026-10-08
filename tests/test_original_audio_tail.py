#!/usr/bin/env python3
"""Required authored sample-15 sentinel overread with real adjacent sound banks.

Selector 51 really admits AX=0x010f through c047. DSOUNDS has records 0..14
and a zero paragraph sentinel. Original op-64 treats the address after that
sentinel as a sample header, reading the next allocation. This is invalid-domain
evidence, not an added valid PCM record or a runtime behavior specification.
"""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from audio_request_contract import BANK_PINS, CHANNELS
from original_audio_request_oracle import BANK_BASES, OriginalAudioRequestOracle
from original_ground_oracle import RETURN
from original_support_audio_oracle import OriginalSupportAudioOracle
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE
from original_unit_oracle import DGROUP
from roster_promotion_contract import store


def adjacent_fixture(name):
    from unicorn import UC_PROT_READ
    audio = OriginalAudioRequestOracle()
    effects, records, old_size = audio.assets['DSOUNDS.BIN']
    neighbor = audio.assets[name][0]
    combined = effects + neighbor
    size = (len(combined) + 4095) & ~4095
    base = BANK_BASES['DSOUNDS.BIN']
    audio.machine.mem_unmap(base, old_size)
    audio.machine.mem_map(base, size)
    audio.machine.mem_write(base, combined)
    audio.machine.mem_protect(base, size, UC_PROT_READ)
    audio.guards[base] = combined + bytes(size - len(combined))
    # This synthetic model entry explicitly represents the invalid sentinel
    # read. Its body is the next allocation, not an original sixteenth record.
    audio.banks[base] = (combined, records + ({'body': len(effects), 'invalid_sentinel': True},))
    return audio, effects, neighbor, size


def allocate_adjacent(audio, neighbor, size):
    """Run both complete original allocations with the loader's alignment=4."""
    base = BANK_BASES['DSOUNDS.BIN']
    frontier = audio.effects_allocation(0)
    if frontier != base:
        raise AssertionError('Actual effects allocation moved unexpectedly')
    before = bytearray(audio.machine.mem_read(0, len(audio.baseline)))
    struct.pack_into('<I', before, 0x90f, base + size)
    audio.machine.mem_write(0, bytes(before))
    pointer = base + len(audio.assets['DSOUNDS.BIN'][0])
    expected = bytearray(before)
    end = pointer + len(neighbor)
    for offset, value in {0x2f5c: RETURN, 0x28b0: 0x85b4, 0x2bd0: len(neighbor),
            0x2d60: 4, 0x2f50: end, 0x85b4: pointer, 0x2a40: pointer, 0x2f54: 2,
            0x2f68: max(struct.unpack_from('<I', before, 0x2f68)[0], 2),
            0x2f6c: max(struct.unpack_from('<I', before, 0x2f6c)[0], end)}.items():
        struct.pack_into('<I', expected, offset, value)
    expected[0x2eed] = 3
    inputs = {'eax': 3, 'ebx': 4, 'ecx': len(neighbor), 'edx': 0x85b4,
              'esi': 0, 'edi': 17, 'ebp': 0}
    for name, value in inputs.items():
        audio.machine.reg_write(audio.registers[name], value)
    audio.owner.call(audio.machine, 0x36bf)
    if bytes(audio.machine.mem_read(0, len(expected))) != expected:
        raise AssertionError('Actual complete adjacent sound allocation writes differ')
    predicted = dict(inputs, eax=pointer, ebx=0x85b4, edx=3, esi=1, ebp=3)
    actual = {name: audio.machine.reg_read(register) for name, register in audio.registers.items()}
    if actual != predicted:
        raise AssertionError('Actual adjacent sound allocation registers differ')
    for address, guard in audio.guards.items():
        if bytes(audio.machine.mem_read(address, len(guard))) != guard:
            raise AssertionError('Actual adjacent allocation modified original bank/map guards')
    return pointer


class TailTests(unittest.TestCase):
    digest = hashlib.sha256()
    cases = {}

    def test_01_complete_authored_sentinel_request_and_adjacent_allocator(self):
        count = 0
        for name in ('WVSOUNDS.BIN', 'EVSOUNDS.BIN'):
            audio, effects, neighbor, size = adjacent_fixture(name)
            self.assertEqual(effects[-2:], bytes(2))
            pointer = allocate_adjacent(audio, neighbor, size)
            frames, rate = struct.unpack_from('<HH', neighbor)
            for channel in CHANNELS:
                for busy in (False, True):
                    for pitch in (0, 1, 0xffffffff):
                        for attenuation in (0, 127, 255):
                            before, registers = audio.fixture(packet=15 | channel << 8,
                                busy=busy, pitch=pitch, attenuation=attenuation)
                            actual, actual_registers, effect = audio.observe(before, registers)
                            self.assertEqual(actual_registers['eax'], pointer)
                            self.assertEqual(actual_registers['ecx'], pitch or rate)
                            if effect['started']:
                                self.assertEqual(struct.unpack_from('<I', actual, 0x15e3 + (channel & 127) * 4)[0], frames)
                            self.digest.update(actual)
                            self.digest.update(struct.pack('<7I', *actual_registers.values()))
                            count += 1
            self.cases[name] = {'effects_records': len(audio.assets['DSOUNDS.BIN'][1]),
                'sentinel_sample_index': 15, 'returned_pointer_offset': pointer - BANK_BASES['DSOUNDS.BIN'],
                'effects_file_size': len(effects), 'adjacent_header_frames': frames,
                'adjacent_header_rate': rate, 'actual_complete_allocator_returns': 2}
        self.assertEqual(count, 216)
        self.cases['complete_kernel_returns'] = count

    def test_02_actual_c047_selector_51_to_complete_original_queue_return(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EAX, UC_X86_REG_SP
        count = 0
        for name in ('WVSOUNDS.BIN', 'EVSOUNDS.BIN'):
            audio, effects, _, _ = adjacent_fixture(name)
            owner = OriginalSupportAudioOracle(audio)
            for selected in (False, True):
                machine, actor = owner.fixture(selected=selected)
                self.assertEqual(bytes(machine.mem_read(DGROUP + 0x9fe1 + 51, 3)), b'\x0f\x01\0')
                machine.reg_write(UC_X86_REG_AX, 51)
                machine.reg_write(UC_X86_REG_DI, actor)
                machine.reg_write(UC_X86_REG_SP, 0x9000)
                machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
                before = bytes(machine.mem_read(DGROUP, 65536))
                code = bytes(machine.mem_read(0, DGROUP))
                text = bytes(machine.mem_read(TEXT_BASE, 65536))
                mailbox = bytes(machine.mem_read(MAILBOX, 4096))
                kernel_before, _ = audio.fixture()
                owner.execute(machine, 0xc047, 0xe2db)
                self.assertEqual(machine.reg_read(UC_X86_REG_AX), 0x010f)
                registers = {key: machine.reg_read(register) for key, register in audio.registers.items()}
                registers['ebx'] = struct.unpack('<I', machine.mem_read(MAILBOX + 0x3f2, 4))[0]
                self.assertEqual(registers['ebx'], 51)
                for key, value in registers.items():
                    audio.machine.reg_write(audio.registers[key], value)
                kernel, returned, _ = audio.observe(kernel_before, registers)
                self.assertEqual(returned['eax'], BANK_BASES['DSOUNDS.BIN'] + len(effects))
                machine.reg_write(UC_X86_REG_EAX, returned['eax'])
                owner.execute(machine, 0xe2de, 0xeff0)
                self.assertEqual(tuple(machine.reg_read(register) for register in owner.machine_registers()),
                                 (actor, 0x1c00, 0x9004, 0x1c00))
                self.assertEqual(machine.reg_read(UC_X86_REG_EAX), returned['eax'])
                after = bytes(machine.mem_read(DGROUP, 65536))
                expected = bytearray(before)
                store(expected, 0x9fdd, 51)
                store(expected, 0xea10, 0x64)
                expected[0x8fc0:0x9000] = after[0x8fc0:0x9000]
                self.assertEqual(after, expected)
                expected_mailbox = bytearray(mailbox)
                struct.pack_into('<I', expected_mailbox, 0x3f2, 51)
                self.assertEqual(bytes(machine.mem_read(MAILBOX, 4096)), expected_mailbox)
                self.assertEqual(bytes(machine.mem_read(0, DGROUP)), code)
                self.assertEqual(bytes(machine.mem_read(TEXT_BASE, 65536)), text)
                self.digest.update(after[:0x8fc0] + after[0x9000:])
                self.digest.update(kernel)
                count += 1
        self.assertEqual(count, 4)
        self.cases['complete_actual_c047_kernel_pairs'] = count


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'audio-tail.json'
    output.unlink(missing_ok=True)
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 2 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 2, 'skips': 0, 'cases': TailTests.cases,
            'scope': 'Authored c047 selector 51 admits sample 15 and reads the next actually allocated original sound bank; this is invalid-domain evidence, no valid sixteenth PCM record or repair is claimed',
            'sound_banks': BANK_PINS, 'output_sha256': TailTests.digest.hexdigest()}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result, sort_keys=True), flush=True)
    raise SystemExit(not success)
