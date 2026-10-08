#!/usr/bin/env python3
"""Required complete b0be artillery admission with four actually allocated guns."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import time
import unittest

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_support_audio_oracle import OriginalSupportAudioOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_units import snapshot
from test_vehicle_motion import start


class SupportListTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.audio = OriginalAudioRequestOracle()
        cls.owner = OriginalSupportAudioOracle(cls.audio)
        cls.fixtures = {}
        cls.counts = collections.Counter()
        cls.branches = collections.Counter()
        cls.indices = set()
        cls.digest = hashlib.sha256()

    def fixture(self, kind, count, mask, amount, busy, used, selected, notice):
        prior = self.fixtures.get(kind)
        if prior is None or prior[3] == 256:
            actor = bytearray(start(kind))
            actor[22] = 0
            gun = bytearray(snapshot(27, flags=0))
            gun[25] = 0
            records = [(0, 1, bytes(actor)), (1, 2, snapshot(5, flags=0))]
            records.extend((i + 2, i + 3, bytes(gun)) for i in range(4))
            machine, objects = self.owner.prepare_saved(records, SEEDS, 0, 0, (bytes(2144), bytes(176)))
            baseline = bytes(machine.mem_read(0, 0x60000))
            if prior is not None:
                self.assertEqual((baseline, objects), (prior[1], prior[2]))
            self.fixtures[kind] = machine, baseline, objects, 0
        machine, baseline, objects, calls = self.fixtures[kind]
        self.fixtures[kind] = machine, baseline, objects, calls + 1
        machine.mem_write(0, baseline)
        data = bytearray(machine.mem_read(DGROUP, 65536))
        actor, target = objects[150][2], objects[0][2]
        guns = [objects[i + 1][2] for i in range(4)]
        data[actor + 22], data[actor + 0x43] = 8, 4
        store(data, actor + 0x97, target)
        store(data, actor + 0x99, 20)
        store(data, 0x6cde, 1800)
        store(data, 0x9794, 0)
        store(data, 0x9f19, 1320)
        store(data, 0x9ce5, busy)
        store(data, 0x9ccd, count)
        store(data, 0x6d34, actor if selected else 0)
        store(data, 0x7ae0, 0)
        store(data, 0xea2c, 0)
        store(data, 0xea2e, 0x4000)
        data[0x6ce6] = notice
        for index, gun in enumerate(guns):
            store(data, 0x9cd7 + index * 2, gun)
            store(data, gun + 0x1f, amount if mask & (1 << index) else 0)
        for index in range(16):
            entry = 0x9dc7 + index * 14
            struct.pack_into('<7H', data, entry, 1 if index < used else 0, 31, 32, 33, 34, 35, 36)
        # AH=65 bypasses smoke, while AL=32 admits actual artillery. One real
        # canonical draw is still required; no guessed return replaces a call.
        struct.pack_into('<5H', data, 0x1f82, 0x1f8a, seed_for_draw(0x4120), *SEEDS[1:])
        machine.mem_write(DGROUP, bytes(data))
        return machine, actor, guns

    def test_complete_authored_counts_first_available_gun_and_all_queue_exits(self):
        for kind, count, mask, amount, busy, used, selected, notice in itertools.product(
                range(4), range(5), range(16), (1, 65535), (0, 1), range(17),
                (False, True), (0, 2, 255)):
            machine, actor, guns = self.fixture(kind, count, mask, amount, busy, used, selected, notice)
            before = bytes(machine.mem_read(DGROUP, 65536))
            after, effect = self.owner.observe(machine, actor, self.audio.fixture())
            self.assertEqual(effect['smoke'], 'not_called')
            self.assertFalse(effect['audio'])
            chosen = next((i for i in range(count) if mask & (1 << i)), None)
            for index, gun in enumerate(guns):
                self.assertEqual(word(after, gun + 0x1f), (word(before, gun + 0x1f) - int(index == chosen)) & 65535)
            if chosen is not None:
                self.indices.add(chosen)
                self.assertEqual(word(after, 0x9f19 + chosen // 2), 1800)
            self.digest.update(after[:0x8fc0])
            self.digest.update(after[0x9000:])
            self.counts['complete_b0be_returns'] += 1
            self.branches[effect['support']] += 1
        self.assertEqual(self.counts['complete_b0be_returns'], 130560)
        self.assertEqual(self.indices, set(range(4)))
        self.assertEqual(set(self.branches), {'artillery_not_in_place', 'artillery_empty',
            'artillery_busy', 'artillery_confirmed', 'artillery_queue_full'})
        self.audio.verify_assets()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'support-list.json'
    output.unlink(missing_ok=True)
    begin = time.monotonic()
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 1 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 1, 'skips': 0, 'seconds': time.monotonic() - begin,
            'counts': dict(SupportListTests.counts), 'branches': dict(SupportListTests.branches),
            'first_available_gun_indices': sorted(SupportListTests.indices),
            'original_image_sha256': IMAGE_SHA256, 'output_sha256': SupportListTests.digest.hexdigest(),
            'scope': 'Complete b0be ordered four-gun resource consumption/queue exits; later support dispatch/flight/shared C remain open',
            'complete_wasm_streak': 0}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result, sort_keys=True), flush=True)
    raise SystemExit(not success)
