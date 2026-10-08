#!/usr/bin/env python3
"""Required complete original audio queue returns and consumed support coupling."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import time
import unittest

from audio_request_contract import BANK_PINS, CHANNELS, SILENCE
from original_asset_oracle import IMAGE_SHA256 as KERNEL_SHA256
from original_audio_request_oracle import OriginalAudioRequestOracle
from original_support_audio_oracle import OriginalSupportAudioOracle
from original_unit_oracle import IMAGE_SHA256 as ENGINE_SHA256
from roster_promotion_contract import word


class AudioRequestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.audio = OriginalAudioRequestOracle()
        cls.support = OriginalSupportAudioOracle(cls.audio)
        cls.groups = {}
        cls.counts = collections.Counter()
        cls.branches = collections.Counter()
        cls.support_branches = collections.Counter()
        cls.smoke_branches = collections.Counter()
        cls.queue_indices = collections.defaultdict(set)
        cls.digest = hashlib.sha256()
        cls.coupling = {}

    def queue(self, **fixture):
        before, registers = self.audio.fixture(**fixture)
        actual, actual_registers, effect = self.audio.observe(before, registers)
        self.digest.update(actual)
        self.digest.update(struct.pack('<7I', *actual_registers.values()))
        self.counts['complete_kernel_returns'] += 1
        self.branches[effect['branch']] += 1
        self.counts['queued_immediate_starts'] += int(effect['branch'] == 'queued' and effect['started'])
        self.counts['queued_deferred'] += int(effect['branch'] == 'queued' and not effect['started'])
        return actual, actual_registers, effect

    def coupled(self, kind=0, *, enabled=1, missing=False, height=17, **fixture):
        machine, actor = self.support.fixture(kind, **fixture)
        before = bytes(machine.mem_read(0x1c000, 65536))
        actual, effect = self.support.observe(machine, actor, self.audio.fixture(
            enabled=enabled, missing_effects=missing), height=height)
        self.digest.update(actual[:0x8fc0])
        self.digest.update(actual[0x9000:])
        self.counts['complete_dos_returns'] += 1
        self.counts['actual_coupled_kernel_returns'] += int(effect['audio'])
        self.counts['actual_type20_constructors_and_height_returns'] += int(effect['allocation'] is not None)
        self.support_branches[effect['support']] += 1
        self.smoke_branches[effect['smoke']] += 1
        if effect['queue_index'] is not None:
            self.queue_indices[effect['support']].add(effect['queue_index'])
        return before, actual, effect, actor

    def test_01_every_real_record_direct_queued_silence_and_busy_channels(self):
        before_count = self.counts['complete_kernel_returns']
        self.audio.place('DSOUNDS.BIN', 0)
        self.audio.place('WVSOUNDS.BIN', 0)
        self.audio.place('EVSOUNDS.BIN', 0)
        for name, (_, records, _) in self.audio.assets.items():
            for index, channel, busy, pitch, attenuation in itertools.product(
                    range(len(records)), CHANNELS, (False, True),
                    (0, 1, 32768, 65535, 0xffffffff), (0, 1, 127, 255)):
                sample = index if name == 'DSOUNDS.BIN' else index | 128
                self.queue(packet=sample | channel << 8, busy=busy, pitch=pitch,
                           attenuation=attenuation, voice=name if sample & 128 else 'WVSOUNDS.BIN')
        for channel, busy, pitch, attenuation, voice in itertools.product(
                CHANNELS, (False, True), (0, 1, 32768, 65535, 0xffffffff),
                (0, 1, 127, 255), ('WVSOUNDS.BIN', 'EVSOUNDS.BIN')):
            _, registers, effect = self.queue(packet=255 | channel << 8, busy=busy,
                pitch=pitch, attenuation=attenuation, voice=voice)
            self.assertEqual((registers['eax'], registers['ecx'], registers['edx']), (SILENCE, 0, 0))
            self.assertEqual(effect['branch'], 'silence')
        count = self.counts['complete_kernel_returns'] - before_count
        self.assertEqual(count, 24720)
        self.audio.verify_assets()
        self.groups['every_bank_record_channels_silence'] = count

    def test_02_full_disabled_packet_sound_byte_and_missing_bank_domains(self):
        before_count = self.counts['complete_kernel_returns']
        for packet in range(65536):
            _, registers, effect = self.queue(packet=0xbeef0000 | packet, enabled=0,
                pitch=packet ^ 0xffffffff, attenuation=packet & 255, busy=bool(packet & 1))
            self.assertEqual(registers['eax'], 0xbeef0000 | packet)
            self.assertEqual(effect['branch'], 'disabled')
        for enabled, channel, sample, voice in itertools.product(
                range(1, 256), CHANNELS, (0, 11, 128, 170, 255), ('WVSOUNDS.BIN', 'EVSOUNDS.BIN')):
            self.queue(packet=sample | channel << 8, enabled=enabled, voice=voice,
                       busy=bool(enabled & 1), attenuation=enabled)
        for channel, sample, pitch, busy in itertools.product(
                CHANNELS, range(256), (0, 0xdeadbeef), (False, True)):
            self.queue(packet=0xcafe0000 | channel << 8 | sample, missing_effects=True,
                       missing_voice=True, pitch=pitch, busy=busy, attenuation=sample)
        count = self.counts['complete_kernel_returns'] - before_count
        self.assertEqual(count, 86980)
        self.audio.verify_assets()
        self.groups['disabled_packet_sound_byte_missing_banks'] = count

    def test_03_full_rate_attenuation_and_sample_pointer_low_byte_turn(self):
        before_count = self.counts['complete_kernel_returns']
        aligned = {self.audio.effects_allocation(frontier) & 255 for frontier in range(256)}
        self.assertEqual(aligned, set(range(0, 256, 4)))
        self.counts['actual_complete_effects_allocator_returns'] += 256
        for pitch in range(65536):
            sample = pitch % 15 if pitch & 1 else 128 | pitch % 43
            self.queue(packet=sample | CHANNELS[pitch % 6] << 8, pitch=pitch,
                       attenuation=(pitch >> 8) & 255, busy=bool(pitch & 4),
                       voice='WVSOUNDS.BIN' if pitch & 2 else 'EVSOUNDS.BIN')
        for attenuation, channel, busy, pitch in itertools.product(
                range(256), CHANNELS, (False, True), (0, 1, 65536, 0x80000000, 0xffffffff)):
            self.queue(packet=11 | channel << 8, pitch=pitch, attenuation=attenuation, busy=busy)
        for offset in range(256):
            for name in BANK_PINS:
                self.audio.place(name, offset)
                records = self.audio.assets[name][1]
                for index, channel in itertools.product(range(len(records)), CHANNELS):
                    sample = index if name == 'DSOUNDS.BIN' else index | 128
                    _, registers, _ = self.queue(packet=sample | channel << 8,
                        voice=name if sample & 128 else 'WVSOUNDS.BIN', busy=bool(offset & 1))
                    self.assertEqual(registers['eax'] & 255, (offset + records[index]['body']) & 255)
                    self.counts['allocator_aligned_record_placements' if offset % 4 == 0
                                else 'additional_unaligned_handler_inputs'] += 1
            self.audio.verify_assets()
        count = self.counts['complete_kernel_returns'] - before_count
        self.assertEqual(count, 236032)
        self.groups['rate_attenuation_and_every_record_pointer_turn'] = count

    def test_04_actual_full_coupled_support_at_every_bank_low_byte(self):
        before_count = self.counts['complete_dos_returns']
        baselines = {}
        for offset, kind, selected, source, context in itertools.product(
                range(256), range(4), (False, True), (False, True), ('enabled', 'disabled', 'absent')):
            self.audio.place('DSOUNDS.BIN', offset)
            before, after, effect, actor = self.coupled(kind, enabled=int(context != 'disabled'),
                missing=context == 'absent', selected=selected, source=source)
            key = kind, selected, source
            if key not in baselines:
                baselines[key] = before
            self.assertEqual(before, baselines[key], 'Only the audio context changes for the paired observer')
            expected_al = (offset + 8) & 255 if context == 'enabled' and source else 11 if source else 39
            self.assertEqual(effect['consumed_al'], expected_al)
            expected_support = ('not_called' if expected_al > 76 else
                                'artillery_confirmed' if expected_al & 32 else 'air_confirmed')
            self.assertEqual(effect['support'], expected_support)
            self.counts['allocator_aligned_coupled_contexts' if offset % 4 == 0
                        else 'additional_unaligned_coupled_contexts'] += 1
            self.assertEqual(word(after, actor), kind)
            if selected and source and kind == 0 and offset in (0, 32, 128):
                self.coupling[f'{context}_offset_{offset}'] = {
                    'consumed_al': effect['consumed_al'], 'support': effect['support'],
                    'original_dgroup_input_sha256': hashlib.sha256(before).hexdigest(),
                    'original_dgroup_output_sha256': hashlib.sha256(after[:0x8fc0] + after[0x9000:]).hexdigest()}
        count = self.counts['complete_dos_returns'] - before_count
        self.assertEqual(count, 12288)
        self.audio.verify_assets()
        self.groups['coupled_every_class_context_pointer_low_byte'] = count

    def test_05_smoke_capacity_stock_admission_and_all_support_queue_exits(self):
        before_count = self.counts['complete_dos_returns']
        self.audio.place('DSOUNDS.BIN', 0)
        for kind, full, stock, source, selected in itertools.product(
                range(4), (False, True), (0, 1, 255), (False, True), (False, True)):
            self.coupled(kind, stock=stock, full=full, source=source, selected=selected)
        for stock in range(65536):
            self.coupled(2, stock=stock, full=bool(stock & 1), selected=bool(stock & 2))
        for kind in (0, 1):
            for stock, full in itertools.product(range(256), (False, True)):
                self.coupled(kind, stock=stock, full=full)
        for kind, mode, draw in itertools.product(range(4), (0, 4, 255),
                (0, 12, 13, 31, 32, 64, 65, 76, 77, 0x4000, 0x40ff, 0x4100, 65535)):
            self.coupled(kind, mode=mode, draw=draw)
        for kind, flags, target, distance, age in itertools.product(range(4), (0, 8, 255),
                (False, True), (0, 19, 20, 65535), (0, 1799, 1800, 65535)):
            self.coupled(kind, flags=flags, target=target, distance=distance, prior=(1800 - age) & 65535)
        for kind, age, count, delay, used, selected, notice in itertools.product(
                range(4), (0, 479, 480, 65535), (0, 1, 65535), (0, 1), range(17),
                (False, True), (0, 2, 255)):
            self.coupled(kind, air_age=age, air_count=count, air_delay=delay,
                         air_used=used, selected=selected, notice_context=notice)
        self.audio.place('DSOUNDS.BIN', 32)
        for kind, age, count, ammo, busy, used, selected, notice in itertools.product(
                range(4), (0, 479, 480, 65535), (0, 1), (0, 1, 65535), (0, 1, 65535),
                range(17), (False, True), (0, 2, 255)):
            self.coupled(kind, artillery_age=age, artillery_count=count, ammunition=ammo,
                         artillery_busy=busy, artillery_used=used, selected=selected, notice_context=notice)
        self.assertEqual(self.queue_indices['air_confirmed'], set(range(16)))
        self.assertEqual(self.queue_indices['artillery_confirmed'], set(range(16)))
        self.assertEqual(set(self.smoke_branches), {'not_called', 'empty', 'capacity', 'created'})
        self.assertEqual(set(self.support_branches), {'not_called', 'air_cooldown', 'air_unavailable',
            'air_confirmed', 'artillery_cooldown', 'artillery_not_in_place', 'artillery_empty',
            'artillery_busy', 'artillery_queue_full', 'artillery_confirmed'})
        count = self.counts['complete_dos_returns'] - before_count
        self.assertEqual(count, 106364)
        self.audio.verify_assets()
        self.groups['smoke_stock_capacity_and_complete_reached_support_exits'] = count

    def test_06_sequential_retained_channels_and_constructor_pose_height(self):
        before_count = self.counts['complete_kernel_returns'] + self.counts['complete_dos_returns']
        self.audio.place('DSOUNDS.BIN', 0)
        for voice in ('WVSOUNDS.BIN', 'EVSOUNDS.BIN'):
            self.audio.place(voice, 0)
            before, _ = self.audio.fixture(voice=voice)
            for cycle in range(64):
                for sample, channel, pitch, attenuation in ((0, 128, 0, 0), (11, 128, 65535, 255),
                        (170, 129, 0, 127), (129, 129, 32768, 1), (255, 0, 0, 255),
                        (11, 128, 0, 0), (255, 130, 0xffffffff, 255), (128, 2, 0, 31),
                        (255, 1, 0, 0), (255, 2, 0, 0)):
                    registers = {'eax': sample | channel << 8, 'ebx': 39, 'ecx': pitch,
                                 'edx': 0xbeef0000 | attenuation, 'esi': cycle, 'edi': 23, 'ebp': 43}
                    for name, value in registers.items():
                        self.audio.machine.reg_write(self.audio.registers[name], value)
                    before, actual_registers, effect = self.audio.observe(before, registers)
                    self.digest.update(before)
                    self.digest.update(struct.pack('<7I', *actual_registers.values()))
                    self.counts['complete_kernel_returns'] += 1
                    self.branches[effect['branch']] += 1
            self.audio.verify_assets()
        for kind, heading, coarse, velocity, pose, height, offset in itertools.product(
                range(4), (0, 1, 16384, 32768, 49152, 65535), (0, 1),
                ((0, 0), (-32768, 32767), (32767, -32768), (12, -17)),
                ((0, 0), (-2147483648, 2147483647)), (0, 1, 127, 255), (0, 32, 128)):
            self.audio.place('DSOUNDS.BIN', offset)
            self.coupled(kind, heading=heading, coarse=coarse, velocity=velocity, pose=pose, height=height)
        count = self.counts['complete_kernel_returns'] + self.counts['complete_dos_returns'] - before_count
        self.assertEqual(count, 5888)
        self.audio.verify_assets()
        self.groups['sequential_retained_channel_and_wrapped_constructor_pose_height'] = count


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'audio-request.json'
    output.unlink(missing_ok=True)
    begin = time.monotonic()
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 6 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 6, 'skips': 0, 'seconds': time.monotonic() - begin,
                  'scope': 'Complete original op-64 queue/return and coupled b0be; PCM, full DPMI/device initialization, intended support repair and full parent/game remain open',
                  'counts': dict(AudioRequestTests.counts), 'coverage': AudioRequestTests.groups,
                  'queue_branches': dict(AudioRequestTests.branches),
                  'smoke_branches': dict(AudioRequestTests.smoke_branches),
                  'support_branches': dict(AudioRequestTests.support_branches),
                  'confirmed_queue_indices': {k: sorted(v) for k, v in AudioRequestTests.queue_indices.items()},
                  'paired_contexts': AudioRequestTests.coupling, 'sound_banks': BANK_PINS,
                  'sound_records': {name: records for name, (_, records, _) in AudioRequestTests.audio.assets.items()},
                  'placement_scope': 'All 64 four-byte-aligned allocator placements plus 192 separately counted unaligned handler guard inputs; canonical paired examples use offsets 0/32/128',
                  'coupled_world_scope': 'Constructed worlds with actual physical allocation/initialization and complete unchanged handlers, original op-54 flat installed fields and actual bank/device inputs; all-47/genuine-parent acceptance remains in WI 0104',
                  'original_engine_sha256': ENGINE_SHA256, 'original_kernel_sha256': KERNEL_SHA256,
                  'output_sha256': AudioRequestTests.digest.hexdigest(), 'complete_wasm_streak': 0}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps({k: v for k, v in result.items() if k != 'paired_contexts'}, sort_keys=True), flush=True)
    raise SystemExit(not success)
