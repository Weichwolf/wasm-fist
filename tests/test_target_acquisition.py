#!/usr/bin/env python3
"""Shared complete acquisition, safe notices, lifetime repair and consuming aim/throttle."""
import argparse
import copy
import hashlib
import itertools
import json
import pathlib
import random
import struct
import subprocess
import tempfile
import types
import unittest

from ground_throttle_contract import PROFILE_COMPONENTS, throttle
from target_acquisition_contract import ACQUISITION_THRESHOLDS, acquisition, target_geometry
from target_discovery_contract import TABLES, discovery
from test_geometry import signed
from test_target_discovery import NONE, physical, pointer
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = REVIEW = None
ORACLE = ORIGINALS = False
ACQUIRE, AUTOMATIC, AIM, THROTTLE, DISCOVER = 1, 2, 4, 8, 16
BATCH = 4096
SEEDS = (1, 2, 32768, 2)
# Text handles are deliberately absent from C. These distinct model tokens only
# make the original semantic notification branch observable to the test model.
MODEL_TABLES = types.SimpleNamespace(**vars(TABLES),
    enemy_text=(*range(26), 65535, 27), friendly_text=(*range(26), 65535, 27),
    variant_text=tuple(range(100, 356)))


def word(raw, offset):
    return struct.unpack_from('<H', raw, offset)[0]


def semantic(case, original=None):
    raw = copy.copy(case['raw'])
    actor = case['actor']
    state = bytearray(raw[actor])
    seeds, cursor = list(case['seeds']), case['cursor']
    notice = (77, 65535, 255, 1)
    attempted = installed = message = False
    voice = (0, 0, 0)
    last = case['last']
    status = 0
    refresh = False
    operation = case['operation']
    candidate = case['candidate']
    if operation & DISCOVER:
        objects = {pointer(slot): value for slot, value in raw.items()}
        scan = discovery(MODEL_TABLES, pointer(actor), bytes(state),
                         [0 if slot == NONE else pointer(slot) for slot, _ in case['registry']],
                         objects, case['link'], case['side'], case['pixels'],
                         coarse=case['coarse'], selected=case['selected'], gate=case['gate'],
                         clock=case['clock'], last_voice=last)
        state = bytearray(scan['actor'])
        last = scan['last_voice']
        candidate = physical(scan['primary'])
    initial = bytes(state)
    if operation & ACQUIRE:
        # Issued runtime identities reject unavailable saved candidates/targets.
        for offset in (0x97, 0x9d):
            if word(state, offset) and physical(word(state, offset)) not in raw:
                struct.pack_into('<H', state, offset, 0)
        automatic = bool(operation & AUTOMATIC)
        requested = physical(word(state, 0x9d)) if automatic else candidate
        if requested not in raw:
            requested = NONE
        used_behavior = automatic and state[0x94] and word(state, 0x97) == 0
        if used_behavior and case['behavior'] >= 4:
            status = -1
        else:
            objects = {pointer(slot): value for slot, value in raw.items()}
            expected = acquisition(MODEL_TABLES, pointer(actor), bytes(state), objects,
                                   (seeds, cursor), case['behavior'], case['side'], case['pixels'],
                                   automatic=automatic, candidate=0 if requested == NONE else pointer(requested),
                                   selected=case['selected'], gate=case['gate'], clock=case['clock'],
                                   last_voice=last, display=(77, 0x1234))
            attempted = (not automatic or
                         expected['draw'] is not None and
                         expected['draw'] & 255 <= ACQUISITION_THRESHOLDS[case['behavior']])
            installed = bool(expected['transfer'] and expected['transfer']['visible'])
            message = bool(expected['messages'])
            if message and word(raw[requested], 0) == 26 and raw[requested][25] > 3:
                status = -1
            else:
                state = bytearray(expected['actor'])
                seeds, cursor = expected['random']
                last = expected['last_voice']
                voice = expected['requests'][0] if expected['requests'] else voice
                if message:
                    target = raw[requested]
                    kind = word(target, 0)
                    notice = (120, kind, target[25] if kind == 26 else 0, int(bool(target[22] & 8)))
                if original is not None:
                    # Safe-domain complete original returns must prove the independent model.
                    assert expected['actor'] == original['actor']
                    assert tuple(expected['random'][0]) == tuple(original['random'][0])
                    assert cursor == original['random'][1]
                    assert expected['requests'] == original['requests']
                    assert last == original['last_voice']
                    assert expected['transfer'] == original['transfer']
                    assert len(expected['messages']) == len(original['messages'])
        if status:
            state = bytearray(initial)
            attempted = installed = message = False
    if status == 0 and operation & AIM:
        target = word(state, 0x97)
        if target and physical(target) not in raw:
            struct.pack_into('<H', state, 0x97, 0)
        state, _ = target_geometry(MODEL_TABLES, bytes(state),
                                  {pointer(slot): value for slot, value in raw.items()}, case['coarse'])
        state = bytearray(state)
    if status == 0 and operation & THROTTLE:
        state[0x43] = 6
        state, refresh = throttle(bytes(state), (case['behavior'], *([0] * 10)))
    values = (status, actor, physical(word(state, 0x97)), physical(word(state, 0x9d)),
              word(state, 0x40), word(state, 0x9b), word(state, 0x38), word(state, 0x99),
              state[0x94], state[25], word(state, 0x8e), *notice,
              int(attempted), int(installed), int(message), int(bool(voice[0])), *voice,
              last, cursor, *seeds, struct.unpack_from('<h', state, 0x57)[0], state[0x90],
              state[PROFILE_COMPONENTS[word(state, 0)]], int(refresh))
    return 'acquisition ' + ' '.join(map(str, values)) + '\n'


