#!/usr/bin/env python3
"""Complete shared route progress, original safe returns and proved capacity repair."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from orders_contract import constructed_blocks, route_line, scenario_order_blocks
from test_ground_goal import assign, encode, sample as goal_sample
from test_mission_world import install, world_lines
from test_units import records_from_scenario, scenario_data
from test_vehicle_start import random_line, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = WORLD_PROBE = ORACLE = None
ORIGINALS = False
SEEDS = (1, 2, 32768, 65535)
BATCH = 2048


def sample(kind=0, *, flags=65535, mode=0, waypoint_mode=0, distance=48,
           count=31, platoon=0, cursor=3):
    case = list(goal_sample(kind, flags=flags, mode=mode, count=count,
                            platoon=platoon, cursor=cursor))
    raw = bytearray(case[0]); struct.pack_into('<H', raw, 0x53, distance)
    case[0] = bytes(raw)
    descriptor = list(case[3]); descriptor[1] = waypoint_mode
    case[3] = tuple(descriptor)
    route = bytearray(case[4])
    for point in range(32):
        # Distinct full-width stored pairs make every active/inactive copy visible.
        struct.pack_into('<ii', route, 12 + point * 8,
                         -2147483648 + point * 65537 + kind * 8 + platoon,
                         2147483647 - point * 16777217 - kind * 8 - platoon)
    case[4] = bytes(route)
    return tuple(case)


def progress(case):
    original, _, _, descriptor, saved, _, cursor = case
    raw, route = bytearray(original), bytearray(saved)
    if cursor >= 4 or raw[0x1b] >= 8 or raw[0x43] > 14 or raw[0x43] % 2:
        return -1, original, saved
    flags, distance = struct.unpack_from('<H', raw, 0x40)[0], struct.unpack_from('<H', raw, 0x53)[0]
    if raw[0x43] != 0 or not flags & 2 or distance > 48:
        return 0, original, saved
    count = route[0]
    if count > 32:
        return -1, original, saved
    if count:
        points = [saved[12 + index * 8:20 + index * 8] for index in range(32)]
        if descriptor[1] == 3:
            points[:count] = points[1:count] + points[:1]
        else:
            # Complete original safe storage moves old-count pairs. Full capacity
            # has no 33rd pair: retain its newly unused last value deliberately.
            moved = count if count < 32 else 31
            points[:moved] = points[1:moved + 1]
            route[0] -= 1
        route[12:] = b''.join(points)
    struct.pack_into('<H', raw, 0x40, flags & 65533)
    return 0, bytes(raw), bytes(route)


def unsafe_original_pop(case):
    raw, _, _, descriptor, route, _, _ = case
    return (raw[0x43] == 0 and struct.unpack_from('<H', raw, 0x40)[0] & 2 and
            struct.unpack_from('<H', raw, 0x53)[0] <= 48 and route[0] == 32 and descriptor[1] != 3)


class RouteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-route-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append(([str(NATIVE_PROBE or BUILD / 'native/fist_ground_goal_probe'), '--routes'],
                                 [str(WORLD_PROBE or BUILD / 'native/fist_mission_world_probe')]))
        if TARGET in ('all', 'wasm'):
            cls.commands.append((['node', str(BUILD / 'wasm/fist_ground_goal_probe.js'), '--routes'],
                                 ['node', str(BUILD / 'wasm/fist_mission_world_probe.js')]))
        cls.fixtures = cls.rejections = cls.repairs = cls.original_returns = cls.saved = cls.boundaries = 0
        cls.supported = cls.unsupported = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Command routes per target: {cls.fixtures} fixtures, {cls.rejections} rejections, '
              f'{cls.repairs} full-capacity pop repairs, {cls.original_returns} unchanged original returns; '
              f'{cls.saved} saved ground states, {cls.boundaries} complete world boundaries, '
              f'{cls.supported} supported/{cls.unsupported} unsupported original worlds', flush=True)

    def run_data(self, data, expected='', valid=True):
        path = pathlib.Path(self.temp.name) / 'cases.bin'; path.write_bytes(data)
        for command, _ in self.commands:
            # Flags follow the data path, matching the shared probe's existing CLI.
            result = subprocess.run([*command[:-1], str(path), command[-1]], capture_output=True,
                                    text=True, timeout=60)
            self.assertEqual(result.returncode, int(not valid), result.stderr)
            self.assertEqual(result.stdout, expected)

    def run_cases(self, cases):
        outcomes = [progress(case) for case in cases]
        safe = [(case, (raw, route)) for case, (status, raw, route) in zip(cases, outcomes)
                if status == 0 and not unsafe_original_pop(case)]
        if ORACLE is not None:
            self.assertEqual(ORACLE.cases([case for case, _ in safe]), [outcome for _, outcome in safe])
            self.__class__.original_returns += len(safe)
        expected = ''.join(f'status {status}\n' + state_lines([(0, 0, raw)]) +
                           (route_line(raw[0x1b], route) if raw[0x1b] < 8 else 'route unavailable\n') +
                           random_line(case[5], case[6])
                           for case, (status, raw, route) in zip(cases, outcomes))
        self.run_data(encode(cases), expected)
        self.__class__.fixtures += len(cases)
        self.__class__.rejections += sum(status != 0 for status, _, _ in outcomes)
        self.__class__.repairs += sum(status == 0 and unsafe_original_pop(case)
                                     for case, (status, _, _) in zip(cases, outcomes))

    def batches(self, cases):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == BATCH:
                self.run_cases(batch); batch = []
        if batch: self.run_cases(batch)

    def test_every_control_word_and_unsigned_range_word(self):
        self.batches(sample(flags=flags, count=32, waypoint_mode=3) for flags in range(65536))
        self.batches(sample(distance=distance, kind=distance % 4, count=2, waypoint_mode=3)
                     for distance in range(65536))

    def test_every_waypoint_mode_word_and_original_fallback(self):
        self.batches(sample(waypoint_mode=mode, kind=mode % 4, platoon=mode % 8)
                     for mode in range(65536))

    def test_complete_mode_bank_all_counts_and_lazy_unused_selectors(self):
        self.batches(sample(kind, mode=mode, flags=flags, distance=distance, count=count,
                            waypoint_mode=65535, platoon=kind + 4)
                     for kind in range(4) for mode in range(256) for flags in (0, 1, 2, 65535)
                     for distance in (0, 48, 49, 65535) for count in (32, 255))
        self.batches(sample(kind, platoon=platoon, count=count, waypoint_mode=mode)
                     for kind in range(4) for platoon in range(8) for count in range(256)
                     for mode in (0, 1, 2, 3, 4, 65535))
        self.run_cases([sample(platoon=platoon) for platoon in range(8, 256)] +
                       [sample(cursor=cursor) for cursor in range(4, 256)])

    def test_full_capacity_pop_and_cycle_preserve_all_stored_coordinates(self):
        cases = [sample(kind, platoon=platoon, count=32, waypoint_mode=mode, distance=distance)
                 for kind in range(4) for platoon in range(8) for mode in (0, 1, 2, 3, 4, 65535)
                 for distance in (0, 47, 48)]
        self.run_cases(cases)
        for case in cases:
            _, _, changed = progress(case)
            old = case[4]
            self.assertEqual(changed[1:12], old[1:12])
            self.assertEqual(changed[12:260], old[20:268])
            self.assertEqual(changed[-8:], old[12:20] if case[3][1] == 3 else old[-8:])

    def test_malformed_batches_have_no_partial_output(self):
        good = encode([sample()])
        bad = bytearray(good[4:]); struct.pack_into('<H', bad, 0, 4)
        for data in (b'', bytes(3), struct.pack('<I', 1), struct.pack('<I', 0) + good[4:],
                     good[:-1], good + b'x', struct.pack('<I', 2) + good[4:] + bad):
            self.run_data(data, valid=False)
        self.run_cases([])
        path = pathlib.Path(self.temp.name) / 'empty.bin'; path.write_bytes(encode([]))
        for command, _ in self.commands:
            for arguments in ([], [str(path), '--unknown'], [str(path), '--routes', 'extra'],
                              [str(path.parent / 'missing.bin'), '--routes']):
                result = subprocess.run([*command[:-1], *arguments], capture_output=True, text=True, timeout=60)
                self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_original_full_capacity_reaching_failure_and_real_returns(self):
        if ORACLE is None:
            self.skipTest('Pinned original instruction oracle requested separately')
        evidence = ORACLE.full_capacity_proof()
        self.assertEqual((len(evidence), sum(bool(v['outside_loads']) for v in evidence)), (192, 96))
        for platoon in range(8):
            for mode in (0, 1, 2, 4, 65535):
                values = [v for v in evidence if v['platoon'] == platoon and v['count'] == 32 and v['mode'] == mode]
                self.assertEqual([v['tail'] for v in values], [v['neighbor'] for v in values])
                self.assertNotEqual(values[0]['tail'], values[1]['tail'])

    def test_canonical_shared_routes_goal_refresh_orphans_and_complete_cycles(self):
        paths, descriptors = constructed_blocks(32)
        info = bytearray(descriptors)
        records = []
        for platoon in range(8):
            struct.pack_into('<H', info, platoon * 22 + 2, platoon % 4)
            for member in range(4):
                raw = bytearray(sample(platoon % 4, platoon=platoon)[0])
                raw[22] |= 32; raw[0x1c] = member
                # Deliberate declared command boundary: every member consumes the
                # same platoon route; duplicate registry zero leaves physical orphans.
                records.append((0, len(records), bytes(raw)))
        data = scenario_data(records, orders=(paths, bytes(info)))
        self.complete_world(data, consuming=True)

    def test_all_original_saved_ranges_routes_and_complete_supported_worlds(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        count = 0
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            records, orders = records_from_scenario(data), scenario_order_blocks(data)
            cases = []
            for _, _, raw in records:
                if int.from_bytes(raw[:2], 'little') >= 4: continue
                platoon = raw[0x1b]
                descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                route = orders[0][platoon * 268:(platoon + 1) * 268]
                base = (raw, bytes(251), 0, descriptor, route, SEEDS, count % 4)
                cases.append(base)
                reached = bytearray(raw); reached[0x43] = 0
                struct.pack_into('<H', reached, 0x40, struct.unpack_from('<H', raw, 0x40)[0] | 2)
                struct.pack_into('<H', reached, 0x53, 48)
                cases.append((bytes(reached), *base[1:])); count += 1
            self.run_cases(cases)
            self.complete_world(data, original=True)
            self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual((count, self.supported, self.unsupported), (960, 10, 37))
        self.__class__.saved = count

    def complete_world(self, data, original=False, consuming=False):
        records, orders = records_from_scenario(data), scenario_order_blocks(data)
        status, world = install(records, SEEDS, 3, 0)
        if original:
            if status == 0: self.__class__.supported += 1
            else: self.__class__.unsupported += 1
        paths = bytearray(orders[0])
        commands = []
        if status == 0:
            ground = [slot for slot, (allocation, _) in world[1].items() if allocation[0] < 4]
            if consuming:
                # Complete repeated goal -> progress returns cross physical members,
                # exhaust finite routes and wrap cyclic ones beyond a full rotation.
                commands = [(ground[platoon * 4 + tick % 4], action)
                            for tick in range(35) for platoon in range(8) for action in (0, 1)]
            else:
                commands = [(slot, 1) for _ in range(2) for slot in ground]
            commands += [(65535, 1), (182, 1)]
        expected = f'status {status}\n'
        owner = machine = objects = loader = None
        goal_owner = None
        if status == 0:
            expected += world_lines(world, (bytes(paths), orders[1]))
            if ORACLE is not None:
                from original_mission_orders_oracle import OriginalMissionOrdersOracle
                from original_mission_world_oracle import OriginalMissionWorldOracle
                owner = OriginalMissionWorldOracle(); machine, objects = owner.prepare(records, SEEDS, 3, 0)
                from original_ground_goal_oracle import OriginalGroundGoalOracle
                goal_owner = OriginalGroundGoalOracle()
                loader = OriginalMissionOrdersOracle(); loader.load(machine, *orders)
                self.assertEqual(owner.world_lines(machine, objects, loader), world_lines(world, orders))
            for slot, action in commands:
                result = -1
                if slot in world[1]:
                    allocation, raw = world[1][slot]
                    platoon = raw[0x1b]
                    route = bytes(paths[platoon * 268:(platoon + 1) * 268])
                    descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                    case = (bytes(raw), bytes(251), 0, descriptor, route, world[3], world[4])
                    if action == 0:
                        result, changed = assign(case)
                        changed_route = route
                    else:
                        result, changed, changed_route = progress(case)
                    world[1][slot] = allocation, bytearray(changed)
                    paths[platoon * 268:(platoon + 1) * 268] = changed_route
                    if ORACLE is not None:
                        if action == 0:
                            self.assertEqual(goal_owner.assign(machine, objects[slot][1], platoon), changed)
                        elif unsafe_original_pop(case):
                            observed_raw, observed_route = ORACLE.advance(machine, objects[slot][1], platoon)
                            self.assertEqual(observed_raw, changed)
                            self.assertEqual(observed_route[:260], changed_route[:260])
                            # Declare the sole intentional inactive-slot repair between
                            # complete calls. This is not an original parity claim.
                            from original_unit_oracle import DGROUP
                            machine.mem_write(DGROUP + 0x7d40 + platoon * 268 + 260, changed_route[-8:])
                        else:
                            self.assertEqual(ORACLE.advance(machine, objects[slot][1], platoon), (changed, changed_route))
                expected += f'{"goal" if action == 0 else "route"} {slot} {result}\n' + world_lines(world, (bytes(paths), orders[1]))
                if ORACLE is not None:
                    self.assertEqual(owner.world_lines(machine, objects, loader), world_lines(world, (bytes(paths), orders[1])))
            self.__class__.boundaries += len(commands) + 1
            if consuming:
                for platoon in range(8):
                    changed = bytes(paths[platoon * 268:(platoon + 1) * 268])
                    old = orders[0][platoon * 268:(platoon + 1) * 268]
                    self.assertEqual(changed[1:12], old[1:12])
                    self.assertEqual(changed[0], 32 if platoon % 4 == 3 else 0)
                    if platoon % 4 == 3:
                        self.assertEqual(changed[12:], old[36:] + old[12:36])
        path = pathlib.Path(self.temp.name) / 'world.fsg'; path.write_bytes(data)
        request = pathlib.Path(self.temp.name) / 'world.bin'
        request.write_bytes(struct.pack('<4HBBBI', *SEEDS, 3, 0, 0, len(commands)) +
                            b''.join(struct.pack('<HH', *command) for command in commands))
        for _, command in self.commands:
            result = subprocess.run([*command, str(request), str(path), '--navigation'], capture_output=True,
                                    text=True, timeout=180)
            self.assertEqual((result.returncode, result.stdout), (0, expected), result.stderr)
            malformed = pathlib.Path(self.temp.name) / 'bad-navigation.bin'
            malformed.write_bytes(struct.pack('<4HBBBIHH', *SEEDS, 3, 0, 0, 1, 150, 2))
            failed = subprocess.run([*command, str(malformed), str(path), '--navigation'],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual((failed.returncode, failed.stdout), (1, ''), failed.stderr)
        if consuming:
            # Re-import the complete changed PATH/PINF values as new owned input.
            # This checks route persistence at the existing decoder boundary;
            # it does not claim the pending user-facing save UI is delivered.
            self.complete_world(scenario_data(records, orders=(bytes(paths), orders[1])))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--world-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, WORLD_PROBE, ORIGINALS = args.build_root, args.target, args.native_probe, args.world_probe, args.originals
    if args.oracle:
        from original_ground_route_oracle import OriginalGroundRouteOracle
        ORACLE = OriginalGroundRouteOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
