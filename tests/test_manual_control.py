#!/usr/bin/env python3
"""Complete shared manual composition, saved bytes and atomic used-state guards."""
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

from manual_control_cases import COUNTS, complete_cases, snapshot
from manual_control_contract import predict
from test_original_mission_ready import ground_reset
from test_units import records_from_scenario
from test_vehicle_start import SEEDS, initialized, state_lines

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False


def fixture(raw, selected=True, mode=0, clock=0, controls=(0, 18, 18), selector=0,
            steps=1, reference=(0, 0)):
    return bytes(raw), selected, mode, clock, controls, selector, steps, reference


def encoded(case):
    raw, selected, mode, clock, controls, selector, steps, reference = case
    return raw + struct.pack('<6HBHQH', mode, clock, *controls, selector, selected, steps, *reference)


def state(raw):
    return state_lines([(0, 0, raw)]) + f'manual_inputs {raw[0xa0]} {raw[0xa4]}\n'


def observation(raw, controls, selector, evidence, reference):
    return (state(raw) + 'manual_controls ' + ' '.join(map(str, (*controls, selector))) + '\n' +
            f'manual_events {int(evidence["profile"] is not None)} {int(evidence["view"])}\n' +
            'manual_target ' + ' '.join(map(str, reference)) + '\n')


def advance(raw, selected, mode, clock, controls, selector, reference):
    used_target = selected and raw[0xa0] in (2, 4, 6, 8)
    if used_target and reference[0] and struct.unpack_from('<H', raw, 0x97)[0] == 0:
        raw = bytearray(raw)
        struct.pack_into('<H', raw, 0x97, 1)
    result, controls, selector, evidence = predict(raw, selected, mode, clock, controls, selector)
    return result, controls, selector, evidence, (0, 0) if used_target else reference


def expected(case):
    raw, selected, mode, clock, controls, selector, steps, reference = case
    output = []
    for _ in range(steps):
        raw, controls, selector, evidence, reference = advance(
            raw, selected, mode, clock, controls, selector, reference)
        output.append(observation(raw, controls, selector, evidence, reference))
    return ''.join(output)


def expected_retention(case):
    raw, _, link, _, _, _, _, _ = case
    records, (words, end) = initialized([(0, 0, raw)], SEEDS, 0, link)
    started = records[0][2]
    return (state(raw) + state(started) + 'manual_random ' +
            ' '.join(map(str, (end, *words))) + '\n' + state(ground_reset(started, link)))


