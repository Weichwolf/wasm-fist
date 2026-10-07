#!/usr/bin/env python3
"""Complete original planar bearing/distance rules and saved actor/goal geometry."""
import argparse
import hashlib
import json
import math
import pathlib
import random
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = ORACLE = None
ORIGINALS = False
BATCH = 4096
MASK = 4294967295
KNOTS = tuple(round(math.atan(index / 256) * 65536 * 8 / math.tau) % 65536
              for index in range(257))


def signed(value):
    return (value + 2147483648) % 4294967296 - 2147483648


def squared(delta):
    magnitude = delta if delta < 2147483648 else (-delta) & MASK
    if magnitude >= 16777216:
        return (magnitude >> 16) ** 2 << 32
    if magnitude >= 65536:
        return (magnitude >> 8) ** 2 << 16
    return magnitude ** 2


def measure(case):
    source, target, coarse = case
    x, y = ((b - a) & MASK for a, b in zip(source, target))
    square = squared(x) + squared(y)
    scale = 0 if square < 2**32 else 4 if square < 2**40 else 8 if square < 2**48 else 16
    distance = math.isqrt(square >> (2 * scale)) << scale
    sector = 0
    if x >= 2147483648:
        x, y, sector = (-x) & MASK, (-y) & MASK, 4
    if y >= 2147483648:
        x, y, sector = (-y) & MASK, x, sector + 2
    while x >= y:
        x -= y
        y = (2 * y + x) & MASK
        sector += 1
        if not y:
            return 0, distance
    shift = 32 - y.bit_length()
    x, y = x << shift, y << shift
    denominator = y >> 16
    ratio = min(x, (denominator << 16) - 1) // denominator
    index, fraction = divmod(ratio, 256)
    angle = KNOTS[index]
    if not coarse:
        angle += (((KNOTS[index + 1] - angle) % 65536) * fraction + 128) // 256
    return ((sector * 65536 + angle + 4) // 8) % 65536, distance


def octants(x, y):
    return ((x, y), (y, x), (y, -x), (x, -y),
            (-x, -y), (-y, -x), (-y, x), (-x, y))


def encode(cases):
    return struct.pack('<I', len(cases)) + b''.join(
        struct.pack('<4iB', *source, *target, coarse) for source, target, coarse in cases)


class GeometryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-geometry-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'cases.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_geometry_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_geometry_probe.js')])
        cls.fixtures = cls.saved = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Planar geometry per target: {cls.fixtures} complete returns, '
              f'{cls.saved} saved ground actor/goal pairs', flush=True)

    def run_data(self, data, expected='', valid=True):
        self.path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, int(not valid), result.stderr)
            self.assertEqual(result.stdout, expected)

    def run_cases(self, cases):
        outcomes = [measure(case) for case in cases]
        if ORACLE is not None:
            self.assertEqual(ORACLE.cases(cases), outcomes)
        expected = ''.join(f'geometry {heading} {distance}\n' for heading, distance in outcomes)
        self.run_data(encode(cases), expected)
        self.__class__.fixtures += len(cases)

    def batches(self, cases):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == BATCH:
                self.run_cases(batch)
                batch = []
        if batch:
            self.run_cases(batch)

    def test_every_ratio_word_fine_coarse_and_octant_edges(self):
        # y=65536 normalizes to 0x80000000; x then gives precisely every u16 quotient.
        self.batches(((0, 0), (ratio, 65536), coarse)
                     for ratio in range(65536) for coarse in (0, 1))
        self.batches(((0, 0), target, coarse)
                     for ratio in range(0, 65536, 64) for target in octants(ratio, 65536)
                     for coarse in (0, 1))

    def test_every_low_word_magnitude_axis_and_diagonal(self):
        self.batches(((0, 0), (value, y), value % 256)
                     for value in range(65536) for y in (0, value))

    def test_coordinate_width_precision_boundaries_and_zero_extremes(self):
        edges = sorted({signed(sign * ((1 << bit) + offset))
                        for bit in range(32) for offset in (-1, 0, 1)
                        for sign in (-1, 1)} | {0, -2147483648, 2147483647})
        self.batches(((0, 0), (x, y), coarse)
                     for x in edges for y in edges for coarse in (0, 1))
        self.batches((source, target, coarse)
                     for source in ((-2147483648, 2147483647), (2147483647, -2147483648),
                                    (-65536, 65536), (-1, -1))
                     for x in (0, 1, 32767, 32768, 65535) for target in octants(x, 65536)
                     for coarse in range(256))
        # The original sum fold, not a mathematical atan2 replacement, owns these returns.
        self.assertEqual(measure(((0, 0), (-2147483648, 0), 0))[0], 0)
        self.assertEqual(measure(((0, 0), (-2147483648, -2147483648), 0))[0], 0)

    def test_deterministic_full_dword_coordinate_spread(self):
        generator = random.Random(86)
        self.batches((tuple(generator.randrange(-2**31, 2**31) for _ in range(2)),
                      tuple(generator.randrange(-2**31, 2**31) for _ in range(2)),
                      generator.randrange(256)) for _ in range(10000))

    def test_complete_malformed_batches_and_missing_inputs(self):
        good = encode([((0, 0), (-1, 2147483647), 255)])
        for length in range(len(good)):
            self.run_data(good[:length], valid=False)
        for data in (good + b'x', struct.pack('<I', 0) + good[4:],
                     struct.pack('<I', 2) + good[4:], struct.pack('<I', 4294967295),
                     struct.pack('<I', 2) + good[4:] + good[4:-1]):
            self.run_data(data, valid=False)
        self.run_cases([])
        for command in self.commands:
            for args in ([], [str(self.path.parent / 'missing.bin')], [str(self.path), 'extra']):
                result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertEqual(result.stdout, '')

    def test_original_complete_table_and_declared_numeric_scratch(self):
        if ORACLE is None:
            self.skipTest('Pinned complete original instruction oracle requested separately')
        self.assertEqual(ORACLE.angle_table, KNOTS)
        self.run_cases([((0, 0), target, coarse)
                        for target in ((0, 0), (-2147483648, 0), (-1, -2147483648),
                                       (-2147483648, -2147483648), (2147483647, 2147483647))
                        for coarse in (0, 1, 255)])

    def test_all_pinned_original_ground_actor_goal_pairs(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        count = 0
        for name, info in manifest.items():
            path = directory / name
            data = path.read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            cases = []
            for _, _, raw in records_from_scenario(data):
                if int.from_bytes(raw[:2], 'little') >= 4:
                    continue
                source = struct.unpack_from('<2i', raw, 4)
                target = struct.unpack_from('<2i', raw, 0x49)
                cases.extend((source, target, coarse) for coarse in (0, 1))
                count += 1
            self.run_cases(cases)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual(count, 960)
        self.__class__.saved = count


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS = args.build_root, args.target, args.native_probe, args.originals
    if args.oracle:
        from original_geometry_oracle import OriginalGeometryOracle
        ORACLE = OriginalGeometryOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
