#!/usr/bin/env python3
"""Complete target proximity, signed direction and actual target-order difference."""
import argparse
import hashlib
import itertools
import json
import pathlib
import random
import struct
import subprocess
import tempfile
import unittest

from test_geometry import BUILD, ROOT, encode, measure, signed
from test_units import records_from_scenario

TARGET = 'all'
NATIVE_PROBE = ORACLE = None
ORIGINALS = False


def proximity(case):
    source, target, _ = case
    lanes = [value if value >= 0 else abs(value) - 1
             for value in (signed(b - a) for a, b in zip(source, target))]
    return max(lanes) + min(lanes) // 2


class ProximityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-proximity-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'cases.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_geometry_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_geometry_probe.js')])
        cls.fixtures = cls.pairs = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Target proximity per platform: {cls.fixtures} complete returns, '
              f'{cls.pairs} original opposing-side pairs in both directions', flush=True)

    def run_data(self, data, expected='', valid=True):
        self.path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, 'proximity', str(self.path)], capture_output=True,
                                    text=True, timeout=60)
            self.assertEqual((result.returncode, result.stdout), (int(not valid), expected), result.stderr)

    def run_cases(self, cases):
        cases = list(cases)
        outcomes = [proximity(case) for case in cases]
        if ORACLE is not None:
            self.assertEqual(ORACLE.cases(cases), outcomes)
        self.run_data(encode(cases), ''.join(f'proximity {value}\n' for value in outcomes))
        self.__class__.fixtures += len(cases)

    def batches(self, cases):
        cases = iter(cases)
        while batch := list(itertools.islice(cases, 4096)):
            self.run_cases(batch)

    def test_every_low_word_axis_diagonal_and_signed_orientation(self):
        self.batches(((0, 0), (x, y), 0) for value in range(65536)
                     for x, y in ((value, 0), (-value, 0), (value, value), (-value, -value),
                                  (value, -value), (-value, value)))
        self.assertEqual(proximity(((0, 0), (-1, -1), 0)), 0)
        self.assertEqual(proximity(((0, 0), (1, 1), 0)), 1)

    def test_wrapped_dword_boundaries_ties_and_target_range_packing(self):
        edges = sorted({signed(sign * (2**bit + delta)) for bit in range(32)
                        for sign in (-1, 1) for delta in (-1, 0, 1)} | {0})
        self.batches(((0, 0), (x, y), 0) for x, y in itertools.product(edges, repeat=2))
        self.batches((source, (signed(source[0] + x), signed(source[1] + y)), 255)
                     for source in ((-2**31, 2**31 - 1), (2**31 - 1, -2**31), (-1, -1))
                     for x, y in itertools.product((-2**31, -65537, -1, 0, 1, 65537, 2**31 - 1), repeat=2))
        self.run_cases([((0, 0), (value, 0), mode) for limit in (150, 450, 625, 1000)
                        for value in (limit * 256 - 1, limit * 256, limit * 256 + 255,
                                      limit * 256 + 256) for mode in range(256)])

    def test_deterministic_full_dword_coordinates(self):
        generator = random.Random(0x8e8)
        self.batches((tuple(generator.randrange(-2**31, 2**31) for _ in range(2)),
                      tuple(generator.randrange(-2**31, 2**31) for _ in range(2)),
                      generator.randrange(256)) for _ in range(10000))

    def test_actual_complete_scan_selects_other_target_than_navigation_distance(self):
        candidates, actor = ((-25600, -25600), (-37120, 0)), (0, 0)
        cases = [(candidate, actor, 0) for candidate in candidates]
        self.run_cases(cases)
        self.assertEqual([proximity(case) >> 8 for case in cases], [150, 145])
        self.assertEqual([measure(case)[1] >> 8 for case in cases], [141, 145])
        if ORACLE is None:
            self.skipTest('Complete unchanged original target scan requested separately')
        result = ORACLE.reaching_selection()
        self.assertEqual((result['selected_registry'], result['range'], result['winner_updates']), (2, 145, 2))
        self.assertEqual(len(result['transfers']), 2)
        self.assertEqual(result['complete_dgroup_bytes'], 65536)

    def test_complete_malformed_batches_and_cli(self):
        good = encode([((0, 0), (-1, 1), 255)])
        for length in range(len(good)):
            self.run_data(good[:length], valid=False)
        for data in (good + b'x', struct.pack('<I', 0) + good[4:],
                     struct.pack('<I', 2) + good[4:], struct.pack('<I', 2**32 - 1)):
            self.run_data(data, valid=False)
        self.run_cases([])
        for command in self.commands:
            for args in (['proximity'], ['proximity', str(self.path), 'extra'],
                         ['badmode', str(self.path)], ['proximity', str(self.path.parent / 'missing.bin')]):
                result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)

    def test_all_original_saved_opposing_side_pair_positions_both_directions(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        directory = ROOT / 'armoredfist/FISTDATA'
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            records = [raw for _, _, raw in records_from_scenario(data)]
            cases = []
            for actor in records:
                if int.from_bytes(actor[:2], 'little') >= 4:
                    continue
                for candidate in records:
                    if not candidate[22] & 4 or bool(actor[22] & 8) == bool(candidate[22] & 8):
                        continue
                    source, target = struct.unpack_from('<2i', candidate, 4), struct.unpack_from('<2i', actor, 4)
                    cases.extend(((source, target, 0), (target, source, 0)))
                    self.__class__.pairs += 1
            self.run_cases(cases)
            self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual(self.__class__.pairs, 17491)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    TARGET, BUILD, NATIVE_PROBE, ORIGINALS = args.target, args.build_root, args.native_probe, args.originals
    if args.oracle:
        from original_proximity_oracle import OriginalProximityOracle
        ORACLE = OriginalProximityOracle()
    program = unittest.main(argv=[__file__], exit=False)
    expected_skips = int(not ORIGINALS) + int(ORACLE is None)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != expected_skips)
