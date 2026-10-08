#!/usr/bin/env python3
"""Required complete original station/support, parent and prepared-world gates."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import time
import unittest

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_remaining_ground_oracle import OriginalRemainingGroundOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from remaining_ground_contract import preferences, station
from roster_promotion_contract import store, word
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_units import records_from_scenario
from test_weapon_control import COUNTS

ROOT = pathlib.Path(__file__).resolve().parents[1]


class RemainingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.audio = OriginalAudioRequestOracle()
        cls.owner = OriginalRemainingGroundOracle(cls.audio)
        cls.digest = hashlib.sha256()
        cls.counts = collections.Counter()
        cls.branches = collections.Counter()
        cls.groups = {}
        cls.corpus = {}
        cls.invalid = {}
        cls.target_allocations = {}
        cls.target_types = set()

    def check(self, machine, actor, *, entry=0xae5c, **audio):
        after, effect = self.owner.observe(machine, actor, self.audio.fixture(**audio), entry=entry)
        self.digest.update(after[:0x8fc0])
        self.digest.update(after[0x9000:])
        self.counts['complete_dos_returns'] += 1
        if self.counts['complete_dos_returns'] % 32768 == 0:
            print('Whole-state checked DOS returns: ' + str(self.counts['complete_dos_returns']), flush=True)
        self.counts['actual_kernel_audio_returns'] += int(effect['audio'])
        self.counts['actual_constructor_height_returns'] += int(effect['allocation'] is not None)
        self.counts[effect['operation']] += 1
        if effect['parent_index'] is not None:
            self.counts['parent_' + str(effect['parent_index']) + '_' + effect['parent_bank']] += 1
        if 'support' in effect:
            self.branches[effect['support']] += 1
            self.branches['smoke_' + effect['smoke']] += 1
        return after, effect

    def setUp(self):
        print('Required remaining-command group: ' + self._testMethodName, flush=True)

    def tearDown(self):
        print('Complete observed DOS returns so far: ' + str(self.counts['complete_dos_returns']), flush=True)

    def fixture(self, kind=0, *, target_type=0, variant=0, rounds=None, selected=255,
                loaded=255, countdown=123, reserve=1, gate=65535, voice_age=30,
                selected_pointer=None, parent_index=None, automatic=1, high=0,
                cursor=0, parent_draw=12345, second_draw=0, platoon=0,
                admission=0, **support):
        machine, actor = self.owner.fixture(kind, **support)
        data = bytearray(machine.mem_read(DGROUP, 65536))
        target = word(data, actor + 0x97)
        if target:
            if target_type != 5:
                # Provision the correct physical arena with the actual allocator,
                # then reuse only its proved byte delta for independent fixtures.
                # A target's type never masquerades in an incorrectly sized slot.
                from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX,
                                              UC_X86_REG_CX, UC_X86_REG_DI,
                                              UC_X86_REG_EFLAGS)
                key = kind, support.get('full', False), target_type
                count = self.target_allocations.get(key, (None, None, 256))[2]
                if count == 256:
                    initial = bytes(data)
                    machine.reg_write(UC_X86_REG_AX, 1)
                    self.owner.far_call(machine, 0x1b2ef)
                    machine.reg_write(UC_X86_REG_AX, target_type)
                    machine.reg_write(UC_X86_REG_BX, 1)
                    machine.reg_write(UC_X86_REG_CX, 2)
                    self.owner.far_call(machine, 0x1b1a2)
                    self.assertFalse(machine.reg_read(UC_X86_REG_EFLAGS) & 1)
                    pointer = machine.reg_read(UC_X86_REG_DI)
                    length = 251 if self.owner.type_flags[target_type] & 1 else 55
                    slot = self.owner.slot(pointer)
                    raw = bytes(machine.mem_read(DGROUP + pointer, length))
                    self.assertEqual(raw, struct.pack('<HH', target_type, slot - 150 if slot >= 150 else slot) + bytes(length - 4))
                    after = bytes(machine.mem_read(DGROUP, 65536))
                    spans = ((target, target + 55), (pointer, pointer + length),
                             (0xdfbc, 0xe294), (0xe294, 0xe29c), (0xe2f7, 0xe3ad))
                    delta = tuple((i, b) for i, (a, b) in enumerate(zip(initial, after))
                                  if a != b and not 0x8fc0 <= i < 0x9004)
                    self.assertTrue(all(any(a <= i < b for a, b in spans) for i, _ in delta))
                    previous = self.target_allocations.get(key)
                    if previous is not None:
                        self.assertEqual((pointer, delta), previous[:2])
                    self.target_allocations[key] = pointer, delta, 0
                    count = 0
                    self.counts['actual_target_release_and_typed_allocation_pairs'] += 1
                pointer, delta, _ = self.target_allocations[key]
                for offset, value in delta:
                    data[offset] = value
                self.target_allocations[key] = pointer, delta, count + 1
                data[pointer + 4:pointer + 12] = data[target + 4:target + 12]
                target = pointer
                store(data, actor + 0x97, target)
            self.target_types.add(target_type)
            data[target + 25] = variant
        base = actor + (0xac if kind == 2 else 0xad)
        for index, amount in enumerate(rounds if rounds is not None else [1] * COUNTS[kind]):
            store(data, base + 2 * index, amount)
        data[actor + 0x91], data[actor + 0xa5] = selected, loaded
        data[actor + 0xa8], data[actor + 0xbb] = countdown, reserve
        store(data, 0x6da2, gate)
        store(data, 0x9fca, word(data, 0x452) - voice_age)
        if selected_pointer is not None:
            store(data, 0x6d34, selected_pointer)
        store(data, 0x7ae0, 0)
        data[0x978a] = admission
        if parent_index is not None:
            data[actor + 0x42] = (high * 16 + parent_index - 1) & 255
            data[actor + 27] = platoon
            store(data, actor + 0x40, automatic)
            words = list(SEEDS)
            words[cursor] = seed_for_draw(parent_draw)
            words[(cursor + 1) % 4] = seed_for_draw(second_draw)
            struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *words)
        machine.mem_write(DGROUP, bytes(data))
        return machine, actor

    def test_01_literal_target_preferences_and_every_available_station_mask(self):
        start = self.counts['complete_dos_returns']
        targets = [(t, 0) for t in range(28) if t != 26] + [(26, v) for v in range(4)]
        for kind in range(4):
            for (target_type, variant), mask, selected, loaded in itertools.product(
                    targets, range(1 << COUNTS[kind]), (0, 2, 4, 6, 255), (0, 2, 4, 6, 255)):
                rounds = [int(mask & (1 << i) != 0) for i in range(COUNTS[kind])]
                machine, actor = self.fixture(kind, target_type=target_type, variant=variant,
                                               rounds=rounds, selected=selected, loaded=loaded, flags=0)
                _, effect = self.check(machine, actor, voice='WVSOUNDS.BIN' if mask & 1 else 'EVSOUNDS.BIN')
                choices = preferences(kind, target_type, variant)
                expected = next((code for code in choices if rounds[code // 2]), choices[-1])
                self.assertEqual(effect['station'], expected)
                self.assertFalse(effect['unsafe_station'])
                if mask == 0:
                    self.assertEqual(effect['choice_index'], 2)
            for selected in (False, True):
                machine, actor = self.fixture(kind, target=False, selected=255,
                                               selected_pointer=0 if not selected else None, flags=0)
                _, effect = self.check(machine, actor)
                self.assertEqual(effect['choices'], preferences(kind, 0))
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 62008)
        self.assertEqual(self.target_types, set(range(28)))
        self.groups['all_authored_preferences_masks_selection_loading'] = count

    def test_02_complete_selected_loaded_ammunition_and_reserve_domains(self):
        start = self.counts['complete_dos_returns']
        for kind in range(4):
            for selected, loaded in itertools.product(range(256), repeat=2):
                machine, actor = self.fixture(kind, target_type=1, selected=selected,
                                               loaded=loaded, flags=0)
                self.check(machine, actor)
            for amount in range(65536):
                choices = preferences(kind, 1 if amount & 1 else 0)
                rounds = [0] * COUNTS[kind]
                rounds[choices[(amount >> 1) % 3] // 2] = amount
                machine, actor = self.fixture(kind, target_type=1 if amount & 1 else 0,
                                               rounds=rounds, flags=0)
                self.check(machine, actor, enabled=int(bool(amount & 2)),
                           voice='WVSOUNDS.BIN' if amount & 4 else 'EVSOUNDS.BIN')
            for reserve, selected, notice in itertools.product(range(256), (False, True), range(256)):
                # Only M3/BMP consume the reserve and notice context in this callback.
                if kind not in (1, 3) and notice != 0:
                    continue
                machine, actor = self.fixture(kind, rounds=[0] * COUNTS[kind], flags=0,
                    target_type=1 if kind in (1, 3) else 0,
                    reserve=reserve, notice_context=notice, selected_pointer=0 if not selected else None)
                self.check(machine, actor)
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 787456)
        self.groups['full_selected_loaded_bytes_ammo_words_reserve_notice_bytes'] = count

    def test_03_complete_voice_admission_word_domains_and_flag_bytes(self):
        start = self.counts['complete_dos_returns']
        for word_value in range(65536):
            kind = word_value % 4
            for domain in ('gate', 'voice_age', 'selected_pointer'):
                options = {domain: word_value}
                machine, actor = self.fixture(kind, target_type=1, flags=0, **options)
                self.check(machine, actor, busy=bool(word_value & 8),
                           voice='WVSOUNDS.BIN' if word_value & 4 else 'EVSOUNDS.BIN')
        for kind, flags, enabled, countdown in itertools.product(range(4), range(256),
                                                                (0, 1), (0, 1, 255)):
            machine, actor = self.fixture(kind, flags=flags, target_type=1, countdown=countdown)
            self.check(machine, actor, enabled=enabled)
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 202752)
        self.groups['voice_gate_age_selected_pointer_words_flags_and_countdowns'] = count

    def test_04_complete_support_random_range_clock_and_mode_domains(self):
        start = self.counts['complete_dos_returns']
        for value in range(65536):
            kind = value % 4
            for mode in (0, 4):
                machine, actor = self.fixture(kind, draw=value, mode=mode)
                self.check(machine, actor, entry=0xb0be)
            machine, actor = self.fixture(kind, distance=value, draw=0, mode=0)
            self.check(machine, actor, entry=0xb0be)
            machine, actor = self.fixture(kind, prior=(1800 - value) & 65535, draw=0, mode=0)
            self.check(machine, actor, entry=0xb0be)
        for kind, flags, target, mode in itertools.product(range(4), range(256), (False, True), range(256)):
            machine, actor = self.fixture(kind, flags=flags, target=target, mode=mode, draw=0)
            self.check(machine, actor, entry=0xb0be)
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 786432)
        self.groups['support_rng_range_age_words_flags_target_presence_mode_bytes'] = count

    def test_05_genuine_parent_control_admission_counters_platoons_rng(self):
        start = self.counts['complete_dos_returns']
        for control in range(65536):
            machine, actor = self.fixture(control % 4, parent_index=9 if control & 2 else 12,
                automatic=control, high=(control >> 2) & 15, cursor=(control >> 6) & 3,
                parent_draw=control, second_draw=control ^ 65535, flags=control & 255)
            self.check(machine, actor, entry=0xab03)
        for kind, automatic, index, high, cursor, platoon in itertools.product(
                range(4), (0, 1), (9, 12), range(16), range(4), range(256)):
            machine, actor = self.fixture(kind, parent_index=index, automatic=automatic,
                high=high, cursor=cursor, platoon=platoon, second_draw=0)
            _, effect = self.check(machine, actor, entry=0xab03)
            self.assertEqual(effect['parent_index'], index)
        for admission, kind, automatic, index in itertools.product(range(1, 256), range(4), (0, 1), (9, 12)):
            machine, actor = self.fixture(kind, parent_index=index, automatic=automatic,
                admission=admission, platoon=255, parent_draw=admission * 257)
            before = bytes(machine.mem_read(DGROUP + actor, 251))
            after, effect = self.check(machine, actor, entry=0xab03)
            self.assertIsNone(effect['parent_index'])
            self.assertEqual(after[actor:actor + 251], before)
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 331760)
        self.groups['parent_control_word_counters_cursor_platoon_bytes_global_admission'] = count

    def test_06_invalid_variant_byte_proof_and_physical_target_reuse(self):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS)
        start = self.counts['complete_dos_returns']
        invalid_stations = collections.Counter()
        for kind, variant in itertools.product(range(4), range(4, 256)):
            machine, actor = self.fixture(kind, target_type=26, variant=variant, flags=8)
            data = bytes(machine.mem_read(DGROUP, 65536))
            text = bytes(machine.mem_read(0x2d740, 65536))
            _, predicted = station(data, actor, text)
            # The complete unsafe preference read executes. Already-loaded
            # selected station isolates this read from another invalid reload table.
            machine.mem_write(DGROUP + actor + 0xa5, bytes([predicted['station']]))
            _, effect = self.check(machine, actor)
            self.assertFalse(effect['valid_variant'])
            invalid_stations[(kind, effect['station'])] += 1
        self.assertTrue(any(code & 1 or code // 2 >= COUNTS[kind]
                            for kind, code in invalid_stations))
        self.invalid['adjacent_variant_reads'] = {f'{kind}:{code}': n for (kind, code), n in invalid_stations.items()}
        alias = []
        for kind, offset in itertools.product(range(4), (0, 32, 128)):
            self.audio.place('DSOUNDS.BIN', offset)
            machine, actor = self.fixture(kind, target_type=5)
            target = word(machine.mem_read(DGROUP, 65536), actor + 0x97)
            _, live = self.check(machine, actor)
            self.assertEqual(live['choices'], preferences(kind, 5))
            machine.reg_write(UC_X86_REG_AX, 1)
            self.owner.far_call(machine, 0x1b2ef)
            self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0xdfc0), 0)
            self.assertEqual(word(machine.mem_read(DGROUP, 65536), actor + 0x97), target)
            _, released = self.check(machine, actor)
            self.assertEqual(released['choices'], live['choices'])
            after, effect = self.check(machine, actor, entry=0xb0be)
            self.assertEqual(effect['allocation'][0], target)
            self.assertEqual(word(after, target), 20)
            if offset == 32:
                self.assertEqual(after[0x9dc7 + 6:0x9dc7 + 14], after[target + 4:target + 12])
            alias.append({'class': kind, 'effects_low_byte': offset,
                          'support': effect['support'], 'reused_physical_target': target})
            self.assertEqual(machine.reg_read(UC_X86_REG_DI), actor)
            self.assertFalse(machine.reg_read(UC_X86_REG_EFLAGS) & 1)
            _, reused = self.check(machine, actor)
            self.assertEqual(reused['choices'], preferences(kind, 20))
        self.audio.place('DSOUNDS.BIN', 0)
        self.invalid['actual_released_target_smoke_reuse'] = alias
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 1056)
        self.groups['invalid_variant_bytes_and_actual_stale_target_allocation'] = count

    def test_07_sequential_retention_support_cooldowns_capacity_and_audio_contexts(self):
        start = self.counts['complete_dos_returns']
        for kind, offset, mode, audio_context in itertools.product(range(4), (0, 32, 128), (0, 4),
                                                                   ('enabled', 'disabled', 'absent')):
            self.audio.place('DSOUNDS.BIN', offset)
            machine, actor = self.fixture(kind, mode=mode, draw=0, stock=255)
            for tick in range(64):
                for field, value in ((0x6cde, (1800 + tick * 1800) & 65535),
                                     (0x452, (12345 + tick * 32) & 65535)):
                    machine.mem_write(DGROUP + field, struct.pack('<H', value))
                self.check(machine, actor, entry=0xb0be, enabled=int(audio_context != 'disabled'),
                           missing_effects=audio_context == 'absent')
                self.check(machine, actor, enabled=int(audio_context != 'disabled'))
        self.audio.place('DSOUNDS.BIN', 0)
        # Real allocations supply the actor even when a later saved binding
        # displaces it from the public registry. Parent resolves but does not
        # dereference malformed platoon descriptors at these two entries.
        from test_vehicle_motion import start as actor_start
        for kind, index, automatic in itertools.product(range(4), (9, 12), (0, 1)):
            raw = bytearray(actor_start(kind))
            raw[0x42], raw[27] = index - 1, 255
            store(raw, 0x40, automatic)
            machine, objects = self.owner.prepare_saved([(0, 1, bytes(raw)), (0, 2, actor_start(1))],
                                                       SEEDS, 0, 0, (bytes(2144), bytes(176)))
            actor = objects[150][2]
            self.assertNotEqual(actor, word(machine.mem_read(DGROUP, 65536), 0xdfbc))
            machine.mem_write(DGROUP + 0x7ae0, bytes(2))
            machine.mem_write(DGROUP + actor + 0x42, bytes([index - 1]))
            machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', automatic))
            self.check(machine, actor, entry=0xab03)
        count = self.counts['complete_dos_returns'] - start
        self.assertEqual(count, 9232)
        self.groups['sequential_support_station_and_actual_registry_orphans'] = count

    def test_08_all_47_prepared_missions_eight_installed_maps_four_details(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        from orders_contract import scenario_order_blocks
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            cursor = 0
            height_name = None
            while cursor < len(data):
                tag, size = struct.unpack_from('<4sH', data, cursor)
                if tag == b'BINF':
                    height_name = data[cursor + 6:cursor + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                cursor += size + 6
            self.assertEqual(cursor, len(data))
            self.assertIn(height_name, terrains)
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual((len(manifest), len(grouped)), (47, 8))
        worlds = actors_total = complete = heights = 0
        for height_name, missions in grouped.items():
            info = terrains[height_name]
            source = (directory / height_name).read_bytes()
            self.assertEqual(hashlib.sha256(source).hexdigest(), info['sha256'])
            decoded = decoder.klc(source)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                actual_side, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(actual_side, side)
                for name, data in missions:
                    machine, objects = self.owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                                 scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    prepared = bytes(machine.mem_read(DGROUP, 65536))
                    self.owner.install_height(side, pixels)
                    actors = [pointer for _, _, pointer, _ in objects.values() if word(prepared, pointer) < 4]
                    candidates = [pointer for _, _, pointer, _ in objects.values()]
                    local = collections.Counter()
                    for actor in actors:
                        for entry in (0xae5c, 0xb0be):
                            machine.mem_write(DGROUP, prepared)
                            self.check(machine, actor, entry=entry)
                            local['unchanged_saved_child_returns'] += 1
                        for index, automatic in itertools.product((9, 12), (0, 1)):
                            state = bytearray(prepared)
                            state[actor + 0x42] = index - 1
                            store(state, actor + 0x40, (word(state, actor + 0x40) & 65534) | automatic)
                            store(state, 0x7ae0, 0)
                            state[0x978a] = 0
                            machine.mem_write(DGROUP, bytes(state))
                            self.check(machine, actor, entry=0xab03)
                            local['genuine_parent_returns'] += 1
                        target = next(pointer for pointer in candidates if pointer != actor)
                        for offset in (0, 32, 128):
                            state = bytearray(prepared)
                            state[actor + 22] |= 8
                            state[actor + 0x43] = 4
                            store(state, actor + 0x97, target)
                            store(state, actor + 0x99, 20)
                            store(state, 0x6cde, 1800)
                            store(state, 0x9794, 0)
                            store(state, 0x9fdf, actor)
                            kind = word(state, actor)
                            from support_audio_contract import SMOKE_STOCK
                            stock = SMOKE_STOCK[kind]
                            if kind == 2:
                                store(state, actor + stock, 1)
                            elif stock is not None:
                                state[actor + stock] = 1
                            struct.pack_into('<5H', state, 0x1f82, 0x1f8a, seed_for_draw(0), *SEEDS[1:])
                            self.audio.place('DSOUNDS.BIN', offset)
                            for entry, index in ((0xb0be, None), (0xab03, 9), (0xab03, 12)):
                                reaching = bytearray(state)
                                if index is not None:
                                    reaching[actor + 0x42] = index - 1
                                    store(reaching, actor + 0x40, word(reaching, actor + 0x40) | 1)
                                    store(reaching, 0x7ae0, 0)
                                    reaching[0x978a] = 0
                                machine.mem_write(DGROUP, bytes(reaching))
                                _, effect = self.check(machine, actor, entry=entry)
                                local['constructed_reaching_' + ('child' if index is None else 'parent') + '_returns'] += 1
                                local['actual_constructor_height_returns'] += int(effect['allocation'] is not None)
                    self.corpus.setdefault(name, {})[str(side)] = {'ground_actors': len(actors), **dict(local)}
                    self.assertEqual(local['genuine_parent_returns'], len(actors) * 4)
                    worlds += 1
                    actors_total += len(actors)
                    complete += sum(v for k, v in local.items() if k.endswith('returns') and k != 'actual_constructor_height_returns')
                    heights += local['actual_constructor_height_returns']
                print(f'Complete remaining-command corpus: {height_name}, detail {side}', flush=True)
            self.assertEqual(hashlib.sha256((directory / height_name).read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual((worlds, actors_total, complete), (188, 3840, 57600))
        self.assertGreater(heights, 0)
        self.audio.verify_assets()
        self.groups['canonical_prepared_worlds'] = {'worlds': worlds, 'ground_actors': actors_total,
            'complete_returns': complete, 'constructed_support_height_returns': heights}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'remaining-ground.json'
    output.unlink(missing_ok=True)
    begin = time.monotonic()
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 8 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 8, 'skips': 0, 'seconds': time.monotonic() - begin,
            'scope': 'Complete original ae5c/b0be and diagnostic-unselected genuine parent nine/twelve; shared C, selected b152/full parent/class/battle/PCM/intended repairs remain open',
            'counts': dict(RemainingTests.counts), 'coverage': RemainingTests.groups,
            'branches': dict(RemainingTests.branches), 'corpus': RemainingTests.corpus,
            'invalid_domain_evidence': RemainingTests.invalid, 'original_engine_sha256': IMAGE_SHA256,
            'output_sha256': RemainingTests.digest.hexdigest(), 'complete_wasm_streak': 0}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps({k: v for k, v in result.items() if k not in ('corpus', 'invalid_domain_evidence')}, sort_keys=True), flush=True)
    raise SystemExit(not success)
