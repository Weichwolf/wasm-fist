#!/usr/bin/env python3
"""Complete decoded axes, original domains, retained refresh and saved-state ownership."""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from analog_drive_contract import COUNTS, domains, predict
from test_original_mission_ready import ground_reset
from test_units import records_from_scenario
from test_vehicle_start import initialized, state_lines

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
RESTORE, INITIALIZE, PREPARE, APPLY = range(4)
SEEDS = (1, 2, 32768, 65535)


def snapshot(kind, steer, pedal, heading, requested, speed, mode):
    raw = bytearray((i * 37 + kind * 11) & 255 for i in range(251))
    struct.pack_into('<H', raw, 0, kind)
    struct.pack_into('<H', raw, 0x26, heading)
    struct.pack_into('<H', raw, 0x30, requested)
    struct.pack_into('<h', raw, 0x55, speed)
    struct.pack_into('<h', raw, 0x57, 12345)
    struct.pack_into('<bb', raw, 0xa1, steer, pedal)
    raw[0x90] = mode
    return bytes(raw)


def fixture(raw, stage=APPLY, steps=1, link=0):
    return raw, stage, steps, link


def encoded(case):
    raw, stage, steps, link = case
    return raw + struct.pack('<HBB', steps, stage, link)


def expected(case):
    raw, stage, steps, link = case
    random = (list(SEEDS), 0)
    if stage == INITIALIZE:
        records, random = initialized([(0, 0, raw)], SEEDS, 0, link)
        raw = records[0][2]
    elif stage == PREPARE:
        raw = ground_reset(raw, link)
    output = []
    for _ in range(steps):
        refresh = False
        if stage == APPLY:
            raw, profile, _ = predict(raw)
            refresh = profile is not None
        steer, pedal = struct.unpack_from('<bb', raw, 0xa1)
        output.append(state_lines([(0, 0, raw)]) +
                      f'axes {steer} {pedal} {int(refresh)}\n' +
                      'axis_random ' + ' '.join(map(str, [*random[0], random[1]])) + '\n')
    return ''.join(output)


class AxisTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-axes-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_ground_axes_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_ground_axes_probe.js')])
        cls.checked = collections.Counter()
        cls.digest = hashlib.sha256()

    @classmethod
    def tearDownClass(cls):
        print('Axes per target: ' + json.dumps(dict(cls.checked), sort_keys=True) +
              ' output_sha256=' + cls.digest.hexdigest(), flush=True)

    def check(self, cases, group):
        path = Path(self.temp.name) / 'input.bin'
        path.write_bytes(struct.pack('<I', len(cases)) + b''.join(encoded(c) for c in cases))
        wanted = ''.join(expected(c) for c in cases)
        for command in self.commands:
            result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
            self.assertEqual(result.stderr, '')
        self.checked[group] += sum(c[2] for c in cases)
        self.digest.update(wanted.encode())

    def test_complete_original_axis_heading_and_profile_domains(self):
        counts = collections.Counter()
        batch = []
        for group, kind, steer, pedal, heading, requested, speed, mode in domains():
            batch.append(fixture(snapshot(kind, steer, pedal, heading, requested, speed, mode)))
            counts[group] += 1
            if len(batch) == 1024:
                self.check(batch, 'domains')
                batch.clear()
        if batch:
            self.check(batch, 'domains')
        self.assertEqual(counts, COUNTS)

    def test_all_signed_bytes_retain_through_restore_initialize_and_prepare(self):
        cases = []
        for kind, value, stage, link in itertools.product(range(4), range(256), range(3), (0, 2)):
            raw = snapshot(kind, value - 128, 127 - value, 65535, 0, -1, 255)
            cases.append(fixture(raw, stage=stage, link=link))
        for offset in range(0, len(cases), 256):
            self.check(cases[offset:offset + 256], 'retention')

    def test_held_calls_preserve_heading_dead_zone_and_refresh_each_reverse_call(self):
        values = (-128, -25, -24, -23, 0, 23, 24, 127)
        cases = []
        for kind, index, speed, mode in itertools.product(
                range(4), range(8), (-32768, -1, 0, 32767), (0, 1, 3, 255)):
            raw = snapshot(kind, values[index], values[-1 - index], 65535, 12345, speed, mode)
            cases.append(fixture(raw, steps=400))
        for offset in range(0, len(cases), 16):
            self.check(cases[offset:offset + 16], 'held')

    def test_invalid_and_late_input_fail_without_output(self):
        good = fixture(snapshot(0, 24, 127, 65535, 0, -1, 3))
        valid = struct.pack('<I', 1) + encoded(good)
        invalid = [b'', valid[:-1], valid + b'x', struct.pack('<I', 2) + encoded(good)]
        invalid += [struct.pack('<I', 1) + encoded(fixture(good[0], steps=0))]
        invalid += [struct.pack('<I', 2) + encoded(good) + encoded(fixture(good[0], stage=255))]
        bad = bytearray(good[0])
        struct.pack_into('<H', bad, 0, 4)
        invalid += [struct.pack('<I', 2) + encoded(good) + encoded(fixture(bytes(bad)))]
        path = Path(self.temp.name) / 'invalid.bin'
        for data in invalid:
            path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)
        self.check([], 'empty')
        for command in self.commands:
            result = subprocess.run([*command, str(path.parent / 'missing')],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_all_original_saved_ground_axes_and_retained_stages(self):
        if not ORIGINALS:
            self.skipTest('Provisioned canonical source corpus requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([p.name for p in files], sorted(manifest))
        actors = 0
        for path in files:
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
            cases = []
            for _, _, raw in records_from_scenario(data):
                if struct.unpack_from('<H', raw)[0] >= 4:
                    continue
                actors += 1
                for stage, link in itertools.product(range(4), (0, 2)):
                    cases.append(fixture(raw, stage=stage, steps=3, link=link))
            for offset in range(0, len(cases), 128):
                self.check(cases[offset:offset + 128], 'originals')
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), manifest[path.name]['sha256'])
        self.assertEqual(actors, 960)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=Path, default=BUILD)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default=TARGET)
    parser.add_argument('--native-probe', type=Path)
    parser.add_argument('--originals', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS = args.build_root, args.target, args.native_probe, args.originals
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
