#!/usr/bin/env python3
"""Required complete original ground parent composition and heading domains.

This is reference recovery. Shared C and complete living-class acceptance stay
open. Every counted parent reaches its actual RET; incomplete or skipped runs
and missing corpus/device coverage fail rather than producing a success receipt.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import struct
import time
import unittest

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_ground_phase_oracle import OriginalGroundPhaseOracle, STACK_BEGIN, STACK_END
from original_unit_oracle import DGROUP, IMAGE_SHA256
from remaining_ground_corpus import Corpus
from roster_promotion_contract import store, word
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_weapon_control import COUNTS

REVIEW = None
SCREEN = bytes((index * 37 + 19) % 256 for index in range(4000))


class GroundParentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.begin = time.monotonic()
        cls.owner = OriginalGroundPhaseOracle(OriginalAudioRequestOracle())
        cls.counts, cls.callbacks, cls.groups = collections.Counter(), collections.Counter(), {}
        cls.digest = hashlib.sha256()

    def setUp(self):
        self.start_count = self.counts['complete_parent_returns']
        print('Required complete-parent group: ' + self._testMethodName, flush=True)

    def tearDown(self):
        self.groups[self._testMethodName] = self.counts['complete_parent_returns'] - self.start_count
        print('Complete genuine parent returns: ' + str(self.counts['complete_parent_returns']), flush=True)

    def fixture(self, kind=0, *, automatic=0, index=0, platoon=0, selected=True,
                admission=0, cursor=0, source=False, gate=0, **support):
        machine, actor = self.owner.fixture(kind, selected=True, source=source, **support)
        data = bytearray(machine.mem_read(DGROUP, 65536))
        data[actor + 27] = platoon
        data[actor + 0x42] = (index - 1) % 256
        data[0x978a] = admission
        store(data, actor + 0x40, (word(data, actor + 0x40) & 65534) | automatic)
        store(data, 0x7ae0, actor if selected else 0)
        store(data, 0x6da2, gate)
        store(data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2)
        machine = self.owner.prepared_machine(bytes(data))
        machine.mem_write(0x50000, SCREEN)
        return machine, actor

    def check(self, machine, actor, **audio):
        data, effect = self.owner.observe(machine, actor, **audio)
        self.digest.update(data[:STACK_BEGIN])
        self.digest.update(data[STACK_END:])
        self.digest.update(bytes(machine.mem_read(0x50000, 4000)))
        self.counts['complete_parent_returns'] += 1
        self.counts['complete_child_returns'] += int(effect['entry'] is not None)
        self.counts['heading_samples'] += effect['heading_sampled']
        self.counts['selected_diagnostics'] += effect['diagnostic']
        for name in ('visibility_returns', 'audio_returns', 'height_returns'):
            self.counts[name] += effect[name]
        if effect['entry'] is not None:
            self.callbacks[f"{'automatic' if effect['automatic'] else 'controlled'}_{effect['callback']}"] += 1
        if self.counts['complete_parent_returns'] % 32768 == 0:
            print('Whole-state checked parent returns: ' + str(self.counts['complete_parent_returns']), flush=True)
        return data, effect

    def test_all_counter_bytes_classes_platoons_and_banks(self):
        self.owner.install_height(2, bytes([17]) * 4)
        for kind in range(4):
            for platoon in range(8):
                for automatic in (0, 1):
                    machine, actor = self.fixture(kind, platoon=platoon, automatic=automatic,
                                                  target=False, flags=0, mode=0)
                    base = bytes(machine.mem_read(DGROUP, 65536))
                    for counter in range(256):
                        data = bytearray(base)
                        data[actor + 0x42] = counter
                        store(data, 0x1f82, 0x1f84 + ((counter % 4 + 3) % 4) * 2)
                        store(data, 0x7ae0, actor if counter & 1 else 0)
                        machine.mem_write(DGROUP, bytes(data))
                        _, effect = self.check(machine, actor)
                        self.assertEqual(effect['callback'], (counter + 1) % 16)
                        self.counts['counter_domain_returns'] += 1
        self.assertEqual(self.counts['counter_domain_returns'], 16384)

    def test_every_heading_word_with_complete_selection_and_wrap(self):
        machine, actor = self.fixture(automatic=0, index=0, target=False, flags=0, mode=0)
        base = bytearray(machine.mem_read(DGROUP, 65536))
        for offset in (0x26, 0x28, 0x2a, 0x2c):
            for value in range(65536):
                data = bytearray(base)
                for lane, fixed in ((0x26, 32768), (0x28, 65535), (0x2a, 32767), (0x2c, 1)):
                    store(data, actor + lane, fixed)
                store(data, actor + offset, value)
                store(data, actor + 0x2e, value ^ 0xa55a)
                # Both low-four-bit sampling and byte overflow retain the
                # complete genuine selector, never a substituted empty child.
                data[actor + 0x42] = 255 if value & 1 else 15
                store(data, 0x7ae0, 0)
                machine.mem_write(DGROUP, bytes(data))
                self.check(machine, actor)
                self.counts['heading_word_returns'] += 1
        self.assertEqual(self.counts['heading_word_returns'], 262144)

    def test_every_inhibition_byte_and_unused_invalid_platoon(self):
        for kind in range(4):
            machine, actor = self.fixture(kind, target=False, flags=0, mode=0, selected=False)
            base = bytes(machine.mem_read(DGROUP, 65536))
            for admission in range(1, 256):
                for cursor in range(4):
                    data = bytearray(base)
                    data[0x978a] = admission
                    data[actor + 27] = (admission + cursor) % 256
                    data[actor + 0x42] = admission
                    store(data, actor + 0x40, (0, 1, 32768, 65535)[cursor])
                    store(data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2)
                    machine.mem_write(DGROUP, bytes(data))
                    after, effect = self.check(machine, actor)
                    self.assertIsNone(effect['entry'])
                    self.assertEqual(after[actor:actor + 251], data[actor:actor + 251])
                    self.assertNotEqual(after[0x1f82:0x1f84], data[0x1f82:0x1f84])
                    self.counts['inhibited_returns'] += 1
        self.assertEqual(self.counts['inhibited_returns'], 4080)

    def test_retained_full_bank_sequences_and_extra_rng(self):
        for kind in range(4):
            for automatic in (0, 1):
                machine, actor = self.fixture(kind, automatic=automatic, mode=0, flags=8,
                                              target=True, draw=0, stock=2)
                for step in range(512):
                    machine.mem_write(DGROUP + 0x978a, bytes([int(step % 37 == 0)]))
                    store_data = bytearray(machine.mem_read(DGROUP, 65536))
                    store(store_data, 0x7ae0, actor if step % 3 else 0)
                    machine.mem_write(DGROUP, bytes(store_data))
                    self.check(machine, actor)
                    self.counts['retained_sequence_returns'] += 1
        self.assertEqual(self.counts['retained_sequence_returns'], 4096)
        # Force the selector's conditional second draw and the idle-turret
        # second/third overall parent draws using actual RNG predecessors.
        for kind in range(4):
            for index in (0, 11):
                for draw in (0, 1, 63, 64, 1024, 16384, 65535):
                    machine, actor = self.fixture(kind, automatic=1, index=index, mode=0,
                                                  target=index == 0, flags=0)
                    data = bytearray(machine.mem_read(DGROUP, 65536))
                    for stream, value in enumerate((draw, draw, draw, draw)):
                        store(data, 0x1f84 + stream * 2, seed_for_draw(value))
                    machine.mem_write(DGROUP, bytes(data))
                    self.check(machine, actor)
                    self.counts['conditional_rng_returns'] += 1
        self.assertEqual(self.counts['conditional_rng_returns'], 56)

    def test_consumed_support_height_and_audio_contexts(self):
        self.owner.install_height(2, bytes([17]) * 4)
        for kind in range(4):
            for mode in (0, 4):
                for enabled in (0, 1):
                    for missing in (False, True):
                        for busy in (False, True):
                            machine, actor = self.fixture(kind, automatic=1, index=12, mode=mode,
                                                          flags=8, source=True, stock=2, draw=0)
                            self.check(machine, actor, enabled=enabled, missing_effects=missing, busy=busy)
                            self.counts['support_device_context_returns'] += 1
        self.assertEqual(self.counts['support_device_context_returns'], 64)
        self.assertGreater(self.counts['audio_returns'], 0)
        self.assertGreater(self.counts['height_returns'], 0)

    def test_discovery_acquisition_station_and_fire_device_tails(self):
        self.owner.install_height(2, bytes(4))
        for kind in range(4):
            for index, automatic in ((6, 0), (6, 1), (8, 1), (9, 1)):
                for gate in (0, 65535):
                    for coarse in (0, 1):
                        machine, actor = self.fixture(kind, automatic=automatic, index=index,
                                                      mode=0, flags=0, target=True, gate=gate,
                                                      pose=(0, 0), target_pose=(2048, 128))
                        data = bytearray(machine.mem_read(DGROUP, 65536))
                        target = word(data, actor + 0x97)
                        data[target + 22] = 4 | 8
                        struct.pack_into('<i', data, target + 12, 0)
                        store(data, 0x9fca, 0)
                        data[0x2040] = coarse
                        for stream in range(4):
                            store(data, 0x1f84 + stream * 2, seed_for_draw(0))
                        if index in (6, 8):
                            store(data, actor + 0x97, 0)
                            store(data, actor + 0x9d, target if index == 8 else 0)
                            data[actor + 0x94] = int(index == 8)
                        else:
                            store(data, actor + 0x97, 0)
                            base = actor + (0xac if kind == 2 else 0xad)
                            for station in range(COUNTS[kind]):
                                store(data, base + station * 2, 1)
                            data[actor + 0x91] = data[actor + 0xa5] = 255
                        machine.mem_write(DGROUP, bytes(data))
                        _, effect = self.check(machine, actor)
                        if index in (6, 8):
                            self.assertEqual(effect['visibility_returns'], 1)
                        self.assertEqual(effect['audio_returns'], int(gate == 65535))
                        self.counts['target_device_context_returns'] += 1
        for kind in (1, 3):
            for index in (5, 10, 14):
                for enabled, missing in ((0, False), (1, False), (1, True), (0, True)):
                    machine, actor = self.fixture(kind, automatic=1, index=index, mode=0,
                                                  flags=0, target=True, source=True)
                    data = bytearray(machine.mem_read(DGROUP, 65536))
                    target = word(data, actor + 0x97)
                    data[target + 22] = 16 | 8
                    data[actor + 0xb5:actor + 0xb9] = bytes((1, 1, 4, 4))
                    for stream in range(4):
                        store(data, 0x1f84 + stream * 2, seed_for_draw(0))
                    machine.mem_write(DGROUP, bytes(data))
                    _, effect = self.check(machine, actor, enabled=enabled, missing_effects=missing)
                    self.assertEqual(effect['audio_returns'], 1)
                    self.counts['target_device_context_returns'] += 1
        self.assertEqual(self.counts['target_device_context_returns'], 88)

    def test_all47_actual_prepared_worlds_all32_parent_entries(self):
        missions, kinds, terrains = {}, set(), set()
        corpus = Corpus()
        for world in corpus.worlds():
            self.owner.install_height(world.side, world.pixels)
            machine = self.owner.prepared_machine(world.data)
            base = bytes(machine.mem_read(DGROUP, 65536))
            missions.setdefault(world.name, set()).add(world.side)
            terrains.add(world.terrain)
            for actor in world.ground_actors().values():
                kinds.add(word(base, actor))
                for automatic in (0, 1):
                    for index in range(16):
                        data = bytearray(base)
                        data[0x978a] = 0
                        data[actor + 0x42] = (index - 1) % 256
                        store(data, actor + 0x40, (word(data, actor + 0x40) & 65534) | automatic)
                        store(data, 0x7ae0, actor)
                        store(data, 0x6da2, 0)
                        store(data, 0x9fdf, 0)
                        machine.mem_write(DGROUP, bytes(data))
                        self.check(machine, actor)
                        self.counts['canonical_parent_returns'] += 1
            self.counts['prepared_worlds'] += 1
            print(f'Complete parent corpus: {world.name}/{world.side}', flush=True)
        self.assertEqual((len(missions), kinds, len(terrains)), (47, {0, 1, 2, 3}, 8))
        self.assertTrue(all(value == {512, 1024, 2048, 4096} for value in missions.values()))
        self.assertEqual(self.counts['canonical_parent_returns'], 122880)
        self.assertEqual(self.counts['prepared_worlds'], 188)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    REVIEW = args.review_dir
    REVIEW.mkdir(parents=True, exist_ok=True)
    destination = REVIEW / 'complete-parent.json'
    destination.unlink(missing_ok=True)
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(GroundParentTests)
    result = unittest.TextTestRunner(verbosity=1).run(suite)
    counts = dict(GroundParentTests.counts)
    required = {'counter_domain_returns': 16384, 'heading_word_returns': 262144,
                'inhibited_returns': 4080, 'retained_sequence_returns': 4096,
                'conditional_rng_returns': 56, 'support_device_context_returns': 64,
                'canonical_parent_returns': 122880, 'prepared_worlds': 188,
                'target_device_context_returns': 88}
    success = result.wasSuccessful() and not result.skipped and result.testsRun == 7
    success = success and all(counts.get(name) == count for name, count in required.items())
    success = success and set(GroundParentTests.callbacks) == {
        f'{bank}_{index}' for bank in ('automatic', 'controlled') for index in range(16)}
    if success:
        evidence = {'success': True, 'groups': result.testsRun, 'skips': 0, 'counts': counts,
                    'callbacks': dict(GroundParentTests.callbacks),
                    'group_complete_returns': GroundParentTests.groups,
                    'output_sha256': GroundParentTests.digest.hexdigest(),
                    'original_image_sha256': IMAGE_SHA256, 'seconds': time.monotonic() - GroundParentTests.begin,
                    'scope': 'Complete actual ab03 composition, independent prefix/diagnostic and previously '
                             'proved complete child boundaries; actual PM height/visibility/audio returns. '
                             'Shared C, full living class, battle, PCM playback and final game gate remain open.',
                    'complete_game_wasm_streak': 0}
        destination.write_text(json.dumps(evidence, indent=2) + '\n')
        print(json.dumps(evidence, sort_keys=True), flush=True)
    raise SystemExit(0 if success else 1)
