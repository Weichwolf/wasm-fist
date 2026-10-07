#!/usr/bin/env python3
"""Original model catalog, immutable ground visual selection and part directions."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario, scenario_data, snapshot

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
NAMES = [f'{family}_{suffix}' for family in ('M1', 'M3', 'T80', 'BMP')
         for suffix in ('A', 'B', 'C', 'D', 'E', 'DEAD')] + [
             'EXPLODE', 'APACHE', 'HIND', 'SMOKE', 'MUZZLE', 'TREES', 'GSMOKE',
             'TARGETS', 'SHOT', 'ARTILL']


def facing(heading, bearing):
    # Nearest 1/32 turn, with midpoint rounding forward; modulo gives wrap.
    return ((heading - bearing + 1024) // 2048) % 32


def visual(state, bearing):
    kind, = struct.unpack_from('<H', state)
    code = 4 + kind * 12
    scale, = struct.unpack_from('<H', state, 20)
    primary, = struct.unpack_from('<H', state, 38)
    secondary, = struct.unpack_from('<H', state, 16)
    headings = (primary, secondary)
    parts = [state[169] ^ 128, state[170] ^ 128]
    poses = []
    for part, flags in enumerate(parts):
        poses += [facing(headings[int(bool(flags & 128))], bearing), part, flags & 127]
    return [code, NAMES[code // 2], scale // 256, *headings, *parts, *poses]


class VehicleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-vehicle-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_vehicle_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_vehicle_probe.js')])

    def run_probe(self, arguments, expected, valid=True):
        for command in self.commands:
            with self.subTest(target=command[0], arguments=arguments):
                result = subprocess.run([*command, *map(str, arguments)], capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, expected)

    def run_records(self, records, bearing):
        data = scenario_data(records)
        self.run_data(data, bearing)

    def run_data(self, data, bearing):
        path = pathlib.Path(self.temp.name) / 'input.fsg'
        path.write_bytes(data)
        lines = []
        for ordinal, (_, _, state) in enumerate(records_from_scenario(data)):
            kind, = struct.unpack_from('<H', state)
            if kind >= 4:
                continue
            fields = visual(state, bearing)
            if ORACLE is not None:
                self.assertEqual(ORACLE.visual(state, bearing), fields)
            lines.append('vehicle ' + ' '.join(map(str, [ordinal, *fields])))
        expected = '\n'.join(lines) + ('\n' if lines else '')
        self.run_probe(['units', path, bearing], expected)

    def test_complete_catalog_and_absent_codes(self):
        names = {index * 2: name for index, name in enumerate(NAMES)}
        if ORACLE is not None:
            self.assertEqual(ORACLE.names, names)
        self.run_probe(['catalog'], ''.join(f'model {code} {name}\n'
                                           for code, name in names.items()))

    def test_every_relative_angle_and_unsigned_bearing_wrap(self):
        reference = [facing(heading, 0) for heading in range(65536)]
        if ORACLE is not None:
            self.assertEqual(ORACLE.all_facings(), reference)
        for bearing in (0, 65535, 45678):
            expected = ''.join(f'{reference[(heading - bearing) % 65536]}\n'
                               for heading in range(65536))
            self.run_probe(['angles', bearing], expected)

    def test_all_variant_bytes_both_heading_selectors_and_separate_headings(self):
        for kind in range(4):
            records = []
            for value in range(256):
                state = bytearray(snapshot(kind, flags=0, heading=64000 - value))
                struct.pack_into('<H', state, 38, value * 256)
                state[169:171] = bytes((value, 255 - value))
                records.append((value % 182, value, bytes(state)))
            for start in range(0, len(records), 64):
                self.run_records(records[start:start + 64], 32768 + kind)

    def test_scale_width_zero_unsupported_classes_and_input_ownership(self):
        records = []
        for index, scale in enumerate((0, 255, 256, 1024, 65535)):
            state = bytearray(snapshot(index % 4, flags=0, heading=index * 16000))
            struct.pack_into('<H', state, 20, scale)
            struct.pack_into('<H', state, 38, 65535 - index)
            state[169:171] = bytes((128, 0))
            records.append((index, 0, bytes(state)))
        self.run_records(records, 65535)
        self.run_records([(kind, 0, snapshot(kind, flags=0)) for kind in range(4, 28)], 0)
        self.run_records([], 0)

    def test_malformed_and_missing_inputs_fail_without_output(self):
        path = pathlib.Path(self.temp.name) / 'bad.fsg'
        for data in (b'', scenario_data([(0, 0, snapshot())])[:-1],
                     scenario_data([(0, 0, snapshot(0, size=55))])):
            path.write_bytes(data)
            self.run_probe(['units', path, 0], '', valid=False)
        self.run_probe(['units', pathlib.Path(self.temp.name) / 'missing.fsg', 0], '', valid=False)
        for bearing in ('', '-1', '65536', 'words', '42x'):
            self.run_probe(['angles', bearing], '', valid=False)
        self.run_probe(['unknown'], '', valid=False)

    def test_all_original_ground_snapshots_and_catalog_files(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([path.name for path in files], sorted(manifest))
        count = 0
        types = set()
        for path in files:
            with self.subTest(file=path.name):
                data = path.read_bytes()
                self.assertEqual(len(data), manifest[path.name]['size'])
                self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
                self.run_data(data, 51700)
                for _, _, state in records_from_scenario(data):
                    kind, = struct.unpack_from('<H', state)
                    if kind < 4:
                        count += 1
                        types.add(kind)
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(),
                                 manifest[path.name]['sha256'])
        self.assertEqual(count, 960)
        self.assertEqual(types, set(range(4)))
        models = json.loads((ROOT / 'tests/model_originals.json').read_text())
        for name in NAMES:
            for suffix in ('MAL', 'M00', 'M08', 'M16', 'M32'):
                filename = f'{name}.{suffix}'
                data = (ROOT / 'armoredfist/FISTDATA' / filename).read_bytes()
                self.assertEqual(len(data), models[filename]['size'])
                self.assertEqual(hashlib.sha256(data).hexdigest(), models[filename]['sha256'])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, ORIGINALS = args.native_probe, args.originals
    if args.oracle:
        from original_vehicle_oracle import OriginalVehicleOracle
        ORACLE = OriginalVehicleOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
