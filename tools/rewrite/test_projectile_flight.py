#!/usr/bin/env python3
"""Consuming untargeted shell flight, post-damage impact continuation and effect lifecycle."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_collision import body, delta, expected as collision_expected, from_raw
from test_object_pool import EXTENDED, NONE
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def fixture(*, bodies=None, operation=0, finish=0, ticks=1, age=0, grace=0,
            frame=0, last=22, period=6, countdown=6, velocity=(0, 0, 0), origin=1,
            seeds=(2, 76, 78, 1), cursor=0, heights=(0, 0, 0, 0)):
    return dict(bodies=bodies or [body(8, index=10, altitude=65536, flags=0),
                                  body(index=1, x=100000)], operation=operation,
                finish=finish, ticks=ticks, age=age, grace=grace, frame=frame, last=last,
                period=period, countdown=countdown, velocity=velocity, origin=origin,
                seeds=seeds, cursor=cursor, heights=heights)


def encode(cases):
    data = struct.pack('<I', len(cases))
    for case in cases:
        data += struct.pack('<BB4H4B3hH4H2B', case['operation'], case['finish'],
                            len(case['bodies']), case['ticks'], case['age'], case['grace'],
                            case['frame'], case['last'], case['period'], case['countdown'],
                            *case['velocity'], case['origin'], *case['seeds'], case['cursor'], 0)
        data += b''.join(struct.pack('<HHHiiiHHBB', *item) for item in case['bodies'])
        data += bytes(case['heights'])
    return data


class Pool:
    def __init__(self, bodies):
        self.slots = [(0, 0)] * 182
        self.registry = [(NONE, 0)] * 182
        self.allocations = []
        for kind, index, value, *_ in bodies:
            area = range(150, 182) if kind in EXTENDED else range(150)
            slot = next(slot for slot in area if not self.slots[slot][0])
            self.slots[slot] = 1, kind
            self.registry[index] = slot, value
            self.allocations.append((kind, slot, index, value))

    def release(self, allocation):
        _, slot, index, value = allocation
        self.slots[slot] = 0, 0
        self.registry[index] = NONE, (value - 1) % 65536

    def allocate_short(self, kind, *, low_priority=False):
        if low_priority and sum(used for used, _ in self.slots[:150]) >= 120:
            return None
        slot = next((slot for slot in range(150) if not self.slots[slot][0]), None)
        index = next((index for index in range(182) if self.registry[index] == (NONE, 0)), None)
        if slot is None or index is None:
            return None
        self.slots[slot] = 1, kind
        self.registry[index] = slot, 1
        return kind, slot, index, 1

    def explosion(self):
        return self.allocate_short(4)

    def state(self):
        counts = sum(used for used, _ in self.slots[:150]), sum(used for used, _ in self.slots[150:])
        return (f'counts {counts[0]} {counts[1]}\nslots' + ''.join(f' {used}:{kind}' for used, kind in self.slots) +
                '\nregistry' + ''.join(f' {slot}:{value}' for slot, value in self.registry) + '\n')


def line(label, values):
    return label + ' ' + ' '.join(map(str, values)) + '\n'


def expected(case):
    pool = Pool(case['bodies'])
    kind, _, _, x, y, altitude, heading, scale, flags, mode = case['bodies'][0]
    allocation = pool.allocations[0]
    age, grace, frame = case['age'], case['grace'], case['frame']
    last, period, countdown = case['last'], case['period'], case['countdown']
    height_offset, ground = age, frame
    phase = 0
    seeds, cursor = case['seeds'], case['cursor']
    origin = pool.allocations[case['origin']][1]
    output = ''
    for _ in range(case['ticks']):
        if case['operation'] == 0:
            age = (age + 1) % 65536
            hit = (NONE, NONE, 0, 0)
            if age >= 480:
                phase, flags = 3, flags | 1
                pool.release(allocation)
            else:
                x, y, altitude = (delta(value + velocity, 0) for value, velocity in
                                   zip((x, y, altitude), case['velocity']))
                if (altitude >> 8) % 65536 < 128:
                    column = ((x << 13) % 2**32) >> 31
                    row = ((-y << 13) % 2**32) >> 31
                    ground = case['heights'][row * 2 + column]
                    if ((altitude >> 8) - ground) % 256 >= 128:
                        phase = 1
                if phase == 0:
                    if grace:
                        grace -= 1
                    else:
                        bodies = list(case['bodies'])
                        bodies[0] = body(kind, index=bodies[0][1], value=bodies[0][2], x=x, y=y,
                                         altitude=altitude, heading=heading, scale=scale, flags=flags, mode=mode)
                        collision = collision_expected((bodies, (0,), seeds, cursor)).splitlines()
                        candidate = tuple(map(int, collision[0].split()[1:]))
                        cursor, *seeds = map(int, collision[1].split()[1:])
                        if candidate[0] not in (NONE, origin):
                            hit, phase = candidate, 2
            shell = [x, y, altitude, heading, *case['velocity'], age, grace, flags,
                     ground, mode, origin, NONE, phase]
            output += line('hit', hit) + line('shell', shell) + line('random', [cursor, *seeds])
            if phase in (1, 2) and case['finish']:
                explosion = pool.explosion()
                pool.release(allocation)
                output += line('impact', [int(explosion is not None), phase, 15, int(phase == 2)])
                if explosion:
                    unit = phase == 2
                    output += line('explosion', [*explosion, x, y, altitude, 20 if unit else 16,
                                                 256 if unit else 768, 2048, 4 if unit else 0,
                                                 0, 0, 10 if unit else 22, 5 if unit else 6,
                                                 5 if unit else 6, 0])
                phase, flags = 3, flags | 1
                shell[-1], shell[9] = phase, flags
                output += line('shell', shell)
        elif case['operation'] == 1:
            countdown = (countdown - 1) % 256
            if countdown == 0:
                countdown = period
                if frame == last:
                    flags |= 1
                    pool.release(allocation)
                else:
                    frame = (frame + 1) % 256
                    if grace & 6 == 0:
                        height_offset = {13: 768, 16: 512, 19: 256}.get(frame, height_offset)
                    elif grace & 6 == 4:
                        height_offset = {6: 512, 8: 256}.get(frame, height_offset)
            output += line('explosion', [*allocation, x, y, altitude, heading, 512, scale,
                                         grace, height_offset, frame, last, period, countdown, flags])
        else:
            age = (age + 1) % 65536
            if age >= 8:
                age = 0
                frame = (frame + 1) % 256
                if frame >= 7:
                    flags |= 1
                    pool.release(allocation)
            output += line('muzzle', [age, frame, flags])
        output += pool.state()
        if (case['operation'] == 0 and phase != 0) or flags & 1:
            break
    return output


class FlightTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-flight-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.commands = []
        cls.fixture_count = cls.observation_count = 0
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_projectile_flight_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_projectile_flight_probe.js')])

    @classmethod
    def tearDownClass(cls):
        print(f'Flight verification: {cls.fixture_count} valid fixtures, {cls.observation_count} complete updates per target', flush=True)

    def check(self, cases):
        wanted = ''.join(expected(case) for case in cases)
        type(self).fixture_count += len(cases)
        type(self).observation_count += wanted.count('counts ')
        if ORACLE:
            self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, wanted)
        return wanted

    def test_flight_age_grace_wrapped_integration_and_expiry(self):
        cases = []
        for age in (0, 15, 16, 39, 40, 478, 479, 480, 65534, 65535):
            for grace in (0, 1, 2, 65535):
                cases.append(fixture(age=age, grace=grace, ticks=4, velocity=(-32768, 32767, 0)))
        for position in (-2147483648, -1, 0, 2147483647):
            for velocity in (-32768, -1, 0, 1, 32767):
                cases.append(fixture(bodies=[body(8, index=10, x=position, y=position,
                                                  altitude=2147483647, flags=0), body(x=1000000)],
                                     velocity=(velocity, velocity, velocity), ticks=4))
        self.check(cases)
        # Every valid launched shell reaches its exact expiry, without integration on tick 480.
        result = self.check([fixture(ticks=480, velocity=(0, 852, 0), grace=2)])
        self.assertIn('shell 0 408108 65536 0 0 852 0 480 0 1', result)

    def test_ground_gate_byte_sign_order_and_current_position(self):
        cases = []
        for ground in range(256):
            for altitude_byte in (*range(128), 128, 129, 255):
                cases.append(fixture(bodies=[body(8, index=10, altitude=altitude_byte * 256, flags=8),
                                              body(x=100000)], heights=(ground,) * 4, grace=2,
                                     finish=1))
        for altitude in (-2147483648, -1, 0, 32767, 32768, 65535, 65536,
                         16777216, 16777217, 2147483647):
            cases.append(fixture(bodies=[body(8, index=10, altitude=altitude, flags=0),
                                          body(x=100000)], heights=(127,) * 4, finish=1))
        for x in (-1, 0, 262143, 262144, 524287, 524288):
            for y in (-1, 0, 262143, 262144, 524287, 524288):
                cases.append(fixture(bodies=[body(8, index=10, x=x, y=y, altitude=0, flags=0),
                                              body(x=100000)], heights=(0, 10, 20, 30),
                                     velocity=(1, -1, 0), grace=2, finish=1))
        for offset in range(0, len(cases), 256):
            self.check(cases[offset:offset + 256])

    def test_post_integration_first_hit_origin_order_and_random_consumption(self):
        cases = []
        for origin_index, target_index in ((0, 1), (1, 0), (0, 0)):
            cases.append(fixture(bodies=[body(8, index=10, altitude=65536, flags=0),
                                          body(index=origin_index, altitude=65536),
                                          body(index=target_index, altitude=65536)], finish=1))
        for grace in (0, 1, 2):
            cases.append(fixture(bodies=[body(8, index=10, altitude=65536, flags=0),
                                          body(index=1, x=100000),
                                          body(index=0, x=852, altitude=65536, scale=0)],
                                 velocity=(852, 0, 0), grace=grace, ticks=4, finish=1))
        for cursor in range(4):
            for sample in range(256):
                seeds = [2] * 4; seeds[cursor] = 2 * (sample + 1)
                cases.append(fixture(bodies=[body(8, index=10, altitude=65536, flags=0),
                                              body(index=20, x=100000),
                                              body(5, index=0, altitude=65536),
                                              body(6, index=1, altitude=65536)],
                                     seeds=tuple(seeds), cursor=cursor, finish=1))
        for offset in range(0, len(cases), 256):
            self.check(cases[offset:offset + 256])

    def test_explosion_allocation_before_release_normal_priority_and_saved_binding(self):
        cases = []
        for count in (1, 119, 120, 149, 150):
            for value in (0, 1, 12, 65535):
                bodies = [body(8, index=181, value=value, altitude=0, flags=8),
                          body(index=180, x=100000)]
                bodies += [body(21, index=index, flags=0) for index in range(count - 1)]
                cases.append(fixture(bodies=bodies, heights=(1,) * 4, finish=1))
        output = self.check(cases)
        self.assertEqual(output.count('impact 0 1 15 0'), 4)
        self.assertEqual(output.count('impact 1 1 15 0'), 16)

    def test_complete_explosion_and_muzzle_sequences_and_wrapped_animation_fields(self):
        cases = []
        for callback, last, period in ((0, 22, 6), (4, 10, 5)):
            cases.append(fixture(operation=1, bodies=[body(4, flags=0)], origin=0,
                                 grace=callback, last=last, period=period, countdown=period, ticks=200))
        cases.append(fixture(operation=2, bodies=[body(18, flags=0)], origin=0, ticks=60))
        for frame in range(256):
            for counter in (0, 7, 8, 65534, 65535):
                cases.append(fixture(operation=2, bodies=[body(18, flags=8)], origin=0,
                                     frame=frame, age=counter, ticks=2))
            for callback in (0, 2, 4, 6, 65535):
                cases.append(fixture(operation=1, bodies=[body(4, flags=8)], origin=0,
                                     frame=frame, age=65535, grace=callback, countdown=1, ticks=2))
        for period in range(256):
            cases.append(fixture(operation=1, bodies=[body(4, flags=0)], origin=0,
                                 frame=0, last=1, period=period, countdown=1, ticks=3))
        for countdown in range(256):
            cases.append(fixture(operation=1, bodies=[body(4, flags=0)], origin=0,
                                 frame=22, countdown=countdown, ticks=2))
        for offset in range(0, len(cases), 256):
            self.check(cases[offset:offset + 256])

    def test_missing_truncated_invalid_fields_and_live_binding(self):
        valid = encode([fixture()])
        invalid = [valid[:length] for length in (0, 3, 4, 35, len(valid) - 1)]
        invalid += [valid + b'\0']
        for position, value in ((4, 3), (5, 2), (34, 4), (35, 1)):
            data = bytearray(valid); data[position] = value; invalid.append(bytes(data))
        # Directed/scheduled orphan projectiles are not accepted by this untargeted update.
        invalid += [encode([fixture(bodies=[body(8, index=0, flags=0), body(index=0)])])]
        for data in invalid:
            self.path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 1, result.stderr)

    def test_original_m1_launch_positions_complete_mission_collision_worlds(self):
        if not ORIGINALS:
            self.skipTest('Complete original mission corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        cases = []
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            bodies = [from_raw(index, value, raw) for index, value, raw in records]
            index = next(index for index in range(182) if all(item[1] != index for item in bodies))
            for ordinal, actor in enumerate(bodies):
                if actor[0] == 0:
                    _, _, _, x, y, altitude, heading, *_ = actor
                    shell = body(8, index=index, x=x, y=y, altitude=delta(altitude + 2048, 0),
                                  heading=heading, flags=actor[8] & 8)
                    # Snapshot contact/elevation are not initialized; this explicitly exercises
                    # installed collision occupancy at authored launch positions, not mission play.
                    cases.append(fixture(bodies=[shell, *bodies], origin=ordinal + 1,
                                         ticks=4, velocity=(0, 852, 0), grace=2))
        self.assertEqual(len(cases), 179)
        for offset in range(0, len(cases), 32):
            self.check(cases[offset:offset + 32])
        print(f'Flight corpus: 179 M1 launch positions, all {len(manifest)} complete mission worlds, {len(self.commands)} targets', flush=True)


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
        from original_projectile_flight_oracle import OriginalProjectileFlightOracle
        ORACLE = OriginalProjectileFlightOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (0 if ORIGINALS else 1))
