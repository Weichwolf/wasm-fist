#!/usr/bin/env python3
"""Shared original ground motion, manual turret and complete heading rotation."""
import argparse
import hashlib
import json
import math
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario, snapshot
from test_vehicle_start import initialized, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
LIMITS = (
    [64] * 17 + [61, 58, 54, 50, 45, 32, 24, 16, 8, 0, 0, 0, 0, 0, 0],
    [32] * 26 + [28, 24, 20, 16, 12, 8], [64] * 32, [32] * 32)
KNOTS = [round(math.sin(index * math.pi / 512) * 65536) for index in range(257)]
DIRTY = ((0xcb, 0xca), (0xc5, 0xec), (0xc9, 0xc8), (0xc4, 0xce))


def signed(value):
    return (value + 32768) % 65536 - 32768


def coefficient(angle, coarse):
    folded = angle % 32768
    folded = min(folded, 32768 - folded)
    if folded == 16384:
        return 65536
    index, fraction = divmod(folded, 64)
    result = KNOTS[index]
    if not coarse:
        result += ((KNOTS[index + 1] - KNOTS[index]) * fraction * 4 + 128) // 256
    return result


def rotate(heading, magnitude, coarse):
    def lane(angle):
        value = abs(magnitude) * coefficient(angle, coarse) // 65536
        return signed(-value if (angle % 65536 >= 32768) != (magnitude < 0) else value)
    return lane(heading), lane(16384 - heading)


def rotate_spatial(heading, elevation, magnitude, coarse):
    # Original 0487 destroys the coefficient's full-scale carry, so the
    # horizontal stage uses the returned 16-bit coefficient even at the pole.
    horizontal = abs(magnitude) * min(65535, coefficient(16384 - elevation, coarse)) // 65536
    if ((16384 - elevation) % 65536 >= 32768) != (magnitude < 0):
        horizontal = -horizontal
    return (*rotate(heading, horizontal, coarse), rotate(elevation, magnitude, coarse)[0])


def start(kind=0, **fields):
    initial, _ = initialized([(0, 0, snapshot(kind, flags=0, pose=(0, 0, 0), heading=0))],
                             (0, 0, 0, 0), 0, 0)
    raw = bytearray(initial[0][2])
    struct.pack_into('<H', raw, 0x97, 0)
    for offset in (0x26, 0x30, 0x34, 0x55, 0x57, 0x59, 0x5b, 0x89, 0x8b):
        struct.pack_into('<H', raw, offset, 0)
    raw[0x19] = raw[0x3d] = 0
    offsets = {'speed': 0x55, 'throttle': 0x57, 'pitch': 0x34, 'hull': 0x26,
               'request': 0x30, 'offset': 0x89, 'turret_request': 0x8b, 'gate': 0x5d,
               'control': 0x40}
    for name, value in fields.items():
        if name in ('flags', 'phase', 'operating', 'profile'):
            raw[{'flags': 0x19, 'phase': 0x3d, 'operating': 0x1a, 'profile': 0x90}[name]] = value
        elif name in ('x', 'y'):
            struct.pack_into('<i', raw, 4 if name == 'x' else 8, value)
        else:
            struct.pack_into('<H', raw, offsets[name], value % 65536)
    return bytes(raw)