class Collector:
    def __init__(self, path, commands):
        self.path, self.commands = path, commands
        self.key = None
        self.canonical = None
        self.cases, self.expected = [], []
        self.count = self.batches = self.canonical_count = self.rejections = 0
        self.digest = hashlib.sha256()

    def enqueue(self, case, original=None):
        key = case['side'], case['pixels'], self.canonical
        if key != self.key or len(self.cases) >= BATCH:
            self.flush()
            self.key = key
        actor, raw = case['actor'], case['raw']
        header = struct.pack('<6H3B4HH', actor, physical(case['selected']), case['clock'],
                             case['gate'], case['last'], physical(word(raw[actor], 0x97)),
                             case['link'], case['coarse'], case['cursor'], *case['seeds'], len(raw))
        header += struct.pack('<B3H', case['operation'], case['behavior'],
                              physical(word(raw[actor], 0x9d)), case['candidate'])
        bindings = b''.join(struct.pack('<HH', slot, value) for slot, value in case['registry'])
        bodies = b''.join(struct.pack('<H', slot) + state for slot, state in sorted(raw.items()))
        self.cases.append(header + bindings + bodies)
        expected = semantic(case, original)
        self.expected.append(expected)
        self.rejections += expected.startswith('acquisition -1 ')
        self.count += 1
        self.canonical_count += self.canonical is not None

    def flush(self):
        if not self.cases:
            return
        self.path.write_bytes(struct.pack('<II', self.key[0], len(self.cases)) + self.key[1] +
                              b''.join(self.cases))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command, '--acquisition', *([str(self.key[2])] if self.key[2] else []), str(self.path)]
            result = subprocess.run(args, capture_output=True, text=True, timeout=180)
            if result.returncode or result.stdout != expected:
                observed, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((n for n in range(min(len(observed), len(desired)))
                                 if observed[n] != desired[n]), min(len(observed), len(desired)))
                raise AssertionError(f'{args[:2]} exit {result.returncode}: {result.stderr}; '
                                     f'complete acquisition differs at {mismatch}: '
                                     f'{observed[mismatch:mismatch+1]} != {desired[mismatch:mismatch+1]}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.cases, self.expected = [], []


def fixture(kind=0, target_kind=0, *, operation=ACQUIRE | AUTOMATIC | AIM | THROTTLE,
            count=1, behavior=0, old=NONE, candidate=None, target_mode=0,
            target_flags=12, secondary=8, actor_flags=4, control=1, seeds=SEEDS, cursor=3,
            clock=30, gate=65535, selected=True, last=0, profile=0, pitch=0,
            source=(0, 0, 4096), destination=(-4096, 0, 4096), pixels=b'\0', side=1,
            orphan=False):
    actor, target = 150, 151 if target_kind in (0, 1, 2, 3, 19) else 0
    raw_actor = bytearray(251)
    struct.pack_into('<H', raw_actor, 0, kind)
    struct.pack_into('<3i', raw_actor, 4, *source)
    raw_actor[22], raw_actor[25], raw_actor[0x94] = actor_flags, 255, count
    struct.pack_into('<h', raw_actor, 0x34, pitch)
    struct.pack_into('<H', raw_actor, 0x38, 0xabc1)
    struct.pack_into('<H', raw_actor, 0x40, control)
    struct.pack_into('<H', raw_actor, 0x53, 0xbeef)
    struct.pack_into('<h', raw_actor, 0x57, -32768)
    struct.pack_into('<H', raw_actor, 0x8e, 0x1234)
    raw_actor[0x90] = profile
    struct.pack_into('<H', raw_actor, 0x97, 0 if old == NONE else pointer(target if old == -1 else old))
    struct.pack_into('<H', raw_actor, 0x99, 0xdcba)
    struct.pack_into('<H', raw_actor, 0x9b, 0x1234)
    struct.pack_into('<H', raw_actor, 0x9d, pointer(target))
    raw_target = bytearray(251 if target >= 150 else 55)
    struct.pack_into('<H', raw_target, 0, target_kind)
    struct.pack_into('<3i', raw_target, 4, *destination)
    raw_target[22:24] = bytes((target_flags, secondary))
    raw_target[25] = target_mode
    registry = [(NONE, 0)] * 182
    registry[0] = actor, 1
    if not orphan:
        registry[1] = target, 1
    return dict(actor=actor, raw={actor: bytes(raw_actor), target: bytes(raw_target)},
                registry=registry, seeds=seeds, cursor=cursor, operation=operation,
                behavior=behavior, candidate=target if candidate is None else candidate,
                selected=pointer(actor) if selected else 0, gate=gate, clock=clock, last=last,
                coarse=0, link=0, pixels=pixels, side=side)


class TargetAcquisitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-acquisition-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        commands = []
        if TARGET in ('all', 'native'):
            commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_target_discovery_probe')])
        if TARGET in ('all', 'wasm'):
            commands.append(['node', str(BUILD / 'wasm/fist_target_discovery_probe.js')])
        cls.collector = Collector(pathlib.Path(cls.temp.name) / 'cases.bin', commands)
        cls.evidence = {'scope': 'complete shared acquisition/aim/throttle child boundaries; no living parent or PCM',
                        'original': None, 'complete_wasm_streak': 0}

    def tearDown(self):
        self.collector.flush()

    def test_every_rng_word_all_behavior_choices_and_rotating_cursors(self):
        base = fixture(operation=ACQUIRE | AUTOMATIC, candidate=NONE)
        state = bytearray(base['raw'][150]); struct.pack_into('<H', state, 0x9d, 0)
        base['raw'][150] = bytes(state)
        for behavior in range(4):
            for value in range(65536):
                case = copy.copy(base)
                cursor = value % 4
                seeds = list(SEEDS); seeds[cursor] = value
                case.update(behavior=behavior, seeds=seeds, cursor=cursor)
                self.collector.enqueue(case)
            print('Shared acquisition full RNG domain:', behavior, flush=True)

    def test_old_target_count_flags_and_lazy_descriptor_domains(self):
        for kind, count, old in itertools.product(range(4), range(256), (NONE, -1)):
            self.collector.enqueue(fixture(kind, count=count, old=old,
                                           behavior=65535 if not count or old != NONE else 0))
        for flags in (0, 1, 2, 4, 8, 127, 128, 129, 32768, 65535):
            for automatic in (0, AUTOMATIC):
                self.collector.enqueue(fixture(control=flags, operation=ACQUIRE | automatic))
        for behavior in (4, 255, 32768, 65535):
            self.collector.enqueue(fixture(behavior=behavior))
            self.collector.enqueue(fixture(behavior=behavior, operation=ACQUIRE | AIM | THROTTLE))

    def test_all_class_views_flag_bytes_variants_and_typed_notices(self):
        for kind, target, side in itertools.product(range(4), range(28), (0, 8)):
            self.collector.enqueue(fixture(kind, target, target_flags=4 | side))
        for kind, offset, flags in itertools.product(range(4), (22, 23), range(256)):
            case = fixture(kind, old=-1, operation=ACQUIRE | AIM | THROTTLE)
            target = case['candidate']; state = bytearray(case['raw'][target]); state[offset] = flags
            case['raw'][target] = bytes(state)
            self.collector.enqueue(case)
        for kind, mode, selected in itertools.product(range(4), range(256), (False, True)):
            self.collector.enqueue(fixture(kind, 26, target_mode=mode, selected=selected,
                                           operation=ACQUIRE | AIM | THROTTLE))
        for mode, flags, secondary in itertools.product(range(4, 256), (12, 13), (8, 64)):
            self.collector.enqueue(fixture(0, 26, target_mode=mode, target_flags=flags,
                                           secondary=secondary, old=-1))

    def test_selected_voice_gate_clock_wrap_side_and_visibility(self):
        for kind, side, actor_side, gate, selected, clock in itertools.product(
                range(4), (0, 8), (0, 8), (0, 1, 65535), (False, True), (0, 29, 30, 31, 65535)):
            self.collector.enqueue(fixture(kind, target_flags=4 | side, actor_flags=4 | actor_side,
                                           gate=gate, selected=selected, clock=clock,
                                           last=65520 if clock < 29 else 0))
        for kind, x, pixels in itertools.product(range(4), (-262145, -262144, -1, 0, 1, 262143, 262144),
                                                (b'\0', b'\xff')):
            self.collector.enqueue(fixture(kind, destination=(x, 0, 4096), pixels=pixels, old=-1,
                                           operation=ACQUIRE | AIM | THROTTLE))
            self.collector.enqueue(fixture(kind, destination=(x, 0, 4096), pixels=pixels))

    def test_geometry_wrapping_packed_range_and_consuming_profile(self):
        for kind, target, coarse, distance, pitch, profile in itertools.product(
                range(4), (0, 5, 26, 27), (0, 1, 255),
                (0, 1, 255, 256, 11488, 11520, 15359, 15360, 23039, 23040,
                 65535, 65536, 16776960, 16777216, 2147483647, -2147483648),
                (-32768, 3583, 3584, 32767), (0, 1, 2, 255)):
            case = fixture(kind, target, old=-1, operation=AIM | THROTTLE,
                           source=(2147483647, -2147483648, 4096),
                           destination=(signed(2147483647 + distance), -2147483648, 4096),
                           pitch=pitch, profile=profile)
            case['coarse'] = coarse
            self.collector.enqueue(case)
        rng = random.Random(0x0097)
        for index in range(2048):
            case = fixture(index % 4, index % 28, old=-1, operation=AIM | THROTTLE,
                           target_mode=index % 256,
                           source=tuple(rng.randrange(-2**31, 2**31) for _ in range(3)),
                           destination=tuple(rng.randrange(-2**31, 2**31) for _ in range(3)))
            case['coarse'] = index % 3
            self.collector.enqueue(case)

    def test_missing_references_live_orphans_and_incomplete_transport(self):
        for old, candidate in itertools.product((NONE, 149, 181), (NONE, 149, 181)):
            for operation in (ACQUIRE, ACQUIRE | AUTOMATIC, AIM, ACQUIRE | AIM | THROTTLE):
                self.collector.enqueue(fixture(old=old, candidate=candidate, operation=operation))
        for kind, target in itertools.product(range(4), range(28)):
            self.collector.enqueue(fixture(kind, target, orphan=True))
        self.collector.flush()
        path = self.collector.path
        valid = struct.pack('<II', 1, 0) + b'\0'
        for data in [valid[:n] for n in range(len(valid))] + [valid + b'x', struct.pack('<II', 1, 1)+b'\0']:
            path.write_bytes(data)
            for command in self.collector.commands:
                result = subprocess.run([*command, '--acquisition', str(path)], capture_output=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, b''), result.stderr)

    def test_required_complete_original_and_canonical_world_consumers(self):
        if not ORACLE:
            self.skipTest('Complete pinned original gate requested separately')
        self.run_original()

    def run_original(self):
        from original_object_pool_oracle import REGISTRY
        from original_unit_oracle import DGROUP
        from test_original_target_acquisition import TargetAcquisitionTests as OriginalTests
        collector = self.collector

        class RequiredTests(OriginalTests):
            @classmethod
            def setUpClass(cls):
                super().setUpClass()
                original_scan = cls.owner.scan

                def scan(machine, actor, kernel, **inputs):
                    case, _ = cls.snapshot(machine, actor, operation=DISCOVER, **inputs)
                    # Canonical readiness must run before the actual C discovery,
                    # rather than substituting an already-scanned raw actor.
                    if collector.canonical is not None:
                        side, pixels = cls.corpus_height
                        case.update(side=side, pixels=pixels)
                    result = original_scan(machine, actor, kernel, **inputs)
                    collector.enqueue(case)
                    return result

                cls.owner.scan = scan

            @classmethod
            def snapshot(cls, machine, actor, *, operation, side=512, pixels=None, **inputs):
                used = bytes(machine.mem_read(DGROUP + 0xe2f7, 150)) + bytes(
                    machine.mem_read(DGROUP + 0xe38d, 32))
                raw = {slot: cls.owner.raw(machine, pointer(slot))
                       for slot, active in enumerate(used) if active}
                # The original numeric-domain probes deliberately write a short
                # class into a long fixture record. Preserve its live semantic
                # projection through a correctly sized C arena, not a mistagged
                # physical allocation. Real saved worlds require no relocation.
                mapping = {slot: slot for slot in raw}
                occupied = set(raw)
                for slot, state in sorted(raw.items()):
                    extended = word(state, 0) in (0, 1, 2, 3, 19)
                    if extended != (slot >= 150):
                        replacement = next(n for n in (range(150, 182) if extended else range(150))
                                           if n not in occupied)
                        mapping[slot] = replacement
                        occupied.add(replacement)

                def address(value):
                    return 0 if not value else pointer(mapping.get(physical(value), physical(value)))

                def normalize(state):
                    state = bytearray(state)
                    if word(state, 0) < 4:
                        for offset in (0x97, 0x9d):
                            struct.pack_into('<H', state, offset, address(word(state, offset)))
                    return bytes(state)

                normalized = {mapping[slot]: normalize(state) for slot, state in raw.items()}
                entries = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))
                registry = [(physical(address(location)), value)
                            for location, value in zip(entries[::2], entries[1::2])]
                seeds, cursor = cls.owner.random_state(machine)
                behavior = word(machine.mem_read(DGROUP + 0x85b6 + raw[physical(actor)][27]*22, 22), 0)
                case = dict(actor=physical(address(actor)), raw=normalized, registry=registry,
                            seeds=seeds, cursor=cursor, operation=operation, behavior=behavior,
                            candidate=physical(address(inputs.get('candidate', 0))),
                            selected=address(inputs.get('selected', 0)), gate=inputs.get('gate', 0),
                            clock=inputs.get('clock', 0), last=inputs.get('last_voice', 0),
                            coarse=inputs.get('coarse', 0),
                            link=machine.mem_read(DGROUP + 0x6dae, 1)[0], side=side,
                            pixels=cls.flat if pixels is None else pixels)
                return case, normalize

            def check(self, machine, actor, *, kernel=None, pixels=None, side=512, **inputs):
                operation = ACQUIRE | (AUTOMATIC if inputs.get('automatic', True) else 0)
                case, normalize = self.snapshot(machine, actor, operation=operation,
                                                side=side, pixels=pixels, **inputs)
                result = super().check(machine, actor, kernel=kernel, pixels=pixels, side=side, **inputs)
                observed = {**result, 'actor': normalize(result['actor'])}
                # Nullified runtime lifetimes are the explicit proved repair;
                # raw-original pointer aliasing does not govern that C boundary.
                references = (word(case['raw'][case['actor']], 0x97),
                              word(case['raw'][case['actor']], 0x9d))
                repaired = any(value and physical(value) not in case['raw'] for value in references)
                collector.enqueue(case, None if repaired else observed)
                return result

            def aim(self, machine, actor, coarse=0):
                case, normalize = self.snapshot(machine, actor, operation=AIM, coarse=coarse)
                if collector.canonical is not None:
                    case.update(side=self.corpus_height[0], pixels=self.corpus_height[1])
                details = super().aim(machine, actor, coarse)
                # The complete original aim return proves the independent
                # geometry expected by both production C targets.
                expected = semantic(case)
                returned = normalize(self.owner.raw(machine, actor))
                values = expected.split()
                self.assertEqual(tuple(map(int, values[6:9])),
                                 (word(returned, 0x9b), word(returned, 0x38), word(returned, 0x99)))
                collector.enqueue(case)
                return details

            def consume(self, machine, actor):
                case, normalize = self.snapshot(machine, actor, operation=THROTTLE)
                if collector.canonical is not None:
                    case.update(side=self.corpus_height[0], pixels=self.corpus_height[1])
                super().consume(machine, actor)
                values = semantic(case).split()
                returned = normalize(self.owner.raw(machine, actor))
                self.assertEqual(tuple(map(int, values[29:32])),
                                 (struct.unpack_from('<h', returned, 0x57)[0], returned[0x90],
                                  returned[PROFILE_COMPONENTS[word(returned, 0)]]))
                collector.enqueue(case)

            def begin_corpus_mission(self, name, side):
                collector.flush()
                collector.canonical = ROOT / 'armoredfist/FISTDATA' / name

            def test_all_pinned_missions_prepared_discovery_acquisition_aim_throttle(self):
                original_prepare = self.owner.visibility.prepare

                def prepare(side, pixels):
                    type(self).corpus_height = side, pixels
                    return original_prepare(side, pixels)

                self.owner.visibility.prepare = prepare
                try:
                    super().test_all_pinned_missions_prepared_discovery_acquisition_aim_throttle()
                finally:
                    collector.flush()
                    collector.canonical = None
                    self.owner.visibility.prepare = original_prepare

        result = unittest.TestResult()
        unittest.defaultTestLoader.loadTestsFromTestCase(RequiredTests).run(result)
        collector.flush()
        self.assertEqual((result.testsRun, result.skipped, result.failures, result.errors), (8, [], [], []))
        self.evidence['original'] = {**RequiredTests.evidence,
                                    'acquisitions': RequiredTests.acquisitions, 'aims': RequiredTests.aims,
                                    'transfers': RequiredTests.transfers, 'requests': RequiredTests.requests,
                                    'throttles': RequiredTests.throttles, 'groups': result.testsRun, 'skips': 0,
                                    'complete_output_sha256': RequiredTests.digest.hexdigest()}


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
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Temporary evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    collector = TargetAcquisitionTests.collector
    success = program.result.wasSuccessful() and len(program.result.skipped) == int(not ORACLE)
    evidence = TargetAcquisitionTests.evidence
    evidence['gate'] = {'success': success, 'groups': program.result.testsRun,
                        'skips': len(program.result.skipped), 'cases_per_target': collector.count,
                        'batches_per_target': collector.batches, 'atomic_rejections': collector.rejections,
                        'canonical_returns_per_target': collector.canonical_count,
                        'complete_output_sha256': collector.digest.hexdigest(), 'target': TARGET}
    print(json.dumps(evidence['gate'], sort_keys=True), flush=True)
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'c-target-acquisition.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
