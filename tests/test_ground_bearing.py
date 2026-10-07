#!/usr/bin/env python3
"""Complete ground direction/range/retreat and deliberate physical target-loss repair."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from ground_bearing_contract import bearing
from test_ground_goal import assign, encode, sample
from test_original_ground_bearing import actor
from test_units import records_from_scenario
from test_vehicle_start import random_line, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
REVIEW = None
NATIVE_PROBE = ORACLE = None
ORIGINALS = False
BATCH = 4096


def fixture(raw=None, *, action=1, coarse=0, behavior=0, **options):
    case = sample(**options)
    if raw is not None:
        case = (raw, *case[1:])
    descriptor = (behavior, *case[3][1:])
    return (*case[:3], descriptor, *case[4:], action, coarse)


def result(case):
    raw, leader, presence, descriptor, route, seeds, cursor, action, coarse = case
    original = raw
    mode, member, platoon = raw[0x43], raw[0x1c], raw[0x1b]
    live = action == 2 or action in (6, 7) or action in (1, 9) and presence != 0
    if cursor >= 4 or platoon >= 8 or mode > 14 or mode % 2:
        return -1, raw, live
    flags, = struct.unpack_from('<H', raw, 0x40)
    used = flags & 1 and (mode == 6 or mode == 4 and not flags & 64)
    if used and (action == 8 or action == 9 and presence != 0):
        return -1, raw, live
    if used and not live:
        raw = bytearray(raw)
        raw[0x43] = 0 if member == 0 else 2
        struct.pack_into('<H', raw, 0x40, flags & (65535 ^ (2 | 64)))
        status, raw = assign((bytes(raw), leader, presence, descriptor, route, seeds, cursor))
        if status:
            return -1, original, live
    target_raw = original if action == 2 or presence == 3 and action == 1 else leader
    target = struct.unpack_from('<ii', target_raw, 4) if live else None
    try:
        return 0, bearing(raw, descriptor, target, coarse), bool(live)
    except ValueError:
        return -1, original, live


class DirectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-bearing-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('native', 'all'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_ground_goal_probe')])
        if TARGET in ('wasm', 'all'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_ground_goal_probe.js')])
        cls.count = cls.rejections = cls.original_returns = 0
        cls.canonical_phases = cls.canonical_worlds = 0
        cls.navigation = None
        cls.digest = hashlib.sha256()

    @classmethod
    def tearDownClass(cls):
        print(f'Complete command bearing per target: {cls.count} cases, {cls.rejections} atomic '
              f'rejections, {cls.original_returns} complete original returns; SHA256 '
              f'{cls.digest.hexdigest()}', flush=True)

    def checks(self, cases):
        expected = []
        for case in cases:
            status, raw, live = result(case)
            if ORACLE is not None and status == 0:
                # For lost targets, the deliberate repaired mode/goal is the
                # declared ordinary-navigation input to original ab88. Never
                # supply unrelated DS bytes as a replacement target.
                from original_unit_oracle import DGROUP
                source, leader, presence, descriptor, _, _, _, action, coarse = case
                pointer = 0x7100
                if live:
                    if action == 2 or presence == 3 and action == 1:
                        pointer = 0x7000
                    else:
                        ORACLE.machine_for_cases.mem_write(DGROUP + pointer, leader)
                declared = bytearray(source)
                if raw[0x43] != source[0x43]:
                    declared[0x43] = raw[0x43]
                    declared[0x40:0x42] = raw[0x40:0x42]
                    declared[0x49:0x51] = raw[0x49:0x51]
                struct.pack_into('<H', declared, 0x97, pointer if live else 0)
                machine = ORACLE.machine_for_cases
                machine.mem_write(DGROUP + 0x7000, bytes(declared))
                machine.mem_write(DGROUP + 0x85b6 + source[0x1b] * 22, struct.pack('<11H', *descriptor))
                machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
                actual = bytearray(ORACLE.direction(machine, 0x7000, source[0x1b]))
                actual[0x97:0x99] = source[0x97:0x99]
                self.assertEqual(bytes(actual), raw)
                type(self).original_returns += 1
            expected.append(f'status {status}\n' + state_lines([(0, 0, raw)]) +
                            f'bearing {raw[0x44]} {struct.unpack_from("<H", raw, 0x47)[0]} {int(live)}\n' +
                            random_line(case[5], case[6]))
            type(self).rejections += status != 0
        data = struct.pack('<I', len(cases)) + b''.join(
            encode([case[:7]])[4:] + bytes(case[7:]) for case in cases)
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        path.write_bytes(data)
        expected = ''.join(expected)
        for command in self.commands:
            output = subprocess.run([*command, str(path), '--bearing'], capture_output=True,
                                    text=True, timeout=120)
            self.assertEqual(output.returncode, 0, output.stderr)
            if output.stdout != expected:
                got, want = output.stdout.splitlines(), expected.splitlines()
                first = next((n for n, (a, b) in enumerate(zip(got, want)) if a != b), min(len(got), len(want)))
                self.fail(f'Complete C bearing output differs at line {first}: '
                          f'{got[first:first+3]} != {want[first:first+3]}')
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

    def test_complete_bank_all_control_words_and_class_admission(self):
        for mode in range(0, 16, 2):
            self.batches(fixture(actor(flags % 4, flags=flags, mode=mode, counter=255, maneuver=4,
                                       platoon=flags % 8)) for flags in range(65536))
            print(f'Complete C bearing mode {mode}: all control words', flush=True)

    def test_every_packed_range_word_and_wrapped_fine_coarse_geometry(self):
        for mode in (4, 6):
            self.batches(fixture(actor(n % 4, mode=mode, source=(0, 0)),
                                 position=(n * 256, 0)) for n in range(65536))
        points = ((0, 0), (-1, 1), (65535, -65536), (16777215, 16777216),
                  (-2147483648, 2147483647), (2147483647, -2147483648))
        self.batches(fixture(actor(kind, mode=mode, flags=3, source=source, goal=target),
                             position=target, coarse=coarse)
                     for kind in range(4) for mode in (0, 2, 4, 6) for source in points
                     for target in points for coarse in (0, 1, 255))

    def test_retreat_counter_duration_maneuver_and_saved_heading_domains(self):
        self.batches(fixture(actor(kind, mode=4, flags=flags, counter=count), behavior=behavior,
                             action=0)
                     for kind in range(4) for behavior in range(4) for count in range(256)
                     for flags in (0, 64, 65))
        self.batches(fixture(actor(n % 4, mode=8, maneuver=4, heading=n), action=8, behavior=65535)
                     for n in range(65536))
        self.batches(fixture(actor(kind, mode=8, flags=flags, maneuver=maneuver),
                             action=8, behavior=65535)
                     for kind in range(4) for maneuver in range(256) for flags in (0, 1, 65535))
        self.batches(fixture(actor(mode=mode, flags=flags, target=65535), behavior=65535, action=8)
                     for mode in (0, 2, 8, 10, 12, 14) for flags in (0, 1, 2, 3, 65535))

    def test_target_release_reuse_orphan_self_retype_and_lost_goal_navigation(self):
        self.batches(fixture(kind=kind, mode=mode, flags=flags, member=member, count=count,
                             presence=presence, formation=formation, action=action, behavior=behavior)
                     for kind in range(4) for mode in (4, 6) for flags in (0, 1, 3, 65)
                     for member in (0, 1, 3) for count in (0, 1, 32) for presence in (0, 1, 2, 3)
                     for formation, behavior in ((0, 0), (5, 3), (65535, 65535))
                     for action in (0, 1, 2, 3, 4, 6, 8, 9, 10)
                     if not (action == 9 and presence == 3))
        # In-place retirement keeps a physical reference; cross-type allocation
        # reuse does not. Keep these extended-arena scenarios explicit.
        self.batches(fixture(kind=kind, mode=mode, action=action, presence=1, leader_kind=0)
                     for kind in range(4) for mode in (4, 6) for action in (5, 7))

    def test_invalid_modes_metadata_used_selectors_and_atomic_batch_decode(self):
        self.batches(fixture(mode=mode, action=0) for mode in range(256))
        self.batches(fixture(platoon=n, action=0) for n in range(8, 256))
        self.batches(fixture(cursor=n, action=0) for n in range(4, 256))
        self.batches(fixture(actor(mode=4, flags=65), action=8, behavior=n)
                     for n in (4, 255, 256, 32768, 65535))
        good = encode([fixture()[:7]])[4:] + b'\1\0'
        bad = bytearray(good); struct.pack_into('<H', bad, 0, 4)
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        for data in (b'', bytes(3), struct.pack('<I', 1), struct.pack('<I', 0) + good,
                     struct.pack('<I', 1) + good[:-1], struct.pack('<I', 1) + good + b'x',
                     struct.pack('<I', 2) + good + bad):
            path.write_bytes(data)
            for command in self.commands:
                output = subprocess.run([*command, str(path), '--bearing'], capture_output=True,
                                        text=True, timeout=120)
                self.assertEqual((output.returncode, output.stdout), (1, ''))
        self.checks([])

    def test_all_47_saved_ground_fields_and_declared_navigation_boundaries(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        paths = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual(len(paths), 47)
        cases = []
        for path in paths:
            for _, _, raw in records_from_scenario(path.read_bytes()):
                if int.from_bytes(raw[:2], 'little') < 4:
                    cases.append(fixture(raw, action=0))
                    for mode in (0, 2):
                        nav = bytearray(raw); nav[0x43] = mode
                        struct.pack_into('<H', nav, 0x40, 3)
                        cases.append(fixture(bytes(nav), action=0))
        self.batches(cases)
        self.assertEqual(len(cases), 2880)

    def test_canonical_all_47_prepared_worlds_four_height_details(self):
        if not ORIGINALS or ORACLE is None:
            self.skipTest('Pinned complete prepared worlds require the original oracle')
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
        phases = 0
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
                    initial = f'status 0\n' + world_lines(world, orders)
                    machine, objects = ORACLE.prepare_saved(records, seeds, 3, 0, orders)
                    status, world = prepare(world, side, pixels, 0)
                    self.assertEqual(status, 0)
                    original = original_prepare(ORACLE, machine, objects, side, pixels)
                    self.assertEqual(world_lines(original, orders), world_lines(world, orders))
                    expected = initial + 'prepare 0\n' + world_lines(world, orders)
                    pool, payloads, roster, _, _ = world
                    for coarse in (0, 1):
                        expected += f'bearings {coarse}\n'
                        for slot, _ in pool.registry:
                            if slot == 65535 or pool.slots[slot][1] >= 4:
                                continue
                            allocation, raw = payloads[slot]
                            platoon = raw[0x1b]
                            leader_slot = roster[platoon * 4]
                            presence = 0 if leader_slot == 65535 else (3 if leader_slot == slot else
                                       (2 if payloads[leader_slot][0][0] == 23 else 1))
                            leader = raw if presence in (0, 3) else payloads[leader_slot][1]
                            descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                            route = orders[0][platoon * 268:(platoon + 1) * 268]
                            case = (bytes(raw), bytes(leader), presence, descriptor, route,
                                    tuple(world[3]), world[4], 0, coarse)
                            status, updated, live = result(case)
                            self.assertEqual(status, 0, (name, slot, raw[0x43]))
                            # Complete original callback from the declared
                            # repaired navigation input, in the actual prepared
                            # world. This never executes unsafe null geometry.
                            declared = bytearray(raw)
                            if updated[0x43] != raw[0x43]:
                                declared[0x43] = updated[0x43]
                                declared[0x40:0x42] = updated[0x40:0x42]
                                declared[0x49:0x51] = updated[0x49:0x51]
                            pointer = objects[slot][2]
                            machine.mem_write(DGROUP + pointer, bytes(declared))
                            machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
                            self.assertEqual(ORACLE.direction(machine, pointer, platoon), updated)
                            payloads[slot] = allocation, bytearray(updated)
                            expected += (f'bearing {slot} 0 {updated[0x44]} '
                                         f'{struct.unpack_from("<H", updated, 0x47)[0]} {int(live)}\n')
                            phases += 1
                        expected += world_lines(world, orders)
                    scenario = pathlib.Path(self.temp.name) / 'world.fsg'
                    request = pathlib.Path(self.temp.name) / 'request.bin'
                    scenario.write_bytes(data)
                    request.write_bytes(struct.pack('<I4HBBH', side, *seeds, 3, 0, 1) + pixels + bytes(4))
                    for command in self.commands:
                        executable = pathlib.Path(command[-1])
                        ready = executable.with_name('fist_mission_ready_probe' + executable.suffix)
                        output = subprocess.run([*command[:-1], str(ready), str(scenario), str(request), '--bearings'],
                                                capture_output=True, text=True, timeout=180)
                        self.assertEqual(output.returncode, 0, output.stderr)
                        if output.stdout != expected:
                            got, want = output.stdout.splitlines(), expected.splitlines()
                            first = next((n for n, (a, b) in enumerate(zip(got, want)) if a != b), min(len(got), len(want)))
                            self.fail(f'Canonical {name}/{side} bearing differs at line {first}: '
                                      f'{got[first:first+3]} != {want[first:first+3]}')
                    self.digest.update(expected.encode())
            print(f'Canonical C bearing {height_name}: four prepared details', flush=True)
        self.assertEqual(phases, 7680)
        type(self).canonical_phases = phases
        type(self).canonical_worlds = 188

    def test_required_original_navigation_throttle_continuation(self):
        if ORACLE is None:
            self.skipTest('Required complete original continuation requested separately')
        from original_ground_navigation_oracle import verify_navigation
        evidence = verify_navigation()
        type(self).navigation = evidence
        self.assertEqual(evidence['complete_ad2f_returns'], 262528)
        self.assertEqual(evidence['deliberate_repair_goal_bearing_throttle_cases'], 384)
        self.assertEqual(evidence['stopped_without_goal'], 176)


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
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or
                   REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Review evidence belongs in a dedicated /tmp directory')
    TARGET, BUILD, NATIVE_PROBE, ORIGINALS = args.target, args.build_root, args.native_probe, args.originals
    if args.oracle:
        if not ORIGINALS:
            parser.error('--oracle requires --originals')
        from original_ground_bearing_oracle import OriginalGroundBearingOracle
        ORACLE = OriginalGroundBearingOracle()
        ORACLE.machine_for_cases = ORACLE.machine((1, 2, 32768, 65535), 3)
    program = unittest.main(argv=[__file__], exit=False)
    success = (program.result.wasSuccessful() and program.result.testsRun == 8 and
               (not args.oracle or not program.result.skipped))
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        evidence = {'scope': 'complete shared child callback; full parent/throttle/gear/living battle remain open',
                    'target': TARGET, 'success': success, 'groups': program.result.testsRun,
                    'skips': len(program.result.skipped), 'cases_per_target': DirectionTests.count,
                    'atomic_rejections_per_target': DirectionTests.rejections,
                    'complete_original_bearing_returns': DirectionTests.original_returns,
                    'canonical_prepared_worlds_per_target': DirectionTests.canonical_worlds,
                    'canonical_fine_coarse_returns_per_target': DirectionTests.canonical_phases,
                    'navigation_continuation': DirectionTests.navigation,
                    'normalized_output_sha256': DirectionTests.digest.hexdigest(),
                    'complete_wasm_streak': 0}
        (REVIEW / 'ground-bearing.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(0 if success else 1)
