#!/usr/bin/env python3
"""Owned ground position histories, original phase cadence and reaching TRAIN1 prefix."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario
from test_vehicle_motion import start
from test_vehicle_start import state_lines

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def update(original):
    raw = bytearray(original)
    if raw[0x3d] & 0x1e == 6:
        raw[0x6d] = (raw[0x6d] + 1) % 256
        if raw[0x6d] >= 12:
            raw[0x6d] = 0
            raw[0x72:0x86] = original[0x6e:0x82]
            raw[0x6e:0x72] = original[5:7] + original[9:11]
    return bytes(raw)


def sample(kind=0, *, counter=11, phase=6, x=0x12345678, y=-0x12345678):
    raw = bytearray(start(kind, x=x, y=y, phase=phase))
    raw[0x6d] = counter
    struct.pack_into('<12H', raw, 0x6e, *[index * 5001 for index in range(12)])
    return bytes(raw)


class HistoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-history-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_vehicle_motion_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_vehicle_motion_probe.js')])
        cls.cases = cls.boundaries = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Position history per target: {cls.cases} fixtures, {cls.boundaries} complete boundaries', flush=True)

    def run_data(self, data, wanted='', valid=True):
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, 'history', str(path)], capture_output=True,
                                    text=True, timeout=60)
            self.assertEqual((result.returncode, result.stdout), (int(not valid), wanted), result.stderr)

    def histories(self, cases):
        observed = []
        for raw, steps in cases:
            for _ in range(steps):
                raw = update(raw)
                observed.append(raw)
        if ORACLE is not None:
            self.assertEqual(ORACLE.histories(cases), observed, 'Complete original callback state differs')
        expected = state_lines([(0, 0, raw) for raw in observed])
        data = struct.pack('<I', len(cases)) + b''.join(raw + struct.pack('<H', steps) for raw, steps in cases)
        self.run_data(data, expected)
        self.__class__.cases += len(cases)
        self.__class__.boundaries += len(observed)

    def test_every_counter_and_class_preserves_complete_state(self):
        self.histories([(sample(kind, counter=counter), 1)
                        for kind in range(4) for counter in range(256)])

    def test_every_phase_byte_and_class_uses_actual_dispatch(self):
        self.histories([(sample(kind, phase=phase), 1)
                        for kind in range(4) for phase in range(256)])

    def test_coordinate_lanes_shift_order_and_multiple_windows(self):
        positions = (0, 1, 255, 256, 65535, 65536, 0xffffff, 0x1000000,
                     2147483647, -2147483648, -1, -255, -256, -257)
        self.histories([(sample(kind, x=x, y=y), 73)
                        for kind in range(4) for x, y in zip(positions, reversed(positions))])
        self.histories([(sample(kind, counter=255), 145) for kind in range(4)])

    def test_invalid_preflight_produces_no_partial_output(self):
        good = sample()
        bad_type = bytearray(good)
        struct.pack_into('<H', bad_type, 0, 4)
        self.run_data(struct.pack('<I', 0))
        for data in (b'', b'\0' * 3, struct.pack('<I', 1),
                     struct.pack('<I', 1) + good + b'\0\0',
                     struct.pack('<I', 1) + good + b'\1\0x',
                     struct.pack('<I', 2) + good + b'\1\0' + bad_type + b'\1\0'):
            self.run_data(data, valid=False)

    def test_complete_original_ground_corpus_and_reaching_player_prefix(self):
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
                if int.from_bytes(raw[:2], 'little') >= 4:
                    continue
                # First preserve the actual saved phase/history; then make the
                # callback reachable without rewriting its counter/history/XY.
                active = bytearray(raw)
                active[0x3d] = (active[0x3d] & 0xe1) | 6
                cases.extend(((raw, 1), (bytes(active), 13)))
                count += 1
            self.histories(cases)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info['sha256'])
        self.assertEqual(count, 960)
        if ORACLE is not None:
            self.reaching_prefix()

    def reaching_prefix(self):
        from unicorn import UC_HOOK_CODE
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SP
        from original_driver_oracle import OriginalDriverOracle
        from original_mission_world_oracle import OriginalMissionWorldOracle
        from original_unit_oracle import DGROUP
        from test_driving import driver_step

        records = records_from_scenario((ROOT / 'armoredfist/FISTDATA/TRAIN1.FSG').read_bytes())
        # Reaching original class prefix, explicitly stopping before the engine
        # audio device call at 7c7e. No PM driver callback may return through the
        # reserved address-0/1 harness instructions and masquerade as completion.
        for counter in (None, 11):
            owner = OriginalMissionWorldOracle()
            machine, objects = owner.prepare(records, (0, 0, 0, 0), 0, 0)
            pointer = int.from_bytes(machine.mem_read(DGROUP + 0x6d3c, 2), 'little')
            selected = owner.slot(pointer)
            self.assertEqual((len(objects), selected), (85, 151))
            allocation, _, size = objects[selected]
            driver = OriginalDriverOracle(512, bytes([32]) * (512 * 512))
            machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', pointer))
            machine.mem_write(DGROUP + 0x6da2, bytes(2))  # Explicit original voice mute gate.
            machine.mem_write(DGROUP + 0x8b43, bytes(2))  # Explicit no-joystick device selection.
            machine.reg_write(UC_X86_REG_DI, pointer)
            driver.take_control(machine)
            if counter is not None:
                machine.mem_write(DGROUP + pointer + 0x6d, bytes([counter]))
            current = allocation[2], allocation[3], bytes(machine.mem_read(DGROUP + pointer, size))
            current = driver.ground.contact(driver.field, [current])[0]
            machine.mem_write(DGROUP + pointer, current[2])
            unrelated = [(p, bytes(machine.mem_read(DGROUP + p, n)))
                         for slot, (_, p, n) in objects.items() if slot != selected]
            random_before = owner.random_state(machine)
            reached = []
            def observe(uc, address, length, context):
                if address in (0, 1, 0xaa37, 0x1a6c8):
                    reached.append(address)
                if address in (0, 1):
                    uc.emu_stop()
            handle = machine.hook_add(UC_HOOK_CODE, observe)
            try:
                for tick in range(1, 4):
                    machine.reg_write(UC_X86_REG_DI, pointer)
                    machine.reg_write(UC_X86_REG_SP, 0x9000)
                    machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
                    owner.execute(machine, 0x7c1d, 0x7c7e, 0)
                    self.assertEqual(machine.reg_read(UC_X86_REG_SP), 0x9000)
                    actual = allocation[2], allocation[3], bytes(machine.mem_read(DGROUP + pointer, size))
                    actual = driver.ground.contact(driver.field, [actual])[0]
                    expected, _ = driver_step(current, 0, 512, bytes([32]) * (512 * 512))
                    self.assertEqual(actual, expected, f'Reaching complete TRAIN1 prefix tick {tick}')
                    if tick == 3:
                        self.assertEqual((actual[2][0x3d], actual[2][0x6d]), (38, 0))
                        if counter is None:
                            self.assertEqual(current[2][0x6d], 255)
                        else:
                            self.assertEqual(actual[2][0x6e:0x72], current[2][5:7] + current[2][9:11])
                    current = actual
                    machine.mem_write(DGROUP + pointer, actual[2])
                self.assertEqual(reached, [0xaa37, 0x1a6c8])
                self.assertEqual(owner.random_state(machine), random_before)
                for address, before in unrelated:
                    self.assertEqual(bytes(machine.mem_read(DGROUP + address, len(before))), before)
            finally:
                machine.hook_del(handle)


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
        from original_vehicle_history_oracle import OriginalVehicleHistoryOracle
        ORACLE = OriginalVehicleHistoryOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