def expected_shared(cases):
    actors = [case[0] for case in cases]
    references = [case[7] for case in cases]
    _, _, _, seed, controls, selector, _, _ = cases[0]
    output = []
    axes = ((-128, -128), (-24, -25), (-23, -24), (0, -23), (23, 0), (24, 23), (127, 24), (127, 127))
    for tick in range(1024):
        for kind in range(4):
            raw = bytearray(actors[kind])
            selected = (tick + kind) % 13 != 0
            raw[0xa0] = (0, 2, 4, 6, 8)[(tick + kind) % 5] if selected else 255
            raw[0xa4] = (tick * 37 + kind * 53) & 255
            struct.pack_into('<bb', raw, 0xa1, *axes[(tick + kind) % 8])
            struct.pack_into('<h', raw, 0x55, (-32768, -1, 0, 32767)[tick % 4])
            if tick % 5 == 0:
                struct.pack_into('<H', raw, 0x97, 1 + (kind + 1) % 4)
            mode = (tick // 32 + kind) % 6
            if tick % 31 == 0:
                struct.pack_into('<H', raw, 0x40, struct.unpack_from('<H', raw, 0x40)[0] | 1)
                mode = 65535
            if not selected:
                mode = 65535
            if tick & 128 and mode != 65535:
                mode += 32768
            clock = (seed + tick * (1, 19, 20, 65535)[tick % 4] + kind) & 65535
            actors[kind], controls, selector, evidence, references[kind] = advance(
                raw, selected, mode, clock, controls, selector, references[kind])
            output.append(observation(actors[kind], controls, selector, evidence, references[kind]))
    return ''.join(output)


class ManualTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-manual-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_manual_control_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_manual_control_probe.js')])
        cls.checked = collections.Counter()
        cls.digest = hashlib.sha256()

    @classmethod
    def tearDownClass(cls):
        print('Manual control per target: ' + json.dumps(dict(cls.checked), sort_keys=True) +
              ' output_sha256=' + cls.digest.hexdigest(), flush=True)

    def check(self, cases, group, mode=None):
        path = Path(self.temp.name) / 'input.bin'
        path.write_bytes(struct.pack('<I', len(cases)) + b''.join(encoded(case) for case in cases))
        if mode == '--shared':
            wanted, count = expected_shared(cases), 4096
        elif mode == '--retention':
            wanted, count = ''.join(expected_retention(case) for case in cases), len(cases) * 3
        else:
            wanted, count = ''.join(expected(case) for case in cases), sum(case[6] for case in cases)
        for command in self.commands:
            result = subprocess.run([*command, str(path), *([mode] if mode else [])],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
            self.assertEqual(result.stderr, '')
        self.checked[group] += count
        self.digest.update(wanted.encode())

    def batches(self, cases, group, size=1024, mode=None):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == size:
                self.check(batch, group, mode)
                batch.clear()
        if batch:
            self.check(batch, group, mode)

    def test_complete_original_parent_domains(self):
        counts = collections.Counter()
        def cases():
            for group, kind, selected, mode, raw, clock, controls, selector in complete_cases():
                counts[group] += 1
                yield fixture(raw, selected, mode, clock, controls, selector)
        self.batches(cases(), 'domains')
        self.assertEqual(counts, COUNTS)

    def test_held_complete_banks_preserve_and_clear_control_admission(self):
        cases = []
        for kind, mode, action, view, flags, target in itertools.product(
                range(4), range(6), (0, 2, 4, 6, 8), (0, 79, 80, 159, 160, 255),
                (0, 1), (0, 65535)):
            raw = snapshot(kind, action, view, flags, target)
            cases.append(fixture(raw, True, mode, 0, (65535, 363, 18), 65535, steps=100))
        self.batches(cases, 'held', size=16)

    def test_shared_controls_across_retained_actors_and_clock_boundaries(self):
        for seed, controls, selector in ((0, (0, 0, 0), 0),
                                         (65530, (65535, 363, 65535), 65535),
                                         (32768, (0, 65535, 18), 32768)):
            cases = [fixture(snapshot(kind, 0, 0, 1, 1+(kind+1)%4, requested=kind*16384),
                             True, 0, seed, controls, selector) for kind in range(4)]
            self.check(cases, 'shared', '--shared')

    def test_all_saved_manual_bytes_restore_start_and_prepare(self):
        cases = (fixture(snapshot(kind, value, 255-value, 65535, 65535), mode=link)
                 for kind, value, link in itertools.product(range(4), range(256), (0, 2)))
        self.batches(cases, 'retention', mode='--retention')

    def test_runtime_target_presence_and_ignored_malformed_targets(self):
        cases = []
        for kind, mode, action, reference in itertools.product(
                range(4), range(6), (0, 2, 4, 6, 8),
                ((0, 0), (1, 150), (2**64-1, 181), (2**32, 0), (42, 0))):
            cases.append(fixture(snapshot(kind, action, 160, 0, 0), True, mode, 0,
                                 (65535, 363, 18), 65535, steps=3, reference=reference))
        for kind, mode, reference in itertools.product(range(4), range(6), ((0, 1), (1, 182), (1, 65535))):
            cases.append(fixture(snapshot(kind, 0, 80, 0, 0), True, mode, reference=reference))
            cases.append(fixture(snapshot(kind, 255, 80, 0, 0), False, 65535, reference=reference))
        self.batches(cases, 'runtime_targets')

    def test_used_domains_and_full_late_failure_transactions(self):
        wanted = 'guards 262096 1004 576 148 33\n'
        for command in self.commands:
            result = subprocess.run([*command, '--guards'], capture_output=True, text=True, timeout=60)
            self.assertEqual((result.returncode, result.stdout, result.stderr), (0, wanted, ''))
        self.checked['guards'] += 262096 + 1004 + 576 + 148 + 33
        self.digest.update(wanted.encode())

    def test_invalid_input_and_late_records_produce_no_partial_output(self):
        good = fixture(snapshot(0, 2, 160, 0, 65535), mode=2)
        valid = struct.pack('<I', 1) + encoded(good)
        bad = [b'', valid[:-1], valid+b'x', struct.pack('<I', 2)+encoded(good),
               struct.pack('<I', 1)+encoded(fixture(good[0], mode=2, steps=0))]
        late = [fixture(good[0], True, 6), fixture(good[0], selected=255),
                fixture(good[0], mode=2, reference=(1, 182)),
                fixture(good[0], mode=2, reference=(0, 1)),
                fixture(good[0], mode=2, reference=(1, 65535))]
        raw = bytearray(good[0]); raw[0xa0] = 255
        late.append(fixture(raw, mode=2))
        raw = bytearray(good[0]); struct.pack_into('<H', raw, 0, 4)
        late.append(fixture(raw))
        bad += [struct.pack('<I', 2)+encoded(good)+encoded(case) for case in late]
        path = Path(self.temp.name) / 'invalid.bin'
        for data in bad:
            path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)
        self.check([], 'empty')
        for command in self.commands:
            for args in ([], [str(path.parent/'missing')], [str(path), '--unknown']):
                result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''))
        shared = [fixture(snapshot(kind, 0, 0, 0, 0)) for kind in range(4)]
        for cases in ([], shared[:3], [shared[0], shared[1], shared[3], shared[2]]):
            path.write_bytes(struct.pack('<I', len(cases))+b''.join(encoded(c) for c in cases))
            for command in self.commands:
                result = subprocess.run([*command, str(path), '--shared'], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_all_original_saved_ground_actors_through_complete_banks(self):
        if not ORIGINALS:
            self.skipTest('Provisioned canonical source corpus requested separately')
        manifest = json.loads((ROOT/'tests/scenario_originals.json').read_bytes())
        files = sorted((ROOT/'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([p.name for p in files], sorted(manifest))
        actors = 0
        for path in files:
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
            self.assertFalse(path.stat().st_mode & 0o222)
            cases = []
            for _, _, source in records_from_scenario(data):
                if struct.unpack_from('<H', source)[0] >= 4:
                    continue
                actors += 1
                for mode, action, admission in itertools.product(range(6), (0, 2, 4, 6, 8),
                                                                 ('selected', 'inhibited', 'unselected')):
                    raw = bytearray(source); raw[0xa0] = action
                    flags = struct.unpack_from('<H', raw, 0x40)[0]
                    if admission != 'unselected':
                        struct.pack_into('<H', raw, 0x40, flags|1 if admission=='inhibited' else flags&65534)
                    cases.append(fixture(raw, admission!='unselected', mode, 65535, (0, 18, 363), 32768))
            self.batches(cases, 'originals')
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
