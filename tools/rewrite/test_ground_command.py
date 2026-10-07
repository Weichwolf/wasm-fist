#!/usr/bin/env python3
"""Complete ground mode priority, conditional RNG and canonical order consumption."""
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
from test_units import records_from_scenario, scenario_data
from test_vehicle_motion import start
from test_vehicle_start import random_line, state_lines, step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
WORLD_PROBE = None
ORIGINALS = False
ORACLE = None
SEEDS = (1, 2, 32768, 65535)
BATCH = 4096


def sample(kind=0, *, flags=1, motion=0, maneuver=0, target=0, member=0, platoon=0,
           behavior=0, waypoint=0, phase_random=0, seeds=SEEDS, cursor=0, mode=255):
    raw = bytearray(start(kind))
    raw[0x19], raw[0x1b], raw[0x1c], raw[0x43], raw[0x45] = motion, platoon, member, mode, maneuver
    struct.pack_into('<H', raw, 0x40, flags)
    struct.pack_into('<H', raw, 0x97, target)
    # Unknown words must survive unchanged and must not affect this consumer.
    descriptor = (behavior, waypoint, 65535, 32768, 17, 65534, 4, 5, 6, 7, 8)
    return bytes(raw), descriptor, phase_random, seeds, cursor


def select(case):
    original, descriptor, phase_random, seeds, cursor = case
    raw, words = bytearray(original), list(seeds)
    flags, = struct.unpack_from('<H', raw, 0x40)
    if cursor >= 4 or raw[0x1b] >= 8:
        return -1, original, (words, cursor)
    mode = 0 if raw[0x1c] == 0 else 2
    if flags & 1:
        behavior, waypoint = descriptor[:2]
        if behavior == 3:
            mode = 14
        elif raw[0x19] & 6:
            mode = 12
        elif flags & 16:
            mode = 10
        elif raw[0x45]:
            mode = 8
        elif flags & 64:
            mode = 4
        else:
            if flags & 32:
                if behavior >= 4:
                    return -1, original, (list(seeds), cursor)
                flags &= 65503
                if phase_random % 256 <= (100, 10, 200, 0)[behavior]:
                    mode = 4
            if mode != 4 and struct.unpack_from('<H', raw, 0x97)[0]:
                if waypoint >= 4:
                    return -1, original, (list(seeds), cursor)
                rolled, cursor = step(words, cursor)
                if rolled % 256 < (0, 50, 255, 0)[waypoint]:
                    mode = 6
    raw[0x43] = mode
    struct.pack_into('<H', raw, 0x40, flags)
    return 0, bytes(raw), (words, cursor)


class CommandTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-command-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append(([str(NATIVE_PROBE or BUILD / 'native/fist_ground_command_probe')],
                                 [str(WORLD_PROBE or BUILD / 'native/fist_mission_world_probe')]))
        if TARGET in ('all', 'wasm'):
            cls.commands.append((['node', str(BUILD / 'wasm/fist_ground_command_probe.js')],
                                 ['node', str(BUILD / 'wasm/fist_mission_world_probe.js')]))
        cls.fixtures = cls.rejections = cls.originals = cls.world_boundaries = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Command selection per target: {cls.fixtures} fixtures, {cls.rejections} rejections, '
              f'{cls.originals} saved ground states, {cls.world_boundaries} complete world boundaries', flush=True)

    def run_cases(self, cases):
        outcomes = [select(case) for case in cases]
        valid = [case for case, outcome in zip(cases, outcomes) if outcome[0] == 0]
        if ORACLE is not None:
            observed = ORACLE.cases(valid)
            wanted = [(raw, random) for status, raw, random in outcomes if status == 0]
            self.assertEqual(observed, wanted, 'Complete original 251-byte/RNG returns differ')
        expected = ''.join(f'status {status}\n' + state_lines([(0, 0, raw)]) + random_line(*random)
                           for status, raw, random in outcomes)
        data = struct.pack('<I', len(cases)) + b''.join(
            raw + struct.pack('<11HH4HB', *descriptor, phase, *seeds, cursor)
            for raw, descriptor, phase, seeds, cursor in cases)
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        path.write_bytes(data)
        for command, _ in self.commands:
            result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, expected)
        self.__class__.fixtures += len(cases)
        self.__class__.rejections += len(cases) - len(valid)

    def batches(self, cases):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == BATCH:
                self.run_cases(batch)
                batch = []
        if batch:
            self.run_cases(batch)

    def test_every_control_word_and_priority_combinations(self):
        self.batches(sample(flags=flags, target=65535, waypoint=2, phase_random=101)
                     for flags in range(65536))
        self.batches(sample(kind, flags=flags | 1, motion=motion, maneuver=maneuver, target=1,
                            behavior=behavior, waypoint=2, phase_random=255)
                     for kind in range(4) for flags in (0, 16, 32, 64, 48, 80, 96, 112)
                     for motion in (0, 2, 4, 6, 249, 255) for maneuver in (0, 1, 255)
                     for behavior in range(4))

    def test_every_roll_byte_full_word_and_rng_stream(self):
        self.batches(sample(kind, flags=33, behavior=behavior, phase_random=roll | high,
                            target=1, waypoint=2)
                     for kind in range(4) for behavior in range(4) for roll in range(256)
                     for high in (0, 65280))
        self.batches(sample(kind, target=1, waypoint=waypoint, cursor=cursor,
                            seeds=tuple((roll + 1) * 2 if n == cursor else SEEDS[n] for n in range(4)))
                     for kind in range(4) for waypoint in range(4) for cursor in range(4)
                     for roll in range(256))

    def test_presence_and_all_saved_selector_bytes(self):
        self.batches(sample(target=target, waypoint=1, seeds=(100, 2, 32768, 65535))
                     for target in range(65536))
        self.batches(sample(kind, member=value, mode=value, maneuver=maneuver, motion=motion,
                            flags=flags, target=65535, waypoint=2, platoon=kind + 4)
                     for kind in range(4) for value in range(256) for maneuver, motion, flags in
                     ((0, 0, 0), (value, 0, 1), (0, value, 1)))

    def test_lazy_choice_domains_and_transactional_failures(self):
        cases = []
        for bad in (4, 255, 256, 32767, 32768, 65535):
            for kind in range(4):
                cases.extend((sample(kind, flags=33, behavior=bad),
                              sample(kind, flags=33, behavior=0, phase_random=101, target=1, waypoint=bad),
                              sample(kind, target=1, waypoint=bad),
                              sample(kind, flags=0, behavior=bad, waypoint=bad),
                              sample(kind, flags=65, behavior=bad, waypoint=bad, target=1),
                              sample(kind, behavior=bad, waypoint=bad),
                              sample(kind, flags=33, behavior=0, phase_random=100, target=1, waypoint=bad),
                              sample(kind, behavior=3, target=1, waypoint=bad)))
        cases.extend(sample(platoon=platoon) for platoon in range(8, 256))
        cases.extend(sample(cursor=cursor) for cursor in range(4, 256))
        self.run_cases(cases)

    def test_malformed_batches_produce_no_partial_output(self):
        good = sample()
        record = good[0] + struct.pack('<11HH4HB', *good[1], good[2], *good[3], good[4])
        bad = bytearray(record); struct.pack_into('<H', bad, 0, 4)
        for data in (b'', bytes(3), struct.pack('<I', 1), struct.pack('<I', 0) + record,
                     struct.pack('<I', 1) + record[:-1], struct.pack('<I', 1) + record + b'x',
                     struct.pack('<I', 2) + record + bad):
            path = pathlib.Path(self.temp.name) / 'bad.bin'; path.write_bytes(data)
            for command, _ in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)
        self.run_cases([])

    def test_original_ui_choice_cycles(self):
        if ORACLE is None:
            self.skipTest('Pinned original instruction oracle requested separately')
        self.assertEqual(len(ORACLE.choice_cycles()), 128)

    def test_malformed_mission_orders_release_decoded_units(self):
        data = scenario_data([(0, 0, sample()[0])])
        request = pathlib.Path(self.temp.name) / 'empty-world.bin'
        request.write_bytes(struct.pack('<4HBBBI', *SEEDS, 0, 0, 0, 0))
        chunks = []
        offset = 0
        while offset < len(data):
            tag, size = struct.unpack_from('<4sH', data, offset)
            chunks.append((tag, data[offset + 6:offset + 6 + size]))
            offset += 6 + size
        for tag in (b'PATH', b'PINF'):
            for missing in (False, True):
                damaged = b''.join(struct.pack('<4sH', current, len(body) - int(current == tag)) +
                                   (body[:-1] if current == tag else body)
                                   for current, body in chunks if not (missing and current == tag))
                path = pathlib.Path(self.temp.name) / 'bad-world.fsg'; path.write_bytes(damaged)
                for _, command in self.commands:
                    result = subprocess.run([*command, str(request), str(path), '--commands'],
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual((result.returncode, result.stdout), (1, ''), result.stderr)
                    self.assertEqual(result.stderr, '', 'Failed loader must release every owned allocation')

    def test_canonical_platoon_descriptors_and_registry_orphans(self):
        paths, descriptors = constructed_blocks()
        descriptors = bytearray(descriptors)
        records = []
        for platoon in range(8):
            struct.pack_into('<2H', descriptors, platoon * 22, platoon % 4, (platoon + 1) % 4)
            raw = bytearray(sample(platoon % 4, flags=33, platoon=platoon, member=0,
                                   target=65535, maneuver=platoon % 3)[0])
            raw[22] |= 32
            records.append((0, platoon, bytes(raw)))
        # Eight physical actors, one overwritten registry entry and eight real
        # platoon leaders. Selection must retain and consume physical orphans.
        self.complete_world(scenario_data(records, orders=(paths, bytes(descriptors))), 8)

    def test_all_original_ground_snapshots_and_complete_train1_world(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        count = nonzero = 0
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            _, descriptors = scenario_order_blocks(data)
            cases = []
            for _, _, raw in records_from_scenario(data):
                if int.from_bytes(raw[:2], 'little') >= 4:
                    continue
                descriptor = struct.unpack_from('<11H', descriptors, raw[0x1b] * 22)
                cases.append((raw, descriptor, 65535, SEEDS, count % 4))
                count += 1
                nonzero += struct.unpack_from('<H', raw, 0x97)[0] != 0
            self.run_cases(cases)
        self.assertEqual((count, nonzero), (960, 14))
        self.__class__.originals = count
        self.complete_world((directory / 'TRAIN1.FSG').read_bytes(), 85)

    def complete_world(self, data, expected_objects):
        records, orders = records_from_scenario(data), scenario_order_blocks(data)
        status, world = install(records, SEEDS, 3, 0)
        self.assertEqual((status, len(world[1])), (0, expected_objects))
        commands = [(slot, phase) for phase in (0, 100, 101, 255, 65535)
                    for slot, (allocation, _) in world[1].items() if allocation[0] < 4]
        commands += [(0, 0), (65535, 65535), (182, 1)]
        expected = 'status 0\n' + world_lines(world, orders)
        if ORACLE is not None:
            from original_mission_orders_oracle import OriginalMissionOrdersOracle
            from original_mission_world_oracle import OriginalMissionWorldOracle
            original = OriginalMissionWorldOracle()
            machine, objects = original.prepare(records, SEEDS, 3, 0)
            loader = OriginalMissionOrdersOracle(); loader.load(machine, *orders)
            self.assertEqual(original.world_lines(machine, objects, loader), world_lines(world, orders))
        path = pathlib.Path(self.temp.name) / 'world.fsg'; path.write_bytes(data)
        request = pathlib.Path(self.temp.name) / 'world.bin'
        request.write_bytes(struct.pack('<4HBBBI', *SEEDS, 3, 0, 0, len(commands)) +
                            b''.join(struct.pack('<HH', *command) for command in commands))
        for slot, phase in commands:
            status = -1
            if slot in world[1] and world[1][slot][0][0] < 4:
                allocation, raw = world[1][slot]
                descriptor = struct.unpack_from('<11H', orders[1], raw[0x1b] * 22)
                status, changed, (words, cursor) = select((bytes(raw), descriptor, phase, world[3], world[4]))
                world[1][slot] = allocation, bytearray(changed)
                world = (*world[:3], words, cursor)
                if ORACLE is not None:
                    pointer = objects[slot][1]
                    actual, random = ORACLE.select(machine, pointer, 0x85b6 + raw[0x1b] * 22, phase)
                    self.assertEqual((actual, random), (changed, (words, cursor)))
            expected += f'select {slot} {status}\n' + world_lines(world, orders)
            if ORACLE is not None:
                self.assertEqual(original.world_lines(machine, objects, loader), world_lines(world, orders))
        for _, command in self.commands:
            result = subprocess.run([*command, str(request), str(path), '--commands'],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, expected)
        self.__class__.world_boundaries += len(commands) + 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--world-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, WORLD_PROBE, ORIGINALS = args.native_probe, args.world_probe, args.originals
    if args.oracle:
        from original_ground_command_oracle import OriginalGroundCommandOracle
        ORACLE = OriginalGroundCommandOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
