#!/usr/bin/env python3
"""Complete terrain visibility: original returns, occlusion and saved-pose corpus."""
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

from test_ground import varied_plane
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = ORACLE = None
ORIGINALS = False
MASK = 2**32 - 1


def signed(value):
    return (value + 2**31) % 2**32 - 2**31


def trace(source, target, side):
    dx, dy = (signed(b - a) for a, b in zip(source[:2], target[:2]))
    if not (-262144 <= dx < 262144 and -262144 <= dy < 262144):
        return None
    delta = (signed(dx * 8192), signed(-dy * 8192), signed((target[2] - source[2]) * 65536))
    divisions = 2
    while any(abs(value // divisions) >= 50331648 for value in delta[:2]):
        divisions *= 2
    step = tuple(value // divisions for value in delta)
    x, y, z = source[0] * 8192, -source[1] * 8192, source[2] * 65536
    cell = 2**32 // side
    points = []
    for _ in range(divisions - 1):
        x, y, z = ((value + amount) & MASK for value, amount in zip((x, y, z), step))
        points.append(((y // cell) * side + x // cell, z))
    return points


def visible(side, pixels, case):
    points = trace(*case, side)
    return points is not None and all(pixels[index] * 2**24 < altitude for index, altitude in points)


def encode(side, pixels, cases):
    return struct.pack('<II', side, len(cases)) + pixels + b''.join(
        struct.pack('<6i', *source, *target) for source, target in cases)


class VisibilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-visibility-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'cases.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_visibility_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_visibility_probe.js')])
        cls.fixtures = cls.saved_pairs = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Terrain visibility per target: {cls.fixtures} complete returns, '
              f'{cls.saved_pairs} saved opposing-side pairs across four installed details', flush=True)

    def run_data(self, data, expected='', valid=True):
        self.path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, int(not valid), result.stderr)
            self.assertEqual(result.stdout, expected)

    def run_cases(self, side, pixels, cases):
        cases = list(cases)
        expected = [visible(side, pixels, case) for case in cases]
        if ORACLE is not None:
            machine = ORACLE.prepare(side, pixels)
            self.assertEqual(ORACLE.visible(machine, cases), expected)
        self.run_data(encode(side, pixels, cases), ''.join(f'visibility {int(value)}\n' for value in expected))
        self.__class__.fixtures += len(cases)

    def test_all_terrain_height_bytes_all_integer_aim_heights_and_unsigned_fraction_edges(self):
        cases = []
        for terrain in range(256):
            # One complete 16x16 field and batch reaches every height byte
            # without launching a separate native/Node process for each byte.
            x, y = (terrain % 16) * 32768 + 16384, -(terrain // 16) * 32768 - 16384
            cases.extend(((x, y, altitude * 256), (x, y, altitude * 256)) for altitude in range(256))
            cases.extend(((x, y, signed(altitude)), (x, y, signed(altitude)))
                         for altitude in (terrain * 256 - 1, terrain * 256 + 1, terrain * 256 + 255,
                                          -2**31, 2**31 - 1, -1, 65536, 65537))
        self.run_cases(16, bytes(range(256)), cases)
        self.assertFalse(visible(1, b'\x0a', ((0, 0, 2560), (0, 0, 2560))))
        self.assertTrue(visible(1, b'\x0a', ((0, 0, 2561), (0, 0, 2561))))

    def test_half_map_asymmetry_subdivision_thresholds_and_wrapped_dword_differences(self):
        edges = sorted({sign * (6144 * 2**bit + offset)
                        for bit in range(6) for sign in (-1, 1) for offset in (-1, 0, 1)} |
                       {-262145, -262144, -262143, -1, 0, 1, 262143, 262144, 262145})
        cases = [((0, 0, 32001), (x, y, 32001)) for x, y in itertools.product(edges, repeat=2)]
        for source in ((-2**31, 2**31 - 1, 1), (2**31 - 1, -2**31, 65535), (-1, -1, -1)):
            cases.extend((source, (signed(source[0] + x), signed(source[1] + y), source[2]))
                         for x, y in itertools.product(edges[::4], repeat=2))
        self.run_cases(1, b'\0', cases)
        self.assertTrue(visible(1, b'\0', ((0, 0, 1), (-262144, 0, 1))))
        self.assertFalse(visible(1, b'\0', ((0, 0, 1), (262144, 0, 1))))
        self.assertEqual(len(trace((0, 0, 1), (12287, 0, 1), 1)), 1)
        self.assertEqual(len(trace((0, 0, 1), (12288, 0, 1), 1)), 3)

    def test_intermediate_obstruction_endpoints_rising_falling_and_directional_rounding(self):
        # At detail 512, a 98304-unit ray has 31 distinct intermediate cells.
        source, target = (0, 0, 100 * 256 + 1), (98304, 0, 100 * 256 + 1)
        points = trace(source, target, 512)
        self.assertEqual(len(points), 31)
        self.assertEqual(len({index for index, _ in points}), 31)
        plane = bytearray(512**2)
        plane[0] = plane[96] = 255
        self.assertTrue(visible(512, plane, (source, target)))
        self.run_cases(512, bytes(plane), [(source, target), (target, source)])
        for index, _ in points:
            plane[index] = 101
            self.assertFalse(visible(512, plane, (source, target)))
            self.run_cases(512, bytes(plane), [(source, target), (target, source)])
            plane[index] = 0
        cases = [((19, -101, z), (x, y, end))
                 for z, end in itertools.product((1, 255, 256, 65535, -1, -65536), repeat=2)
                 for x, y in ((1, 0), (-1, 0), (12289, 1023), (-12289, -1023),
                              (524287, 524287), (-262144, 262143))]
        for side in (1, 2, 8, 512, 1024):
            self.run_cases(side, varied_plane(side), cases)

    def test_cell_seams_altitude_wrapping_and_deterministic_full_coordinate_spread(self):
        generator = random.Random(0x8030)
        cases = []
        for _ in range(4096):
            source = tuple(generator.randrange(-2**31, 2**31) for _ in range(3))
            target = (signed(source[0] + generator.randrange(-262144, 262144)),
                      signed(source[1] + generator.randrange(-262144, 262144)),
                      generator.randrange(-2**31, 2**31))
            cases.append((source, target))
        for side in (1, 2, 4, 16, 512, 1024, 2048, 4096):
            self.run_cases(side, varied_plane(side), cases)

    def test_null_invalid_atomic_contracts_complete_and_malformed_batches(self):
        for command in self.commands:
            result = subprocess.run([*command, 'contracts'], capture_output=True, text=True, timeout=30)
            self.assertEqual((result.returncode, result.stdout), (0, ''), result.stderr)
        good = encode(2, bytes(4), [((0, 0, 1), (1, 1, 1))])
        for length in range(len(good)):
            self.run_data(good[:length], valid=False)
        for data in (good + b'x', good[:4] + struct.pack('<I', 0) + good[8:],
                     good[:4] + struct.pack('<I', 2) + good[8:], struct.pack('<II', 2, MASK),
                     encode(3, bytes(9), [((0, 0, 1), (1, 1, 1))]),
                     struct.pack('<II', 0, 0), struct.pack('<II', MASK, 0)):
            self.run_data(data, valid=False)
        self.run_cases(1, b'\0', [])
        for command in self.commands:
            for args in ([], [str(self.path.parent / 'missing.bin')], [str(self.path), 'extra']):
                result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)

    def test_all_original_height_maps_and_saved_opposing_side_candidate_positions(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        directory = ROOT / 'armoredfist/FISTDATA'
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrain = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            offset, height_name = 0, None
            while offset < len(data):
                tag, length = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += length + 6
            self.assertIsNotNone(height_name)
            records = [raw for _, _, raw in records_from_scenario(data)]
            # Saved XYZ are explicitly effective service inputs for this test.
            # No runtime identity, readiness or class-specific aim-offset claim.
            pairs = [(struct.unpack_from('<3i', actor, 4), struct.unpack_from('<3i', candidate, 4))
                     for actor in records if int.from_bytes(actor[:2], 'little') < 4
                     for candidate in records if candidate[0x16] & 4 and
                     bool(actor[0x16] & 8) != bool(candidate[0x16] & 8)]
            grouped.setdefault(height_name, []).extend(pairs)
            self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual((len(grouped), sum(map(len, grouped.values()))), (8, 17491))
        for name, pairs in grouped.items():
            data = (directory / name).read_bytes()
            info = terrain[name]
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            decoded = decoder.klc(data)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed_side, expanded = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed_side, side)
                self.run_cases(side, expanded, pairs)
            self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(), info['sha256'])
        self.__class__.saved_pairs = 17491


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
        from original_visibility_oracle import OriginalVisibilityOracle
        ORACLE = OriginalVisibilityOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
