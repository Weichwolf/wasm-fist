#!/usr/bin/env python3
"""Complete shared obstacle callbacks against independent and unchanged original returns."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from ground_maneuver_contract import OFFSETS, idle_turret, maneuver, motion_obstacle
from original_unit_oracle import DGROUP
from roster_promotion_contract import store, word
from test_target_discovery import NONE, physical, pointer
from test_units import snapshot
from test_vehicle_motion import rotate, start

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = REVIEW = None
ORACLE = ORIGINALS = False
SEEDS = (1, 2, 32768, 65535)
OPERATIONS = {'maneuver': 64, 'idle_turret': 65, 'motion_obstacle': 66}
MODELS = {'maneuver': maneuver, 'idle_turret': idle_turret, 'motion_obstacle': motion_obstacle}


def seed_for_draw(draw):
    following = (draw + 1) & 65535
    odd = following >> 15
    return (((following ^ (0xb400 if odd else 0)) << 1) | odd) & 65535


def observed_line(data, actor):
    cursor = ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4
    values = [0, data[actor + 0x45], data[actor + 0x46], data[actor + 0x51],
              word(data, actor + 0x47), word(data, actor + 0x40), word(data, actor + 0x8b),
              cursor, *struct.unpack_from('<4H', data, 0x1f84)]
    return 'maneuver ' + ' '.join(map(str, values)) + '\n'


class Collector:
    def __init__(self, commands, temporary):
        self.commands = commands
        self.path = pathlib.Path(temporary) / 'cases.bin'
        self.canonical = None
        self.side, self.pixels = 1, b'\0'
        self.cases, self.expected = [], []
        self.count = self.batches = self.canonical_count = 0
        self.states, self.exits, self.draws = collections.Counter(), collections.Counter(), collections.Counter()
        self.digest = hashlib.sha256()

    def begin(self, name=None, side=1, pixels=b'\0'):
        self.flush()
        self.canonical = ROOT / 'armoredfist/FISTDATA' / name if name else None
        self.side, self.pixels = side, pixels

    def enqueue(self, before, actor, addresses, registry, *, operation='maneuver', candidate=0,
                original_after=None, stimulus=False):
        seeds = struct.unpack_from('<4H', before, 0x1f84)
        cursor = ((word(before, 0x1f82) - 0x1f84) // 2 + 1) % 4
        body = struct.pack('<6H3B4HH', addresses[actor], NONE, 0, 0, 0, NONE,
                           0, before[0x2040], cursor, *seeds, len(addresses))
        body += struct.pack('<B3H', OPERATIONS[operation], int(stimulus), addresses.get(candidate, NONE), NONE)
        body += b''.join(struct.pack('<HH', *entry) for entry in registry)
        for address, slot in sorted(addresses.items(), key=lambda pair: pair[1]):
            length = 251 if word(before, address) in (0, 1, 2, 3, 19) else 55
            body += struct.pack('<H', slot) + before[address:address + length]
        model = MODELS[operation]
        after, effect = model(before, actor, candidate) if operation == 'motion_obstacle' else model(before, actor)
        if original_after is not None:
            self.assert_original(after, original_after, addresses)
        self.cases.append(body)
        self.expected.append(observed_line(after, actor))
        self.count += 1
        self.canonical_count += self.canonical is not None
        if 'state' in effect:
            self.states[effect['state']] += 1
        if effect.get('search_index') is not None:
            self.exits[effect['search_index']] += 1
        if 'random_draws' in effect:
            self.draws[effect['random_draws']] += 1
        if len(self.cases) >= 2048:
            self.flush()

    @staticmethod
    def assert_original(after, actual, addresses):
        for address in addresses:
            length = 251 if word(after, address) in (0, 1, 2, 3, 19) else 55
            if after[address:address + length] != actual[address:address + length]:
                raise AssertionError('Complete original actor/body bytes differ from independent model')
        if after[0x1f82:0x1f8c] != actual[0x1f82:0x1f8c]:
            raise AssertionError('Complete original RNG differs from independent model')

    def flush(self):
        if not self.cases:
            return
        self.path.write_bytes(struct.pack('<II', self.side, len(self.cases)) + self.pixels + b''.join(self.cases))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command, '--maneuver']
            if self.canonical:
                args.append(str(self.canonical))
            args.append(str(self.path))
            result = subprocess.run(args, capture_output=True, text=True, timeout=120)
            if result.returncode or result.stdout != expected:
                actual, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((n for n in range(min(len(actual), len(desired))) if actual[n] != desired[n]),
                                min(len(actual), len(desired)))
                raise AssertionError(f'{args[:2]} exit {result.returncode}: {result.stderr}; '
                                     f'complete maneuver differs at {mismatch}: '
                                     f'{actual[mismatch:mismatch+1]} != {desired[mismatch:mismatch+1]}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.cases, self.expected = [], []


def fixture(kind=0, *, selector=0, remaining=0, blocked_count=0, flags=0, heading=0,
            turret_heading=0, pose=(0, 0), scale=1024, velocity=(0, 0), target=0,
            cursor=0, first_draw=None, second_draw=None, coarse=0):
    actor = pointer(150)
    data = bytearray(65536)
    raw = bytearray(start(kind))
    raw[0x45], raw[0x46], raw[0x51], raw[22] = selector, remaining, blocked_count, 64
    struct.pack_into('<2i', raw, 4, *pose)
    for offset, value in ((0x40, flags), (0x26, heading), (0x10, turret_heading),
                          (0x14, scale), (0x97, target), (0x8b, 12345)):
        store(raw, offset, value)
    struct.pack_into('<2h', raw, 0x59, *velocity)
    data[actor:actor + len(raw)] = raw
    seeds = list(SEEDS)
    if first_draw is not None:
        seeds[cursor] = seed_for_draw(first_draw)
    if second_draw is not None:
        seeds[(cursor + 1) % 4] = seed_for_draw(second_draw)
    struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *seeds)
    data[0x2040] = coarse
    registry = [(NONE, 0)] * 182
    registry[150] = (150, 1)
    store(data, 0xdfbc + 150 * 4, actor)
    return data, actor, {actor: 150}, registry


def add_body(context, kind=26, *, flags=64, pose=(0, 3000), scale=1024, slot=0, registry_index=0):
    data, actor, addresses, registry = context
    address = pointer(slot)
    raw = bytearray(snapshot(kind, flags=flags, pose=(*pose, 0)))
    store(raw, 0x14, scale)
    data[address:address + len(raw)] = raw
    addresses[address] = slot
    registry[registry_index] = (slot, 1)
    store(data, 0xdfbc + registry_index * 4, address)
    return address


def physical_context(data):
    used = data[0xe2f7:0xe2f7 + 150] + data[0xe38d:0xe38d + 32]
    addresses = {pointer(slot): slot for slot, alive in enumerate(used) if alive}
    bindings = struct.unpack_from('<364H', data, 0xdfbc)
    registry = [(physical(bindings[n * 2]), bindings[n * 2 + 1]) for n in range(182)]
    return addresses, registry


class ManeuverTests(unittest.TestCase):
    original = None

    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='fist-maneuver-', dir='/tmp')
        commands = []
        if TARGET in ('all', 'native'):
            commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_target_discovery_probe')])
        if TARGET in ('all', 'wasm'):
            commands.append(['node', str(BUILD / 'wasm/fist_target_discovery_probe.js')])
        cls.collector = Collector(commands, cls.temp.name)

    @classmethod
    def tearDownClass(cls):
        cls.collector.flush()
        cls.temp.cleanup()

    def test_01_selector_count_blocked_control_domains(self):
        for selector, remaining in itertools.product(range(256), repeat=2):
            self.collector.enqueue(*fixture((selector + remaining) % 4, selector=selector,
                remaining=remaining, flags=8 if remaining & 1 else 0, blocked_count=selector))
        for flags in range(65536):
            self.collector.enqueue(*fixture(flags % 4, selector=flags & 255, remaining=flags >> 8,
                                           flags=flags, blocked_count=(flags * 17) & 255))
        for kind, blocked, remaining in itertools.product(range(4), range(256), (0, 1, 2, 255)):
            self.collector.enqueue(*fixture(kind, flags=8, remaining=remaining, blocked_count=blocked))
        self.collector.flush()
        self.assertEqual(set(self.collector.states), {0, 2, 4, 6})

    def test_02_complete_first_second_draws_and_presence_words(self):
        for value in range(65536):
            self.collector.enqueue(*fixture(value % 4, cursor=value % 4, first_draw=value,
                                           second_draw=value ^ 65535), operation='idle_turret')
            self.collector.enqueue(*fixture(value % 4, cursor=(value >> 2) % 4, first_draw=65,
                                           second_draw=value), operation='idle_turret')
            self.collector.enqueue(*fixture(value % 4, flags=value, first_draw=value,
                                           cursor=value % 4), operation='idle_turret')
            self.collector.enqueue(*fixture(value % 4, target=value, first_draw=0,
                                           cursor=value % 4), operation='idle_turret')
        self.collector.flush()
        self.assertEqual(set(self.collector.draws), {0, 1, 2})

    def test_03_wrapped_radius_coordinates_vectors_and_filters(self):
        for scale in range(65536):
            context = fixture(scale % 4, scale=scale, flags=scale)
            candidate = add_body(context, pose=(0, 4608))
            self.collector.enqueue(*context, operation='motion_obstacle', candidate=candidate)
        for kind, coarse, selector, pose, velocity in itertools.product(range(4), (0, 1, 255),
                (0, 1, 2, 255), ((0, 0), (-2**31, 2**31 - 1), (65535, -65537)),
                ((-32768, 32767), (-1, 0), (0, 0), (1, -1), (64, 64))):
            context = fixture(kind, pose=pose, selector=selector, heading=kind * 16384 + 65535,
                              velocity=velocity, coarse=coarse, turret_heading=32768)
            from test_geometry import signed
            candidate = add_body(context, 23, pose=tuple(signed(a + b) for a, b in zip(pose, (65536, -65537))), scale=65535)
            self.collector.enqueue(*context, operation='motion_obstacle', candidate=candidate)
        for kind, body_kind, flags in itertools.product(range(4), range(28), (0, 1, 8, 64, 65, 255)):
            context = fixture(kind, selector=2, remaining=1, turret_heading=32768)
            add_body(context, body_kind, flags=flags, pose=(0, 0), slot=151 if body_kind in (0, 1, 2, 3, 19) else 0)
            self.collector.enqueue(*context)
        self.collector.flush()

    def test_04_every_search_exit_and_competing_bodies(self):
        for kind, coarse, blocked in itertools.product(range(4), (0, 1, 255), range(16)):
            context = fixture(kind, selector=2, remaining=1, coarse=coarse)
            for index in range(blocked):
                point = tuple(value * 72 for value in rotate(OFFSETS[index], 32, coarse))
                add_body(context, pose=point, scale=63488, slot=index, registry_index=index)
            self.collector.enqueue(*context)
        for kind, reverse in itertools.product(range(4), (0, 1)):
            context = fixture(kind, selector=2, remaining=1)
            for index, point in enumerate(((0, 3000), (400, 3000))):
                add_body(context, 26 + index, pose=point, slot=index,
                         registry_index=1 if index == reverse else 181)
            self.collector.enqueue(*context)
        self.collector.flush()
        self.assertEqual(set(self.collector.exits), set(range(16)))

    def test_05_incomplete_transport_fails(self):
        self.collector.enqueue(*fixture())
        self.collector.flush()
        valid = self.collector.path.read_bytes()
        for content in (b'', valid[:-1], valid + b'\0'):
            self.collector.path.write_bytes(content)
            for command in self.collector.commands:
                result = subprocess.run([*command, '--maneuver', str(self.collector.path)], capture_output=True, timeout=30)
                self.assertNotEqual(result.returncode, 0)

    def test_06_required_original_domains_canonical_consumers_search_retention(self):
        if not (ORACLE and ORIGINALS):
            self.skipTest('Complete pinned original gate requested separately')
        from test_original_ground_maneuver import ACTOR, CANDIDATE, ManeuverTests as OriginalTests
        from test_original_ground_maneuver_search import SearchTests
        from test_original_ground_maneuver_retention import RetentionTests
        collector = self.collector

        class RequiredTests(OriginalTests):
            last_child = None

            def begin_prepared_mission(self, name, side, pixels, machine, objects):
                collector.begin(name, side, pixels)
                self.last_child = bytes(machine.mem_read(DGROUP, 65536))

            def check(self, machine, actor=ACTOR, *, operation='maneuver', candidate=0):
                before = bytes(machine.mem_read(DGROUP, 65536))
                actual, effect = super().check(machine, actor, operation=operation, candidate=candidate)
                if operation == 'parent':
                    return actual, effect  # Full parent is observed, not installed as a partial bank.
                stimulus = False
                if actor == ACTOR:
                    if collector.canonical:
                        collector.begin()
                    addresses = {ACTOR: 150}
                    bindings = struct.unpack_from('<364H', before, 0xdfbc)
                    if CANDIDATE in bindings[::2] or operation == 'motion_obstacle':
                        addresses[CANDIDATE] = 151 if word(before, CANDIDATE) in (0, 1, 2, 3, 19) else 0
                    registry = [(NONE if not bindings[n * 2] else addresses[bindings[n * 2]], bindings[n * 2 + 1])
                                for n in range(182)]
                else:
                    addresses, registry = physical_context(before)
                    if collector.canonical and self.last_child is not None:
                        old = self.last_child[actor:actor + 251]
                        current = before[actor:actor + 251]
                        differences = {n for n, (a, b) in enumerate(zip(old, current)) if a != b}
                        stimulus = bool(differences) and differences <= {0x45, 0x46} and current[0x45:0x47] == b'\x02\x01'
                        if differences and not stimulus:
                            raise AssertionError('Unmodeled canonical actor stimulus')
                collector.enqueue(before, actor, addresses, registry, operation=operation,
                                  candidate=candidate, original_after=actual, stimulus=stimulus)
                self.last_child = actual
                return actual, effect

        runner = unittest.TextTestRunner(stream=__import__('sys').stderr)
        result = runner.run(unittest.defaultTestLoader.loadTestsFromTestCase(RequiredTests))
        self.assertTrue(result.wasSuccessful())
        self.assertEqual(result.testsRun, 5)
        self.assertFalse(result.skipped)
        self.assertEqual(RequiredTests.digest.hexdigest(), '0df24b58abe1665452fb757eaf9a00562b320da3b46cfb006ba608769311c8f8')
        collector.begin()
        from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
        observe = OriginalGroundManeuverOracle.observe

        def observe_search(owner, machine, actor, **kwargs):
            before = bytes(machine.mem_read(DGROUP, 65536))
            actual, effect = observe(owner, machine, actor, **kwargs)
            addresses, registry = physical_context(before)
            collector.enqueue(before, actor, addresses, registry, original_after=actual)
            return actual, effect

        try:
            OriginalGroundManeuverOracle.observe = observe_search
            search = runner.run(unittest.defaultTestLoader.loadTestsFromTestCase(SearchTests))
        finally:
            OriginalGroundManeuverOracle.observe = observe
        self.assertTrue(search.wasSuccessful())
        self.assertEqual(search.testsRun, 1)
        self.assertFalse(search.skipped)
        self.assertEqual(SearchTests.evidence['output_sha256'], 'c650016c4aade27c0a5fd1b5ff732e4a32dd53804d002816d336c2f464555bc6')
        retention = runner.run(unittest.defaultTestLoader.loadTestsFromTestCase(RetentionTests))
        self.assertTrue(retention.wasSuccessful())
        self.assertEqual(retention.testsRun, 2)
        self.assertFalse(retention.skipped)
        self.assertEqual(RetentionTests.digest.hexdigest(), '7330a93d4f8a9684b59e1f500ea50b684d770966cb8f18705d4110b5aba2f27e')
        type(self).original = {'groups': 8, 'skips': 0, 'counts': dict(RequiredTests.counts),
            'coverage': RequiredTests.groups, 'corpus': RequiredTests.corpus,
            'output_sha256': RequiredTests.digest.hexdigest(),
            'search': SearchTests.evidence, 'retention': RetentionTests.evidence,
            'retention_sha256': RetentionTests.digest.hexdigest()}
        collector.flush()


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
    success = program.result.wasSuccessful() and program.result.testsRun == 6
    if ORACLE or ORIGINALS:
        success = success and ORACLE and ORIGINALS and not program.result.skipped
    collector = ManeuverTests.collector
    evidence = {'success': bool(success), 'groups': program.result.testsRun, 'skips': len(program.result.skipped),
        'target': TARGET, 'cases_per_target': collector.count, 'batches_per_target': collector.batches,
        'canonical_returns_per_target': collector.canonical_count, 'maneuver_states': dict(collector.states),
        'search_indices': dict(collector.exits), 'idle_draw_counts': dict(collector.draws),
        'output_sha256': collector.digest.hexdigest(), 'original': ManeuverTests.original,
        'scope': 'Complete shared maneuvers/idle turret/obstacle producer; full parent/class/battle/PCM remain open',
        'complete_wasm_streak': 0}
    print(json.dumps({k:v for k,v in evidence.items() if k != 'original'}, sort_keys=True), flush=True)
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'c-ground-maneuver.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
