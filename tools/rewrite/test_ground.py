#!/usr/bin/env python3
"""Shared ground samples/contact: full state, all headings, original installed fields."""
import argparse
import hashlib
import json
import pathlib
import random
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario
from test_vehicle_start import initialized, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
BASIS = json.loads((ROOT / 'tools/rewrite/ground_basis.json').read_text())
Q31 = BASIS['q31_sine']


def contact(side, pixels, pose):
    x, y, heading = pose
    index = ((-heading) % 65536) // 128
    sine, cosine = Q31[index] // 64, Q31[(index + 128) % 512] // 64
    cell = 4294967296 // side
    x, y = x * 8192, -y * 8192
    def height(px, py):
        return pixels[((py % 4294967296) // cell) * side + (px % 4294967296) // cell]
    def angle(a, b):
        return ((a - b + 128) % 256 - 128) * 128
    return (height(x, y), angle(height(x - cosine, y + sine), height(x + cosine, y - sine)),
            angle(height(x - sine, y - cosine), height(x + sine, y + cosine)))


def installed(side, pixels, records):
    result = []
    for identity, generation, original in records:
        raw = bytearray(original)
        x, y = struct.unpack_from('<ii', raw, 4)
        hull, = struct.unpack_from('<H', raw, 0x26)
        turret, = struct.unpack_from('<H', raw, 0x10)
        h, roll, pitch = contact(side, pixels, (x, y, hull))
        _, turret_roll, turret_pitch = contact(side, pixels, (x, y, turret))
        raw[0x1d] = h
        struct.pack_into('<hh', raw, 0x32, roll, pitch)
        struct.pack_into('<hh', raw, 0x22, turret_roll, turret_pitch)
        result.append((identity, generation, bytes(raw)))
    return result


def varied_plane(side):
    return bytes((column * 13 + row * 29 + (column ^ row) * 7) % 256
                 for row in range(side) for column in range(side))


class GroundTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-ground-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_ground_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_ground_probe.js')])

    def run_case(self, side, pixels, poses=(), records=(), valid=True):
        request = (struct.pack('<III', side, len(poses), len(records)) + pixels +
                   b''.join(struct.pack('<iiH', *pose) for pose in poses) +
                   b''.join(struct.pack('<HH', identity, generation) + raw
                            for identity, generation, raw in records))
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(request)
        expected = ''
        if valid:
            samples = [contact(side, pixels, pose) for pose in poses]
            states, _ = initialized(records, (0, 0, 0, 0), 0, 0)
            updated = installed(side, pixels, states)
            if ORACLE is not None:
                machine = ORACLE.prepare(side, pixels)
                self.assertEqual(ORACLE.samples(machine, poses), samples)
                self.assertEqual(ORACLE.contact(machine, states), updated)
            expected = ''.join('sample ' + ' '.join(map(str, value)) + '\n' for value in samples)
            expected += state_lines(updated)
        for command in self.commands:
            with self.subTest(target=command[0], side=side, queries=len(poses), vehicles=len(records)):
                result = subprocess.run([*command, str(path)], capture_output=True, timeout=60)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr.decode())
                self.assertEqual(result.stdout.decode(), expected)

    def test_complete_original_basis_pin_and_no_symmetric_quarter_wave_assumption(self):
        self.assertEqual(len(Q31), 512)
        self.assertEqual(hashlib.sha256(struct.pack('<640i', *(Q31 + Q31[:128]))).hexdigest(),
                         BASIS['complete_table_sha256'])
        self.assertEqual(Q31[128], 2147483647)
        self.assertEqual(Q31[384], -2147483648)
        self.assertEqual(Q31[256], 1)
        # Arithmetic shift gives +33554431 versus -33554432 at the cardinal extrema.
        self.assertNotEqual(Q31[128] // 64, -(Q31[384] // 64))
        # Complete original 7fa0/8480 returns at this reached grid seam.
        # A symmetric +/-4096 footprint would give roll 12288, pitch -4096.
        self.assertEqual(contact(512, varied_plane(512), (0, 0, 0)), (0, 14848, 512))

    def test_all_heading_values_four_corner_footprints_and_periodic_axes(self):
        self.run_case(512, varied_plane(512), [(407552, -509951, heading) for heading in range(65536)])

    def test_every_byte_difference_including_sign_wrap_on_complete_samples(self):
        side = 512
        plane = bytes(a if column % 2 == 0 else column // 2
                      for a in range(256) for _ in range(2) for column in range(side))
        # Duplicate rows: row 2*a holds even-column value a. At heading zero,
        # samples differ by seven columns, reaching all 65536 a,b height pairs.
        poses = [((2 * b + 4) * 1024, -2 * a * 1024, 0)
                 for a in range(256) for b in range(256)]
        rolls = [contact(side, plane, pose)[1] for pose in poses]
        self.assertEqual(set(rolls), {value * 128 for value in range(-128, 128)})
        self.run_case(side, plane, poses)

    def test_coordinate_cell_and_wrap_boundaries_with_tiny_and_constant_fields(self):
        positions = (-2147483648, -1048577, -524289, -524288, -524287, -1, 0, 1,
                     511, 512, 513, 1023, 1024, 1025, 524287, 524288, 524289, 2147483647)
        poses = [(x, y, heading) for x in positions for y in positions
                 for heading in (0, 1, 127, 128, 16383, 16384, 32768, 49152, 65535)]
        for side in (1, 2, 4, 8, 16, 512, 1024):
            self.run_case(side, varied_plane(side), poses)
        self.run_case(1, b'\xff', [(x, y, h) for x, y, h in poses[:256]])

    def test_four_class_contact_preserves_altitude_controls_components_and_independent_turret(self):
        records = []
        randomizer = random.Random(0x1109)
        for kind in range(4):
            for heading in range(0, 65536, 128):
                raw = bytearray(randomizer.randbytes(251))
                struct.pack_into('<HHiiiH', raw, 0, kind, 17, randomizer.randrange(-2147483648, 2147483648),
                                 randomizer.randrange(-2147483648, 2147483648), 0x12345678,
                                 (heading + 12345) % 65536)
                struct.pack_into('<H', raw, 0x26, heading)
                records.append((heading % 182, kind + 19, bytes(raw)))
        self.run_case(512, varied_plane(512), records=records)

    def test_null_bad_fields_classes_components_and_atomic_publication(self):
        for command in self.commands:
            result = subprocess.run([*command, 'contracts'], capture_output=True, timeout=15)
            self.assertEqual(result.returncode, 0, result.stderr.decode())
            self.assertEqual(result.stdout, b'')
        self.run_case(3, bytes(9), [(0, 0, 0)], valid=False)
        self.run_case(2, bytes(3), [(0, 0, 0)], valid=False)
        self.run_case(2, bytes(5), [(0, 0, 0)], valid=False)
        self.run_case(1, b'\0', valid=False)
        self.run_case(1, b'\0', records=[(0, 0, b'\x04\0' + bytes(249))], valid=False)

    def test_all_original_maps_detail_levels_and_ground_snapshots(self):
        if not ORIGINALS:
            self.skipTest('Complete original corpus is an explicit additional gate')
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        directory = ROOT / 'armoredfist/FISTDATA'
        scenario_manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        terrain_manifest = json.loads((ROOT / 'tools/rewrite/terrain_originals.json').read_text())['files']
        self.assertEqual([p.name for p in sorted(directory.glob('*.FSG'))], sorted(scenario_manifest))
        grouped = {}
        count = 0
        for name, info in scenario_manifest.items():
            path = directory / name
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            offset, height_name = 0, None
            while offset < len(data):
                tag, length = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += length + 6
            self.assertIsNotNone(height_name)
            records = [r for r in records_from_scenario(data) if struct.unpack_from('<H', r[2])[0] < 4]
            grouped.setdefault(height_name, []).extend(records)
            count += len(records)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual(len(grouped), 8)
        self.assertEqual(count, 960)
        for name, records in grouped.items():
            info = terrain_manifest[name]
            path = directory / name
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            decoded = decoder.klc(data)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed_side, expanded = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed_side, side)
                poses = [(524287, -1, heading) for heading in range(0, 65536, 128)]
                self.run_case(side, expanded, poses, records)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info['sha256'])
        print(f'Ground corpus: {count} snapshots in {len(scenario_manifest)} missions; '
              'all eight original height maps at four installed sizes, 16384 direct samples '
              'and 3840 complete contacts on each target', flush=True)


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
        from original_ground_oracle import OriginalGroundOracle
        ORACLE = OriginalGroundOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS))
