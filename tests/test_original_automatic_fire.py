#!/usr/bin/env python3
"""Required whole-return af97/afa2, missile launch and genuine parent evidence."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from automatic_fire_contract import FIRE_THRESHOLDS
from orders_contract import scenario_order_blocks
from original_automatic_fire_oracle import OriginalAutomaticFireOracle
from original_object_pool_oracle import REGISTRY, SHORT_SLOTS
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_original_ground_maneuver import seed_for_draw
from test_units import records_from_scenario, snapshot
from test_vehicle_motion import start

ROOT = pathlib.Path(__file__).resolve().parents[1]
ACTOR, TARGET = 0x7000, 0x7100
SEEDS = (1, 2, 32768, 65535)
REVIEW = None


class AutomaticFireTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalAutomaticFireOracle()
        cls.machine = cls.owner.fresh()
        cls.machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
        for offset in (0x6d34, 0x7ae0, 0x9fdf):
            cls.machine.mem_write(DGROUP + offset, bytes(2))
        cls.baseline = bytes(cls.machine.mem_read(DGROUP, 65536))
        cls.starts = tuple(start(kind) for kind in range(4))
        cls.digest = hashlib.sha256()
        cls.branches = collections.Counter()
        cls.calls = collections.Counter()
        cls.groups = {}
        cls.corpus = {}

    def fixture(self, kind=0, *, entry=0xaf97, behavior=0, target=TARGET,
                actor_flags=8, target_flags=0, phase=0, distance=0, control=128,
                actual=0, requested=0, states=(1, 1), counts=(1, 1), selected=False,
                audible=False, base=None, machine=None, actor=ACTOR, target_kind=5):
        machine = machine or self.machine
        machine.mem_write(DGROUP, self.baseline if base is None else base)
        raw = bytearray(self.starts[kind] if base is None else self.owner.raw(machine, actor))
        raw[22] = actor_flags
        raw[0xb5:0xb9] = bytes((*counts, *states))
        raw[0x92] = 93
        for offset, value in ((0x40, control), (0x97, target), (0x99, distance),
                              (0x89, actual), (0x8b, requested)):
            store(raw, offset, value)
        machine.mem_write(DGROUP + actor, bytes(raw))
        if base is None:
            candidate = bytearray(snapshot(target_kind, flags=target_flags))
            machine.mem_write(DGROUP + TARGET, bytes(candidate))
        elif target:
            machine.mem_write(DGROUP + target + 22, bytes([target_flags]))
        descriptor = word(machine.mem_read(DGROUP, 65536), 0x85a0 + raw[27] * 2)
        machine.mem_write(DGROUP + descriptor, struct.pack('<H', behavior))
        for offset, value in ((0x9796, descriptor), (0x978c, phase),
                              (0x6d34, actor if selected else 0),
                              (0x9fdf, actor if audible else 0), (0x7ae0, 0)):
            machine.mem_write(DGROUP + offset, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
        machine.mem_write(DGROUP + 0x978a, b'\0')
        return machine, actor, entry

    def check(self, fixture):
        machine, actor, entry = fixture
        after, effect = self.owner.observe(machine, actor, entry=entry)
        self.digest.update(after[:0x8fc0])
        self.digest.update(after[0x9000:])
        self.digest.update(bytes(machine.mem_read(0x40000, 4096)))
        self.branches[effect['branch']] += 1
        self.calls[hex(entry)] += 1
        if effect['request'] is not None:
            self.calls['actual_audio_requests'] += 1
        if effect['allocation'] is not None:
            self.calls['missile_allocations'] += 1
        return after, effect

    def test_01_complete_alignment_range_phase_control_and_unused_behavior_words(self):
        count = 0
        for value in range(65536):
            kind = value % 4
            entry = 0xaf97 if kind in (1, 3) else 0xafa2
            self.check(self.fixture(kind, entry=entry, actual=(value * 137) & 65535,
                                    requested=(value * 138) & 65535))
            self.check(self.fixture(kind, entry=entry, distance=value, behavior=value % 3,
                                    phase=(0, 4, 40, 80, 140, 252)[value % 6], control=0))
            self.check(self.fixture(kind, entry=entry, distance=201, phase=value,
                                    actor_flags=8 if value & 256 else 0, behavior=value % 3, control=0))
            self.check(self.fixture(kind, entry=entry, distance=201, phase=252, control=value))
            # The threshold index is unused at short range; only exact behavior
            # three admits an early return. Invalid authored choices are stimuli.
            self.check(self.fixture(kind, entry=0xafa2, behavior=value, distance=200, control=0))
            count += 5
        for kind, entry, distance, difference, behavior in itertools.product(
                range(4), (0xaf97, 0xafa2), (0, 199, 200, 201, 65535),
                (0, 181, 182, 183, 65353, 65354, 65355, 65535), range(4)):
            self.check(self.fixture(kind, entry=entry, distance=distance, requested=difference,
                                    behavior=behavior, control=0, phase=0))
            count += 1
        for target in range(65536):
            # A nonzero target is not dereferenced when the genuine descriptor
            # admission is three. This proves every retained presence word.
            self.check(self.fixture(target % 4, entry=0xafa2, behavior=3, target=target))
            count += 1
        for kind, entry, actor_flags, target_flags, phase in itertools.product(
                range(4), (0xaf97, 0xafa2), range(256), (0, 16), range(4)):
            self.check(self.fixture(kind, entry=entry, actor_flags=actor_flags,
                                    target_flags=target_flags, phase=phase, behavior=0, control=0))
            count += 1
        self.assertEqual(count, 410880)
        self.groups['admission_complete_word_and_actor_flag_domains'] = count

    def test_02_complete_rack_state_pairs_reserve_bytes_and_nested_target_gates(self):
        count = 0
        for kind, first, second in itertools.product((1, 3), range(256), range(256)):
            reserves = (0, 1, 255)
            self.check(self.fixture(kind, states=(first, second), counts=(reserves[first % 3], reserves[second % 3]),
                                    target_flags=16, selected=True, audible=True))
            count += 1
        for kind, rack, reserve, other_count, selected, audible in itertools.product(
                (1, 3), (0, 1), range(256), (0, 1, 255), (False, True), (False, True)):
            counts = (reserve, other_count) if rack == 0 else (other_count, reserve)
            states = (4, 1) if rack == 0 else (1, 4)
            self.check(self.fixture(kind, states=states, counts=counts, target_flags=16,
                                    selected=selected, audible=audible))
            count += 1
        for kind, reserve, target_flags, present in itertools.product((1, 3), (0, 1, 255), range(256), (False, True)):
            entry = 0x8711 if kind == 1 else 0x96c0
            self.check(self.fixture(kind, entry=entry, states=(4, 4), counts=(reserve, reserve),
                                    target_flags=target_flags, target=TARGET if present else 0,
                                    selected=True, audible=True))
            count += 1
        self.assertEqual(count, 146432)
        self.groups['rack_state_pair_and_reserve_target_domains'] = count

    def test_03_actual_allocated_targets_all_types_flags_and_physical_actor_orphans(self):
        count = 0
        for kind, target_kind in itertools.product((1, 3), range(28)):
            records = [(0, 1, snapshot(kind, flags=32)), (1, 7, snapshot(target_kind, flags=0))]
            machine, objects = self.owner.prepare_saved(records, SEEDS, 3, 0, (bytes(2144), bytes(176)))
            actor = objects[150][2]
            target = next(p for _, _, p, _ in objects.values() if p != actor)
            base = bytes(machine.mem_read(DGROUP, 65536))
            for flags in range(256):
                fixture = self.fixture(kind, base=base, machine=machine, actor=actor, target=target,
                                       target_flags=flags, counts=(1, 255), states=(4, 4),
                                       selected=True, audible=True)
                after, effect = self.check(fixture)
                self.assertEqual(word(after, target), target_kind)
                if flags & 16:
                    self.assertEqual(effect['branch'], 'missile_launched')
                    pointer, _, _ = effect['allocation']
                    self.assertEqual(word(after, pointer + 0x1a), target)
                    self.assertEqual(after[pointer + 22], 0)  # No invented side flag.
                else:
                    self.assertEqual(effect['branch'], 'fire_requested')
                count += 1
            # A later import overwrites only the actor's logical registry cell.
            orphan_machine, orphan_objects = self.owner.prepare_saved(
                [(0, 1, snapshot(kind, flags=32)), (0, 2, snapshot(target_kind, flags=16))],
                SEEDS, 3, 0, (bytes(2144), bytes(176)))
            orphan = orphan_objects[150][2]
            target = next(p for _, _, p, _ in orphan_objects.values() if p != orphan)
            base = bytes(orphan_machine.mem_read(DGROUP, 65536))
            self.assertEqual(word(base, REGISTRY), target)
            after, effect = self.check(self.fixture(kind, base=base, machine=orphan_machine,
                                                   actor=orphan, target=target, target_flags=16,
                                                   states=(4, 4), audible=True))
            self.assertEqual(effect['branch'], 'missile_launched')
            self.assertEqual(word(after, REGISTRY), target)
            count += 1
        self.groups['allocated_all_28_target_types_and_flag_bytes'] = 14336
        self.groups['physical_actor_registry_orphans'] = 56
        self.assertEqual(count, 14392)
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI
        lifecycle = []
        for kind in (1, 3):
            machine, objects = self.owner.prepare_saved(
                [(0, 1, snapshot(kind, flags=32)), (1, 2, snapshot(5, flags=16))],
                SEEDS, 3, 0, (bytes(2144), bytes(176)))
            actor, target = objects[150][2], objects[0][2]
            machine.reg_write(UC_X86_REG_AX, 1)
            self.owner.far_call(machine, 0x1b2ef)
            released = bytes(machine.mem_read(DGROUP, 65536))
            # The original pointer has no lifetime. It reads the deleted target's
            # retained air flag, then can reuse that same slot for its missile.
            fixture = self.fixture(kind, base=released, machine=machine, actor=actor,
                                   target=target, target_flags=17, states=(4, 4))
            after, effect = self.check(fixture)
            self.assertEqual(effect['allocation'][0], target)
            self.assertEqual(word(after, target + 0x1a), target)
            machine.mem_write(DGROUP, released)
            machine.reg_write(UC_X86_REG_AX, 5)
            machine.reg_write(UC_X86_REG_BX, 1)
            machine.reg_write(UC_X86_REG_CX, 3)
            self.owner.far_call(machine, 0x1b1a2)
            self.assertEqual(machine.reg_read(UC_X86_REG_DI), target)
            reused = bytes(machine.mem_read(DGROUP, 65536))
            after, effect = self.check(self.fixture(kind, base=reused, machine=machine, actor=actor,
                target=target, target_flags=16, states=(4, 4)))
            self.assertNotEqual(effect['allocation'][0], target)
            self.assertEqual(word(after, effect['allocation'][0] + 0x1a), target)
            lifecycle.append({'class': kind, 'released_target_becomes_self_targeted_missile': True,
                              'reused_target_pointer_follows_successor': True})
        self.groups['original_unsafe_retained_pointer_release_reuse'] = {
            'returns': 4, 'classes': lifecycle,
            'shared_requirement': 'Resolve only the exact live canonical reference when this branch uses its payload. Existing target-clearing owners remain responsible; never borrow a reused successor.'}

    def test_04_actual_capacity_holes_reserved_bindings_and_wrapped_constructor(self):
        from unicorn.x86_const import UC_X86_REG_AX
        count = 0
        for kind, fill in itertools.product((1, 3), (0, 118, 119, 120, 148, 149)):
            records = [(0, 1, snapshot(kind, flags=32)), (1, 9, snapshot(5, flags=16))]
            records += [(2 + i, i + 1, snapshot(18, flags=0)) for i in range(fill)]
            machine, objects = self.owner.prepare_saved(records, SEEDS, 3, 0, (bytes(2144), bytes(176)))
            actor, target = objects[150][2], objects[0][2]
            base = bytes(machine.mem_read(DGROUP, 65536))
            for counts, selected, audible in itertools.product(((1, 1), (255, 0)), (False, True), (False, True)):
                after, effect = self.check(self.fixture(kind, base=base, machine=machine, actor=actor,
                    target=target, target_flags=16, states=(4, 4), counts=counts, selected=selected, audible=audible))
                self.assertEqual(effect['branch'], 'missile_capacity' if fill == 149 else 'missile_launched')
                self.assertEqual(after[actor + 0xb5], counts[0] - 1)
                count += 1
            if fill == 149:
                for hole in (1, 75, 149):
                    machine.mem_write(DGROUP, base)
                    # Short slot zero is the retained target. Release another
                    # actual binding through the unchanged original method.
                    machine.reg_write(UC_X86_REG_AX, hole + 1)
                    self.owner.far_call(machine, 0x1b2ef)
                    released = bytes(machine.mem_read(DGROUP, 65536))
                    after, effect = self.check(self.fixture(kind, base=released, machine=machine, actor=actor,
                        target=target, target_flags=16, states=(4, 4), selected=True, audible=True))
                    self.assertEqual(effect['allocation'][1], hole)
                    self.assertEqual(word(after, 0xe294), SHORT_SLOTS)
                    count += 1
        for kind, pose, heading in itertools.product((1, 3),
                ((0, 0, 0), (-2**31, 2**31 - 1, 2**31 - 1), (2**31 - 1, -2**31, -2**31)),
                (0, 1, 16384, 32768, 65535)):
            fixture = self.fixture(kind, states=(4, 4), target_flags=16, audible=True)
            raw = bytearray(self.owner.raw(fixture[0], ACTOR))
            struct.pack_into('<3iH', raw, 4, *pose, heading)
            fixture[0].mem_write(DGROUP + ACTOR, bytes(raw))
            after, effect = self.check(fixture)
            pointer, _, index = effect['allocation']
            self.assertEqual(struct.unpack_from('<2i', after, pointer + 4), pose[:2])
            self.assertEqual(struct.unpack_from('<I', after, pointer + 12)[0], (pose[2] + 3072) & 0xffffffff)
            self.assertEqual(word(after, pointer + 16), heading)
            self.assertEqual(index, 0)
            count += 1
        # An unoccupied registry cell with a retained generation is unavailable.
        fixture = self.fixture(1, states=(4, 4), target_flags=16)
        fixture[0].mem_write(DGROUP + REGISTRY, struct.pack('<HH', 0, 65535))
        after, effect = self.check(fixture)
        self.assertEqual(effect['allocation'][2], 1)
        count += 1
        sequences = {}
        for kind in (1, 3):
            fixture = self.fixture(kind, counts=(1, 2), states=(4, 4), target_flags=16,
                                   selected=True, audible=True)
            sequence = []
            for _ in range(4):
                after, effect = self.check(fixture)
                sequence.append((effect['branch'], effect['rack'], list(after[ACTOR + 0xb5:ACTOR + 0xb9])))
                count += 1
            wanted = ['missile_launched'] * 3 + ['missile_empty'] if kind == 1 else ['missile_launched'] + ['missile_empty'] * 3
            self.assertEqual([branch for branch, _, _ in sequence], wanted)
            sequences[str(kind)] = sequence
        self.groups['sequential_ready_rack_consumption'] = sequences
        self.assertEqual(count, 141)
        self.groups['actual_capacity_order_holes_and_wrapped_payloads'] = count

    def test_05_genuine_parent_banks_counters_rng_domains_and_global_admission(self):
        count = 0
        for kind, automatic, index, value in itertools.product(range(4), (0, 1), (5, 10, 14), range(256)):
            phase = value * 257
            fixture = self.fixture(kind, entry=0xab03, states=(4, 4), target_flags=16,
                                   audible=True, behavior=value % 4, control=128 | automatic)
            machine, actor, _ = fixture
            counter = (((value // 16) * 16) + index - 1) & 255
            machine.mem_write(DGROUP + actor + 0x42, bytes([counter]))
            cursor = value % 4
            seeds = list(SEEDS)
            seeds[cursor] = seed_for_draw(phase)
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
            after, effect = self.check(fixture)
            self.assertEqual(word(after, 0x978c), phase)
            self.assertEqual(effect['parent_index'], index)
            self.assertEqual(after[actor + 0x42], (counter + 1) & 255)
            count += 1
        for admission in range(1, 256):
            fixture = self.fixture(admission % 4, entry=0xab03)
            machine, actor, _ = fixture
            machine.mem_write(DGROUP + 0x978a, bytes([admission]))
            machine.mem_write(DGROUP + actor + 27, b'\xff')
            machine.mem_write(DGROUP + actor + 0x42, bytes([admission]))
            after, effect = self.check(fixture)
            self.assertEqual(effect['branch'], 'global_rejected')
            self.assertEqual(after[actor + 0x42], admission)
            count += 1
        self.assertEqual(count, 6399)
        self.groups['genuine_parent_banks_rng_counter_and_admission'] = count

    def test_06_all_pinned_missions_actual_prepared_worlds_and_genuine_parent_fire_entries(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        self.assertEqual(len(manifest), 47)
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            offset, height_name = 0, None
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size + 6
            self.assertEqual(offset, len(data))
            self.assertIn(height_name, terrains)
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual(len(grouped), 8)
        worlds = actors_total = calls = 0
        for height_name, missions in grouped.items():
            info = terrains[height_name]
            data = (directory / height_name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            decoded = decoder.klc(data)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed, side)
                for name, data in missions:
                    machine, objects = self.owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                                scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    actors = [p for _, _, p, _ in objects.values() if word(self.owner.raw(machine, p), 0) < 4]
                    machine.mem_write(DGROUP + 0x7ae0, bytes(2))
                    prepared = bytes(machine.mem_read(DGROUP, 65536))
                    local = collections.Counter()
                    for entry in (0xaf97, 0xafa2):
                        machine.mem_write(DGROUP, prepared)
                        for actor in actors:
                            raw = self.owner.raw(machine, actor)
                            descriptor = word(machine.mem_read(DGROUP, 65536), 0x85a0 + raw[27] * 2)
                            machine.mem_write(DGROUP + 0x9796, struct.pack('<H', descriptor))
                            _, effect = self.check((machine, actor, entry))
                            local[effect['branch']] += 1
                            calls += 1
                    for index, automatic in itertools.product((5, 10, 14), (0, 1)):
                        machine.mem_write(DGROUP, prepared)
                        machine.mem_write(DGROUP + 0x978a, b'\0')
                        for actor in actors:
                            machine.mem_write(DGROUP + actor + 0x42, bytes([index - 1]))
                            control = word(self.owner.raw(machine, actor), 0x40)
                            machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', (control & 65534) | automatic))
                            _, effect = self.check((machine, actor, 0xab03))
                            local[effect['branch']] += 1
                            calls += 1
                    self.corpus.setdefault(name, {})[str(side)] = {'ground_actors': len(actors),
                        'complete_child_parent_returns': len(actors) * 8, 'branches': dict(local)}
                    worlds += 1
                    actors_total += len(actors)
            print(f'Complete automatic-fire corpus: {height_name}, four details', flush=True)
        self.assertEqual((worlds, actors_total, calls), (188, 3840, 30720))
        self.groups['canonical'] = {'worlds': worlds, 'ground_actors': actors_total,
                                    'complete_child_parent_returns': calls}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Required evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 6 and not program.result.skipped
    complete_returns = sum(AutomaticFireTests.calls.get(hex(entry), 0)
                           for entry in (0xaf97, 0xafa2, 0x8711, 0x96c0, 0xab03))
    required_branches = {'class_rejected', 'behavior_rejected', 'target_absent', 'phase_rejected',
                         'probability_rejected', 'alignment_rejected', 'fire_requested', 'racks_busy',
                         'rack_loading', 'missile_empty', 'missile_target_rejected', 'missile_capacity',
                         'missile_launched', 'global_rejected', 'genuine_ret'}
    success = success and complete_returns == 608968 and set(AutomaticFireTests.branches) == required_branches
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        evidence = {'scope': 'Complete af97/afa2, missile readiness/constructor and genuine diagnostic-unselected parent entries 5/10/14; PCM and full parent remain open',
                    'success': True, 'groups': 6, 'skips': 0, 'calls': dict(AutomaticFireTests.calls),
                    'complete_returns': complete_returns,
                    'branches': dict(AutomaticFireTests.branches), 'coverage': AutomaticFireTests.groups,
                    'corpus': AutomaticFireTests.corpus, 'original_image_sha256': IMAGE_SHA256,
                    'output_sha256': AutomaticFireTests.digest.hexdigest(), 'complete_wasm_streak': 0,
                    'probabilities': FIRE_THRESHOLDS, 'class_names': ['M1', 'M3', 'T80', 'BMP']}
        (REVIEW / 'automatic-fire.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
