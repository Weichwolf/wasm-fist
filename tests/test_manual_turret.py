#!/usr/bin/env python3
"""Complete original turret curve, fixed callers and retained shared controls."""
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

from manual_turret_contract import COUNTS, domains, predict
from test_units import records_from_scenario
from test_vehicle_start import state_lines

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False


def snapshot(kind, view=1, flags=0, target=0, requested=65535):
    raw = bytearray((offset * 37 + kind * 11) & 255 for offset in range(251))
    struct.pack_into('<H', raw, 0, kind)
    struct.pack_into('<H', raw, 0x40, flags)
    struct.pack_into('<H', raw, 0x97, target)
    struct.pack_into('<H', raw, 0x8b, requested)
    struct.pack_into('<h', raw, 0x38, -32768)
    raw[0x86] = view
    return bytes(raw)


def fixture(raw, selector, right, caller=False, steps=1, reference=(0, 0)):
    return raw, selector, right, caller, steps, reference


def encoded(case):
    raw, selector, right, caller, steps, reference = case
    return raw + struct.pack('<HBBHQH', selector, right, caller, steps, *reference)


def observation(raw, selector, reference):
    return (state_lines([(0, 0, raw)]) + f'turret_controls {selector}\n' +
            'turret_target ' + ' '.join(map(str, reference)) + '\n')


def expected(case):
    raw, selector, right, caller, steps, reference = case
    output = []
    for _ in range(steps):
        if caller:
            selector = 232 if right else 88
        # A canonical runtime binding represents an original present pointer.
        if reference[0] and struct.unpack_from('<H', raw, 0x97)[0] == 0:
            raw = bytearray(raw)
            struct.pack_into('<H', raw, 0x97, 1)
        raw, _, _ = predict(raw, selector, bool(right))
        reference = (0, 0)
        output.append(observation(raw, selector, reference))
    return ''.join(output)


def expected_shared(cases):
    actors = [case[0] for case in cases]
    selector = cases[0][1]
    output = []
    for tick in range(512):
        for kind in range(4):
            right = bool((tick // 256 + kind) & 1)
            raw = bytearray(actors[kind])
            raw[0x86] = tick & 255
            struct.pack_into('<H', raw, 0x40, 65535 if tick & 1 else 0)
            struct.pack_into('<H', raw, 0x97, 1 + (kind + 1) % 4 if tick % 3 == 0 else 0)
            selector = 232 if right else 88
            actors[kind], _, _ = predict(raw, selector, right)
            output.append(observation(actors[kind], selector, (0, 0)))
    # Direct helpers consume that retained shared selector after the callers.
    for round_index in range(8):
        for kind in range(4):
            actors[kind], _, _ = predict(actors[kind], selector, bool((round_index + kind) & 1))
            output.append(observation(actors[kind], selector, (0, 0)))
    return ''.join(output)


class TurretTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-turret-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_manual_turret_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_manual_turret_probe.js')])
        cls.checked = collections.Counter()
        cls.digest = hashlib.sha256()

    @classmethod
    def tearDownClass(cls):
        print('Manual turret per target: ' + json.dumps(dict(cls.checked), sort_keys=True) +
              ' output_sha256=' + cls.digest.hexdigest(), flush=True)

    def check(self, cases, group, shared=False):
        path = Path(self.temp.name) / 'input.bin'
        path.write_bytes(struct.pack('<I', len(cases)) + b''.join(encoded(c) for c in cases))
        wanted = expected_shared(cases) if shared else ''.join(expected(c) for c in cases)
        for command in self.commands:
            result = subprocess.run([*command, str(path), *(['--shared'] if shared else [])],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
            self.assertEqual(result.stderr, '')
        self.checked[group] += 2080 if shared else sum(c[4] for c in cases)
        self.digest.update(wanted.encode())

    def test_complete_original_curve_view_selector_and_requested_word_domains(self):
        counts = collections.Counter()
        batch = []
        for group, kind, right, selector, view, target, flags, requested in domains():
            batch.append(fixture(snapshot(kind, view, flags, target, requested), selector, right))
            counts[group] += 1
            if len(batch) == 1024:
                self.check(batch, 'domains')
                batch.clear()
        if batch:
            self.check(batch, 'domains')
        self.assertEqual(counts, COUNTS)

    def test_retained_view_scaling_wrap_target_and_control_refresh(self):
        cases = []
        for kind, right, caller, view, selector, target, flags in itertools.product(
                range(4), (False, True), (False, True), (0, 1, 2, 3, 4, 6, 127, 255),
                (0, 88, 160, 232, 65535), (0, 65535), (0, 65535)):
            cases.append(fixture(snapshot(kind, view, flags, target), selector, right, caller,
                                 steps=200))
        for offset in range(0, len(cases), 16):
            self.check(cases[offset:offset + 16], 'held')

    def test_fixed_and_direct_helpers_retain_one_shared_selector_across_four_actors(self):
        for seed in (0, 32768, 65535):
            cases = [fixture(snapshot(kind, requested=kind * 16384), seed, False)
                     for kind in range(4)]
            self.check(cases, 'shared', shared=True)

    def test_runtime_target_presence_is_cancelled_without_resolution(self):
        cases = []
        for kind, right, caller, view, reference in itertools.product(
                range(4), (False, True), (False, True), (0, 1, 2, 4, 255),
                ((0, 0), (1, 150), (2**64 - 1, 181), (2**32, 0), (42, 0))):
            cases.append(fixture(snapshot(kind, view, 65535, 0), 65535, right, caller,
                                 steps=3, reference=reference))
        for offset in range(0, len(cases), 128):
            self.check(cases[offset:offset + 128], 'runtime_targets')

    def test_invalid_and_late_records_fail_without_partial_output(self):
        good = fixture(snapshot(0, flags=65535), 65535, False)
        valid = struct.pack('<I', 1) + encoded(good)
        bad = [b'', valid[:-1], valid + b'x', struct.pack('<I', 2) + encoded(good)]
        bad += [struct.pack('<I', 1) + encoded(fixture(good[0], 65535, False, steps=0))]
        bad += [struct.pack('<I', 2) + encoded(good) + encoded(fixture(good[0], 65535, 255))]
        bad += [struct.pack('<I', 2) + encoded(good) + encoded(fixture(good[0], 65535, False, 255))]
        for reference in ((1, 182), (0, 1), (1, 65535)):
            for caller in (False, True):
                bad += [struct.pack('<I', 2) + encoded(good) +
                        encoded(fixture(good[0], 0, True, caller, reference=reference))]
        raw = bytearray(good[0])
        struct.pack_into('<H', raw, 0, 4)
        bad += [struct.pack('<I', 2) + encoded(good) + encoded(fixture(bytes(raw), 0, False))]
        path = Path(self.temp.name) / 'invalid.bin'
        for data in bad:
            path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)
        self.check([], 'empty')
        for command in self.commands:
            for args in ([], [str(path.parent / 'missing')], [str(path), '--unknown'],
                         [str(path), '--shared']):
                result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''))
        shared = [fixture(snapshot(kind), 0, False) for kind in range(4)]
        for cases in (shared[:3], [shared[0], shared[1], shared[3], shared[2]]):
            path.write_bytes(struct.pack('<I', len(cases)) + b''.join(encoded(c) for c in cases))
            for command in self.commands:
                result = subprocess.run([*command, str(path), '--shared'],
                                        capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_all_original_saved_ground_turret_inputs_and_fixed_callers(self):
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
                for right, caller, selector in itertools.product((False, True), (False, True), (0, 65535)):
                    cases.append(fixture(raw, selector, right, caller, steps=3))
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
