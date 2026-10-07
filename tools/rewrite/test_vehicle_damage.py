#!/usr/bin/env python3
"""M1 primary damage, four-class reactions and immediate destruction/retirement."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_object_pool import NONE
from test_projectile_flight import Pool, line
from test_units import expected as unit_definitions_expected, records_from_scenario
from test_vehicle_motion import start
from test_vehicle_start import initialized, state_lines, step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
RECORDS = ((90, 25), (120, 90), (90, 60), (120, 90))
FACTORS = ((100, 150, 220, 245, 248, 251, 253, 255, 255, 253, 251, 248, 245, 220, 150, 100),
           (200, 210, 220, 230, 240, 245, 250, 255, 255, 250, 245, 240, 230, 220, 210, 200),
           (180, 190, 200, 210, 220, 230, 240, 255, 255, 240, 230, 220, 210, 200, 190, 180),
           (220, 225, 230, 235, 240, 245, 250, 255, 255, 250, 245, 240, 235, 230, 225, 220))


def fixture(kind=0, *, raw=None, bindings=None, target=1, seeds=(2, 2, 2, 2), cursor=0,
            scales=(64, 32), source_flags=0, aspect=0, selected=0, damage=0, flags=None,
            motion_flags=0, secondary=None, operating=None, speed=17, throttle=100,
            steps=1, retire=4, finish=1, roster=None, counters=(0, 0, 0), sizes=(4, 4, 4, 4)):
    if raw is None:
        raw = initialized([(5, 7, start(kind))], (0, 0, 0, 0), 0, 0)[0][0][2]
    raw = bytearray(raw)
    raw[0x3a], raw[0x19], raw[0x95] = damage, motion_flags, 9
    if flags is not None:
        raw[0x16] = flags
    if secondary is not None:
        raw[0x17] = secondary
    if operating is not None:
        raw[0x1a] = operating
    struct.pack_into('<hh', raw, 0x55, speed, throttle)
    return dict(raw=bytes(raw), bindings=bindings or [(8, 181, 1), (kind, 5, 7), (0, 180, 1)],
                target=target, seeds=seeds, cursor=cursor, scales=scales, source_flags=source_flags,
                aspect=aspect, selected=selected, steps=steps, retire=retire, finish=finish,
                roster=roster if roster is not None else (1, 2, 1, 2) + (NONE,) * 28,
                counters=counters, sizes=sizes, flash=11)


def encode(cases):
    data = struct.pack('<I', len(cases))
    for case in cases:
        data += struct.pack('<4H4B5H4B7H', *case['seeds'], case['cursor'], case['source_flags'],
                            case['aspect'], case['selected'], *case['scales'], len(case['bindings']),
                            case['target'], case['steps'], case['retire'], case['finish'], case['flash'],
                            0, *case['counters'], *case['sizes'])
        data += case['raw'] + b''.join(struct.pack('<3H', *record) for record in case['bindings'])
        data += struct.pack('<32H', *case['roster'])
    return data


def vehicle_lines(raw, kind, index, value):
    saved = bytearray(raw)
    actual_type, = struct.unpack_from('<H', saved)
    struct.pack_into('<H', saved, 0, kind)
    output = state_lines([(index, value, bytes(saved))])
    first, rest = output.split('\n', 1)
    fields = first.split(); fields[1] = str(actual_type)
    return ' '.join(fields) + '\n' + rest + line('damage', [raw[0x3a], raw[0x95], raw[0x1b], raw[0x1c]])


def effect_line(allocation, pose, template):
    model, extent, callback, last, period = template
    return line('effect', [*allocation[1:], *pose, model, extent, 2048, callback, 0, 0, last, period, period, 0])


def expected(case, *, prepared=None, capture=None):
    pool = Pool([(*record, 0, 0, 0, 0, 0, 0, 0) for record in case['bindings']])
    raw = bytearray(case['raw'])
    kind, = struct.unpack_from('<H', raw)
    target = pool.allocations[case['target']]
    source = pool.allocations[0]
    roster = [NONE if ordinal == NONE else pool.allocations[ordinal][1] for ordinal in case['roster']]
    sizes = list(case['sizes'])
    counters = list(case['counters'])
    selected = case.get('selected_slot', target[1] if case['selected'] else NONE)
    flash, cursor, words = case['flash'], case['cursor'], list(case['seeds'])
    if prepared is not None:
        pool, raw, target, source, roster, sizes, counters, selected, flash, words, cursor = prepared
    output = ''
    destroyed = False
    def random():
        nonlocal cursor
        value, cursor = step(words, cursor)
        return value
    for _ in range(case['steps']):
        voices = []
        struct.pack_into('<H', raw, 0x40, struct.unpack_from('<H', raw, 0x40)[0] | 32)
        base, spread = RECORDS[kind]
        rolled = base + 1 + ((random() % 256 * spread) >> 8)
        angled = (rolled * FACTORS[kind][case['aspect']] % 65536) >> 8
        damage = (angled * case['scales'][int(bool(case['source_flags'] & 8))] % 65536) >> 8
        total = raw[0x3a] + damage
        raw[0x3a] = total % 256
        destroyed = total >= 256 or raw[0x3a] >= 100
        effects, wreck = [], None
        sound, destruction_sound = 255, 255
        if destroyed:
            side = int(bool(raw[0x16] & 8))
            counters[side] = (counters[side] + 1) % 65536
            if side == 0 and not case['source_flags'] & 8:
                counters[2] = (counters[2] + 1) % 65536
            pose = struct.unpack_from('<3i', raw, 4)
            for template in ((20, 448, 4, 10, 7), (19, 768, 2, 21, 6)):
                allocation = pool.explosion()
                if allocation:
                    effects.append(effect_line(allocation, pose, template))
            allocation = pool.explosion()
            replacement = NONE
            if allocation:
                _, slot, index, value = allocation
                pool.slots[slot] = 1, 23
                replacement = slot
                wreck = line('wreck', [slot, index, value, *pose, struct.unpack_from('<H', raw, 0x26)[0],
                                       (10, 22, 34, 46)[kind], kind, 256, 768, raw[0x1b], raw[0x1c], 64, 4])
            roster = [replacement if slot == target[1] else slot for slot in roster]
            pool.slots[target[1]] = 1, 19
            struct.pack_into('<H', raw, 0, 19)
            raw[0x16] |= 1
            raw[0x17] = 4
            sizes = [sum(slot != NONE and pool.slots[slot][1] != 23 for slot in roster[index:index + 4])
                     for index in range(0, 16, 4)]
            sound, destruction_sound = 45, 9
        else:
            value = random()
            if value % 256 <= FACTORS[kind][2]:
                value = random()
                if value & 120 == 0:
                    if raw[0x19] & 6 == 0:
                        voices.append(26)
                    value = (2, 4, 2, 4, 2, 4, 2, 6)[value & 7]
                    raw[0x19] |= value
                    speed, = struct.unpack_from('<h', raw, 0x55)
                    struct.pack_into('<hh', raw, 0x55, speed // 2, 0)
            if value >> 8 <= FACTORS[kind][3] and not raw[0x19] & 16:
                if random() & 248 == 0:
                    voices.append(34); raw[0x19] |= 16
            value = random()
            if value % 256 <= FACTORS[kind][4] and not raw[0x19] & 8:
                if random() & 120 == 0:
                    voices.append(28); raw[0x19] |= 8
                    raw[0x1a] = raw[0x1a] & 239 | 32
            if selected == target[1] and not raw[0x17] & 2:
                flash = 20 if damage > 1 else 4
                raw[0x95] = flash
                sound = 18
        output += line('result', [damage, int(destroyed), int(wreck is not None),
                                  int(destroyed and selected == target[1]), 1, sound, destruction_sound,
                                  len(effects), len(voices)]) + line('voices', voices).rstrip() + '\n'
        output += ''.join(effects) + (wreck or '')
        output += vehicle_lines(raw, kind, target[2], target[3])
        output += line('combat', [selected, flash, *counters]) + line('roster', roster) + line('platoons', sizes)
        output += line('random', [cursor, *words]) + pool.state()
        if capture is not None:
            capture(dict(raw=raw, roster=roster, sizes=sizes, counters=counters, flash=flash,
                         words=words, cursor=cursor, effects=effects, wreck=wreck,
                         event=(damage, int(destroyed), int(wreck is not None),
                                int(destroyed and selected == target[1]), 1, sound,
                                destruction_sound, len(effects), len(voices)), voices=voices))
        if destroyed:
            break
    if case['finish']:
        allocation = pool.explosion()
        output += line('impact', [int(allocation is not None), 2, 15, 1])
        if allocation:
            output += effect_line(allocation, case.get('source_pose', struct.unpack_from('<3i', raw, 4)),
                                  (20, 256, 4, 10, 5))
        pool.release(source)
        output += pool.state()
    if destroyed:
        for _ in range(case['retire']):
            raw[0x17] = (raw[0x17] - 1) % 256
            removed = raw[0x17] == 0
            if removed:
                pool.release(target)
            output += line('retire', [raw[0x17], int(removed)]) + pool.state()
            if removed:
                break
    return output


def seed_for(value):
    shifted = (value + 1) % 65536
    return (((shifted ^ 0xb400) << 1) | 1) % 65536 if shifted & 32768 else shifted << 1


def pipeline_expected():
    actors = []
    for kind, index, pose in ((0, 3, (0, 0, 65536)), (2, 4, (0, 2556, 66560))):
        raw = bytearray(251)
        struct.pack_into('<H', raw, 0, kind)
        struct.pack_into('<3i', raw, 4, *pose)
        raw[0x16] = 64
        actors.append(initialized([(index, 1, bytes(raw))], (0, 0, 0, 0), 0, 0)[0][0][2])
    origin = bytearray(actors[0])
    struct.pack_into('<H', origin, 0xad, 14)
    origin[0xe4], origin[0xa8], origin[0x3c], origin[0x92] = 3, 20, 16, 0
    output = 'pipeline_launch 0 1 12\n' + state_lines([(3, 1, bytes(origin))])
    for tick in range(1, 4):
        output += line('pipeline_flight', [2 if tick == 3 else 0, 151 if tick == 3 else NONE,
                                           0, 0, 852 * tick, 67584, tick, max(2 - tick, 0)])
    bindings = [(8, 0, 1), (0, 3, 1), (2, 4, 1), (18, 1, 1)]
    case = fixture(2, bindings=bindings, target=2, seeds=(0, 0, 0, 0), scales=(256, 256),
                   roster=(2, 1) + (NONE,) * 30, sizes=(0, 0, 0, 0))
    case.update(raw=actors[1], flash=0, selected_slot=150, source_pose=(0, 2556, 67584))
    output += expected(case)
    pool = Pool(bindings)
    shock, fire, wreck, impact = [pool.explosion() for _ in range(4)]
    pool.slots[wreck[1]] = 1, 23
    pool.release(pool.allocations[0]); pool.release(pool.allocations[2])
    for allocation in (shock, fire, impact, pool.allocations[3]):
        pool.release(allocation)
    return output + 'pipeline_effects 1 1 1 1\n' + pool.state()


class DamageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-damage-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.fixture_count = 0
        cls.damage_count = 0
        cls.impact_count = 0
        cls.retirement_count = 0
        cls.pipeline_count = 0
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_vehicle_damage_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_vehicle_damage_probe.js')])

    @classmethod
    def tearDownClass(cls):
        print(f'Damage verification per target: {cls.fixture_count} valid fixtures, '
              f'{cls.damage_count} damage, {cls.impact_count} impact and '
              f'{cls.retirement_count} retirement updates, '
              f'{cls.pipeline_count} reaching pipelines', flush=True)

    def check(self, cases):
        type(self).fixture_count += len(cases)
        wanted = ''.join(expected(case) for case in cases)
        type(self).damage_count += wanted.count('result ')
        type(self).impact_count += wanted.count('impact ')
        type(self).retirement_count += wanted.count('retire ')
        if ORACLE:
            self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
        return wanted

    def test_every_first_random_byte_aspect_class_source_side_and_cursor(self):
        for kind in range(4):
            for angle in range(16):
                cases = []
                for sample in range(256):
                    cursor = sample % 4
                    seeds = [seed_for(0xfe00), seed_for(7), seed_for(0), seed_for(1)]
                    seeds[cursor] = seed_for(sample)
                    cases.append(fixture(kind, aspect=angle, seeds=tuple(seeds), cursor=cursor,
                                         source_flags=8 if sample & 1 else 0, selected=sample & 1,
                                         flags=66 | (8 if sample & 2 else 0)))
                self.check(cases)

    def test_reaction_return_high_byte_admissions_masks_and_signed_speed(self):
        cases = []
        for kind in range(4):
            for motion in (0, 2, 4, 6, 8, 16, 24, 255):
                for value in (0, 1, 2, 6, 7, 8, 120, 248, 255, 0xfe00, 65535):
                    for speed in (-32768, -3, -1, 0, 3, 32767):
                        cases.append(fixture(kind, motion_flags=motion, speed=speed, scales=(0, 0),
                                             seeds=tuple(seed_for(x) for x in (0, 0, value, 0))))
        for offset in range(0, len(cases), 256):
            self.check(cases[offset:offset + 256])

    def test_every_reaction_low_byte_with_paired_high_byte_and_all_fire_choices(self):
        for kind in range(4):
            for reaction in ('fire', 'turret', 'track'):
                cases = []
                for low in range(256):
                    word = low | ((255 - low) << 8)
                    outputs = {'fire': (0, 0, word, 0), 'turret': (0, 0, 0, word),
                               'track': (0, 0xfeff, 0, word)}[reaction]
                    cases.append(fixture(kind, speed=-3, scales=(0, 0), selected=low & 1,
                                         seeds=tuple(seed_for(x) for x in outputs)))
                self.check(cases)

    def test_critical_byte_carry_scale_words_selected_feedback_and_census_wrap(self):
        cases = []
        for kind in range(4):
            for health in (0, 1, 99, 100, 255):
                for scale in (0, 1, 2, 64, 255, 256, 257, 32768, 65535):
                    for selected in (0, 1):
                        cases.append(fixture(kind, damage=health, scales=(scale, scale), selected=selected,
                                             counters=(65535, 65535, 65535), finish=1))
            for secondary in (0, 2, 255):
                cases.append(fixture(kind, scales=(0, 0), secondary=secondary, selected=1))
            for source in (0, 8):
                for victim in (0, 8):
                    for selected in (0, 1):
                        cases.append(fixture(kind, damage=99, scales=(256, 256), source_flags=source,
                                             flags=66 | victim, selected=selected,
                                             counters=(65535, 65535, 65535)))
        self.check(cases)

    def test_destruction_order_capacity_roster_replacement_and_four_tick_retirement(self):
        cases = []
        for count in (1, 119, 120, 147, 148, 149, 150):
            for kind in range(4):
                bindings = [(8, 181, 1), (kind, 180, 0), (0, 179, 1)]
                bindings += [(21, index, 1) for index in range(count - 1)]
                cases.append(fixture(kind, damage=255, bindings=bindings, roster=(1,) * 32,
                                     retire=6, scales=(256, 256), finish=1))
        output = self.check(cases)
        self.assertIn('retire 0 1', output)
        self.assertIn('impact 0 2 15 1', output)
        # One damage→post-damage shell continuation leaves all three destruction slots in use.
        self.check([fixture(2, damage=0, scales=(256, 256), steps=10, retire=4, finish=1)])

    def test_repeated_noncritical_hits_preserve_components_and_stop_on_destruction(self):
        self.check([fixture(kind, scales=(64, 32), selected=selected, steps=40, finish=1)
                    for kind in range(4) for selected in (0, 1)])

    def test_missing_truncated_wrong_type_profile_binding_and_invalid_cursor(self):
        valid = encode([fixture()])
        cases = [valid[:length] for length in (0, 3, 4, 43, 290, len(valid) - 1)] + [valid + b'\0']
        for position, value in ((12, 4), (14, 16), (15, 2), (27, 2), (29, 1), (44, 4)):
            data = bytearray(valid); data[position] = value; cases.append(bytes(data))
        cases.append(encode([fixture(bindings=[(8, 0, 1), (0, 1, 1), (8, 0, 1)])]))
        for data in cases:
            self.path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 1, result.stderr)

    def test_reaching_launch_flight_damage_impact_and_complete_retirement(self):
        wanted = pipeline_expected()
        if ORACLE:
            self.assertEqual(ORACLE.pipeline(), wanted)
        for command in self.commands:
            result = subprocess.run([*command, '-p'], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
        type(self).pipeline_count += 1

    def test_all_registered_original_ground_targets_with_complete_mission_occupancy(self):
        if not ORIGINALS:
            self.skipTest('The complete original mission corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        cases, snapshots, orphaned = [], 0, 0
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            current = {index: ordinal for ordinal, (index, _, _) in enumerate(records)}
            vacancy = next(index for index in range(182) if index not in current)
            bindings = [(8, vacancy, 1)] + [(int.from_bytes(raw[:2], 'little'), index, value)
                                           for index, value, raw in records]
            roster = [NONE if ordinal == NONE else ordinal + 1
                      for ordinal in unit_definitions_expected(records)[3]]
            for ordinal, (index, value, raw) in enumerate(records):
                kind = int.from_bytes(raw[:2], 'little')
                if kind >= 4:
                    continue
                snapshots += 1
                if current[index] != ordinal:
                    orphaned += 1
                    continue
                installed = initialized([(index, value, raw)], (0, 0, 0, 0), 0, 0)[0][0][2]
                cases.append(fixture(kind, raw=installed, bindings=bindings, target=ordinal + 1,
                                     roster=tuple(roster), damage=0, steps=4, finish=1))
        self.assertEqual(snapshots, 960)
        self.assertEqual(len(cases) + orphaned, 960)
        for offset in range(0, len(cases), 16):
            self.check(cases[offset:offset + 16])
        print(f'Damage corpus: {len(cases)} current ground targets from 960 snapshots, {orphaned} excluded orphan targets, all {len(manifest)} complete worlds', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS = args.build_root, args.target, args.native_probe, args.originals
    if args.oracle:
        from original_vehicle_damage_oracle import OriginalVehicleDamageOracle
        ORACLE = OriginalVehicleDamageOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (0 if ORIGINALS else 1))
