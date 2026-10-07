#!/usr/bin/env python3
"""Complete command throttle/profile/setter and reached motion consumption."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from ground_throttle_contract import DISPLAY_OFFSETS, throttle
from test_original_ground_bearing import actor
from test_units import records_from_scenario
from test_vehicle_motion import update
from test_vehicle_start import random_line, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = ORACLE = REVIEW = None
ORIGINALS = False
BATCH = 4096
MOTION = 258


def fixture(raw=None, *, kind=0, mode=0, flags=3, distance=91, target_range=59,
            pitch=0, profile=0, choice=0, operation=0, cursor=3, platoon=0,
            maneuver=255):
    if raw is None:
        raw = bytearray(actor(kind, mode=mode, flags=flags, platoon=platoon, maneuver=maneuver))
        struct.pack_into('<H', raw, 0x53, distance)
        struct.pack_into('<H', raw, 0x99, target_range)
        struct.pack_into('<h', raw, 0x34, pitch)
        raw[0x90] = profile
    return bytes(raw), (65535, 32768, 17, choice, 19, 23, 29, 31, 37, 41, 43), (1, 2, 32768, 65535), cursor, operation


def result(case):
    raw, descriptor, _, cursor, operation = case
    if operation in (0, 1, MOTION) and (raw[0x1b] >= 8 or cursor >= 4):
        return -1, raw, True
    try:
        updated, refresh = throttle(raw, descriptor, operation)
    except ValueError:
        return -1, raw, True
    if operation == MOTION:
        updated, _ = update(updated)
    return 0, updated, refresh


def encode(cases):
    return struct.pack('<I', len(cases)) + b''.join(raw + struct.pack('<11H4HBH', *descriptor, *seeds, cursor, operation)
                                                   for raw, descriptor, seeds, cursor, operation in cases)


class ThrottleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-throttle-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('native', 'all'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_ground_throttle_probe')])
        if TARGET in ('wasm', 'all'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_ground_throttle_probe.js')])
        cls.count = cls.rejections = cls.original_returns = cls.motion_returns = 0
        cls.canonical_worlds = cls.canonical_returns = cls.ui_cycles = 0
        cls.repaired_returns = cls.repaired_stops = 0
        cls.digest = hashlib.sha256()
        cls.machine = ORACLE.machine() if ORACLE else None

    @classmethod
    def tearDownClass(cls):
        print(f'Complete command throttle per target: {cls.count} scalar cases, {cls.rejections} atomic '
              f'rejections, {cls.original_returns} original command/setter returns, {cls.motion_returns} '
              f'reached original motion returns; SHA256 {cls.digest.hexdigest()}', flush=True)

    def original(self, case, expected, refresh):
        from original_unit_oracle import DGROUP
        raw, descriptor, seeds, cursor, operation = case
        machine = self.machine
        machine.mem_write(DGROUP + 0x7000, raw)
        machine.mem_write(DGROUP + 0x85b6 + raw[0x1b] * 22, struct.pack('<11H', *descriptor))
        machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H', 0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
        for index, offset in enumerate(DISPLAY_OFFSETS):
            machine.mem_write(DGROUP + offset, bytes([17 * (index + 1)]))
        actual, dirty = ORACLE.command(machine, 0x7000, raw[0x1b], 0 if operation == MOTION else operation)
        self.assertEqual(dirty, (3,) * 4 if refresh else (17, 34, 51, 68))
        if operation == MOTION:
            actual = ORACLE.motion([(actual, 1)])[0][0]
            type(self).motion_returns += 1
        self.assertEqual(actual, expected)
        type(self).original_returns += 1

    def checks(self, cases):
        expected = []
        for case in cases:
            status, raw, refresh = result(case)
            if ORACLE is not None and status == 0:
                self.original(case, raw, refresh)
            expected.append(f'status {status}\n' + state_lines([(0, 0, raw)]) +
                            f'throttle {struct.unpack_from("<H", raw, 0x99)[0]} {int(refresh)}\n' + random_line(case[2], case[3]))
            type(self).rejections += status != 0
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        path.write_bytes(encode(cases))
        expected = ''.join(expected)
        for command in self.commands:
            output = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=180)
            self.assertEqual(output.returncode, 0, output.stderr)
            if output.stdout != expected:
                got, want = output.stdout.splitlines(), expected.splitlines()
                first = next((n for n, (a, b) in enumerate(zip(got, want)) if a != b), min(len(got), len(want)))
                self.fail(f'Complete throttle state differs at line {first}: {got[first:first+3]} != {want[first:first+3]}')
        type(self).count += len(cases)
        self.digest.update(expected.encode())

    def batches(self, cases):
        pending = []
        for case in cases:
            pending.append(case)
            if len(pending) == BATCH:
                self.checks(pending)
                pending = []
        if pending:
            self.checks(pending)

    def test_complete_bank_every_control_word_and_class_admission(self):
        for mode in range(0, 16, 2):
            self.batches(fixture(kind=n % 4, mode=mode, flags=n, distance=n, target_range=65535 - n,
                                 profile=n % 4, pitch=(3583, 3584, -32768, 32767)[n % 4],
                                 platoon=n % 8, choice=n % 4) for n in range(65536))
            print(f'Complete throttle mode {mode}: all control words', flush=True)

    def test_every_unsigned_navigation_and_independent_target_range_word(self):
        for mode in (0, 2, 6):
            self.batches(fixture(kind=n % 4, mode=mode, distance=n, target_range=65535 - n,
                                 choice=n % 4, profile=2) for n in range(65536))
        # Distinct retained ranges prove +99 ownership and unsigned comparisons.
        self.batches(fixture(kind=kind, mode=6, distance=distance, target_range=target,
                             flags=flags, profile=3)
                     for kind in range(4) for distance in (0, 45, 60, 90, 65535)
                     for target in (0, 44, 45, 59, 60, 89, 90, 32768, 65535)
                     for flags in (0, 1, 2, 3, 65535))

    def test_every_signed_pitch_and_retained_profile_byte(self):
        for profile in (0, 1):
            self.batches(fixture(kind=n % 4, pitch=n - 32768, profile=profile, operation=1,
                                 mode=255, choice=65535) for n in range(65536))
        self.batches(fixture(kind=kind, pitch=pitch, profile=profile, operation=1, mode=255, choice=65535)
                     for kind in range(4) for pitch in (-32768, -1, 0, 3583, 3584, 32767)
                     for profile in range(256))

    def test_direct_class_setter_every_byte_and_real_throttle_ui(self):
        self.batches(fixture(kind=kind, profile=old, operation=2 + new, mode=255, choice=65535)
                     for kind in range(4) for old in (0, 1, 2, 3, 255) for new in range(256))
        if ORACLE is not None:
            from original_ground_command_oracle import OriginalGroundCommandOracle
            cycles = OriginalGroundCommandOracle().throttle_cycles()
            self.assertEqual(len(cycles), 64)
            type(self).ui_cycles = len(cycles)

    def test_unused_selectors_maneuvers_and_atomic_invalid_batches(self):
        self.batches(fixture(kind=kind, mode=8, flags=flags, maneuver=maneuver, choice=65535)
                     for kind in range(4) for flags in (0, 1, 65535) for maneuver in range(256))
        self.batches(fixture(kind=kind, mode=mode, flags=flags, distance=distance, choice=choice)
                     for kind in range(4) for mode in range(0, 16, 2) for flags in (0, 1, 2, 3)
                     for distance in (0, 8, 9) for choice in (3, 4, 255, 32768, 65535))
        self.batches(fixture(mode=mode) for mode in range(256))
        self.batches(fixture(platoon=n) for n in range(8, 256))
        self.batches(fixture(cursor=n) for n in range(4, 256))
        good = encode([fixture()])[4:]
        bad = bytearray(good); struct.pack_into('<H', bad, len(bad) - 2, 259)
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        for data in (b'', bytes(3), struct.pack('<I', 1), struct.pack('<I', 0) + good,
                     struct.pack('<I', 1) + good[:-1], struct.pack('<I', 1) + good + b'x',
                     struct.pack('<I', 2) + good + bad):
            path.write_bytes(data)
            for command in self.commands:
                output = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
                self.assertEqual((output.returncode, output.stdout), (1, ''))
        self.checks([])

    def test_profile_throttle_consumed_by_complete_original_motion(self):
        self.repaired_navigation()
        cases = []
        for kind in range(4):
            for mode in (0, 2, 6, 8):
                for profile in range(4):
                    raw = bytearray(fixture(kind=kind, mode=mode, profile=profile, choice=3,
                                           distance=120, target_range=120)[0])
                    struct.pack_into('<H', raw, 0x97, 0)
                    struct.pack_into('<h', raw, 0x55, 30)
                    raw[0x3d] = 0
                    for pitch in (3584, 6000, 3583, 2620, -32768, 32767, 0, 3584):
                        struct.pack_into('<h', raw, 0x34, pitch)
                        case = fixture(bytes(raw), operation=MOTION, choice=3)
                        cases.append(case)
                        status, updated, _ = result(case)
                        self.assertEqual(status, 0)
                        raw = bytearray(updated)
                        raw[0x3d] = (raw[0x3d] + 2) % 256
        self.checks(cases)
        self.assertEqual(len(cases), 512)

    def repaired_navigation(self):
        from test_ground_bearing import fixture as goal_fixture, result as goal_result
        from test_ground_goal import encode as goal_encode
        goals = [goal_fixture(kind=kind, mode=mode, member=member, presence=presence,
                              count=count, formation=5, flags=1, action=action)
                 for kind in range(4) for mode in (4, 6) for member in range(4)
                 for presence in range(4) for count in (0, 1, 32) for action in (0, 3)]
        goals = [(*case[:3], (*case[3][:3], 3, *case[3][4:]), *case[4:]) for case in goals]
        expected, continuations = [], []
        for case in goals:
            status, raw, live = goal_result(case)
            self.assertEqual((status, live), (0, False))
            expected.append('status 0\n' + state_lines([(0, 0, raw)]) +
                            f'bearing {raw[0x44]} {struct.unpack_from("<H", raw, 0x47)[0]} 0\n' +
                            random_line(case[5], case[6]))
            continuation = (raw, case[3], case[5], case[6], 0)
            continuations.append(continuation)
            status, updated, _ = result(continuation)
            self.assertEqual(status, 0)
            type(self).repaired_stops += struct.unpack_from('<h', updated, 0x57)[0] == 0
        path = pathlib.Path(self.temp.name) / 'goal-cases.bin'
        path.write_bytes(struct.pack('<I', len(goals)) + b''.join(goal_encode([case[:7]])[4:] + bytes(case[7:]) for case in goals))
        for command in self.commands:
            executable = pathlib.Path(command[-1])
            goal = executable.with_name('fist_ground_goal_probe' + executable.suffix)
            output = subprocess.run([*command[:-1], str(goal), str(path), '--bearing'],
                                    capture_output=True, text=True, timeout=180)
            self.assertEqual(output.returncode, 0, output.stderr)
            self.assertEqual(output.stdout, ''.join(expected))
        # The complete C goal/bearing result above supplies every owned field
        # consumed below. Full snapshots, RNG and lengths are compared, and the
        # goal probe preserves all other world bytes; no foreign target bytes
        # or synthetic position are supplied to the repaired continuation.
        self.checks(continuations)
        self.assertEqual(len(goals), 768)
        self.assertEqual(type(self).repaired_stops, 352)
        type(self).repaired_returns = len(goals)

    def test_all_47_saved_ground_records_and_target_range_retention(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        paths = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual(len(paths), 47)
        cases = [fixture(raw) for path in paths for _, _, raw in records_from_scenario(path.read_bytes())
                 if int.from_bytes(raw[:2], 'little') < 4]
        self.assertEqual(len(cases), 960)
        self.batches(cases)

    def test_canonical_all_47_prepared_worlds_four_details_complete_bank(self):
        if not ORIGINALS or ORACLE is None:
            self.skipTest('Pinned complete prepared worlds require original oracle')
        from mission_ready_contract import prepare, original_prepare
        from orders_contract import scenario_order_blocks
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        from original_unit_oracle import DGROUP
        from test_mission_world import install, world_lines
        directory = ROOT / 'armoredfist/FISTDATA'
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrain = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        grouped = {}
        for name, pin in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (pin['size'], pin['sha256']))
            offset = 0
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size + 6
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual((len(manifest), len(grouped)), (47, 8))
        returns = 0
        for height_name, missions in grouped.items():
            encoded = (directory / height_name).read_bytes()
            pin = terrain[height_name]
            self.assertEqual(hashlib.sha256(encoded).hexdigest(), pin['sha256'])
            decoded = decoder.klc(encoded)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), pin['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed, pixels = scaler.resample(pin['width'], plane, [side])
                self.assertEqual(installed, side)
                for name, data in missions:
                    records = records_from_scenario(data)
                    orders = scenario_order_blocks(data)
                    seeds = (1, 2, 32768, 65535)
                    status, world = install(records, seeds, 3, 0)
                    self.assertEqual(status, 0)
                    initial = 'status 0\n' + world_lines(world, orders)
                    machine, objects = ORACLE.prepare_saved(records, seeds, 3, 0, orders)
                    status, world = prepare(world, side, pixels, 0)
                    self.assertEqual(status, 0)
                    original = original_prepare(ORACLE, machine, objects, side, pixels)
                    self.assertEqual(world_lines(original, orders), world_lines(world, orders))
                    expected = initial + 'prepare 0\n' + world_lines(world, orders)
                    pool, payloads, _, _, _ = world
                    for mode in range(0, 16, 2):
                        expected += f'throttles {mode}\n'
                        for slot, _ in pool.registry:
                            if slot == 65535 or pool.slots[slot][1] >= 4:
                                continue
                            allocation, raw = payloads[slot]
                            raw = bytearray(raw); raw[0x43] = mode
                            platoon = raw[0x1b]
                            descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                            updated, refresh = throttle(raw, descriptor)
                            pointer = objects[slot][2]
                            machine.mem_write(DGROUP + pointer, bytes(raw))
                            for i, address in enumerate(DISPLAY_OFFSETS):
                                machine.mem_write(DGROUP + address, bytes([17 * (i + 1)]))
                            actual, dirty = ORACLE.command(machine, pointer, platoon)
                            self.assertEqual((actual, dirty), (updated, (3,) * 4 if refresh else (17, 34, 51, 68)))
                            payloads[slot] = allocation, bytearray(updated)
                            expected += f'throttle {slot} 0 {struct.unpack_from("<H", updated, 0x99)[0]} {int(refresh)}\n'
                            returns += 1
                        expected += world_lines(world, orders)
                    scenario = pathlib.Path(self.temp.name) / 'world.fsg'
                    request = pathlib.Path(self.temp.name) / 'request.bin'
                    scenario.write_bytes(data)
                    request.write_bytes(struct.pack('<I4HBBH', side, *seeds, 3, 0, 1) + pixels + bytes(4))
                    for command in self.commands:
                        executable = pathlib.Path(command[-1])
                        ready = executable.with_name('fist_mission_ready_probe' + executable.suffix)
                        output = subprocess.run([*command[:-1], str(ready), str(scenario), str(request), '--throttles'],
                                                capture_output=True, text=True, timeout=180)
                        self.assertEqual(output.returncode, 0, output.stderr)
                        if output.stdout != expected:
                            got, want = output.stdout.splitlines(), expected.splitlines()
                            first = next((n for n, (a, b) in enumerate(zip(got, want)) if a != b), min(len(got), len(want)))
                            self.fail(f'Canonical {name}/{side} throttle differs at line {first}: {got[first:first+3]} != {want[first:first+3]}')
                    self.digest.update(expected.encode())
            print(f'Canonical throttle {height_name}: four details and all eight modes', flush=True)
        self.assertEqual(returns, 30720)
        type(self).canonical_returns, type(self).canonical_worlds = returns, 188


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Review evidence belongs in a dedicated /tmp directory')
    TARGET, BUILD, NATIVE_PROBE, ORIGINALS = args.target, args.build_root, args.native_probe, args.originals
    if args.oracle:
        if not ORIGINALS:
            parser.error('--oracle requires --originals')
        from original_ground_throttle_oracle import OriginalGroundThrottleOracle
        ORACLE = OriginalGroundThrottleOracle()
    program = unittest.main(argv=[__file__], exit=False)
    success = (program.result.wasSuccessful() and program.result.testsRun == 8 and
               (not args.oracle or not program.result.skipped))
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        evidence = {'scope': 'complete shared throttle/profile child callbacks; full parent/battle remain open',
                    'target': TARGET, 'success': success, 'groups': program.result.testsRun, 'skips': len(program.result.skipped),
                    'scalar_cases_per_target': ThrottleTests.count, 'atomic_rejections_per_target': ThrottleTests.rejections,
                    'complete_original_command_setter_returns': ThrottleTests.original_returns,
                    'reached_original_motion_returns': ThrottleTests.motion_returns, 'original_throttle_ui_cycles': ThrottleTests.ui_cycles,
                    'reached_c_repaired_navigation_returns_per_target': ThrottleTests.repaired_returns,
                    'reached_c_stops_without_goal_per_target': ThrottleTests.repaired_stops,
                    'canonical_prepared_worlds_per_target': ThrottleTests.canonical_worlds,
                    'canonical_complete_bank_returns_per_target': ThrottleTests.canonical_returns,
                    'normalized_output_sha256': ThrottleTests.digest.hexdigest(), 'complete_wasm_streak': 0}
        (REVIEW / 'ground-throttle.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(0 if success else 1)
