#!/usr/bin/env python3
"""Complete route/formation goal assignment and retained heading/goal ownership."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from orders_contract import constructed_blocks, scenario_order_blocks
from test_mission_world import install, world_lines
from test_units import expected as unit_expected, records_from_scenario, scenario_data, snapshot
from test_vehicle_motion import rotate, start
from test_vehicle_start import random_line, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = WORLD_PROBE = ORACLE = None
ORIGINALS = False
SEEDS = (1, 2, 32768, 65535)
BATCH = 4096
FORMATIONS = (((0, 0), (47320, 512), (18200, 512), (0, 512)),
              ((0, 0), (49140, 512), (16380, 512), (16380, 1024)),
              ((0, 0), (32760, 512), (32760, 1024), (0, 512)),
              ((0, 0), (43680, 512), (43680, 1024), (10920, 512)),
              ((0, 0), (21840, 512), (21840, 1024), (54600, 512)),
              ((0, 0), (43680, 848), (21840, 848), (32760, 912)))


def wrap(value):
    return (value + 2147483648) % 4294967296 - 2147483648


def sample(kind=0, *, flags=1, mode=2, member=1, platoon=0, formation=0, presence=1,
           heading=0, leader_kind=1, position=(-100, 200), count=1,
           waypoint=(-2147483648, 2147483647), seeds=SEEDS, cursor=0):
    raw = bytearray(start(kind))
    raw[0x1b], raw[0x1c], raw[0x43], raw[0x45] = platoon, member, mode, 255
    struct.pack_into('<H', raw, 0x40, flags)
    struct.pack_into('<4H', raw, 0x28, 65535, 32768, 1, heading)
    struct.pack_into('<ii', raw, 0x49, 123456789, -987654321)
    struct.pack_into('<H', raw, 0x97, 65535)
    leader = bytearray(start(leader_kind, x=position[0], y=position[1]))
    struct.pack_into('<4H', leader, 0x28, 12345, 32767, 32768, heading)
    if presence == 2:
        leader = bytearray(snapshot(23, flags=0, pose=(*position, 0)))
    paths, _ = constructed_blocks(count, salt=kind)
    route = bytearray(paths[:268])
    struct.pack_into('<ii', route, 12, *waypoint)
    descriptor = (65535, 32768, formation, 65534, 4, 5, 6, 7, 8, 9, 10)
    return bytes(raw), bytes(leader), presence, descriptor, bytes(route), seeds, cursor


def assign(case):
    original, leader, presence, descriptor, route, seeds, cursor = case
    raw = bytearray(original)
    mode, platoon = raw[0x43], raw[0x1b]
    if cursor >= 4 or platoon >= 8 or mode > 14 or mode % 2:
        return -1, original
    flags, = struct.unpack_from('<H', raw, 0x40)
    if mode == 0:
        if route[0] > 32:
            return -1, original
        if route[0]:
            raw[0x49:0x51] = route[12:20]
            flags |= 2
    elif mode == 2 and flags & 1:
        formation, member = descriptor[2], raw[0x1c]
        if formation >= 6 or member >= 4:
            return -1, original
        if presence in (1, 3):
            leader = original if presence == 3 else leader
            position = struct.unpack_from('<ii', leader, 4)
            heading, = struct.unpack_from('<H', leader, 0x2e)
            angle, distance = FORMATIONS[formation][member]
            delta = rotate((heading + angle) % 65536, distance, False)
            struct.pack_into('<ii', raw, 0x49, *(wrap(p + d * 32) for p, d in zip(position, delta)))
            flags |= 2
    struct.pack_into('<H', raw, 0x40, flags)
    return 0, bytes(raw)


def encode(cases):
    return struct.pack('<I', len(cases)) + b''.join(
        raw + leader.ljust(251, b'\0') + bytes([presence]) + struct.pack('<11H', *descriptor) +
        route + struct.pack('<4HB', *seeds, cursor)
        for raw, leader, presence, descriptor, route, seeds, cursor in cases)


class GoalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-goal-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append(([str(NATIVE_PROBE or BUILD / 'native/fist_ground_goal_probe')],
                                 [str(WORLD_PROBE or BUILD / 'native/fist_mission_world_probe')]))
        if TARGET in ('all', 'wasm'):
            cls.commands.append((['node', str(BUILD / 'wasm/fist_ground_goal_probe.js')],
                                 ['node', str(BUILD / 'wasm/fist_mission_world_probe.js')]))
        cls.fixtures = cls.rejections = cls.saved = cls.boundaries = cls.supported = cls.unsupported = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Command goals per target: {cls.fixtures} fixtures, {cls.rejections} rejections, '
              f'{cls.saved} saved ground states, {cls.boundaries} complete world boundaries, '
              f'{cls.supported} supported/{cls.unsupported} unsupported original worlds', flush=True)

    def run_cases(self, cases):
        outcomes = [assign(case) for case in cases]
        if ORACLE is not None:
            valid = [case for case, (status, _) in zip(cases, outcomes) if status == 0]
            self.assertEqual(ORACLE.cases(valid), [raw for status, raw in outcomes if status == 0])
        expected = ''.join(f'status {status}\n' + state_lines([(0, 0, raw)]) + random_line(case[5], case[6])
                           for case, (status, raw) in zip(cases, outcomes))
        self.run_data(encode(cases), expected)
        self.__class__.fixtures += len(cases)
        self.__class__.rejections += sum(status != 0 for status, _ in outcomes)

    def run_data(self, data, expected='', valid=True):
        path = pathlib.Path(self.temp.name) / 'cases.bin'; path.write_bytes(data)
        for command, _ in self.commands:
            result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, int(not valid), result.stderr)
            self.assertEqual(result.stdout, expected)

    def batches(self, cases):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == BATCH:
                self.run_cases(batch); batch = []
        if batch: self.run_cases(batch)

    def test_every_leader_heading_word_and_formation_rotation(self):
        self.batches(sample(formation=5, member=3, heading=heading, position=(2147483647, -2147483648))
                     for heading in range(65536))
        self.batches(sample(kind, formation=formation, member=member, heading=heading,
                            leader_kind=(kind + 1) % 4, position=(2147483647, -2147483648))
                     for kind in range(4) for formation in range(6) for member in range(4)
                     for heading in (0, 1, 63, 64, 65, 8191, 8192, 16383, 16384, 16385,
                                     32767, 32768, 32769, 49151, 49152, 49153, 65534, 65535))

    def test_all_control_words_route_admission_and_complete_saved_slots(self):
        self.batches(sample(flags=flags, mode=0, count=32) for flags in range(65536))
        self.batches(sample(kind, mode=0, count=count, waypoint=point, platoon=platoon)
                     for kind in range(4) for platoon in range(8) for count in range(256)
                     for point in ((-2147483648, 2147483647), (-1, 0), (123456789, -987654321)))

    def test_complete_mode_bank_presence_and_lazy_selectors(self):
        self.batches(sample(kind, mode=mode, flags=flags, presence=presence,
                            formation=formation, member=member, platoon=kind + 4)
                     for kind in range(4) for mode in range(256) for flags in (0, 1)
                     for presence in (0, 1, 2, 3) for formation, member in ((0, 0), (5, 3), (65535, 255)))
        self.batches(sample(formation=formation, member=member)
                     for formation in (6, 255, 256, 32767, 32768, 65535) for member in range(256))
        self.run_cases([sample(platoon=platoon) for platoon in range(8, 256)] +
                       [sample(cursor=cursor) for cursor in range(4, 256)])

    def test_malformed_batches_have_no_partial_output(self):
        good = encode([sample()])[4:]
        bad = bytearray(good); struct.pack_into('<H', bad, 0, 4)
        for data in (b'', bytes(3), struct.pack('<I', 1), struct.pack('<I', 0) + good,
                     struct.pack('<I', 1) + good[:-1], struct.pack('<I', 1) + good + b'x',
                     struct.pack('<I', 2) + good + bad):
            self.run_data(data, valid=False)
        self.run_cases([])

    def test_original_formation_table_and_complete_ui_cycles(self):
        if ORACLE is None:
            self.skipTest('Pinned original instruction oracle requested separately')
        self.assertEqual(ORACLE.formations, tuple(tuple(n for pair in row for n in pair) for row in FORMATIONS))
        from original_ground_command_oracle import OriginalGroundCommandOracle
        self.assertEqual(len(OriginalGroundCommandOracle().formation_cycles()), 96)

    def test_canonical_all_platoons_members_orphans_and_wreck_leader(self):
        paths, descriptors = constructed_blocks()
        descriptors = bytearray(descriptors)
        records = []
        for platoon in range(8):
            struct.pack_into('<H', descriptors, platoon * 22 + 4, platoon % 6)
            for member in range(4):
                raw = bytearray(sample(platoon % 4, mode=0 if member == 0 else 2,
                                       platoon=platoon, member=member, heading=platoon * 8191)[0])
                raw[22] |= 32
                records.append((0, len(records), bytes(raw)))
        records.append((9, 77, snapshot(23, flags=0, platoon=1, member=0)))
        self.complete_world(scenario_data(records, orders=(paths, bytes(descriptors))))

    def test_all_original_ground_states_and_complete_supported_worlds(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        count = 0
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            records, orders = records_from_scenario(data), scenario_order_blocks(data)
            _, _, _, roster = unit_expected(records)
            cases = []
            for _, _, raw in records:
                if int.from_bytes(raw[:2], 'little') >= 4: continue
                platoon = raw[0x1b]
                index = roster[platoon * 4]
                leader = records[index][2] if index != 65535 else b''
                presence = 0 if not leader else (2 if int.from_bytes(leader[:2], 'little') == 23 else 1)
                descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                route = orders[0][platoon * 268:(platoon + 1) * 268]
                cases.append((raw, leader, presence, descriptor, route, SEEDS, count % 4)); count += 1
            self.run_cases(cases)
            self.complete_world(data, original=True)
            self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual((count, self.supported, self.unsupported), (960, 10, 37))
        self.__class__.saved = count

    def complete_world(self, data, original=False):
        records, orders = records_from_scenario(data), scenario_order_blocks(data)
        status, world = install(records, SEEDS, 3, 0)
        if original:
            if status == 0: self.__class__.supported += 1
            else: self.__class__.unsupported += 1
        commands = [] if status else [(slot, 0) for _ in range(2) for slot, (allocation, _) in world[1].items() if allocation[0] < 4]
        commands += [] if status else [(65535, 0), (182, 0)]
        expected = f'status {status}\n'
        if status == 0:
            expected += world_lines(world, orders)
            if ORACLE is not None:
                from original_mission_orders_oracle import OriginalMissionOrdersOracle
                from original_mission_world_oracle import OriginalMissionWorldOracle
                original_world = OriginalMissionWorldOracle(); machine, objects = original_world.prepare(records, SEEDS, 3, 0)
                loader = OriginalMissionOrdersOracle(); loader.load(machine, *orders)
                self.assertEqual(original_world.world_lines(machine, objects, loader), world_lines(world, orders))
            for slot, _ in commands:
                result = -1
                if slot in world[1]:
                    allocation, raw = world[1][slot]
                    platoon = raw[0x1b]
                    leader_slot = world[2][platoon * 4]
                    leader = bytes(world[1][leader_slot][1]) if leader_slot != 65535 else b''
                    presence = 0 if not leader else (2 if int.from_bytes(leader[:2], 'little') == 23 else 1)
                    descriptor = struct.unpack_from('<11H', orders[1], platoon * 22)
                    route = orders[0][platoon * 268:(platoon + 1) * 268]
                    result, changed = assign((bytes(raw), leader, presence, descriptor, route, world[3], world[4]))
                    world[1][slot] = allocation, bytearray(changed)
                    if ORACLE is not None:
                        self.assertEqual(ORACLE.assign(machine, objects[slot][1], platoon), changed)
                expected += f'goal {slot} {result}\n' + world_lines(world, orders)
                if ORACLE is not None:
                    self.assertEqual(original_world.world_lines(machine, objects, loader), world_lines(world, orders))
            self.__class__.boundaries += len(commands) + 1
        path = pathlib.Path(self.temp.name) / 'world.fsg'; path.write_bytes(data)
        request = pathlib.Path(self.temp.name) / 'world.bin'
        request.write_bytes(struct.pack('<4HBBBI', *SEEDS, 3, 0, 0, len(commands)) + b''.join(struct.pack('<HH', *c) for c in commands))
        for _, command in self.commands:
            result = subprocess.run([*command, str(request), str(path), '--goals'], capture_output=True, text=True, timeout=60)
            self.assertEqual((result.returncode, result.stdout), (0, expected), result.stderr)


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
        from original_ground_goal_oracle import OriginalGroundGoalOracle
        ORACLE = OriginalGroundGoalOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
