#!/usr/bin/env python3
"""Complete shared mission preparation, current bindings and original reset returns."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from mission_ready_contract import inject, observed_world, original_prepare, prepare
from orders_contract import constructed_blocks, scenario_order_blocks
from test_mission_world import install, record as saved_record, world_lines
from test_units import records_from_scenario, scenario_data

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = False
SEEDS = (1, 2, 32768, 65535)


def record(kind, index=0, value=1, *, flags=0, variant=0):
    index, value, raw = saved_record(kind, index, value, flags=flags, mode=variant)
    raw = bytearray(raw)
    raw[25] = variant
    return index, value, bytes(raw)


class MissionReadyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-mission-ready-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_mission_ready_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_mission_ready_probe.js')])
        cls.owner = None
        if ORACLE:
            from original_mission_ready_oracle import OriginalMissionReadyOracle
            cls.owner = OriginalMissionReadyOracle()
        cls.fixtures = cls.records = cls.passes = cls.rejected = cls.original_passes = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Mission preparation per target: {cls.fixtures} fixtures, {cls.records} saved records, '
              f'{cls.passes} complete passes, {cls.rejected} atomic rejections; '
              f'{cls.original_passes} complete original DGROUP returns', flush=True)

    def run_data(self, data, *, seeds=SEEDS, cursor=0, link=0, side=2,
                 pixels=bytes((0, 63, 128, 255)), commands=((0, 0),), oracle=True):
        records = records_from_scenario(data)
        orders = scenario_order_blocks(data)
        status, world = install(records, seeds, cursor, link)
        output = f'status {status}\n'
        if status == 0:
            output += world_lines(world, orders)
            machine = objects = None
            if self.owner is not None and oracle:
                machine, objects = self.owner.prepare_saved(records, seeds, cursor, link, orders)
                self.assertEqual(world_lines(observed_world(self.owner, machine, objects), orders),
                                 world_lines(world, orders))
            for operation, flags in commands:
                if operation:
                    kind, injected = inject(world, operation, flags)
                    output += f'inject {kind} {injected}\n'
                    # Synthetic runtime payloads are tested separately from saved loading.
                    self.assertIsNone(machine, 'Original coverage must not skip a runtime allocation')
                else:
                    ready, world = prepare(world, side, pixels, link)
                    output += f'prepare {ready}\n' + world_lines(world, orders)
                    type(self).passes += ready == 0
                    type(self).rejected += ready != 0
                    if machine is not None:
                        self.assertEqual(ready, 0, 'Invalid input is a separate C atomicity gate')
                        self.assertEqual(world_lines(original_prepare(self.owner, machine, objects, side, pixels), orders),
                                         world_lines(world, orders))
                        type(self).original_passes += 1
        scenario = self.path / 'world.fsg'
        scenario.write_bytes(data)
        request = self.path / 'request.bin'
        request.write_bytes(struct.pack('<I4HBBH', side, *seeds, cursor, link, len(commands)) +
                            pixels + b''.join(struct.pack('<HH', *command) for command in commands))
        for command in self.commands:
            result = subprocess.run([*command, str(scenario), str(request)], capture_output=True,
                                    text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stderr)
            if result.stdout != output:
                difference = next((entry for entry in enumerate(itertools.zip_longest(
                    result.stdout.splitlines(), output.splitlines())) if entry[1][0] != entry[1][1]), None)
                self.fail(f'Complete preparation differs: {difference}; '
                          f'{len(result.stdout)} versus {len(output)} bytes')
        type(self).fixtures += 1
        type(self).records += len(records) if status == 0 else 0
        return output

    def run_records(self, records, **kwargs):
        return self.run_data(scenario_data(records, orders=constructed_blocks()), **kwargs)

    def test_ground_full_flag_link_domains_and_retained_orphan(self):
        for flags in range(256):
            records = [record(kind, 30 - kind, flags=flags) for kind in range(4)]
            self.run_records(records, cursor=flags % 4, link=flags)
        for link in (0, 1, 2, 3, 255):
            # Old roster/physical ground entries survive an overwritten binding.
            self.run_records([record(0, 7, flags=32), record(21, 7), record(2, 8, flags=255)],
                             link=link, commands=((0, 0), (0, 0)))

    def test_complete_tree_target_artillery_and_saved_base_domains(self):
        for value in range(256):
            flags = value & ~32
            records = [record(21, 60 - variant, flags=flags, variant=variant) for variant in range(4)]
            records += [record(26, 40 - variant, flags=flags, variant=variant) for variant in range(8)]
            records += [record(27, 20 - variant, flags=flags, variant=variant) for variant in range(2)]
            records += [record(kind, 70 + n, value * 257, flags=flags)
                        for n, kind in enumerate((11, 13, 16, 17, 18, 23, 25))]
            varied = []
            for n, (index, generation, raw) in enumerate(records):
                raw = bytearray(raw)
                struct.pack_into('<3i', raw, 4, -2147483648 + n * 65537,
                                 2147483647 - value * 65537, value * 16843009 - 2147483648)
                raw[23] = value
                varied.append((index, generation, bytes(raw)))
            self.run_records(varied, cursor=value % 4)

    def test_repeated_passes_order_census_and_complete_artillery_capacity(self):
        records = [record(27, 80 - n, flags=8 if n & 1 else 0, variant=n % 2) for n in range(8)]
        records += [record(21, 20 + n, variant=n) for n in range(4)]
        records += [record(16, 100), record(16, 101), record(25, 102), record(5, 0), record(6, 1)]
        for seeds in (SEEDS, (0, 0, 0, 0), (65535, 32767, 32768, 1)):
            for cursor in range(4):
                self.run_records(records, seeds=seeds, cursor=cursor,
                                 commands=((0, 0), (0, 0), (0, 0)))
        self.run_records([record(16, 1), record(21, 1), record(11, 2, 0), record(13, 3, 65535)],
                         commands=((0, 0), (0, 0)))
        self.run_records([])

    def test_runtime_effect_shell_retiring_release_and_generation_wrap(self):
        for flags in range(256):
            self.run_records([record(16, 10)], oracle=False,
                             commands=((1, flags), (2, flags), (3, flags), (0, 0),
                                       (1, flags), (2, flags), (3, flags), (0, 0)))

    def test_invalid_used_variants_capacity_and_height_are_atomic(self):
        for variant in (4, 5, 127, 255):
            self.run_records([record(16, 0), record(21, 1, variant=variant)], oracle=False)
        for variant in (2, 3, 127, 255):
            self.run_records([record(21, 0), record(27, 1, variant=variant)], oracle=False)
        for side_flag in (0, 8):
            self.run_records([record(21, 0)] + [record(27, n + 1, flags=side_flag) for n in range(5)], oracle=False)
        for side in (0, 3):
            self.run_records([record(21, 0)], side=side, pixels=bytes(side * side), oracle=False)
        # An invalid retained orphan variant is never dispatched.
        self.run_records([record(21, 0, variant=255), record(21, 0)])

    def test_all_pinned_missions_complete_preparation_and_saved_targets(self):
        if not ORIGINALS:
            self.skipTest('Full pinned mission corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        self.assertEqual(len(manifest), 47)
        total = targets = 0
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            total += len(records)
            targets += sum(int.from_bytes(raw[:2], 'little') < 4 and
                           struct.unpack_from('<H', raw, 0x97)[0] != 0 for _, _, raw in records)
            self.run_data(data, cursor=total % 4, commands=((0, 0), (0, 0)))
        self.assertEqual((total, targets), (4213, 14))

    def test_malformed_requests_fail_without_output(self):
        scenario = self.path / 'world.fsg'
        scenario.write_bytes(scenario_data([record(21)]))
        request = self.path / 'request.bin'
        valid = struct.pack('<I4HBBH', 2, *SEEDS, 0, 0, 1) + bytes(4) + struct.pack('<HH', 0, 0)
        invalid = [valid[:n] for n in (0, 1, 15, 16, 19, 23)] + [valid + b'x']
        for offset, value in ((12, 4), (20, 4), (22, 1)):
            changed = bytearray(valid)
            changed[offset] = value
            invalid.append(changed)
        for raw in invalid:
            request.write_bytes(raw)
            for command in self.commands:
                result = subprocess.run([*command, str(scenario), str(request)], capture_output=True, timeout=30)
                self.assertEqual((result.returncode, result.stdout), (1, b''), result.stderr)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS, ORACLE = args.build_root, args.target, args.native_probe, args.originals, args.oracle
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS))
