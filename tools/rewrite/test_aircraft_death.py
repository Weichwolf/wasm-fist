#!/usr/bin/env python3
"""Complete retained aircraft death, immutable emission pose and natural effect cleanup."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_collision import delta
from test_destruction import smoke_lines
from test_ground import contact, varied_plane
from test_other_damage import actor_lines, fixture as damage_fixture, expected as damage_expected
from test_projectile_flight import Pool, line, advance_explosion
from test_units import records_from_scenario
from test_vehicle_motion import rotate, signed
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
NONE = 65535


def fixture(kind=5, *, raw=None, speed=96, target_speed=32, heading=0, target_heading=0,
            motion=64, countdown=1, ground=1, altitude=0x1234207b, offset=0, damage=100,
            tick=0, animation=0, ticks=1, side=2, pixels=None, enabled=1, coarse=0,
            seeds=(2, 2, 0, 0), cursor=0, bindings=None, target=0, operation=0,
            releases=(NONE, NONE), wind=(17, -23)):
    if raw is None:
        raw = bytearray(55)
        struct.pack_into('<H', raw, 0, kind)
        struct.pack_into('<3iH2H', raw, 4, 2147483640, -2147483640, altitude,
                         heading, 1234, 1024)
        raw[22:26] = bytes((64, 3, ground, 91))
        struct.pack_into('<hh', raw, 27, speed, target_speed)
        struct.pack_into('<H', raw, 35, countdown)
        struct.pack_into('<H', raw, 46, target_heading)
        raw[37], raw[48], raw[50], raw[51] = 12 if operation == 0 else 0, motion, offset, damage
    else:
        raw = bytearray(raw)
    if bindings is None:
        bindings = [(kind, 5, 1)] if operation == 0 else [(8, 181, 1), (kind, 5, 1)]
        target = int(operation == 1)
    pixels = bytes(side * side) if pixels is None else bytes(pixels)
    return dict(raw=bytes(raw), tick=tick, animation=animation, ticks=ticks, side=side,
                pixels=pixels, enabled=enabled, coarse=coarse, seeds=seeds, cursor=cursor,
                bindings=bindings, target=target, operation=operation, releases=releases, wind=wind)


def encode(cases):
    output = struct.pack('<I', len(cases))
    for case in cases:
        output += struct.pack('<4H4B8H2i', *case['seeds'], case['cursor'], case['enabled'],
                              case['coarse'], case['operation'], case['tick'], case['animation'],
                              case['ticks'], case['side'], len(case['bindings']), case['target'],
                              *case['releases'], *case['wind'])
        output += case['raw'] + case['pixels']
        output += b''.join(struct.pack('<3H', *record) for record in case['bindings'])
    return output


def effect_lines(effect):
    allocation, raw = effect
    return line('effect', [*allocation[1:], *struct.unpack_from('<3i3H', raw, 4),
                           *struct.unpack_from('<2H', raw, 26), raw[25], raw[30], raw[31],
                           raw[32], raw[22]])


def new_effect(allocation, pose, template):
    model, extent, callback, last, period = template
    raw = bytearray(55)
    struct.pack_into('<H', raw, 0, 4)
    struct.pack_into('<3i3H', raw, 4, *pose, model, extent, 2048)
    struct.pack_into('<H', raw, 26, callback)
    raw[30:33] = bytes((last, period, period))
    return allocation, raw


def expected(case, *, prepared=None, capture=None, class_only=False):
    pool = Pool(case['bindings'])
    primary = pool.allocations[case['target']]
    for ordinal in case['releases']:
        if ordinal != NONE:
            pool.release(pool.allocations[ordinal])
    raw = bytearray(case['raw'])
    words, cursor = list(case['seeds']), case['cursor']
    smokes, effects = {}, {}
    output = ''
    live = True

    if prepared is not None:
        pool, raw, primary, words, cursor = prepared

    def random():
        nonlocal cursor
        value, cursor = step(words, cursor)
        return value

    def shared():
        return line('random', [cursor, *words]) + pool.state()

    if case['operation'] == 1:
        request = damage_fixture(primary[0], raw=raw, flags=raw[22], secondary=raw[23],
                                 damage=raw[51], behavior=raw[37], bindings=case['bindings'],
                                 target=case['target'], selected=NONE, roster=(NONE,) * 32,
                                 seeds=case['seeds'], cursor=case['cursor'], scales=(256, 256))
        request['raw'] = bytes(raw)
        # This reaching operation does not combine pre-release fixtures with damage.
        if case['releases'] != (NONE, NONE):
            raise ValueError('Pre-releases belong to direct death fixtures')
        transcript = damage_expected(request).splitlines()
        result = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('result '))
        values = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('actor '))
        struct.pack_into('<3iH2H', raw, 4, *values[4:10]); raw[22:26] = bytes(values[10:14])
        pair = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('pair '))
        struct.pack_into('<H', raw, 29, pair[0]); raw[37], raw[50], raw[51] = pair[1:4]
        live = not result[2]
        output += line('damage', [result[0], result[1], result[2], result[4]])
        for text in transcript:
            if text.startswith('effect '):
                fields = list(map(int, text.split()[1:]))
                allocation = 4, *fields[:3]
                effect = new_effect(allocation, tuple(fields[3:6]), tuple([fields[6], fields[7], fields[9], fields[12], fields[13]]))
                # Both constructor effects start at zero height/frame and period countdown.
                effects[allocation[1]] = effect
                output += effect_lines(effect)
        output += line('impact', [next(text.split()[1] for text in transcript if text.startswith('impact '))])
        pool.registry = [tuple(map(int, entry.split(':'))) for entry in transcript[-1].split()[1:]]
        pool.slots = [tuple(map(int, entry.split(':'))) for entry in transcript[-2].split()[1:]]
        state = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('random '))
        cursor, words = state[0], state[1:]
    output += actor_lines(raw, primary) + shared()
    for clock in range(case['ticks']):
        tick, animation = (case['tick'] + clock) % 65536, (case['animation'] + clock) % 65536
        updated, released, sound, created_smoke, created_effect = live, False, 255, None, None
        if live:
            roll = random()
            x, y, z, heading = struct.unpack_from('<3iH', raw, 4)
            if tick & 3 == 0:
                ground = contact(case['side'], case['pixels'], (x, y, heading))[0]
                old = (z >> 8) % 256
                altitude = max(old, ground)
                desired = (ground + raw[50]) % 256
                altitude += (desired > altitude) - (desired < altitude)
                z = delta(z + (altitude - old) * 256, 0)
                struct.pack_into('<i', raw, 12, z); raw[24] = (altitude - ground) % 256
            raw[26] = animation & 7
            speed, desired = struct.unpack_from('<hh', raw, 27)
            if speed > desired: speed -= 1
            elif speed < desired and tick & 15 == 0: speed += 1
            struct.pack_into('<h', raw, 27, speed)
            heading_byte = heading >> 8
            heading_byte = (heading_byte + 128) % 256 - 128
            current = (raw[48] + 128) % 256 - 128
            raw[48] = (raw[48] + (heading_byte > current) - (heading_byte < current)) % 256
            desired_heading = struct.unpack_from('<H', raw, 46)[0]
            difference = signed(desired_heading - heading)
            amount = min(abs(difference), 45 + (roll & 15))
            heading = (heading + (-amount if difference < 0 else amount)) % 65536
            struct.pack_into('<H', raw, 16, heading)
            countdown = (struct.unpack_from('<H', raw, 35)[0] - 1) % 65536
            struct.pack_into('<H', raw, 35, countdown)
            if countdown & 31 == 0:
                if raw[24]:
                    desired_heading = (desired_heading + 1456) % 65536
                    struct.pack_into('<H', raw, 46, desired_heading); raw[48] = desired_heading >> 8
                else:
                    allocation = pool.explosion()
                    if allocation:
                        created_effect = new_effect(allocation, (x, y, z), (20, 768, 4, 10, 9))
                        effects[allocation[1]] = created_effect
                    sound = 9; released = True; live = False; raw[22] |= 1
                    pool.release(primary)
            if raw[51] > 10 and tick & 3 == 0 and random() & 3 == 0:
                allocation = pool.allocate_short(17, low_priority=True) if case['enabled'] == 1 else None
                if allocation:
                    extent = (384 + (random() & 63)) % 65536
                    smoke = bytearray(55); struct.pack_into('<H', smoke, 0, 17)
                    struct.pack_into('<3iH2H', smoke, 4, x, y, delta(z + 768, 0), 0, extent, extent * 4 % 65536)
                    created_smoke = allocation, smoke; smokes[allocation[1]] = created_smoke
            # The replacement constructor's two aircraft motion fields are zero.
            if not (released and created_smoke and created_smoke[0][1] == primary[1]):
                vx, vy = rotate(raw[48] << 8, speed, case['coarse'])
                struct.pack_into('<2i', raw, 4, delta(x + vx, 0), delta(y + vy, 0))
        output += line('tick', [tick, animation, int(updated), int(released), sound,
                                int(created_effect is not None), int(created_smoke is not None)])
        if updated: output += actor_lines(raw, primary)
        if created_effect: output += effect_lines(created_effect)
        if created_smoke: output += smoke_lines(created_smoke[1], created_smoke[0])
        if capture is not None:
            capture(dict(raw=raw, words=words, cursor=cursor, smoke=created_smoke,
                         effect=created_effect, released=released, sound=sound))
        if class_only:
            return output + shared()
        for slot, (allocation, smoke) in sorted(list(smokes.items())):
            gone = case['enabled'] != 1
            if not gone:
                x, y, z = struct.unpack_from('<3i', smoke, 4)
                low = (z + 8) % 65536; z = delta((z % 2**32 & 0xffff0000) | low, 0)
                struct.pack_into('<3i', smoke, 4, delta(x + case['wind'][0], 0), delta(y + case['wind'][1], 0), z)
                counter = (struct.unpack_from('<H', smoke, 26)[0] + 1) % 65536
                if counter >= 12:
                    counter = 0; smoke[25] = (smoke[25] + 1) % 256; gone = smoke[25] >= 30
                struct.pack_into('<H', smoke, 26, counter)
            if gone: smoke[22] |= 1; pool.release(allocation); del smokes[slot]
            output += smoke_lines(smoke, allocation)
        for slot, (allocation, effect) in sorted(list(effects.items())):
            frame, countdown, height, released = advance_explosion(
                effect[25], effect[30], effect[31], effect[32],
                struct.unpack_from('<H', effect, 26)[0], struct.unpack_from('<H', effect, 28)[0])
            effect[25], effect[32] = frame, countdown
            struct.pack_into('<H', effect, 28, height)
            if released:
                effect[22] |= 1; pool.release(allocation); del effects[slot]
            output += effect_lines((allocation, effect))
        output += shared()
    return output


class AircraftDeathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-aircraft-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'input.bin'
        cls.commands = []
        if TARGET in ('all', 'native'): cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_aircraft_death_probe')])
        if TARGET in ('all', 'wasm'): cls.commands.append(['node', str(BUILD / 'wasm/fist_aircraft_death_probe.js')])
        cls.fixtures = cls.updates = cls.smokes = cls.effects = cls.releases = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Aircraft per target: {cls.fixtures} fixtures, {cls.updates} class updates, {cls.smokes} smoke states, {cls.effects} effect states, {cls.releases} class releases', flush=True)
        if ORACLE:
            if ORACLE.repaired_emissions < 2:
                raise AssertionError('Missing required independently proved emitter-loss cases')
            print(f'Original lost-emitter constructors independently proved: {ORACLE.repaired_emissions}', flush=True)

    def check(self, cases):
        wanted = ''.join(expected(case) for case in cases)
        if ORACLE: self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=90)
            self.assertEqual(result.returncode, 0, result.stderr); self.assertEqual(result.stdout, wanted)
        type(self).fixtures += len(cases); type(self).updates += sum(int(row.split()[3]) for row in wanted.splitlines() if row.startswith('tick '))
        type(self).releases += sum(int(row.split()[4]) for row in wanted.splitlines() if row.startswith('tick '))
        type(self).smokes += wanted.count('smoke '); type(self).effects += wanted.count('effect ')

    def batches(self, cases, size=96):
        for start in range(0, len(cases), size): self.check(cases[start:start + size])

    def test_altitude_byte_ground_offset_phase_and_flags(self):
        self.batches([fixture(kind, altitude=delta(0x1234007b + altitude * 256, 0), pixels=bytes([height]) * 4,
                              offset=offset, tick=tick, countdown=2)
                      for kind in (5, 6) for altitude in (0, 1, 127, 128, 254, 255)
                      for height in range(256) for offset in (0, 1, 127, 255) for tick in (0, 1)])
        cases = []
        for flags in range(256):
            case = fixture(tick=1); raw = bytearray(case['raw']); raw[22]=flags; raw[23]=255-flags; case['raw']=bytes(raw); cases.append(case)
        self.batches(cases)

    def test_signed_speed_heading_motion_and_countdown_boundaries(self):
        self.batches([fixture(speed=speed, target_speed=target, tick=tick, countdown=countdown)
                      for speed in (-32768, -32767, -1, 0, 1, 31, 32, 32766, 32767)
                      for target in (-32768, -1, 0, 32, 32767) for tick in (0, 1, 15, 16, 65535)
                      for countdown in (0, 1, 2, 31, 32, 33, 65535)])
        self.batches([fixture(heading=heading, target_heading=(heading+difference)%65536, motion=motion,
                              tick=1, countdown=2, coarse=coarse)
                      for heading in (0, 1, 32767, 32768, 65535) for difference in (-32768,-61,-60,-45,-1,0,1,45,60,61,32767)
                      for motion in range(256) for coarse in (0, 1)])

    def test_random_quality_damage_rotor_and_capacity(self):
        self.batches([fixture(seeds=(seed, 2, 0, 0), cursor=cursor, enabled=setting, countdown=2,
                              animation=setting, damage=damage)
                      for seed in range(32) for cursor in range(4) for setting in (0, 1, 2, 255)
                      for damage in (0, 10, 11, 100, 255)])
        self.batches([fixture(enabled=setting, animation=setting, altitude=0, ground=0) for setting in range(256)])
        self.batches([fixture(bindings=[(5,181,1)]+[(21,index,1) for index in range(count-1)],
                              altitude=0, ground=0, ticks=2) for count in range(1,151)])

    def test_full_death_reaching_damage_reuse_and_natural_cleanup(self):
        self.check([fixture(kind, altitude=altitude*256+123, ticks=1500, countdown=countdown)
                    for kind in (5,6) for altitude in (0,1,32,255) for countdown in (0,1,32)])
        self.check([fixture(kind, operation=1, damage=99, altitude=32*256+123, ticks=700,
                            seeds=(2,seed,2,2)) for kind in (5,6) for seed in (0,1,2,256)])
        self.check([fixture(tick=65535,animation=65535,ticks=65),
                    fixture(altitude=252*256+255, pixels=bytes([252])*4, ground=0, ticks=365),
                    fixture(bindings=[(21,0,1),(21,1,1),(5,5,1)], target=2, releases=(0,1),
                            altitude=0, ground=0, ticks=365)])

    def test_malformed_and_unsupported_requests_fail(self):
        valid=encode([fixture()]); invalid=[valid[:n] for n in (0,3,4,39,90,len(valid)-1)]+[valid+b'\0']
        for behavior in (0,2,4,10,255):
            case=fixture();raw=bytearray(case['raw']);raw[37]=behavior;case['raw']=bytes(raw);invalid.append(encode([case]))
        for contents in invalid:
            self.path.write_bytes(contents)
            for command in self.commands:
                result=subprocess.run([*command,str(self.path)],capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,1)

    def test_required_mission_positions_complete_occupancy_and_constructed_successors(self):
        if not ORIGINALS: self.skipTest('The complete pinned corpus is an explicit additional gate')
        manifest=json.loads((ROOT/'tools/rewrite/scenario_originals.json').read_text());self.assertEqual(len(manifest),47)
        cases=[];saved_aircraft=0;positions=0
        for name,info in manifest.items():
            data=(ROOT/'armoredfist/FISTDATA'/name).read_bytes();self.assertEqual(hashlib.sha256(data).hexdigest(),info['sha256'])
            records=records_from_scenario(data)
            bindings=[(int.from_bytes(raw[:2],'little'),index,value) for index,value,raw in records]
            saved_aircraft+=sum(kind in (5,6) for kind,_,_ in bindings)
            used={index for _,index,_ in bindings};vacancies=[index for index in range(182) if index not in used]
            for _,_,ground_raw in records:
                if int.from_bytes(ground_raw[:2],'little')>=4:continue
                positions+=1
                for kind in (5,6):
                    case=fixture(kind,operation=1,damage=99,altitude=32*256+123,
                                 bindings=[(8,vacancies[0],1),(kind,vacancies[1],1)]+bindings,
                                 target=1,ticks=2,side=4,pixels=varied_plane(4))
                    raw=bytearray(case['raw']);raw[4:12]=ground_raw[4:12];raw[16:18]=ground_raw[16:18];case['raw']=bytes(raw);cases.append(case)
        self.assertEqual(saved_aircraft,0);self.assertEqual(positions,960);self.assertEqual(len(cases),1920)
        self.batches(cases,16)
        print('Aircraft corpus: no saved type-5/6 records; 1920 constructed critical successors at all 960 ground positions, all 47 complete occupancy contexts',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--target',choices=['all','native','wasm'],default=TARGET)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true');parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();BUILD,TARGET,NATIVE_PROBE,ORIGINALS=args.build_root,args.target,args.native_probe,args.originals
    if args.oracle:
        from original_aircraft_death_oracle import OriginalAircraftDeathOracle
        ORACLE=OriginalAircraftDeathOracle()
    program=unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped)!=(0 if ORIGINALS else 1))
