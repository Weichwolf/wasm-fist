#!/usr/bin/env python3
"""Complete owned mission orders, source release and original bounded input evidence."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import tempfile
import unittest

from orders_contract import constructed_blocks, orders_lines, scenario_order_blocks
from test_scenario import envelope, synthetic_chunks
from test_units import scenario_data as unit_scenario_data

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def scenario_data(blocks):
    return unit_scenario_data([], orders=blocks)


class OrderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-orders-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.filename = pathlib.Path(cls.temp.name) / 'orders.fsg'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_scenario_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_scenario_probe.js')])
        cls.fixtures = cls.bytes = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Owned orders per target: {cls.fixtures} fixtures; {cls.bytes} complete valid '
              'input bytes. Every valid fixture also checks all 2048 count/platoon inputs, '
              '2320 truncated block lengths, oversized/null views and unchanged failure outputs.', flush=True)

    def run_data(self, data, valid=True, checks=False):
        self.filename.write_bytes(data)
        wanted = orders_lines(scenario_order_blocks(data)) if valid and not checks else ''
        if valid and ORACLE is not None:
            blocks = scenario_order_blocks(data)
            machine = ORACLE.machine((1, 2, 32768, 65535), 0)
            before = ORACLE.random_state(machine)
            requests = ORACLE.load(machine, *blocks)
            self.assertEqual(len(requests), 2)
            self.assertEqual(ORACLE.blocks(machine), blocks)
            self.assertEqual(ORACLE.random_state(machine), before)
        for command in self.commands:
            result = subprocess.run([*command, '--orders-checks' if checks else '--orders',
                                     str(self.filename)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
            self.assertEqual(result.stdout, wanted)
        self.assertEqual(self.filename.read_bytes(), data, 'Probe mutated its source file')
        type(self).fixtures += 1
        type(self).bytes += 2320 if valid else 0

    def test_all_valid_counts_signed_edges_and_retained_unused_data(self):
        for count in range(33):
            self.run_data(scenario_data(constructed_blocks(count, salt=count)))

    def test_complete_header_byte_domain_and_unknown_descriptor_words(self):
        # All 256 byte values occur in retained headers across four fixtures.
        for base in range(0, 256, 64):
            paths, descriptors = constructed_blocks(0)
            paths = bytearray(paths)
            for platoon in range(8):
                paths[platoon * 268 + 1:platoon * 268 + 12] = bytes(
                    (base + platoon * 8 + index) % 256 for index in range(11))
            self.run_data(scenario_data((bytes(paths), descriptors)))

    def test_every_prefix_count_and_null_api_checks_after_nonzero_owned_output(self):
        self.run_data(scenario_data(constructed_blocks()), checks=True)

    def test_malformed_required_blocks_no_partial_publication(self):
        paths, descriptors = constructed_blocks()
        for blocks in ((paths[:-1], descriptors), (paths + b'x', descriptors),
                       (paths, descriptors[:-1]), (paths, descriptors + b'x'),
                       (b'', descriptors), (paths, b'')):
            self.run_data(scenario_data(blocks), valid=False)
        for platoon in range(8):
            changed = bytearray(paths)
            changed[platoon * 268] = 255 if platoon % 2 else 33
            self.run_data(scenario_data((bytes(changed), descriptors)), valid=False)
        chunks = synthetic_chunks()
        for missing in (b'PATH', b'PINF'):
            self.run_data(envelope([chunk for chunk in chunks if chunk[0] != missing]), valid=False)

    def test_all_pinned_originals_and_original_editor_admission_boundary(self):
        if not ORIGINALS:
            self.skipTest('Complete original corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([file.name for file in files], sorted(manifest))
        for file in files:
            data = file.read_bytes()
            self.assertEqual(len(data), manifest[file.name]['size'])
            self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[file.name]['sha256'])
            self.run_data(data)
            self.assertEqual(hashlib.sha256(file.read_bytes()).hexdigest(), manifest[file.name]['sha256'])
        if ORACLE is not None:
            self.assertEqual(ORACLE.append_boundaries(), [(platoon, count, count < 32)
                             for platoon in range(8) for count in range(256)])


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
        from original_mission_orders_oracle import OriginalMissionOrdersOracle
        ORACLE = OriginalMissionOrdersOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS))
