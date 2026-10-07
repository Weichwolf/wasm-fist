#!/usr/bin/env python3
"""Canonical controlled-world ownership and complete timed state traces on both targets."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

import test_driving as driving
from prepare_terrain_preview import synthetic_inputs
from test_ground import installed
from test_heightfield import resize
from test_mission_world import install, record, world_lines
from test_terrain_scene import fixture_models
from test_units import records_from_scenario, scenario_data
from test_vehicle_motion import start

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = False
SEEDS = (1, 2, 32768, 65535)
INTERVALS = [(0, 521), (166670, 33), (166670, 8210), (166670, 64), (166670, 0),
             (0, 128), (1000000, 256), (0, 384), (0, 0), (166670, 16384), (166670, 0)]


def expected(records, intervals, side, pixels, seeds=SEEDS, cursor=0):
    status, world = install(records, seeds, cursor)
    if status:
        return status, None
    pool, objects, roster, _, _ = world
    selected = roster[0]
    if selected == 65535 or objects[selected][0][0] >= 4:
        return -1, None
    if ORACLE:
        from original_mission_world_oracle import OriginalMissionWorldOracle
        observed = OriginalMissionWorldOracle().install(records, seeds, cursor, 0, ())
        wanted = 'status 0\n' + world_lines(world) * 2
        if observed != wanted:
            raise AssertionError('Original complete world installation differs')
    allocation, raw = objects[selected]
    # The delivered manual stage has no targeting owner. Supply its explicit
    # untargeted original-oracle boundary, as in the standalone driving gate.
    raw = bytearray(raw)
    struct.pack_into('<H', raw, 0x97, 0)
    controlled = allocation[2], allocation[3], driving.take_control(raw)
    if ORACLE:
        from original_driver_oracle import OriginalDriverOracle
        oracle = OriginalDriverOracle(side, pixels)
        if oracle.control((allocation[2], allocation[3], bytes(raw))) != controlled:
            raise AssertionError('Original complete initial take-control differs')
        state = oracle.ground.contact(oracle.field, [controlled])[0]
        if state != installed(side, pixels, [controlled])[0]:
            raise AssertionError('Original initial ground contact differs')
    else:
        state = installed(side, pixels, [controlled])[0]
    def extra(updated):
        objects[selected] = allocation, bytearray(updated[2])
        return f'selection {selected} 65535\n' + world_lines(world)
    return 0, driving.trace(state, intervals, side, pixels, extra_state=extra)


class MissionDrivingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-mission-driving-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name)
        synthetic_inputs(cls.path, flat=True)
        fixture_models(cls.path)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_driving_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_driving_probe.js')])
        cls.fixtures = cls.boundaries = cls.objects = cls.rejections = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Canonical driving per target: {cls.fixtures} fixtures, {cls.boundaries} complete '
              f'boundaries, {cls.objects} installed objects, {cls.rejections} explicit rejections', flush=True)

    def run_scene(self, records=None, *, scenario=None, directory=None, intervals=INTERVALS,
                  seeds=SEEDS, cursor=0, side=512, pixels=None, status_override=None):
        if scenario is None:
            scenario = self.path / 'WORLD.FSG'
            scenario.write_bytes(scenario_data(records))
        else:
            records = records_from_scenario(scenario.read_bytes())
        if status_override is not None:
            status, wanted = status_override, None
        else:
            status, wanted = expected(records, intervals, side, pixels or bytes([32]) * side * side, seeds, cursor)
        request = self.path / 'intervals.bin'
        request.write_bytes(struct.pack('<II4HB', side, len(intervals), *seeds, cursor) + b''.join(
            struct.pack('<IH', *interval) for interval in intervals))
        wanted = f'load {status}\n' if status else wanted
        wanted_bytes = wanted.encode()
        for command in self.commands:
            result = subprocess.run([*command, str(scenario), str(directory or self.path), str(request), 'mission'],
                                    capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, int(status != 0), result.stderr)
            actual_bytes = result.stdout.encode()
            if (len(actual_bytes), hashlib.sha256(actual_bytes).digest()) != (
                    len(wanted_bytes), hashlib.sha256(wanted_bytes).digest()):
                difference = next(((i, a, b) for i, (a, b) in enumerate(itertools.zip_longest(
                    result.stdout.splitlines(), wanted.splitlines())) if a != b), (None, None, None))
                self.fail(f'Complete canonical driving trace differs ({len(actual_bytes)} versus {len(wanted_bytes)} bytes) at line {difference[0]}: '
                          f'actual={difference[1]!r} expected={difference[2]!r}')
        type(self).fixtures += 1
        type(self).boundaries += len(intervals) + 1 if status == 0 else 0
        type(self).objects += len(records) if status == 0 else 0
        type(self).rejections += status != 0
        return wanted

    @staticmethod
    def player(kind=0, index=9, member=0):
        raw = bytearray(start(kind, x=128 * 256, y=1024 * 256))
        raw[0x91] = raw[0xa5] = raw[0x3c] = 0
        raw[22] |= 32
        raw[26], raw[27] = 0, member
        raw[0xa9:0xac] = b'\x80\0\0'
        return index, 12, bytes(raw)

    def test_ordered_rng_all_ground_classes_and_complete_nonplayer_preservation(self):
        for kind in range(4):
            for cursor in range(4):
                # Selected player is not the first physical ground object.
                records = [record(21, 0), self.player((kind + 1) % 4, 7, 1), self.player(kind),
                           record(5, 20), record(6, 21), record(17, 22), record(23, 23),
                           record(26, 24), record(27, 25), self.player((kind + 2) % 4, 40, 2)]
                output = self.run_scene(records, cursor=cursor)
                self.assertTrue('selection 151 65535\n' in output[:2000], 'Canonical player must occupy physical slot 151')
                self.assertTrue('advance -1\n' in output, 'Invalid keys must be rejected')

    def test_initialized_movement_maintenance_and_paused_cadence(self):
        intervals = [(0, 1), (250000, 0), (250000, 0), (0, 128), (1000000, 0)]
        for kind, speed, counter in itertools.product(range(4), (32, 64), (5, 250)):
            identity, generation, original = self.player(kind)
            raw = bytearray(original)
            # A participating roster actor starts with the original constructor's
            # movement word. Saved speed/counter/phase still reach maintenance;
            # paused intervals must preserve the complete canonical world.
            raw[0x3d], raw[0x5f] = 32, counter
            struct.pack_into('<hh', raw, 0x55, speed, 128 if speed == 32 else 254)
            struct.pack_into('<H', raw, 0x5d, 1)
            output = self.run_scene([(identity, generation, bytes(raw))], intervals=intervals)
            states = [line.split() for line in output.splitlines() if line.startswith('state ')]
            self.assertEqual(int(states[0][14]), 65535, 'The participating constructor resets the saved movement word')
            self.assertEqual(int(states[-1][14]), 65531 if speed == 32 else 65529,
                             'Both admitted maintenance calls must consume movement')
            self.assertEqual(states[-1], states[-3], 'Paused time must preserve the complete selected actor')
            self.assertIn('clock 30 ', output, 'Paused time must not consume more maintenance calls')
            counters = [int(line.split()[1]) for line in output.splitlines() if line.startswith('speed_counter ')]
            self.assertEqual(counters[-1], counter - 2 if speed == 32 else counter + (2 if counter < 248 else 0))

    def test_selected_physical_orphan_survives_overwritten_registry(self):
        records = [self.player(2, 7, 1), self.player(0), self.player(1, 9, 2), record(21, 30)]
        output = self.run_scene(records)
        self.assertTrue('selection 151 65535\n' in output[:2000], 'The old physical roster entry remains selected')

    def test_pause_partitions_conflicts_and_weapon_edges(self):
        for kind in range(4):
            records = [self.player(kind), self.player((kind + 1) % 4, 10, 1), record(21, 0)]
            for intervals in ([(0, 1), (1234567, 1)], [(0, 1)] + [(12345, 1)] * 100 + [(67, 1)],
                              [(0, 63), (100000, 8192), (0, 8192), (100000, 4096), (5000000, 0)],
                              [(0, 1), (33334, 129), (1000000, 129), (2000000, 1),
                               (0xffffffff, 129), (33334, 129), (0, 0), (33334, 0)]):
                self.run_scene(records, intervals=intervals, seeds=(0, 0, 0, 0))

    def test_whole_unsupported_empty_absent_player_and_source_failure(self):
        self.run_scene([self.player(), record(7, 0)], intervals=[])
        self.run_scene([], intervals=[])
        self.run_scene([record(21, 0)], intervals=[])
        # Use a completely absent asset directory after a valid full world load.
        self.run_scene([self.player()], intervals=[], directory=self.path / 'missing', status_override=-1)
        self.run_scene([self.player()], intervals=[], cursor=4, status_override=-1)
        for side in (0, 3, 768, 0xffffffff):
            # Invalid detail must fail after allocation without leaking or publishing.
            self.run_scene([self.player()], intervals=[], side=side, pixels=b' ', status_override=-1)

    def test_all_pinned_worlds_complete_supported_or_explicit_rejection(self):
        if not ORIGINALS:
            self.skipTest('Full pinned original corpus is an explicit additional gate')
        from original_asset_oracle import OriginalAssetOracle
        decoder = OriginalAssetOracle()
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        maps = {}
        supported = rejected = objects = 0
        for name, info in manifest.items():
            scenario = ROOT / 'armoredfist/FISTDATA' / name
            data = scenario.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            status, _ = install(records, SEEDS, 0)
            if status:
                self.assertEqual(status, 2)
                rejected += 1
                self.run_scene(scenario=scenario, directory=scenario.parent, intervals=[])
                continue
            supported += 1
            objects += len(records)
            probe = subprocess.run([str(BUILD / 'native/fist_scenario_probe'), str(scenario)],
                                   check=True, capture_output=True, text=True)
            height = next(line.split(' ', 2)[2] for line in probe.stdout.splitlines()
                          if line.startswith('asset ') and line.endswith('.KLC') and ' D' in line)
            if height not in maps:
                decoded = decoder.klc((scenario.parent / height).read_bytes())
                dimensions, body = decoded.split(b'\n', 1)
                width, _ = map(int, dimensions.split())
                maps[height] = resize(width, body[768:], [2048])[1]
            output = self.run_scene(scenario=scenario, directory=scenario.parent, side=2048, pixels=maps[height])
            if name == 'TRAIN1.FSG':
                self.assertEqual(len(records), 85)
                self.assertTrue('selection 151 65535\n' in output[:2000], 'Canonical player must occupy physical slot 151')
        self.assertEqual((supported, objects, rejected), (10, 671, 37))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS, ORACLE = args.build_root, args.target, args.native_probe, args.originals, args.oracle
    driving.ORACLE = ORACLE
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS))
