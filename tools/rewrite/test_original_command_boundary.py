#!/usr/bin/env python3
"""Required original mission-order I/O and reached complete living-prefix evidence.

This verifies original-input boundaries, not a native/WASM command implementation.
It intentionally lives outside production build gates and requires the complete
local original corpus and pinned optional Unicorn dependency; missing inputs fail.
"""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from original_controlled_m1_oracle import OriginalControlledM1Oracle
from original_mission_orders_oracle import (DESCRIPTOR_RECORD, PATH_HEADER, PATH_RECORD,
                                           PLATOONS, WAYPOINTS, OriginalMissionOrdersOracle)
from original_unit_oracle import DGROUP
from original_weapon_control_oracle import DISPATCH
from test_driving import driver_step
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[2]
REVIEW = None


def chunks(data):
    output = {}
    cursor = 0
    while cursor < len(data):
        if cursor + 6 > len(data):
            raise ValueError('Incomplete original chunk header')
        tag, size = struct.unpack_from('<4sH', data, cursor)
        end = cursor + 6 + size
        if end > len(data) or tag in output:
            raise ValueError('Incomplete or duplicate original chunk')
        output[tag] = data[cursor + 6:end]
        cursor = end
    if output.get(b'TERM') != b'':
        raise ValueError('Missing complete original termination')
    return output


class CommandBoundaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalMissionOrdersOracle()
        cls.manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        cls.evidence = {'scope': 'original order input and actual prefix only; no C command acceptance',
                        'corpus': {}, 'complete_wasm_streak': 0}
        cls.loads = cls.bytes = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Original order boundary: {cls.loads} complete chunk returns, {cls.bytes} input bytes; '
              'whole DGROUP and RNG preserved outside declared destinations', flush=True)
        if REVIEW:
            REVIEW.mkdir(parents=True, exist_ok=True)
            (REVIEW / 'command-boundary.json').write_text(json.dumps(cls.evidence, indent=2) + '\n')

    def original(self, name):
        path = ROOT / 'armoredfist/FISTDATA' / name
        data = path.read_bytes()
        info = self.manifest[name]
        self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
        return path, data, chunks(data)

    def test_all_47_complete_order_blocks_and_loader_admission(self):
        self.assertEqual(sorted(path.name for path in (ROOT / 'armoredfist/FISTDATA').glob('*.FSG')),
                         sorted(self.manifest))
        descriptors = [[] for _ in range(DESCRIPTOR_RECORD // 2)]
        counts = []
        for name in self.manifest:
            path, data, blocks = self.original(name)
            paths, info = blocks[b'PATH'], blocks[b'PINF']
            self.assertEqual((len(paths), len(info)), (PLATOONS * PATH_RECORD, PLATOONS * DESCRIPTOR_RECORD))
            for mode in (1, 0, 2, 255):
                machine = self.owner.machine((1, 2, 32768, 65535), 3)
                random_before = self.owner.random_state(machine)
                before = self.owner.blocks(machine)
                requests = self.owner.load(machine, paths, info, mode=mode)
                self.assertEqual(self.owner.blocks(machine), (paths, info) if mode == 1 else before)
                self.assertEqual(self.owner.random_state(machine), random_before)
                self.assertEqual([entry[1] for entry in requests], ['read' if mode == 1 else 'seek'] * 2)
                type(self).loads += 2
                type(self).bytes += len(paths) + len(info) if mode == 1 else 0
            for platoon in range(PLATOONS):
                route = paths[platoon * PATH_RECORD:(platoon + 1) * PATH_RECORD]
                self.assertLessEqual(route[0], WAYPOINTS)
                counts.append(route[0])
                for word, value in enumerate(struct.unpack_from('<11H', info, platoon * DESCRIPTOR_RECORD)):
                    descriptors[word].append(value)
            self.evidence['corpus'][name] = {'sha256': hashlib.sha256(data).hexdigest(),
                                            'paths_bytes': len(paths), 'descriptor_bytes': len(info)}
            self.assertEqual(path.read_bytes(), data)
        self.assertEqual(len(self.manifest), 47)
        self.assertEqual((min(counts), max(counts), len(counts)), (0, 21, 376))
        self.evidence['descriptor_word_values'] = [sorted(set(values)) for values in descriptors]
        self.evidence['route_count_range'] = [min(counts), max(counts)]

    def test_actual_all_class_banks_and_nested_callbacks(self):
        image = self.owner.image
        for _, _, _, table in DISPATCH:
            words = struct.unpack_from('<16H', image, table)
            self.assertEqual([2 * index for index, entry in enumerate(words) if entry == 0xab03], [14, 30])
        automatic = struct.unpack_from('<16H', image, DGROUP + 0x98dc)
        controlled = struct.unpack_from('<16H', image, DGROUP + 0x98fc)
        self.assertEqual(automatic, (0xab82, 0xac75, 0xab88, 0xad2f, 0xad08, 0xaf97, 0xb011,
                                    0xae66, 0xae32, 0xae5c, 0xafa2, 0xb017, 0xb0be, 0xb053, 0xaf97, 0xae66))
        self.assertEqual(controlled, (0xab82, 0xac75, 0xab88, 0xad3b, 0xad08, 0xb111, 0xb011,
                                     0xb111, 0xb111, 0xb111, 0xb111, 0xb111, 0xb111, 0xb053, 0xb111, 0xb111))
        # These entries are actual single RET instructions; unknown callbacks
        # are never changed into returns in order to recover the prefix.
        self.assertEqual(image[0xb111], 0xc3)
        self.assertEqual(image[0xab82:0xab87], bytes.fromhex('9aefb4690f'))
        self.assertEqual(image[0xb011:0xb016], bytes.fromhex('9a78b3690f'))
        self.evidence['phase_banks'] = {'automatic': list(automatic), 'controlled': list(controlled),
                                       'class_bank_indices': [14, 30]}

    def test_train1_actual_orders_reach_goal_and_direction_consumers(self):
        _, data, blocks = self.original('TRAIN1.FSG')
        records = records_from_scenario(data)
        pixels = bytes([32]) * (512 * 512)
        observations = []
        # The real TRAIN1 snapshot already contains its first goal and bit 2.
        # Construct identical stale goal/flag inputs to make the actual loaded
        # PATH consumer observable; this is not an untouched saved-player claim.
        patches = ((0x49, bytes(8)), (0x40, bytes(2)))
        with OriginalControlledM1Oracle(records, 512, pixels, patches=patches) as loaded, \
                OriginalControlledM1Oracle(records, 512, pixels, patches=patches) as empty:
            requests = self.owner.load(loaded.machine, blocks[b'PATH'], blocks[b'PINF'])
            self.assertEqual((loaded.state, loaded.random_state()), (empty.state, empty.random_state()))
            self.assertEqual((len(loaded.objects), loaded.selected), (85, 151))
            first_goal = struct.unpack_from('<ii', blocks[b'PATH'], PATH_HEADER)
            for tick in range(1, 24):
                prior = loaded.state
                actual = loaded.step()
                absent = empty.step()
                if tick <= 6:
                    self.assertEqual(actual, driver_step(prior, 0, 512, pixels)[0])
                if tick == 15:
                    self.assertEqual(struct.unpack_from('<ii', actual[2], 0x49), first_goal)
                    self.assertEqual(struct.unpack_from('<H', actual[2], 0x40)[0] & 2, 2)
                    self.assertEqual(struct.unpack_from('<H', absent[2], 0x40)[0] & 2, 0)
                if tick in (7, 15, 23):
                    observations.append({'tick': tick, 'phase': actual[2][0x3d],
                                         'actor': actual[2].hex(), 'empty_orders_actor': absent[2].hex(),
                                         'rng': loaded.random_state(),
                                         'differences_from_empty_orders': [offset for offset, (a, b)
                                                                          in enumerate(zip(actual[2], absent[2])) if a != b]})
                self.assertTrue(loaded.unchanged_others())
            self.assertEqual(loaded.random_state(), ([0, 0, 0, 0], 1))
            self.assertNotEqual(loaded.state[2][0x53:0x55], empty.state[2][0x53:0x55],
                                'The complete direction callback must consume the installed goal')
        self.evidence['train1'] = {'prefix': '7c1d..7c7e before engine PCM', 'requests': requests,
                                  'objects': 85, 'selected': 151,
                                  'constructed_actor_boundary': 'raw +40 = 0 and goal XY +49..50 = 0',
                                  'observations': observations}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True,
                        help='Require all pinned original inputs; no skipped corpus or C acceptance claim')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Temporary review evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or bool(program.result.skipped))
