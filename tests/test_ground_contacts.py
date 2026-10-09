#!/usr/bin/env python3
"""Shared canonical contacts against independent original and repair predictions."""
import argparse
import collections
import contextlib
import hashlib
import inspect
import io
import itertools
import json
from pathlib import Path
import runpy
import struct
import subprocess
import sys
import tempfile
import unittest

from ground_maneuver_contract import motion_obstacle
from original_unit_oracle import DGROUP
import physical_contact_contract
from roster_promotion_contract import store, word
from test_ground_maneuver import add_body, fixture, physical_context
from test_target_discovery import NONE, physical, pointer
from test_geometry import signed

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = SANITIZED_PROBE = REVIEW = None
ORIGINALS = False
PREDICT = physical_contact_contract.predict
REGISTERS = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
             0, 0x1c00, 0x1c00, 0x9000, 0)


def group_image(context, *, selected=False, cooldown=0, contact=0, scales=(256, 256),
                source_side=0, matched_sound=False, sound=(13, 0, 270, 0)):
    group, actor, addresses, registry = context
    group = bytearray(group)
    for address, slot in addresses.items():
        group[(0xe2f7 + slot) if slot < 150 else (0xe38d + slot - 150)] = 1
        store(group, address + 2, slot if slot < 150 else slot - 150)
    for index, (slot, generation) in enumerate(registry):
        struct.pack_into('<2H', group, 0xdfbc + index * 4,
                         0 if slot == NONE else pointer(slot), generation)
    store(group, 0xe294, sum(slot < 150 for slot in addresses.values()))
    store(group, 0x930a, sum(word(group, address) == 21 for address in addresses))
    group[0x9b5d:0x9b5f] = bytes((100, 0))
    store(group, 0x6d34, actor if selected else 0)
    group[actor + 0x62], group[actor + 0x93] = contact, cooldown
    store(group, 0xe3ae, scales[0]); store(group, 0xe3b0, scales[1])
    # Explicit initial null-source side input, not an invented projectile.
    group[0x16] = source_side
    store(group, 0x9fdf, actor if matched_sound else 0)
    for offset, packet, attenuation in ((45, *sound[:2]), (48, *sound[2:])):
        store(group, 0x9fe1 + offset, packet)
        group[0x9fe3 + offset] = attenuation
    result = bytearray(0x60000)
    result[DGROUP:DGROUP + 65536] = group
    return result, actor


