#!/usr/bin/env python3
"""Complete untargeted M1 launch state, allocated payloads and capacity ordering."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_object_pool import bind, allocate, release, trace, EXTENDED, NONE
from test_units import records_from_scenario
from test_vehicle_motion import start, rotate, rotate_spatial
from test_vehicle_start import initialized, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def wrap(value):
    return (value + 2**31) % 2**32 - 2**31


def fixture(*, snapshot=None, bindings=None, origin=0, steps=1, coarse=0, flags=0x66,
            ammo=15, reload=0, recoil=0, trigger=48, releases=()):
    snapshot = bytearray(snapshot or start())
    snapshot[0x91] = snapshot[0xa5] = 0  # Explicit station-0 handler boundary.
    return (bytes(snapshot), bindings or [(0, 9, 12)], origin, steps, coarse,
            flags, ammo, reload, recoil, trigger, releases)


def initial(case):
    snapshot, bindings, origin, _, _, flags, ammo, reload, recoil, trigger, _ = case
    _, index, value = bindings[origin]
    records, _ = initialized([(index, value, snapshot)], (0, 0, 0, 0), 0, 0)
    raw = bytearray(records[0][2])
    raw[0x16], raw[0xa8], raw[0x3c], raw[0x92] = flags, reload, recoil, trigger
    struct.pack_into('<H', raw, 0xad, ammo)
    origin_slot = 150 + sum(kind in EXTENDED for kind, _, _ in bindings[:origin])
    struct.pack_into('<H', raw, 2, origin_slot - 150)
    return raw, index, value, origin_slot


def allocation(commands, kind, low=0):
    commands.append(allocate(kind, low=low))
    result = trace(commands).splitlines()[-4].split()
    return None if int(result[1]) else tuple(map(int, result[2:]))


def payloads(raw, origin, coarse, shell, smoke):
    x, y, altitude = struct.unpack_from('<3i', raw, 4)
    heading, = struct.unpack_from('<H', raw, 0x10)
    elevation, = struct.unpack_from('<H', raw, 0x38)
    vx, vy, vz = rotate_spatial(heading, elevation, 853, coarse)
    sx, sy = rotate(heading, 800, coarse)
    output = ''
    objects = []
    if shell:
        kind, slot, index, value = shell
        fields = (*shell, x, y, wrap(altitude + 2048), heading, vx, vy, vz, 853, 2,
                  origin, NONE, 0, raw[0x16] & 8, 0, 5)
        output += 'projectile ' + ' '.join(map(str, fields)) + '\n'
        record = bytearray(55)
        struct.pack_into('<HH3iH', record, 0, kind, slot, x, y, wrap(altitude + 2048), heading)
        record[0x16] = raw[0x16] & 8
        struct.pack_into('<hhhhH', record, 0x1b, 853, vx, vy, vz, 2)
        # Original near pointer denotes the physical firer, not registry identity.
        struct.pack_into('<H', record, 0x27, 0xc05c + (origin - 150) * 251)
        record[0x2a] = 5
        objects.append((slot, index, value, bytes(record)))
    if smoke:
        kind, slot, index, value = smoke
        fields = (*smoke, wrap(x + sx), wrap(y + sy), wrap(altitude + 2112),
                  (heading + 32768) % 65536, 512, 0, 0, 0)
        output += 'muzzle ' + ' '.join(map(str, fields)) + '\n'
        record = bytearray(55)
        struct.pack_into('<HH3iH', record, 0, kind, slot, wrap(x + sx), wrap(y + sy),
                         wrap(altitude + 2112), (heading + 32768) % 65536)
        struct.pack_into('<H', record, 0x14, 512)
        objects.append((slot, index, value, bytes(record)))
    return output, objects


def expected(case):
    _, bindings, _, steps, coarse, *_ = case
    raw, index, value, origin_slot = initial(case)
    commands = [bind(*entry) for entry in bindings] + [release(index) for index in case[-1]]
    output = ''
    observations = []
    for _ in range(steps):
        shell = smoke = None
        ammo, = struct.unpack_from('<H', raw, 0xad)
        outcome = 1
        if ammo:
            struct.pack_into('<H', raw, 0xad, ammo - 1)
            raw[0xe4] = 3
            outcome = 2
            shell = allocation(commands, 8)
            if shell:
                smoke = allocation(commands, 18, low=1)
                raw[0xa8], raw[0x3c], raw[0x92] = 20, 16, 0
                outcome = 0
        parts, objects = payloads(raw, origin_slot, coarse, shell, smoke)
        metadata = '\n'.join(trace(commands).splitlines()[-3:]) + '\n'
        output += f'launch {outcome} {int(smoke is not None)} {12 if shell else 255}\n' + parts
        output += state_lines([(index, value, bytes(raw))]) + metadata
        observations.append((bytes(raw), metadata, sorted(objects), int(outcome != 0)))
    return output, observations


def encode(cases):
    output = struct.pack('<I', len(cases))
    for snapshot, bindings, origin, steps, coarse, flags, ammo, reload, recoil, trigger, releases in cases:
        output += struct.pack('<HHHBBHBBBH', len(bindings), origin, steps, coarse, flags,
                              ammo, reload, recoil, trigger, len(releases)) + snapshot
        output += b''.join(struct.pack('<HHH', *entry) for entry in bindings)
        output += b''.join(struct.pack('<H', index) for index in releases)
    return output


class LaunchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-projectile-launch-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.request = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_projectile_launch_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_projectile_launch_probe.js')])

    def run_probe(self, data, wanted, valid=True):
        self.request.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, str(self.request)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
            self.assertEqual(result.stdout, wanted)

    def check(self, cases, *, original_safe=True):
        wanted = ''
        for case in cases:
            output, observations = expected(case)
            wanted += output
            if ORACLE and original_safe:
                raw, _, _, _ = initial(case)
                _, bindings, origin, steps, coarse, *_ = case
                observed = ORACLE.launch(bytes(raw), bindings, origin, steps, coarse, case[-1])
                self.assertEqual(observed, observations)
        self.run_probe(encode(cases), wanted)
        return wanted

    def test_complete_pose_velocity_smoke_and_wrap_boundaries(self):
        angles = (0, 1, 31, 32, 33, 8192, 16383, 16384, 16385, 32767, 32768, 49152, 65535)
        cases = []
        for coarse in (0, 1):
            for heading in angles:
                for elevation in angles:
                    raw = bytearray(start(x=2147483647, y=-2147483648))
                    struct.pack_into('<HH', raw, 0x10, heading, 40)
                    struct.pack_into('<H', raw, 0x38, elevation)
                    struct.pack_into('<i', raw, 12, 2147483647)
                    cases.append(fixture(snapshot=bytes(raw), coarse=coarse, flags=255))
        self.check(cases)
        output = self.check([fixture(snapshot=start(), flags=0)])
        self.assertIn('projectile 8 0 0 1', output)
        self.assertIn('muzzle 18 1 1 1', output)
        self.assertIn('launch 0 1 12', output)

    def test_empty_ammunition_and_failed_allocation_preserve_mechanical_state(self):
        for ammo in (0, 1, 2, 65535):
            for occupied in (0, 118, 119, 120, 149, 150):
                bindings = [(8, index, 1) for index in range(occupied)] + [(0, 181, 12)]
                self.check([fixture(bindings=bindings, origin=occupied, ammo=ammo,
                                    reload=231, recoil=247, trigger=48, steps=3)])

    def test_full_arena_repeated_shots_and_optional_smoke(self):
        self.check([fixture(ammo=200, steps=201)])
        # 59 smoke-producing shots reach 118 short objects; shot 60 reaches
        # 119 and admits its smoke, then shot 61 reaches 121 and skips smoke.
        bindings = [(8, index, 1) for index in range(118)] + [(0, 181, 12)]
        output = self.check([fixture(bindings=bindings, origin=118, steps=34, ammo=40)])
        launches = [line for line in output.splitlines() if line.startswith('launch')]
        self.assertEqual(launches[0], 'launch 0 1 12')
        self.assertEqual(launches[1], 'launch 0 0 12')
        self.assertEqual(launches[-1], 'launch 2 0 255')

    def test_saved_registry_reservations_and_overwritten_origin_binding(self):
        bindings = [(0, 9, 12), (0, 9, 2), (8, 0, 1), (18, 0, 1), (19, 1, 1)]
        self.check([fixture(bindings=bindings, origin=0, steps=5)])
        # All 182 live bindings: actual original short failure has a vacancy
        # search that escapes the registry. The repaired C boundary is tested
        # without executing that known corrupt original route.
        bindings = [(8 if index < 150 else 0, index, 2) for index in range(182)]
        self.check([fixture(bindings=bindings, origin=150, ammo=2, steps=3)], original_safe=False)

    def test_holes_reservations_and_identity_exhaustion_with_free_physical_space(self):
        bindings = [(8, 0, 2), (8, 1, 1), (18, 2, 1), (0, 9, 12)]
        self.check([fixture(bindings=bindings, origin=3, releases=(0, 2), steps=4)])
        bindings = [(8 if index < 150 else 0, index, 2) for index in range(182)]
        releases = tuple(index for index in range(182) if index != 150)
        output = self.check([fixture(bindings=bindings, origin=150, releases=releases, steps=3)], original_safe=False)
        self.assertEqual(output.count('launch 2 0 255'), 3)
        self.assertIn('counts 0 1', output)
        # One reusable identity admits the shell, but all remaining vacant
        # identities retain value 1. Smoke is optional even below admission 120.
        bindings[0] = (8, 0, 1)
        output = self.check([fixture(bindings=bindings, origin=150, releases=releases)], original_safe=False)
        self.assertIn('launch 0 0 12', output)
        self.assertIn('counts 1 1', output)

    def test_every_side_flag_and_physical_origin_slot(self):
        self.check([fixture(flags=flags) for flags in range(256)])
        bindings = [(0, index, 1) for index in range(32)]
        self.check([fixture(bindings=bindings, origin=index) for index in range(32)])

    def test_invalid_or_incomplete_requests_publish_nothing(self):
        record = encode([fixture()])
        invalid = [b'', b'\0', record[:-1], record + b'x', struct.pack('<I', 2) + record[4:]]
        for offset, value in ((4 + 6, 2), (4 + 13, 1), (4 + 4, 0), (4 + 2, 1)):
            raw = bytearray(record); raw[offset] = value; invalid.append(bytes(raw))
        raw = bytearray(record); struct.pack_into('<H', raw, 4 + 15 + 0x97, 1); invalid.append(bytes(raw))
        for data in invalid:
            self.run_probe(data, '', valid=False)
        self.run_probe(struct.pack('<I', 0), '')

    def test_all_original_m1_actors_with_complete_mission_occupancy(self):
        if not ORIGINALS:
            self.skipTest('Complete original snapshot corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        cases = []
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            bindings = [(int.from_bytes(raw[:2], 'little'), index, value) for index, value, raw in records]
            for origin, (_, _, raw) in enumerate(records):
                if bindings[origin][0] == 0:
                    raw = bytearray(raw); struct.pack_into('<H', raw, 0x97, 0)
                    cases.append(fixture(snapshot=bytes(raw), bindings=bindings, origin=origin, steps=2))
        self.assertEqual(len(cases), 179)
        self.check(cases)
        print(f'Launch corpus: all 179 M1 actors with full occupancy in {len(manifest)} original missions on {len(self.commands)} targets', flush=True)


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
        from original_projectile_launch_oracle import OriginalProjectileLaunchOracle
        ORACLE = OriginalProjectileLaunchOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (0 if ORIGINALS else 1))