def update(original):
    raw = bytearray(original)
    def word(offset):
        return struct.unpack_from('<H', raw, offset)[0]
    def put(offset, value):
        struct.pack_into('<H', raw, offset, value % 65536)
    kind = word(0)
    events = [0, 0, 0]
    direction = raw[0x19] & 6
    if raw[0x3d] & 3 == 0:
        if raw[0x19] & 16 or direction == 6 or word(0x5d) == 0:
            put(0x57, 0)
        slope = max(0, min(31, signed(word(0x34)) // 512 + 16))
        target = min(signed(word(0x57)) // 4, LIMITS[raw[0x90] & 3][slope])
        speed = signed(word(0x55))
        step = (target > speed) - (target < speed)
        blocked = direction and raw[0x3d] & 12 and (
            (step > 0 and speed >= 0) or (step < 0 and speed < 0))
        if step and not blocked:
            put(0x55, speed + step)
            if word(0x55):
                put(0x40, word(0x40) & 0xffef)
            events[0] = 1
            raw[DIRTY[kind][0]] = 3
    if not (raw[0x19] & 22 or word(0x5d) == 0):
        if raw[0x1a] & 128:
            offset = signed(word(0x89))
            step = max(-910, min(910, offset))
            if step == offset:
                raw[0x1a] &= 127
            for address in (0x26, 0x30):
                put(address, word(address) + step)
            for address in (0x89, 0x8b):
                put(address, word(address) - step)
            events[1] = 1
        else:
            step = max(-182, min(182, signed(word(0x30) - word(0x26))))
            put(0x26, word(0x26) + step)
            events[1] = int(step != 0)
    magnitude = signed(word(0x55)) // 2
    if direction in (2, 4):
        put(0x26, word(0x26) + magnitude * (8 if direction == 4 else -8))
        put(0x30, word(0x26))
    velocity = rotate(word(0x26), magnitude, False)
    for offset, value in zip((0x59, 0x5b), velocity):
        put(offset, value)
    for offset, value in zip((4, 8), velocity):
        position = struct.unpack_from('<i', raw, offset)[0]
        position = (position + value + 2147483648) % 4294967296 - 2147483648
        struct.pack_into('<i', raw, offset, position)
    step = max(-364, min(364, signed(word(0x8b) - word(0x89))))
    put(0x89, word(0x89) + step)
    put(0x10, word(0x26) + word(0x89))
    events[2] = int(step != 0)
    raw[0xa9:0xac] = b'\x80\0\0'
    if events[1] or events[2]:
        raw[DIRTY[kind][1]] = 3
        if kind == 3:
            raw[0xd1] = 3
    return bytes(raw), tuple(events)


class MotionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-motion-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_vehicle_motion_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_vehicle_motion_probe.js')])

    def run_probe(self, mode, data, expected, valid=True):
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(data)
        for command in self.commands:
            with self.subTest(target=command[0], mode=mode):
                result = subprocess.run([*command, mode, str(path)], capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, expected)

    def rotations(self, cases):
        values = [rotate(*case) for case in cases]
        if ORACLE is not None:
            self.assertEqual(ORACLE.rotations(cases), values)
        data = struct.pack('<I', len(cases)) + b''.join(struct.pack('<HhB', *case) for case in cases)
        self.run_probe('rotation', data, ''.join(f'velocity {x} {y}\n' for x, y in values))

    def spatial_rotations(self, cases):
        values = [rotate_spatial(*case) for case in cases]
        if ORACLE is not None:
            self.assertEqual(ORACLE.spatial_rotations(cases), values)
        data = struct.pack('<I', len(cases)) + b''.join(struct.pack('<HHhB', *case) for case in cases)
        self.run_probe('spatial', data, ''.join(f'velocity {x} {y} {z}\n' for x, y, z in values))
        return values

    def test_spatial_rotation_complete_heading_and_elevation_turns(self):
        for coarse in (0, 1):
            self.spatial_rotations([(heading, 7312 if coarse == 0 else 58224, 853, coarse)
                                    for heading in range(65536)])
            self.spatial_rotations([(49999, elevation, -32768, coarse)
                                    for elevation in range(65536)])

    def test_spatial_rotation_every_signed_magnitude_and_truncation_boundaries(self):
        self.spatial_rotations([(65501, 32799, magnitude, 0)
                                for magnitude in range(-32768, 32768)])
        angles = (0, 1, 31, 32, 33, 63, 64, 8192, 16351, 16352, 16383, 16384,
                  16385, 32767, 32768, 49151, 49152, 49153, 65535)
        self.spatial_rotations([(heading, elevation, magnitude, coarse)
                                for heading in angles for elevation in angles
                                for magnitude in (-32768, -32767, -1, 0, 1, 2, 853, 32767)
                                for coarse in (0, 1)])
        cases = [(0, 0, 853, 0), (0, 0, 1, 0), (8192, 32768, -32768, 0),
                 (8192, 8192, 2, 0), (0, 16384, 853, 0), (0, 49152, 853, 0)]
        self.assertEqual(self.spatial_rotations(cases),
                         [(0, 852, 0), (0, 0, 0), (23169, 23169, 0), (0, 0, 1),
                          (0, 0, 853), (0, 0, -853)])
        # These represent the reaching mistakes made by substituting the planar
        # routine or keeping one real-valued product until the final rounding.
        self.assertNotEqual(rotate(0, 853, 0), (0, 852))
        self.assertNotEqual((math.floor(math.sin(math.pi / 4) ** 2 * 7),) * 2,
                            rotate_spatial(8192, 8192, 7, 0)[:2])

    def test_spatial_rotation_rejects_incomplete_and_invalid_requests(self):
        record = struct.pack('<HHhB', 0, 0, 853, 0)
        valid = struct.pack('<I', 1) + record
        for data in (b'', valid[:-1], valid + b'x', struct.pack('<I', 2) + record,
                     struct.pack('<I', 0xffffffff) + record,
                     struct.pack('<I', 2) + record + record[:-1] + b'\x02'):
            self.run_probe('spatial', data, '', valid=False)
        self.run_probe('spatial', struct.pack('<I', 0), '')

    def test_primary_m1_launch_velocity_from_complete_original_shots(self):
        cases = [(heading, elevation, 853, 0)
                 for heading in (0, 1, 8192, 16384, 32768, 49152, 65535)
                 for elevation in (0, 1, 8192, 16384, 32768, 49152, 65535)]
        # A weapon handler uses the normal interpolated mode. Coarse rotation is
        # checked separately above; do not change its original machine boundary.
        originals = []
        for heading, elevation, _, _ in cases:
            raw = bytearray(start())
            struct.pack_into('<H', raw, 0x10, heading)
            struct.pack_into('<H', raw, 0x38, elevation)
            raw[0x92] = 48
            originals.append(bytes(raw))
        if ORIGINALS:
            manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
            files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
            self.assertEqual([path.name for path in files], sorted(manifest))
            for path in files:
                data = path.read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
                states, _ = initialized(records_from_scenario(data), (0, 0, 0, 0), 0, 0)
                for _, _, original in states:
                    if struct.unpack_from('<H', original)[0] != 0:
                        continue
                    raw = bytearray(original)
                    struct.pack_into('<H', raw, 0x97, 0)  # Explicit untargeted handler boundary.
                    raw[0x92] = 48
                    originals.append(bytes(raw))
                    heading, = struct.unpack_from('<H', raw, 0x10)
                    elevation, = struct.unpack_from('<H', raw, 0x38)
                    cases.append((heading, elevation, 853, 0))
            self.assertEqual(len(cases), 49 + 179)
        values = self.spatial_rotations(cases)
        if ORACLE is not None:
            shots = ORACLE.m1_primary_shots(originals)
            self.assertEqual(len(shots), len(cases))
            for ordinal, (actor, objects, carry) in enumerate(shots):
                self.assertEqual(carry, 0)
                self.assertEqual([(index, generation, struct.unpack_from('<H', raw)[0])
                                  for index, generation, raw in objects], [(0, 1, 8), (1, 1, 18)])
                shell = objects[0][2]
                self.assertEqual(struct.unpack_from('<hhh', shell, 0x1d), values[ordinal])
                self.assertEqual(struct.unpack_from('<H', shell, 0x1b)[0], 853)
                self.assertEqual(struct.unpack_from('<H', actor, 0xad)[0], 14)
                self.assertEqual((actor[0xa8], actor[0x3c], actor[0x92]), (20, 16, 0))
            empty = bytearray(start())
            struct.pack_into('<H', empty, 0xad, 0)
            empty[0x92] = 48
            self.assertEqual(ORACLE.m1_primary_shots([bytes(empty)]), [(bytes(empty), [], 1)])

    def motion(self, cases):
        expected = []
        for raw, steps in cases:
            for _ in range(steps):
                raw, events = update(raw)
                expected.append((raw, events))
                raw = raw[:0x3d] + bytes([(raw[0x3d] + 2) % 256]) + raw[0x3e:]
        if ORACLE is not None:
            self.assertEqual(ORACLE.motion(cases), expected)
        text = ''.join(state_lines([(0, 0, raw)]) + 'events ' + ' '.join(map(str, events)) + '\n'
                       for raw, events in expected)
        data = struct.pack('<I', len(cases)) + b''.join(raw + struct.pack('<H', steps)
                                                      for raw, steps in cases)
        self.run_probe('motion', data, text)
        return expected

    def test_complete_heading_turn_in_both_math_modes(self):
        for magnitude, coarse in ((321, 0), (-16384, 1)):
            self.rotations([(heading, magnitude, coarse) for heading in range(65536)])

    def test_rotation_cardinal_interpolation_and_every_signed_magnitude(self):
        headings = (0, 1, 31, 32, 33, 63, 64, 16351, 16352, 16383, 16384,
                    16385, 32767, 32768, 49152, 65535)
        self.rotations([(heading, magnitude, coarse) for heading in headings
                        for magnitude in (-32768, -32767, -1, 0, 1, 32767) for coarse in (0, 1)])
        self.rotations([(16384, magnitude, 0) for magnitude in range(-32768, 32768)])
        self.assertEqual(rotate(0, 321, 0), (0, 321))
        self.assertEqual(rotate(33, 321, 0)[1], 320)
        self.assertEqual(rotate(32, 321, 0)[1], 321)

    def test_phase_motion_flags_gates_and_speed_sign(self):
        cases = []
        for value in range(256):
            for kind in range(4):
                for speed, throttle in ((-1, -254), (0, 254), (4, -254)):
                    cases.append((start(kind, flags=value, phase=value, speed=speed,
                                        throttle=throttle, hull=65500, request=100,
                                        offset=65500, turret_request=100), 1))
        self.motion(cases)
        self.motion([(start(kind, gate=gate, flags=flags, phase=phase, speed=speed,
                            throttle=254, request=32768), 1)
                     for kind in range(4) for gate in (0, 1, 65535) for flags in (0, 2, 4, 6, 16)
                     for phase in (0, 1, 4, 12, 252, 255) for speed in (-1, 0, 1)])

    def test_pitch_bins_and_complete_four_speed_profiles(self):
        # Batch all signed pitches without starting 262144 separate processes.
        self.motion([(start(profile=profile, pitch=pitch, speed=50, throttle=254), 1)
                     for profile in range(4) for pitch in range(-32768, 32768, 64)])
        self.motion([(start(profile=profile, pitch=pitch, speed=32, throttle=254), 1)
                     for profile in range(256) for pitch in (-32768, -8193, -8192, -1, 0,
                                                            511, 512, 5119, 5120, 32767)])
        observed = self.motion([(start(profile=profile, pitch=(index - 16) * 512,
                                      throttle=32767), 256)
                               for profile in range(4) for index in range(32)])
        # Reach every actual cap, rather than only observing its acceleration sign.
        for case, limit in enumerate(value for row in LIMITS for value in row):
            final = observed[(case + 1) * 256 - 1][0]
            self.assertEqual(struct.unpack_from('<h', final, 0x55)[0], limit)

    def test_wrapped_heading_slew_and_recenter_boundaries(self):
        self.motion([(start(kind, hull=65535, request=value, offset=value,
                            turret_request=32768 - value, operating=128), 1)
                     for kind in range(4) for value in (0, 1, 181, 182, 183, 363, 364, 365,
                                                       909, 910, 911, 32767, 32768, 64625,
                                                       64626, 64627, 65535)])
        self.motion([(start(hull=65000, request=difference, offset=difference,
                            turret_request=0, operating=128), 1)
                     for difference in range(0, 65536, 64)])
        self.motion([(start(kind, hull=65535, request=(65535 + delta) % 65536), 1)
                     for kind in range(4) for delta in (0, 1, -1, 181, -181, 182, -182,
                                                       183, -183, 32767, -32767, 32768)])
        self.motion([(start(kind, hull=32768, offset=10000,
                            turret_request=(10000 + delta) % 65536), 1)
                     for kind in range(4) for delta in (0, 1, -1, 363, -363, 364, -364,
                                                       365, -365, 32767, -32767, 32768)])

    def test_sustained_driving_turning_braking_and_coordinate_wrap(self):
        observed = self.motion([(start(kind, throttle=254, request=16384,
                                       turret_request=32768), 1024) for kind in range(4)])
        for kind in range(4):
            raw = observed[(kind + 1) * 1024 - 1][0]
            self.assertEqual(struct.unpack_from('<h', raw, 0x55)[0], 63)
            self.assertEqual(struct.unpack_from('<H', raw, 0x26)[0], 16384)
            self.assertEqual(struct.unpack_from('<H', raw, 0x89)[0], 32768)
            self.assertGreater(struct.unpack_from('<i', raw, 4)[0], 20000)
        self.motion([(start(kind, speed=63, throttle=0, hull=32768), 256) for kind in range(4)])
        self.motion([(start(kind, speed=-64, throttle=-254, hull=16384,
                            x=-2147483648, y=2147483647), 256) for kind in range(4)])
        self.motion([(start(kind, speed=64, throttle=254, hull=0,
                            x=-2147483648, y=2147483647), 256) for kind in range(4)])

    def test_request_validation_empty_sets_and_atomic_api_failures(self):
        self.motion([])
        self.rotations([])
        valid = struct.pack('<I', 1) + start() + struct.pack('<H', 1)
        for data in (b'', valid[:-1], valid + b'x', struct.pack('<I', 2) + valid[4:],
                     struct.pack('<I', 1) + start() + b'\0\0',
                     struct.pack('<I', 1) + struct.pack('<H', 4) + start()[2:] + b'\x01\0'):
            self.run_probe('motion', data, '', valid=False)
        self.run_probe('rotation', struct.pack('<I HhB', 1, 0, 0, 2), '', valid=False)
        self.run_probe('bad', struct.pack('<I', 0), '', valid=False)
        for command in self.commands:
            result = subprocess.run([*command, 'motion', str(pathlib.Path(self.temp.name) / 'missing')],
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_complete_original_ground_corpus(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([path.name for path in files], sorted(manifest))
        count = 0
        for path in files:
            data = path.read_bytes()
            self.assertEqual(len(data), manifest[path.name]['size'])
            self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
            cases = []
            for _, _, snapshot_bytes in records_from_scenario(data):
                kind, = struct.unpack_from('<H', snapshot_bytes)
                if kind >= 4:
                    continue
                initial, _ = initialized([(0, 0, snapshot_bytes)], (0, 0, 0, 0), 0, 0)
                raw = bytearray(initial[0][2])
                struct.pack_into('<H', raw, 0x97, 0)  # Explicit manual-stage boundary.
                cases.append((bytes(raw), 1))
            self.motion(cases)
            count += len(cases)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), manifest[path.name]['sha256'])
        self.assertEqual(count, 960)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, ORIGINALS = args.native_probe, args.originals
    if args.oracle:
        from original_vehicle_motion_oracle import OriginalVehicleMotionOracle
        ORACLE = OriginalVehicleMotionOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
