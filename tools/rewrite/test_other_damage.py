#!/usr/bin/env python3
"""Remaining collision-reachable M1 target damage and ordered impact continuation."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_collision import delta
from test_object_pool import NONE
from test_projectile_flight import Pool, line, advance_explosion
from test_units import expected as unit_expected, records_from_scenario
from test_vehicle_damage import effect_line, seed_for
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
ACTIVE = ((5, 0), (6, 0), (26, 0), (26, 1), (26, 2), (26, 3), (27, 0))
TEMPLATES = {'pair': (20, 768, 4, 10, 9), 'air': (20, 1280, 4, 10, 11),
             'ground': (16, 768, 0, 22, 6), 'wide': (16, 2048, 0, 22, 10),
             'last': (20, 448, 4, 10, 7), 'impact': (20, 256, 4, 10, 5)}


def fixture(kind=5, *, raw=None, mode=0, damage=0, limit=90, flags=64, secondary=0,
            behavior=0, seeds=(2, 0, 0, 0), cursor=0, scales=(256, 64), source_flags=0,
            aspect=0, bindings=None, target=1, selected=2, steps=1, finish=1,
            effects=0, reach=0, counters=(11, 13, 17, 19, 23), roster=None):
    if raw is None:
        raw = bytearray(55)
        struct.pack_into('<H', raw, 0, kind)
        struct.pack_into('<3iH', raw, 4, 0, 0, 65536, 0)
        struct.pack_into('<2H', raw, 18, 1234, 1024)
        struct.pack_into('<HH', raw, 29, 0xabcd, 0xfedc)
        raw[24] = 197
    else:
        raw = bytearray(raw)
    raw[22], raw[23], raw[25] = flags, secondary, mode
    if kind in (5, 6):
        raw[37], raw[50], raw[51] = behavior, 173, damage
    elif kind == 26:
        raw[26], raw[27] = damage, limit
    elif kind == 27:
        raw[26] = damage
    return dict(raw=bytes(raw), bindings=bindings or [(8, 181, 1), (kind, 5, 7), (0, 180, 1)],
                target=target, seeds=seeds, cursor=cursor, scales=scales, source_flags=source_flags,
                aspect=aspect, selected=selected, steps=steps, finish=finish, effects=effects,
                reach=reach, counters=counters, flash=29,
                roster=roster if roster is not None else (selected,) + (NONE,) * 31)


def encode(cases):
    data = struct.pack('<I', len(cases))
    for case in cases:
        data += struct.pack('<4H4B6H2B5H2BH', *case['seeds'], case['cursor'], case['source_flags'],
                            case['aspect'], case['finish'], *case['scales'], len(case['bindings']),
                            case['target'], case['steps'], case['effects'], case['reach'], 0,
                            *case['counters'], case['flash'], 0, case['selected'])
        data += case['raw'] + b''.join(struct.pack('<3H', *record) for record in case['bindings'])
        data += struct.pack('<32H', *case['roster'])
    return data


def actor_lines(raw, allocation):
    kind, slot, index, value = allocation
    output = line('actor', [*allocation, *struct.unpack_from('<3iH', raw, 4),
                            *struct.unpack_from('<2H', raw, 18), *raw[22:26]])
    if kind in (5, 6):
        output += line('pair', [struct.unpack_from('<H', raw, 29)[0], raw[37], raw[50], raw[51],
                                struct.unpack_from('<h', raw, 27)[0], struct.unpack_from('<H', raw, 35)[0],
                                struct.unpack_from('<H', raw, 46)[0], raw[48], raw[26]])
    elif kind == 26:
        output += line('type26', [raw[26], raw[27], struct.unpack_from('<H', raw, 28)[0], raw[30]])
    elif kind == 27:
        output += line('type27', [raw[26], *struct.unpack_from('<2H', raw, 29), struct.unpack_from('<H', raw, 27)[0]])
    return output


def expected(case, *, prepared=None, capture=None):
    pool = Pool(case['bindings'])
    raw = bytearray(case['raw'])
    kind, = struct.unpack_from('<H', raw)
    target, source = pool.allocations[case['target']], pool.allocations[0]
    counters = list(case['counters'])
    words, cursor = list(case['seeds']), case['cursor']
    pose = struct.unpack_from('<3i', raw, 4)
    selected = NONE if case['selected'] == NONE else pool.allocations[case['selected']][1]
    roster = [NONE if ordinal == NONE else pool.allocations[ordinal][1] for ordinal in case['roster']]
    if prepared is not None:
        pool, raw, target, source, counters, words, cursor, selected, roster = prepared
        pose = struct.unpack_from('<3i', raw, 4)
    output, created = '', []
    def random():
        nonlocal cursor
        value, cursor = step(words, cursor)
        return value
    def rolled(record):
        value = record[0] + 1 + (random() % 256 * record[1] >> 8)
        return (value * case['scales'][int(bool(case['source_flags'] & 8))] % 65536) >> 8
    def create(template):
        allocation = pool.explosion()
        if allocation:
            created.append(dict(allocation=allocation, pose=pose, template=template,
                                frame=0, height=0, countdown=template[4], flags=0))
        return allocation
    if case['reach']:
        for tick in range(1, 4):
            if tick == 3 and kind in (5, 6):
                if random() % 256 >= 38:
                    raise AssertionError('Constructed reaching fixture misses its probabilistic target')
            output += line('flight', [2 if tick == 3 else 0, target[1] if tick == 3 else NONE,
                                     (-struct.unpack_from('<H', raw, 16)[0] % 65536) >> 12 if tick == 3 else 0,
                                     pose[0], delta(pose[1] - 852 * (3 - tick), 0), pose[2], tick, max(2 - tick, 0)])
    destroyed, released = False, False
    for _ in range(case['steps']):
        damage, sound, voice, explosion = 0, 255, 255, None
        destroyed, released = False, False
        if kind in (5, 6) and raw[37] != 12:
            damage = rolled((90, 50))
            raw[51] = (raw[51] + damage) % 256
            if raw[51] >= 100:
                destroyed = True
                index = 3 + int(bool(raw[22] & 8))
                counters[index] = (counters[index] + 1) % 65536
                if random() % 256 >= 128:
                    explosion = create(TEMPLATES['pair']); sound = 9
                    pool.release(target); raw[22] |= 1; released = True
                else:
                    raw[37], raw[50] = 12, 0
                    struct.pack_into('<H', raw, 29, 32)
            elif not raw[22] & 8 and not raw[23] & 2:
                raw[23] |= 2; raw[37] = 4
                struct.pack_into('<H', raw, 29, 56); voice = 38
        elif kind == 26 and not raw[25] & 4:
            subtype = raw[25]
            damage = rolled(((100, 40), (60, 40), (100, 40), (40, 30))[subtype])
            total = raw[26] + damage
            raw[26] = total % 256
            if total >= 256 or raw[26] >= raw[27]:
                raw[22], raw[23], raw[25] = (raw[22] | 1) & 249, raw[23] & 247, raw[25] | 4
                struct.pack_into('<H', raw, 28, 1152)
                struct.pack_into('<H', raw, 20, (640, 640, 896, 1152)[subtype])
                explosion = create(TEMPLATES[('air', 'ground', 'air', 'wide')[subtype]])
                sound, destroyed = 9, True
        elif kind == 27 and raw[25] == 0:
            damage = rolled((120, 90))
            total = raw[26] + damage
            raw[26] = total % 256
            if total >= 256 or raw[26] >= 80:
                raw[25], raw[22], raw[23] = 1, raw[22] & 249 | 1, raw[23] & 231
                struct.pack_into('<H', raw, 18, 320)
                struct.pack_into('<2H', raw, 29, 768, 0)
                explosion = create(TEMPLATES['last']); sound, destroyed = 9, True
        output += line('result', [damage, int(destroyed), int(released), int(explosion is not None), sound, voice, 1])
        if explosion:
            output += effect_line(explosion, pose, created[-1]['template'])
        output += actor_lines(raw, target)
        output += line('combat', [selected, case['flash'], *counters]) + line('roster', roster) + 'platoons 0 0 0 0\n'
        output += line('random', [cursor, *words]) + pool.state()
        if capture is not None:
            capture(dict(raw=raw, counters=counters, words=words, cursor=cursor,
                         effects=list(created), event=(damage, int(destroyed), int(released),
                         1, sound, voice, int(explosion is not None))))
        if destroyed or released:
            break
    if case['finish']:
        allocation = create(TEMPLATES['impact'])
        output += line('impact', [int(allocation is not None), 2, 15, 1])
        if allocation:
            output += effect_line(allocation, pose, TEMPLATES['impact'])
        pool.release(source)
        output += pool.state()
    for _ in range(case['effects']):
        for effect in created:
            model, extent, callback, last, period = effect['template']
            if not effect['flags'] & 1:
                effect['frame'], effect['countdown'], effect['height'], released = advance_explosion(
                    effect['frame'], last, period, effect['countdown'], callback, effect['height'])
                if released:
                    effect['flags'] |= 1; pool.release(effect['allocation'])
            output += line('effect', [*effect['allocation'][1:], *effect['pose'], model, extent, 2048,
                                      callback, effect['height'], effect['frame'], last, period,
                                      effect['countdown'], effect['flags']])
        output += pool.state()
    return output


class OtherDamageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-other-damage-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.fixtures = cls.damage = cls.impact = cls.flight = cls.animation = 0
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_other_damage_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_other_damage_probe.js')])

    @classmethod
    def tearDownClass(cls):
        print(f'Other damage per target: {cls.fixtures} fixtures, {cls.damage} damage, '
              f'{cls.impact} impact, {cls.flight} flight and {cls.animation} animation observations', flush=True)

    def check(self, cases):
        wanted = ''.join(expected(case) for case in cases)
        if ORACLE:
            self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
        type(self).fixtures += len(cases)
        type(self).damage += wanted.count('result ')
        type(self).impact += wanted.count('impact ')
        type(self).flight += wanted.count('flight ')
        type(self).animation += sum(case['effects'] for case in cases)
        return wanted

    def batches(self, cases, size=256):
        for offset in range(0, len(cases), size):
            self.check(cases[offset:offset + size])

    def test_every_random_byte_active_class_subtype_source_side_and_cursor(self):
        cases = []
        for kind, mode in ACTIVE:
            for side in (0, 8):
                for sample in range(256):
                    cursor = sample % 4
                    seeds = [seed_for(255)] * 4; seeds[cursor] = seed_for(sample)
                    cases.append(fixture(kind, mode=mode, damage=0, seeds=tuple(seeds), cursor=cursor,
                                         source_flags=side, aspect=sample % 16))
        self.batches(cases)

    def test_full_damage_and_type26_limit_bytes_and_word_scale_carry(self):
        cases = [fixture(kind, mode=mode, damage=value) for kind, mode in ACTIVE for value in range(256)]
        cases += [fixture(26, mode=mode, limit=limit, scales=(64, 64)) for mode in range(4) for limit in range(256)]
        cases += [fixture(kind, mode=mode, damage=health, scales=(scale, scale))
                  for kind, mode in ACTIVE for health in (0, 79, 99, 100, 255)
                  for scale in (0, 1, 2, 64, 255, 256, 257, 32768, 65535)]
        self.batches(cases)

    def test_pair_death_choice_every_random_byte_side_and_census_word_wrap(self):
        cases = [fixture(kind, damage=20, flags=64 | side, scales=(256, 256),
                         seeds=(seed_for(0), seed_for(choice), 0, 0), counters=(65535,) * 5)
                 for kind in (5, 6) for side in (0, 8) for choice in range(256)]
        self.batches(cases)

    def test_flags_secondary_disabled_behavior_and_modes_do_not_add_masks(self):
        cases = [fixture(kind, mode=mode, flags=value, damage=0, scales=(0, 0))
                 for kind, mode in ACTIVE for value in range(256)]
        cases += [fixture(kind, mode=mode, secondary=value, damage=0, scales=(0, 0))
                  for kind, mode in ACTIVE for value in range(256)]
        cases += [fixture(kind, behavior=value, scales=(0, 0), steps=2) for kind in (5, 6) for value in range(256)]
        cases += [fixture(27, mode=value, damage=255, steps=2) for value in range(256)]
        cases += [fixture(26, mode=mode, damage=255, flags=255, secondary=255, steps=2)
                  for mode in range(8)]
        cases += [fixture(23, mode=value, flags=value, secondary=255, steps=2) for value in range(256)]
        self.batches(cases)

    def test_every_short_occupancy_and_creation_before_immediate_release_then_impact(self):
        cases = []
        for count in range(2, 151):
            for kind, mode in ACTIVE:
                bindings = [(8, 181, 1), (kind, 180, 1), (0, 179, 1)]
                bindings += [(21, index, 1) for index in range(count - 2)]
                cases.append(fixture(kind, mode=mode, damage=99, bindings=bindings,
                                     scales=(256, 256), seeds=(2, 0, 0, 0)))
        self.batches(cases)
        self.check([fixture(kind, damage=99, bindings=[(8, 181, 1), (kind, 5, value), (0, 180, 1)],
                            scales=(256, 256)) for kind in (5, 6) for value in (0, 65535)])

    def test_reaching_flight_damage_impact_and_complete_authored_effect_lifetimes(self):
        cases = [fixture(kind, mode=mode, damage=99, seeds=(2, 2, 0, 0), scales=(256, 256),
                         reach=1, effects=240) for kind, mode in ACTIVE]
        cases.append(fixture(23, seeds=(2, 2, 0, 0), reach=1, effects=240))
        self.check(cases)

    def test_repeated_noncritical_hits_and_destroyed_actor_noop_without_random(self):
        self.check([fixture(kind, mode=mode, scales=(32, 32), steps=50) for kind, mode in ACTIVE])
        self.check([fixture(kind, behavior=12, damage=255, steps=50) for kind in (5, 6)] +
                   [fixture(26, mode=mode, damage=255, steps=50) for mode in range(4, 8)] +
                   [fixture(27, mode=1, damage=255, steps=50), fixture(23, steps=50)])

    def test_truncation_wrong_type_subtype_binding_and_metadata_fail_before_output(self):
        valid = encode([fixture()])
        invalid = [valid[:length] for length in (0, 3, 4, 43, 98, len(valid) - 1)] + [valid + b'\0']
        for position, value in ((12, 4), (14, 16), (15, 2), (28, 2), (29, 1), (41, 1), (44, 4)):
            data = bytearray(valid); data[position] = value; invalid.append(bytes(data))
        invalid.append(encode([fixture(26, mode=8)]))
        invalid.append(encode([fixture(bindings=[(8, 0, 1), (5, 1, 1), (8, 0, 1)])]))
        for data in invalid:
            self.path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 1, result.stderr)

    def test_all_original_remaining_target_snapshots_and_complete_occupancy_rosters(self):
        if not ORIGINALS:
            self.skipTest('The complete pinned original corpus is an explicit required additional gate')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        self.assertEqual(len(manifest), 47)
        cases, counts, orphans = [], {}, 0
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            current = {index: ordinal for ordinal, (index, _, _) in enumerate(records)}
            vacancy = next(index for index in range(182) if index not in current)
            bindings = [(8, vacancy, 1)] + [(int.from_bytes(raw[:2], 'little'), index, value)
                                           for index, value, raw in records]
            roster = tuple(NONE if ordinal == NONE else ordinal + 1 for ordinal in unit_expected(records)[3])
            for ordinal, (index, _, raw) in enumerate(records):
                kind = int.from_bytes(raw[:2], 'little')
                if kind not in (5, 6, 23, 26, 27):
                    continue
                counts[kind] = counts.get(kind, 0) + 1
                if current[index] != ordinal:
                    orphans += 1; continue
                case = fixture(kind, raw=raw, bindings=bindings, target=ordinal + 1, selected=NONE,
                               roster=roster, steps=4, scales=(128, 64))
                case['raw'] = raw
                cases.append(case)
        self.assertEqual(counts, {23: 47, 26: 802, 27: 236})
        self.assertEqual(len(cases), 1085)
        self.assertEqual(orphans, 0)
        self.batches(cases, 16)
        print(f'Other damage corpus: {len(cases)} current targets, {counts} snapshots, '
              f'{orphans} unreachable orphan targets, all {len(manifest)} complete occupancy/roster contexts', flush=True)


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
        from original_other_damage_oracle import OriginalOtherDamageOracle
        ORACLE = OriginalOtherDamageOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (0 if ORIGINALS else 1))
