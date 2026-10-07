#!/usr/bin/env python3
"""Ground movement maintenance, original width/cadence and reaching controlled integration."""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from ground_phase_contract import GroundPhaseTests
from test_units import records_from_scenario
from test_vehicle_motion import start
from test_vehicle_start import COMPONENT_OFFSET

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
PHASE = (10, 10, 16, 20)
COMPONENTS = ((0xcc, 0xcd), (0xc6, 0xc7), (0xf1, 0xf0), (0xf8, 0xf7))
BATCH = 2048


def update(original):
    raw = bytearray(original)
    kind, = struct.unpack_from('<H', raw)
    if raw[0x3d] & 0x1e == PHASE[kind]:
        speed, = struct.unpack_from('<h', raw, 0x55)
        movement, = struct.unpack_from('<H', raw, 0x5d)
        struct.pack_into('<H', raw, 0x5d, max(0, movement - abs(speed) // 16))
        if speed >= 60:
            if raw[0x5f] < 248:
                raw[0x5f] += 1
        elif raw[0x5f]:
            raw[0x5f] -= 1
        if raw[0x3d] & 0xe0 == 0:
            for offset in COMPONENTS[kind]:
                raw[offset] = 3
    return bytes(raw)


def sample(kind=0, *, speed=64, movement=65535, counter=0, phase=None):
    raw = bytearray(start(kind, speed=speed, gate=movement, phase=PHASE[kind] if phase is None else phase))
    raw[0x5f] = counter
    # Explicitly distinguish every component, including the two refreshed slots.
    offset = COMPONENT_OFFSET[kind]
    for index in range((57, 62, 59, 61)[kind]):
        raw[offset + index] = (index * 13 + 17) % 256
    return bytes(raw)


class MaintenanceTests(GroundPhaseTests):
    mode = 'maintenance'
    label = 'Movement maintenance'
    update = staticmethod(update)
    original_method = 'maintenance_cases'

    @classmethod
    def configuration(cls):
        return BUILD, TARGET, NATIVE_PROBE, ORACLE

    def batches(self, cases):
        batch = []
        for case in cases:
            batch.append(case)
            if len(batch) == BATCH:
                self.run_cases(batch)
                batch = []
        if batch:
            self.run_cases(batch)

    def test_every_signed_speed_and_class(self):
        self.batches((sample(kind, speed=speed, counter=250), 1)
                     for kind in range(4) for speed in range(-32768, 32768))

    def test_all_movement_subtraction_edges_for_every_consumption(self):
        cases = []
        for kind in range(4):
            for consumed in range(2049):
                speed = -32768 if consumed == 2048 else consumed * 16
                for movement in sorted({0, consumed // 2, max(0, consumed - 1), consumed, consumed + 1, 65535}):
                    cases.append((sample(kind, speed=speed, movement=movement, counter=123), 1))
        self.batches(cases)

    def test_every_counter_byte_and_signed_threshold_direction(self):
        self.batches((sample(kind, speed=speed, counter=counter), 1)
                     for kind in range(4) for speed in (-32768, -60, 0, 59, 60, 61, 32767)
                     for counter in range(256))

    def test_every_class_phase_byte_and_exact_component_writes(self):
        self.run_cases([(sample(kind, phase=phase, counter=255), 1)
                        for kind in range(4) for phase in range(256)])

    def test_repeated_exhaustion_counter_bounds_and_source_release(self):
        self.run_cases([(sample(kind, speed=speed, movement=1, counter=counter), 300)
                        for kind in range(4) for speed, counter in ((-32768, 255), (59, 250), (60, 0), (32767, 249))])

    def test_invalid_whole_batch_has_no_partial_output(self):
        good = sample()
        bad = bytearray(good)
        struct.pack_into('<H', bad, 0, 4)
        self.run_data(struct.pack('<I', 0))
        for data in (b'', bytes(3), struct.pack('<I', 1),
                     struct.pack('<I', 1) + good + bytes(2),
                     struct.pack('<I', 1) + good + b'\1\0x',
                     struct.pack('<I', 2) + good + b'\1\0' + bad + b'\1\0'):
            self.run_data(data, valid=False)

    def test_all_original_ground_states_and_reaching_m1_prefix(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        count = 0
        for name, info in manifest.items():
            path = directory / name
            data = path.read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            cases = []
            for _, _, raw in records_from_scenario(data):
                kind = int.from_bytes(raw[:2], 'little')
                if kind >= 4:
                    continue
                active = bytearray(raw)
                active[0x3d] = (active[0x3d] & 0xe1) | PHASE[kind]
                cases.extend(((raw, 1), (bytes(active), 3)))
                count += 1
            self.run_cases(cases)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual(count, 960)
        if ORACLE is not None:
            self.reaching_prefix()

    def reaching_prefix(self):
        from original_controlled_m1_oracle import OriginalControlledM1Oracle
        from test_driving import driver_step
        records = records_from_scenario((ROOT / 'armoredfist/FISTDATA/TRAIN1.FSG').read_bytes())
        for movement, counter in ((65535, 0), (1, 1), (0, 255)):
            patches = ((0x5d, struct.pack('<H', movement)), (0x5f, bytes([counter])))
            pixels = bytes([32]) * (512 * 512)
            with OriginalControlledM1Oracle(records, 512, pixels, patches=patches) as original:
                self.assertEqual((len(original.objects), original.selected), (85, 151))
                for tick in range(1, 7):
                    before = original.state
                    actual = original.step()
                    expected, _ = driver_step(before, 0, 512, pixels)
                    self.assertEqual(actual, expected, f'Reaching complete TRAIN1 prefix tick {tick}')
                    if tick == 5:
                        self.assertEqual(actual[2][0x3d], 42)
                        self.assertEqual(struct.unpack_from('<H', actual[2], 0x5d)[0], max(0, movement - 2))
                        self.assertEqual(actual[2][0x5f], max(0, counter - 1))
                self.assertEqual([address for address in original.calls if address in (0x7cbf, 0x19ffc)], [0x7cbf, 0x19ffc])
                self.assertEqual(original.random_state(), original.initial_random)
                self.assertTrue(original.unchanged_others())


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
        from original_vehicle_maintenance_oracle import OriginalVehicleMaintenanceOracle
        ORACLE = OriginalVehicleMaintenanceOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