def observed_line(before, after, actor, effect):
    raw = DGROUP + actor
    candidate_pointer = (word(before, DGROUP + 0xdfbc + effect['hit_index'] * 4)
                         if effect['hit_index'] is not None else 0)
    candidate = physical(candidate_pointer)
    tree = DGROUP + candidate_pointer if effect['tree_calls'] else None
    cursor = ((word(after, DGROUP + 0x1f82) - 0x1f84) // 2 + 1) % 4
    values = [0, int(effect['branch'] != 'ignored'), int(candidate_pointer != 0), candidate,
              effect['tree_released'], int(effect['tree_calls'] != 0),
              effect['audio_packet'] or 0, effect['audio_attenuation'] or 0, 0,
              int(effect['audio_requests'] != 0), after[raw + 0x62], after[raw + 0x93],
              word(after, raw + 0x40), *struct.unpack_from('<2i', after, raw + 4),
              *struct.unpack_from('<2h', after, raw + 0x55), word(after, DGROUP + 0x9fdd),
              word(after, DGROUP + 0x930a), cursor,
              *struct.unpack_from('<4H', after, DGROUP + 0x1f84)]
    group = after[DGROUP:DGROUP + 65536]
    used = group[0xe2f7:0xe2f7 + 150] + group[0xe38d:0xe38d + 32]
    binding = next((index for index in range(182)
                    if word(before, DGROUP + 0xdfbc + index * 4) == candidate_pointer), None) if tree else None
    values += [after[tree + 0x1a] if tree else 0, after[tree + 0x16] if tree else 0,
               sum(used[:150]), sum(used[150:]),
               physical(word(after, DGROUP + 0xdfbc + binding * 4)) if binding is not None else NONE,
               word(after, DGROUP + 0xdfbc + binding * 4 + 2) if binding is not None else 0]
    return 'contact ' + ' '.join(map(str, values)) + '\n'


class Collector:
    def __init__(self, commands, temporary):
        self.commands = commands
        self.path = Path(temporary) / 'cases.bin'
        self.bodies, self.expected = [], []
        self.count = self.batches = self.original_count = self.canonical_count = 0
        self.digest = hashlib.sha256()
        self.branches = collections.Counter()
        self.guard_rejections = self.guard_unused = 0
        self.held_sequences = self.held_contacts = 0
        self.canonical = None
        self.side, self.pixels = 1, b'\0'

    def begin(self, canonical=None, side=1, pixels=b'\0'):
        self.flush()
        self.canonical, self.side, self.pixels = canonical, side, pixels

    def enqueue(self, before, actor, entry, *, after=None, effect=None, original=False,
                forced_overlap=False, captured_side=None, steps=1, invalid=0, invalid_slot=NONE, reject=False):
        initial = list(REGISTERS); initial[6] = actor
        if after is None:
            after, _, effect = PREDICT(before, actor, entry, initial,
                                       near_obstacle=motion_obstacle, sound=True)
        model_before = before
        source = word(before, DGROUP + 0xe3b2)
        source_enemy = bool(before[DGROUP + source + 0x16] & 8)
        if captured_side is not None:
            source_enemy = bool(captured_side & 8)
            # Deliberate C policy oracle only. The original fixture still gets
            # its unmodified prediction and unchanged instruction comparison.
            model_before = bytearray(before)
            model_before[DGROUP + source + 0x16] = (
                before[DGROUP + source + 0x16] & 247) | (8 if source_enemy else 0)
            after, _, effect = PREDICT(model_before, actor, entry, initial,
                                       near_obstacle=motion_obstacle, sound=True)
            after[DGROUP + source + 0x16] = before[DGROUP + source + 0x16]
        group = before[DGROUP:DGROUP + 65536]
        addresses, registry = physical_context(group)
        self.branches[effect['branch']] += 1
        header = bytearray(48)
        struct.pack_into('<2H2BH', header, 0, physical(actor), physical(word(group, 0x6d34)),
                         int(entry != 0xa631), group[0x2040], physical(word(group, 0x9fdf)))
        struct.pack_into('<2H2B4H2H', header, 8, *struct.unpack_from('<2H', group, 0xe3ae),
                         int(source_enemy), ((word(group, 0x1f82) - 0x1f84) // 2 + 1) % 4,
                         *struct.unpack_from('<4H', group, 0x1f84), word(group, 0x930a), word(group, 0x9fdd))
        struct.pack_into('<HBxHB', header, 26, word(group, 0x9fe1 + 45), group[0x9fe3 + 45],
                         word(group, 0x9fe1 + 48), group[0x9fe3 + 48])
        header[33] = invalid
        struct.pack_into('<2H', header, 34, len(addresses), steps)
        header[38] = int(forced_overlap)
        struct.pack_into('<H', header, 40, invalid_slot)
        body = bytes(header) + b''.join(struct.pack('<2H', *binding) for binding in registry)
        for address, slot in sorted(addresses.items(), key=lambda pair: pair[1]):
            length = 251 if word(group, address) in (0, 1, 2, 3, 19) else 55
            body += struct.pack('<H', slot) + group[address:address + length]
        expected = []
        retained = model_before
        for step in range(steps):
            if step:
                following, _, next_effect = PREDICT(retained, actor, entry, initial,
                                                   near_obstacle=motion_obstacle, sound=True)
            else:
                following, next_effect = after, effect
            expected.append(observed_line(retained, following, actor, next_effect))
            retained = following
        if invalid == 2:
            for n, line in enumerate(expected):
                values = line.split()[1:]
                values[19] = '4'
                expected[n] = 'contact ' + ' '.join(values) + '\n'
        if reject:
            self.guard_rejections += 1
            unused = {'branch':'ignored', 'hit_index':None, 'tree_calls':0,
                      'tree_released':0, 'audio_packet':None, 'audio_attenuation':None,
                      'audio_requests':0}
            values = observed_line(before, before, actor, unused).split()[1:]
            values[0] = '-1'
            if invalid == 2:
                values[19] = '4'
            expected = ['contact ' + ' '.join(values) + '\n'] * steps
        elif invalid:
            self.guard_unused += 1
        self.bodies.append(body); self.expected.append(''.join(expected))
        self.count += steps
        self.held_sequences += steps > 1
        self.held_contacts += steps if steps > 1 else 0
        self.original_count += original
        self.canonical_count += self.canonical is not None
        if len(self.bodies) >= 512:
            self.flush()

    def flush(self):
        if not self.bodies:
            return
        self.path.write_bytes(struct.pack('<2I', self.side, len(self.bodies)) + self.pixels + b''.join(self.bodies))
        expected = ''.join(self.expected)
        for command in self.commands:
            args = [*command]
            if self.canonical is not None:
                args += ['--prepared', str(self.canonical)]
            result = subprocess.run([*args, str(self.path)], capture_output=True, text=True, timeout=180)
            if result.returncode != 0 or result.stdout != expected:
                actual, desired = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((n for n in range(min(len(actual), len(desired))) if actual[n] != desired[n]),
                                min(len(actual), len(desired)))
                raise AssertionError(f'{command}: exit {result.returncode}; {result.stderr}; '
                                     f'contact mismatch {mismatch}: {actual[mismatch:mismatch+1]} != '
                                     f'{desired[mismatch:mismatch+1]}')
        self.digest.update(expected.encode())
        self.batches += 1
        self.bodies, self.expected = [], []


class ContactTests(unittest.TestCase):
    original = None

    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory(prefix='fist-ground-contacts-', dir='/tmp')
        commands = []
        if TARGET in ('native', 'all'):
            commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_ground_contact_probe')])
        if TARGET in ('wasm', 'all'):
            commands.append(['node', str(BUILD / 'wasm/fist_ground_contact_probe.js')])
        if SANITIZED_PROBE:
            commands.append([str(SANITIZED_PROBE)])
        cls.collector = Collector(commands, cls.temporary.name)

    @classmethod
    def tearDownClass(cls):
        cls.collector.flush(); cls.temporary.cleanup()

    def enqueue(self, context, **options):
        wrapper = options.pop('wrapper', options.get('selected', False))
        before, actor = group_image(context, **options)
        self.collector.enqueue(before, actor, 0x1a0a4 if wrapper else 0xa631)

    def test_01_admission_and_crossed_saved_bytes(self):
        for value in range(65536):
            context = fixture(value % 4, flags=value)
            self.enqueue(context, selected=bool(value & 256), contact=value & 255, cooldown=value >> 8)
        for kind, selected, wrapper, cooldown in itertools.product(range(4), (False, True), (False, True), (0, 255)):
            self.enqueue(fixture(kind, flags=65535), selected=selected, wrapper=wrapper,
                         contact=255, cooldown=cooldown)
        self.collector.flush()

    def test_02_complete_wrapped_extents_and_first_hit_scalars(self):
        for value in range(65536):
            context = fixture(value % 4, scale=256, flags=value,
                              velocity=(value - 32768, (value ^ 32768) - 32768),
                              pose=(2147483647, -2147483648))
            data, actor, _, _ = context
            struct.pack_into('<2h', data, actor + 0x55, value - 32768, (value ^ 32768) - 32768)
            candidate = add_body(context, pose=(2147483647, signed(-2147483648 - 383)), scale=256)
            self.enqueue(context, selected=bool(value & 1))
            store(data, candidate + 0x14, value)
            margin = ((256 + value + 256) & 65535) >> 1
            struct.pack_into('<i', data, candidate + 8, signed(-2147483648 - margin))
            self.enqueue(context, selected=bool(value & 1), contact=255)
        self.collector.flush()

    def test_03_tree_domains_and_sound_record_admission(self):
        for damage, factor in itertools.product(range(256), range(1, 257)):
            context = fixture((damage + factor) % 4, scale=256, velocity=(-1, 1))
            data, actor, _, _ = context
            tree = add_body(context, 21, pose=(0, 0), scale=256)
            data[tree + 26] = damage
            struct.pack_into('<2h', data, actor + 0x55, -3, -1)
            self.enqueue(context, selected=bool(factor & 1), scales=(factor, factor), source_side=8 if damage & 1 else 0)
        for kind, tree, attenuation, packet in itertools.product(range(4), (False, True), range(256), (13, 270, 16, 255, 65535)):
            context = fixture(kind, scale=256)
            add_body(context, 21 if tree else 26, pose=(0, 0), scale=256)
            self.enqueue(context, selected=bool(attenuation & 1), matched_sound=True,
                         sound=(packet, attenuation, packet, attenuation))
        self.collector.flush()

    def test_04_incomplete_transport_fails(self):
        self.enqueue(fixture())
        self.collector.flush()
        valid = self.collector.path.read_bytes()
        for content in (b'', valid[:-1], valid + b'\0'):
            self.collector.path.write_bytes(content)
            for command in self.collector.commands:
                result = subprocess.run([*command, str(self.collector.path)], capture_output=True, timeout=30)
                self.assertNotEqual(result.returncode, 0)

    def test_05_saved_field_retention(self):
        for command in self.collector.commands:
            result = subprocess.run([*command, '--retention'], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, 'retention 6144 6144\n')

    def test_06_genuine_capture_retirement_and_neutral_constructor_reuse(self):
        for command in self.collector.commands:
            result = subprocess.run([*command, '--source-lifetime'], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, 'source-lifetime 8 24 8 8 8\n')

    def test_07_used_late_failures_and_unused_inputs(self):
        for kind, selected, tree, cooldown, repeated in itertools.product(
                range(4), (False, True), (False, True), (0, 255), (False, True)):
            context = fixture(kind, flags=65535, scale=256)
            candidate = add_body(context, 21 if tree else 26, pose=(0,0), scale=256)
            before, actor = group_image(context, selected=selected, contact=2 if repeated else 0,
                                       cooldown=cooldown, matched_sound=True)
            entry = 0x1a0a4 if selected else 0xa631
            for invalid in (1,2,4 if tree else 3,5):
                reject = cooldown == 0 and (
                    (invalid == 1 and not repeated) or
                    (invalid in (2,4) and tree and not repeated) or invalid == 3)
                self.collector.enqueue(before, actor, entry, invalid=invalid,
                                       invalid_slot=physical(candidate), reject=reject)
                # The opposite selection wrapper ignores every used-path defect.
                self.collector.enqueue(before, actor, 0xa631 if selected else 0x1a0a4,
                                       invalid=invalid, invalid_slot=physical(candidate))
        # A genuine earlier near-obstacle visit sets the blocked bit before
        # a later malformed projection fails. The entire mutation rolls back.
        for kind, invalid in itertools.product(range(4), (3,5)):
            context = fixture(kind, flags=0, scale=256, velocity=(0,64))
            candidate = add_body(context, pose=(0,1000), scale=256)
            late = add_body(context, pose=(0,1000000), scale=256, slot=1, registry_index=181)
            before, actor = group_image(context)
            self.collector.enqueue(before, actor, 0xa631, invalid=invalid,
                                   invalid_slot=physical(late), reject=True)
        self.collector.flush()
        self.assertEqual(self.collector.guard_rejections, 56)
        self.assertEqual(self.collector.guard_unused, 464)

    def test_08_required_complete_original_consumption(self):
        if not ORIGINALS:
            self.skipTest('Complete unchanged original coupling is an explicitly required acceptance gate')
        accepted = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        fixtures = ['contact_direct_original.py', 'contact_prediction_original.py',
                    'contact_domains_original.py', 'contact_tree_original.py',
                    'contact_world_original.py', 'contact_source_flight_original.py',
                    'contact_sound_original.py']
        expected_digests = [
            '9b6b91c2b42f99fc232bb8765cbc51ac79ad9c5439570dd42c07fb3ea74d182b',
            '01a78957f004716c47a57d2231418b5575fffb8e4859bc07548b53149fe5f5d1',
            '032e3b81a19403a170c570d5bb8a4109816aaed83a898cd2882c51a12b3656af',
            '96a5d954d31f71e5a5f0a02ba07f4d077baf8a3cce001f6ff66b4305e41030dd',
            'bc60014d549ad6a92ebd34e1837ce5490fd76075c3850699259f900e2605fc0c',
            'e107dec2ea8a1c3830ae90fb145f6bf1b62a8beff069d751f1a4bdbf36bce6d1',
            'a502e544890c83927cb085c25ba84f706ff486aa43a8ee3cde57d90e2d45fd23']
        collector = self.collector
        evidence = []
        current_name = None
        current_side = None
        expected_world_calls = 0

        def observe(before, actor, entry, registers, near_obstacle=None, sound=False):
            nonlocal current_name, current_side, expected_world_calls
            after, abi, effect = PREDICT(before, actor, entry, registers, near_obstacle=near_obstacle, sound=sound)
            frame = inspect.currentframe().f_back
            caller = frame.f_locals
            forced = False
            captured = None
            if running == 'contact_world_original.py':
                module = frame.f_globals
                name, side = module['name'], module['side']
                if (name, side) != (current_name, current_side):
                    collector.begin(ROOT / 'armoredfist/FISTDATA' / name, side, module['pixels'])
                    current_name, current_side = name, side
                forced = caller['forced_tree']
                expected_world_calls += 1
            elif running == 'contact_source_flight_original.py':
                captured = caller['source_side']
            collector.enqueue(before, actor, entry, after=after, effect=effect,
                              original=True, forced_overlap=forced, captured_side=captured)
            return after, abi, effect

        old_argv = sys.argv
        physical_contact_contract.predict = observe
        try:
            for running in fixtures:
                collector.begin()
                directory = Path(self.temporary.name) / running.removesuffix('.py')
                sys.argv = [running, '--review-dir', str(directory)]
                captured_output = io.StringIO()
                with contextlib.redirect_stdout(captured_output):
                    runpy.run_path(str(ROOT / 'tests/fixtures/ground_class_callbacks' / running), run_name='__main__')
                result = json.loads(captured_output.getvalue().splitlines()[-1])
                self.assertIs(result['success'], True)
                self.assertEqual(result['output_sha256'], expected_digests[len(evidence)])
                self.assertEqual(result['model_sha256'], hashlib.sha256((ROOT / 'tests/physical_contact_contract.py').read_bytes()).hexdigest())
                evidence.append(result)
                collector.begin()
                print(f'{running}: {result["cases"]} complete original/C contacts', flush=True)
        finally:
            physical_contact_contract.predict = PREDICT
            sys.argv = old_argv
        self.assertEqual(sum(row['cases'] for row in evidence), 427968)
        self.assertEqual(collector.original_count, 427968)
        self.assertEqual(expected_world_calls, 15360)
        self.assertEqual(collector.canonical_count, 15360)
        self.assertEqual(len(accepted), 47)
        type(self).original = evidence

    def test_09_held_contacts_cooldowns_and_released_successors(self):
        cases = [(None, 0, 3, (0, 0)), (26, 0, 3, (0, 0)),
                 (26, 0, 3, (64, -1)), (21, 0, 3, (0, 0)),
                 (21, 19, 3, (0, 0)), (21, 255, 3, (0, 0)),
                 (21, 200, 256, (0, 0)), (21, 250, 256, (0, 0))]
        for kind, selected, wrapper, case in itertools.product(
                range(4), (False, True), (False, True), cases):
            candidate_type, damage, factor, velocity = case
            context = fixture(kind, flags=65535, scale=256, velocity=velocity)
            if candidate_type is not None:
                candidate = add_body(context, candidate_type, pose=(0, 0), scale=256)
                if candidate_type == 21:
                    context[0][candidate + 26] = damage
            before, actor = group_image(context, selected=selected, contact=128,
                                        matched_sound=True, scales=(factor, factor))
            self.collector.enqueue(before, actor, 0x1a0a4 if wrapper else 0xa631, steps=32)
        self.collector.flush()
        self.assertEqual(self.collector.held_sequences, 128)
        self.assertEqual(self.collector.held_contacts, 4096)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default=TARGET)
    parser.add_argument('--build-root', type=Path, default=BUILD)
    parser.add_argument('--native-probe', type=Path)
    parser.add_argument('--sanitized-probe', type=Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--review-dir', type=Path)
    args = parser.parse_args()
    TARGET, BUILD, NATIVE_PROBE, SANITIZED_PROBE, ORIGINALS, REVIEW = args.target, args.build_root, args.native_probe, args.sanitized_probe, args.originals, args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(Path('/tmp')) or REVIEW.resolve() == Path('/tmp')):
        parser.error('Disposable evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 9
    if ORIGINALS:
        success = success and not program.result.skipped
    collector = ContactTests.collector
    evidence = {'success':bool(success), 'tests':program.result.testsRun, 'skips':len(program.result.skipped),
                'target':TARGET, 'cases_per_target':collector.count, 'batches_per_target':collector.batches,
                'original_contacts':collector.original_count, 'canonical_contacts':collector.canonical_count,
                'output_sha256':collector.digest.hexdigest(), 'branches':dict(collector.branches),
                'guard_rejections':collector.guard_rejections, 'unused_invalid_inputs':collector.guard_unused,
                'held_sequences':collector.held_sequences, 'held_contacts':collector.held_contacts,
                'programs':collector.commands,
                'original':ContactTests.original, 'complete_wasm_streak':0}
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'c-ground-contacts.json').write_text(json.dumps(evidence, indent=2) + '\n')
    print(json.dumps({k:v for k,v in evidence.items() if k!='original'}, sort_keys=True), flush=True)
    raise SystemExit(not success)
