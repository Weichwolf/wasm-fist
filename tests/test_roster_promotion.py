#!/usr/bin/env python3
"""Shared complete physical roster promotion and proved used-domain repair."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from original_unit_oracle import DGROUP
from roster_promotion_contract import ROSTER, promote, store, word
from test_target_discovery import NONE, physical, pointer
from test_units import snapshot

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = REVIEW = None
ORACLE = ORIGINALS = False
SEEDS = (1, 2, 32768, 65535)
BATCH = 2048


def expected_line(data, actor, addresses, status):
    values = [status, addresses[actor]]
    values += [NONE if not word(data, ROSTER + 2 * n) else addresses.get(word(data, ROSTER + 2 * n), physical(word(data, ROSTER + 2 * n)))
               for n in range(32)]
    for address, slot in sorted(addresses.items(), key=lambda pair: pair[1]):
        kind = word(data, address)
        if kind in (0, 1, 2, 3, 19, 23):
            values += [slot, kind, data[address + (36 if kind == 23 else 28)],
                       0 if kind == 23 else word(data, address + 0x40)]
    return 'promotion ' + ' '.join(map(str, values)) + '\n'


def typed_status(data, actor, addresses):
    member, platoon = data[actor + 28], data[actor + 27]
    if member == 0:
        return 0
    if member >= 4 or platoon >= 8:
        return -1
    predecessor = word(data, ROSTER + 2 * (platoon * 4 + member - 1))
    if predecessor and predecessor not in addresses:
        return -1
    if predecessor and word(data, predecessor) != 23 and not data[actor + 25] & 0x16:
        if word(data, predecessor) not in (0, 1, 2, 3, 19):
            return -1
    return 0


class Collector:
    def __init__(self, commands, temporary):
        self.commands = commands
        self.path = pathlib.Path(temporary) / 'cases.bin'
        self.canonical = None
        self.side, self.pixels = 1, b'\0'
        self.cases, self.expected = [], []
        self.count = self.rejections = self.batches = self.canonical_count = 0
        self.digest = hashlib.sha256()

    def begin(self, name=None, side=1, pixels=b'\0'):
        self.flush()
        self.canonical = ROOT / 'armoredfist/FISTDATA' / name if name else None
        self.side, self.pixels = side, pixels

    def enqueue(self, before, actor, addresses, registry, *, repeats=1, original_after=None):
        raw = {slot: before[address:address + (251 if word(before, address) in (0, 1, 2, 3, 19) else 55)]
               for address, slot in addresses.items()}
        seeds = struct.unpack_from('<4H', before, 0x1f84)
        cursor = ((word(before, 0x1f82) - 0x1f84) // 2 + 1) % 4
        roster = [NONE if not word(before, ROSTER + n * 2) else addresses.get(word(before, ROSTER + n * 2), 181)
                  for n in range(32)]
        body = struct.pack('<6H3B4HH', addresses[actor], NONE, 0, 0, 0, NONE,
                           0, 0, cursor, *seeds, len(raw))
        body += struct.pack('<B3H32H', 32, repeats, NONE, NONE, *roster)
        body += b''.join(struct.pack('<HH', *entry) for entry in registry)
        body += b''.join(struct.pack('<H', slot) + value for slot, value in sorted(raw.items()))
        self.cases.append(body)
        state = bytearray(before)
        for _ in range(repeats):
            status = typed_status(state, actor, addresses)
            if status == 0:
                state = promote(state, actor)
                if original_after is not None:
                    for address in addresses:
                        length = 251 if word(state, address) in (0, 1, 2, 3, 19) else 55
                        assert state[address:address + length] == original_after[address:address + length]
                    assert state[ROSTER:ROSTER + 64] == original_after[ROSTER:ROSTER + 64]
            self.expected.append(expected_line(state, actor, addresses, status))
            self.count += 1
            self.rejections += status != 0
            self.canonical_count += self.canonical is not None
            if status:
                break
        if len(self.cases) >= BATCH:
            self.flush()

    def flush(self):
        if not self.cases:
            return
        self.path.write_bytes(struct.pack('<II', self.side, len(self.cases)) + self.pixels + b''.join(self.cases))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command, '--promotion', *([str(self.canonical)] if self.canonical else []), str(self.path)]
            result = subprocess.run(args, capture_output=True, text=True, timeout=180)
            if result.returncode or result.stdout != expected:
                actual, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((n for n in range(min(len(actual), len(desired))) if actual[n] != desired[n]),
                                min(len(actual), len(desired)))
                raise AssertionError(f'{args[:2]} exit {result.returncode}: {result.stderr}; '
                                     f'complete roster differs at {mismatch}: '
                                     f'{actual[mismatch:mismatch+1]} != {desired[mismatch:mismatch+1]}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.cases, self.expected = [], []


def fixture(kind=0, predecessor_kind=0, *, platoon=0, member=1, actor_flags=0,
            predecessor_flags=16, predecessor_member=0, presence=1, control=65535):
    actor = pointer(150)
    predecessor_slot = 0 if predecessor_kind == 23 else 151
    previous = pointer(predecessor_slot)
    data = bytearray(65536)
    raw_actor = bytearray(snapshot(kind, platoon=platoon, member=member))
    raw_actor[25] = actor_flags
    store(raw_actor, 0x40, control)
    raw_previous = bytearray(snapshot(predecessor_kind, platoon=platoon, member=predecessor_member))
    raw_previous[25] = predecessor_flags
    data[actor:actor + len(raw_actor)] = raw_actor
    data[previous:previous + len(raw_previous)] = raw_previous
    addresses = {actor: 150, previous: predecessor_slot}
    if member:
        index = ((platoon * 4) & 0xff00) | ((platoon * 4 + member) & 255)
        store(data, ROSTER + 2 * index - 2, (0, previous, actor, pointer(181))[presence])
        store(data, ROSTER + 2 * index, actor)
    struct.pack_into('<5H', data, 0x1f82, 0x1f8a, *SEEDS)
    registry = [(NONE, 0)] * 182
    registry[150], registry[predecessor_slot] = (150, 1), (predecessor_slot, 1)
    return data, actor, addresses, registry


class PromotionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='fist-roster-', dir='/tmp')
        commands = []
        if TARGET in ('all', 'native'):
            commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_target_discovery_probe')])
        if TARGET in ('all', 'wasm'):
            commands.append(['node', str(BUILD / 'wasm/fist_target_discovery_probe.js')])
        cls.collector = Collector(commands, cls.temp.name)
        cls.original = None

    @classmethod
    def tearDownClass(cls):
        cls.collector.flush()
        cls.temp.cleanup()

    def test_full_actor_predecessor_flag_domains_and_control_word_domain(self):
        masks = (0, 2, 4, 6, 16, 18, 20, 22)
        for kind, previous_kind, full, masked in itertools.product(range(4), (0, 1, 2, 3, 19), range(256), masks):
            for actor_flags, previous_flags in ((full, masked), (masked, full)):
                self.collector.enqueue(*fixture(kind, previous_kind, actor_flags=actor_flags,
                    predecessor_flags=previous_flags, platoon=full % 8, member=1 + (full // 8) % 3))
        for control in range(65536):
            self.collector.enqueue(*fixture(control % 4, control=control, platoon=(control >> 2) % 8,
                member=1 + (control >> 5) % 3))

    def test_null_wreck_alias_repeated_promotion_and_leader_unused_domains(self):
        for kind, platoon, member, flags, previous_member in itertools.product(
                range(4), range(8), range(4), (0, 2, 4, 16, 255), (0, 1, 3, 254, 255)):
            for previous_kind, presence in ((0, 0), (23, 1), (0, 2)):
                self.collector.enqueue(*fixture(kind, previous_kind, platoon=platoon, member=member,
                    actor_flags=flags, predecessor_member=previous_member, presence=presence), repeats=4)
        for kind, platoon in itertools.product(range(4), range(256)):
            self.collector.enqueue(*fixture(kind, platoon=platoon, member=0))

    def test_used_member_index_and_missing_predecessor_failures_are_atomic(self):
        for kind, platoon, member in itertools.product(range(4), range(8), range(4, 256)):
            self.collector.enqueue(*fixture(kind, platoon=platoon, member=member, presence=0))
        for kind, platoon in itertools.product(range(4), range(8, 256)):
            self.collector.enqueue(*fixture(kind, platoon=platoon, member=1, presence=0))
        for kind, member, flags in itertools.product(range(4), range(1, 4), (0, 255)):
            self.collector.enqueue(*fixture(kind, member=member, actor_flags=flags, presence=3))

    def test_incomplete_transport_fails(self):
        data, actor, addresses, registry = fixture()
        self.collector.enqueue(data, actor, addresses, registry)
        self.collector.flush()
        valid = self.collector.path.read_bytes()
        for content in (b'', valid[:-1], valid + b'\0'):
            self.collector.path.write_bytes(content)
            for command in self.collector.commands:
                result = subprocess.run([*command, '--promotion', str(self.collector.path)], capture_output=True, timeout=30)
                self.assertNotEqual(result.returncode, 0)

    def test_required_complete_original_and_actual_canonical_consumers(self):
        if not (ORACLE and ORIGINALS):
            self.skipTest('Complete pinned original gate requested separately')
        from test_original_roster_promotion import ACTOR, PREDECESSOR, PromotionTests as OriginalTests
        collector = self.collector

        class RequiredTests(OriginalTests):
            def begin_prepared_mission(self, name, side, pixels, machine, objects):
                collector.begin(name, side, pixels)

            def check(self, machine, actor, *, through_parent=False):
                before = bytes(machine.mem_read(DGROUP, 65536))
                actual, changed = super().check(machine, actor, through_parent=through_parent)
                if through_parent:
                    return actual, changed  # Complete original parent remains part of the required gate.
                if actor == ACTOR:
                    collector.begin() if collector.canonical else None
                    kind = word(before, PREDECESSOR)
                    addresses = {ACTOR: 150, PREDECESSOR: 0 if kind == 23 else 151}
                    registry = [(NONE, 0)] * 182
                    for slot in addresses.values():
                        registry[slot] = (slot, 1)
                else:
                    # Actual current physical arena, including registry-overwritten live orphans.
                    used = before[0xe2f7:0xe2f7 + 150] + before[0xe38d:0xe38d + 32]
                    addresses = {pointer(slot): slot for slot, alive in enumerate(used) if alive}
                    bindings = struct.unpack_from('<364H', before, 0xdfbc)
                    registry = [(physical(bindings[n * 2]), bindings[n * 2 + 1]) for n in range(182)]
                    if collector.canonical and actor not in addresses:
                        raise AssertionError('Canonical actor is not a live original allocation')
                collector.enqueue(before, actor, addresses, registry, original_after=actual)
                return actual, changed

        result = unittest.TextTestRunner(stream=__import__('sys').stderr).run(unittest.defaultTestLoader.loadTestsFromTestCase(RequiredTests))
        self.assertTrue(result.wasSuccessful())
        self.assertEqual(result.testsRun, 5)
        self.assertFalse(result.skipped)
        self.assertEqual(RequiredTests.calls, 110451)
        self.assertEqual(RequiredTests.digest.hexdigest(), 'bfcc7a60796d740c9279def6e9b79110e4a3bfc664c66cc23b2416ab6ad37473')
        type(self).original = {**RequiredTests.evidence, 'groups': 5, 'skips': 0,
                              'complete_returns': RequiredTests.calls, 'complete_parent_returns': RequiredTests.parents,
                              'output_sha256': RequiredTests.digest.hexdigest()}
        collector.flush()
        collector.begin()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--oracle', action='store_true')
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    TARGET, BUILD, NATIVE_PROBE = args.target, args.build_root, args.native_probe
    ORACLE, ORIGINALS, REVIEW = args.oracle, args.originals, args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 5
    if ORACLE or ORIGINALS:
        success = success and ORACLE and ORIGINALS and not program.result.skipped
    collector = PromotionTests.collector
    evidence = {'success': bool(success), 'groups': program.result.testsRun, 'skips': len(program.result.skipped),
                'target': TARGET, 'cases_per_target': collector.count, 'atomic_rejections': collector.rejections,
                'canonical_returns_per_target': collector.canonical_count, 'batches_per_target': collector.batches,
                'output_sha256': collector.digest.hexdigest(), 'original': PromotionTests.original,
                'scope': 'Complete shared roster promotion; full parent/class/battle/PCM remain open', 'complete_wasm_streak': 0}
    print(json.dumps({k:v for k,v in evidence.items() if k != 'original'}, sort_keys=True), flush=True)
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'c-roster-promotion.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
