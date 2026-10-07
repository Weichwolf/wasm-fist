#!/usr/bin/env python3
"""Shared complete target selection, runtime lifetimes and logical voice requests."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from target_discovery_contract import TABLES, discovery

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORACLE = ORIGINALS = False
REVIEW = None
NONE = 65535


def pointer(slot):
    return 0xa022 + slot * 55 if slot < 150 else 0xc05c + (slot - 150) * 251


def physical(address):
    if not address:
        return NONE
    return (address - 0xa022) // 55 if address < 0xc05c else 150 + (address - 0xc05c) // 251


def output(actor, expected, raw):
    state = expected['actor']
    old = struct.unpack_from('<H', state, 0x97)[0]
    # Deliberate proved lifetime repair: an unallocated old target is invalid.
    if old and physical(old) not in raw:
        old = 0
    voice = expected['requests'][0] if expected['requests'] else (0, 0, 0)
    values = (actor, physical(expected['primary']), physical(expected['secondary']),
              expected['primary_range'], expected['secondary_operand'], expected['priority'],
              expected['count'], state[25], struct.unpack_from('<H', state, 0x8e)[0],
              physical(old), physical(expected['primary']), expected['count'],
              int(bool(expected['requests'])), *voice, expected['last_voice'])
    return 'target ' + ' '.join(map(str, values)) + '\n'


class Collector:
    def __init__(self, path, commands):
        self.path, self.commands = path, commands
        self.key = None
        self.canonical = None
        self.cases = []
        self.expected = []
        self.scans = self.batches = self.canonical_scans = 0
        self.digest = hashlib.sha256()

    def enqueue(self, side, pixels, actor, registry, raw, seeds, cursor, expected, *, link=0,
                coarse=0, gate=0, selected=None, clock=0, last_voice=0):
        key = side, pixels, self.canonical
        if key != self.key or len(self.cases) >= 128:
            self.flush()
            self.key = key
            self.side, self.pixels = side, pixels
        old = physical(struct.unpack_from('<H', raw[actor], 0x97)[0])
        header = struct.pack('<6H3B4HH', actor, actor if selected is None else physical(selected),
                             clock, gate, last_voice, old, link, coarse, cursor, *seeds, len(raw))
        assert len(header) == 25
        bindings = b''.join(struct.pack('<HH', slot, value) for slot, value in registry)
        bodies = b''.join(struct.pack('<H', slot) + state for slot, state in sorted(raw.items()))
        self.cases.append(header + bindings + bodies)
        self.expected.append(output(actor, expected, raw))
        self.scans += 1
        self.canonical_scans += self.canonical is not None

    def flush(self):
        if not self.cases:
            return
        self.path.write_bytes(struct.pack('<II', self.side, len(self.cases)) + self.pixels +
                              b''.join(self.cases))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command, *([str(self.key[2])] if self.key[2] else []), str(self.path)]
            result = subprocess.run(args, capture_output=True, text=True, timeout=180)
            if result.returncode or result.stdout != expected:
                observed, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((index for index in range(min(len(observed), len(desired)))
                                 if observed[index] != desired[index]), None)
                detail = ((observed[mismatch], desired[mismatch]) if mismatch is not None else
                          (len(observed), len(desired)))
                raise AssertionError(f'{args[:2]} exit {result.returncode}: {result.stderr}; '
                                     f'complete targets differ at {mismatch}: {detail}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.cases, self.expected = [], []


class TargetDiscoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-targets-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_target_discovery_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_target_discovery_probe.js')])
        cls.collector = Collector(pathlib.Path(cls.temp.name) / 'targets.bin', cls.commands)
        cls.evidence = {'scope': 'complete shared target discovery and reference repair; no parent/PCM',
                        'original': None}

    def tearDown(self):
        self.collector.flush()

    def constructed(self, kinds, *, side=1, pixels=b'\0', modes=None, link=0, automatic=0,
                    positions=None, old_count=0, **inputs):
        slots, registry, raw = [0, 150], [(NONE, 0)] * 182, {}
        for index, kind in enumerate(kinds):
            extended = kind in (0, 1, 2, 3, 19)
            slot = slots[extended]
            slots[extended] += 1
            state = bytearray(251 if extended else 55)
            struct.pack_into('<H', state, 0, kind)
            pose = positions[index] if positions else ((0, 0, 4096) if index == 0 else (-4096, 0, 4096))
            struct.pack_into('<3i', state, 4, *pose)
            state[22:24] = bytes((4 if index == 0 else 12, 0 if index == 0 else 8))
            state[25] = modes[index] if modes else 0
            if kind < 4:
                struct.pack_into('<H', state, 0x40, automatic if index == 0 else 0)
                struct.pack_into('<H', state, 0x8e, 0x1234)
                state[0x94] = old_count if index == 0 else 0
            raw[slot], registry[index] = bytes(state), (slot, 1)
        actor = 150
        objects = {pointer(slot): state for slot, state in raw.items()}
        expected = discovery(TABLES, pointer(actor), raw[actor],
                             [0 if slot == NONE else pointer(slot) for slot, _ in registry],
                             objects, link, side, pixels, **inputs)
        self.collector.enqueue(side, pixels, actor, registry, raw, (1, 2, 32768, 65535), 3,
                               expected, link=link, **inputs)

    def test_all_class_views_effective_aim_height_edges_and_type26_modes(self):
        for kind in range(4):
            for candidate in range(28):
                aim = TABLES.variant_heights[0] if candidate == 26 else TABLES.target_heights[candidate]
                middle = (8192 + TABLES.source_heights[kind] + aim) // 2
                for height in (max(0, middle // 256 - 1), middle // 256, middle // 256 + 1):
                    self.constructed([kind, candidate], pixels=bytes([height]))
            for terrain in range(24, 32):
                pixels = bytes([terrain])
                for mode in range(terrain - 24, 256, 8):
                    self.constructed([kind, 26], modes=(255, mode), pixels=pixels)

    def test_range_directional_ties_priority_secondary_and_wrapped_altitude(self):
        for kind in range(4):
            for link in (0, 1, 2, 255):
                limit = 1000 if link < 2 else 150
                for offset in (-1, 0, 255, 256):
                    self.constructed([kind, 0, 5], link=link, positions=(
                        (0, 0, 4096), (-(limit * 256 + offset), 0, 4096), (-1024, 0, 4096)))
            for x in (-262145, -262144, -262143, -1, 0, 1, 262143, 262144, 262145,
                      -2147483648, 2147483647):
                self.constructed([kind, 0, 5], positions=((0, 0, 4096), (x, x, 4096), (-1024, 0, 4096)))
            preferred, other = (0, 5) if kind % 2 == 0 else (5, 0)
            for coarse in (0, 1, 255):
                self.constructed([kind, other, preferred], coarse=coarse, positions=(
                    (0, 0, 4096), (-256, 0, 4096), (-260000, 0, 4096)))
                self.constructed([kind, preferred, other], coarse=coarse, positions=(
                    (0, 0, 4096), (-4096, 0, 4096), (262144, 0, 4096)))
        for altitude in (-2147483648, -65536, -1, 0, 65535, 65536, 2147483647):
            self.constructed([0, 26], positions=((0, 0, altitude), (-4096, 0, altitude)))

    def test_logical_notification_admission_and_counter_edges(self):
        for clock, last in ((0, 0), (29, 0), (30, 0), (31, 0),
                            (0, 65506), (0, 65507), (0, 1), (65535, 65505)):
            for gate in (0, 65535):
                for selected in (None, 0):
                    for count in (0, 1, 255):
                        self.constructed([0, 0], gate=gate, selected=selected, clock=clock,
                                         last_voice=last, old_count=count)

    def test_malformed_requests_missing_inputs_and_incomplete_batches(self):
        path = self.collector.path
        valid = struct.pack('<II', 1, 0) + b'\0'
        invalid = [valid[:index] for index in range(len(valid))] + [valid + b'x',
                    struct.pack('<II', 3, 0) + bytes(9),
                    struct.pack('<II', 1, 1) + b'\0']
        for data in invalid:
            path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, b''), result.stderr)
        for command in self.commands:
            for args in ([], [str(path.parent / 'missing')], [str(path), 'extra', 'extra']):
                result = subprocess.run([*command, *args], capture_output=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, b''), result.stderr)

    def test_required_complete_original_and_canonical_all_47_missions(self):
        if not ORACLE:
            self.skipTest('Complete pinned original gate requested separately')
        from original_object_pool_oracle import REGISTRY
        from original_unit_oracle import DGROUP
        from test_original_target_discovery import TargetDiscoveryTests as OriginalTests
        collector = self.collector

        class RequiredTests(OriginalTests):
            def check(self, machine, actor, **inputs):
                used = bytes(machine.mem_read(DGROUP + 0xe2f7, 150)) + bytes(
                    machine.mem_read(DGROUP + 0xe38d, 32))
                raw = {slot: self.owner.raw(machine, pointer(slot))
                       for slot, active in enumerate(used) if active}
                entries = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))
                registry = [(physical(address), value) for address, value in zip(entries[::2], entries[1::2])]
                seeds, cursor = self.owner.random_state(machine)
                result = super().check(machine, actor, **inputs)
                side, pixels = inputs.get('side', 512), inputs.get('pixels', self.flat)
                context = {key: value for key, value in inputs.items() if key not in ('kernel', 'side', 'pixels')}
                collector.enqueue(side, pixels, physical(actor), registry, raw, seeds, cursor,
                                  result, link=machine.mem_read(DGROUP + 0x6dae, 1)[0], **context)
                return result

            def begin_corpus_mission(self, name, side):
                collector.flush()
                collector.canonical = ROOT / 'armoredfist/FISTDATA' / name

            def test_all_pinned_missions_after_actual_preparation_on_four_details(self):
                try:
                    super().test_all_pinned_missions_after_actual_preparation_on_four_details()
                finally:
                    collector.flush()
                    collector.canonical = None

        result = unittest.TestResult()
        unittest.defaultTestLoader.loadTestsFromTestCase(RequiredTests).run(result)
        collector.flush()
        self.assertEqual((result.testsRun, result.skipped, result.failures, result.errors),
                         (7, [], [], []))
        self.evidence['original'] = {**RequiredTests.evidence,
                                    'scans': RequiredTests.scans, 'transfers': RequiredTests.transfers,
                                    'requests': RequiredTests.requests,
                                    'output_sha256': RequiredTests.digest.hexdigest(),
                                    'groups': result.testsRun, 'skips': 0}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default=TARGET)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--oracle', action='store_true')
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORACLE, ORIGINALS, REVIEW = (
        args.build_root, args.target, args.native_probe, args.oracle, args.originals, args.review_dir)
    if ORACLE and not ORIGINALS:
        parser.error('Required original coverage also requires --originals')
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or
                   REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Temporary evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    collector = TargetDiscoveryTests.collector
    success = program.result.wasSuccessful() and len(program.result.skipped) == int(not ORACLE)
    evidence = TargetDiscoveryTests.evidence
    evidence['gate'] = {'success': success, 'groups': program.result.testsRun,
                        'skips': len(program.result.skipped), 'scans_per_target': collector.scans,
                        'batches_per_target': collector.batches,
                        'canonical_mission_scans_per_target': collector.canonical_scans,
                        'complete_output_sha256': collector.digest.hexdigest(), 'target': TARGET}
    print(json.dumps(evidence['gate'], sort_keys=True), flush=True)
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'c-target-discovery.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
