#!/usr/bin/env python3
"""Owned driving sessions, full timed input/state traces and original stage returns."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from prepare_terrain_preview import synthetic_inputs
from test_terrain_scene import fixture_models
from test_units import expected, records_from_scenario, scenario_data
from test_vehicle_start import initialized, state_lines
from test_vehicle_motion import start, update
from test_vehicle_history import update as history_update
from test_vehicle_maintenance import update as maintenance_update
from test_ground import installed
from test_heightfield import resize
from test_weapon_control import update as weapon_update, COUNTS, CONTINUOUS, STOCK_STATION

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = False
PIT_RATE = 1193182
TICK_PHASE = 19886 * 1000000
KEY_MASK = 16383


def take_control(original):
    raw = bytearray(original)
    control, = struct.unpack_from('<H', raw, 0x40)
    if control & 1:
        struct.pack_into('<H', raw, 0x40, control & 65534)
        kind, = struct.unpack_from('<H', raw)
        raw[(0xbf, 0xbc, 0xbe, 0xbc)[kind] + (8, 8, 40, 27)[kind]] = 3
    return bytes(raw)


def driver_step(record, keys, side, pixels):
    identity, generation, original = record
    raw = bytearray(original)
    throttle = bool(keys & 1) - bool(keys & 2)
    steer = bool(keys & 8) - bool(keys & 4)
    turret = bool(keys & 32) - bool(keys & 16)
    if throttle or steer or turret:
        raw = bytearray(take_control(raw))
    value, = struct.unpack_from('<h', raw, 0x57)
    if throttle > 0 and value < 254:
        value += 1
    elif throttle < 0 and value > -254:
        value -= 1
    struct.pack_into('<h', raw, 0x57, 0 if keys & 64 else value)
    for offset, delta in ((0x30, steer * 182), (0x8b, turret * 364)):
        value, = struct.unpack_from('<H', raw, offset)
        struct.pack_into('<H', raw, offset, (value + delta) % 65536)
    raw[0x0d] = raw[0x1d]
    raw, _ = weapon_update(raw, 3, 0)
    raw, _ = update(raw)
    raw = bytearray(raw)
    raw[0x3d] = (raw[0x3d] + 2) % 256
    raw, events = weapon_update(raw, 4, 0)
    raw = maintenance_update(history_update(raw))
    return installed(side, pixels, [(identity, generation, bytes(raw))])[0], events


def trace(record, intervals, side, pixels, *, extra_state=None):
    ticks = phase = keys = paused = 0
    feedback = [0, 0, 0, 255, 255, 0]
    oracle = None
    if ORACLE:
        from original_driver_oracle import OriginalDriverOracle
        oracle = OriginalDriverOracle(side, pixels)
    def state():
        raw = record[2]
        kind, = struct.unpack_from('<H', raw)
        ammo, = struct.unpack_from('<H', raw, (0xac if kind == 2 else 0xad) + raw[0x91])
        has_reserve = int(STOCK_STATION[kind] != 255)
        status = (ammo, COUNTS[kind], raw[0x91], raw[0xa8], int(raw[0x91] == CONTINUOUS[kind]),
                  raw[0xbb] if has_reserve else 0, has_reserve)
        return (f'clock {ticks} {phase} {keys} {paused}\n' + state_lines([record]) +
                'feedback ' + ' '.join(map(str, feedback)) + '\n' +
                'weapon_status ' + ' '.join(map(str, status)) + '\n' +
                (extra_state(record) if extra_state else ''))
    def notify(events):
        feedback[0] += events[0]
        feedback[1] += events[1]
        if events[2] != 255:
            feedback[2] += 1
            feedback[3] = events[2]
        if events[3] != 255:
            feedback[4], feedback[5] = events[3], ticks + events[4]
        if ticks >= feedback[5]:
            feedback[4] = 255
    output = state()
    for elapsed, following_keys in intervals:
        if following_keys > KEY_MASK:
            output += 'advance -1\n' + state()
            continue
        if not paused:
            count, phase = divmod(phase + elapsed * PIT_RATE, TICK_PHASE)
            for _ in range(count):
                next_record, events = driver_step(record, keys, side, pixels)
                if oracle:
                    observed = oracle.step(record, keys)
                    if observed != next_record:
                        raise AssertionError(f'Original stage difference at tick {ticks}:\n'
                                             + state_lines([observed, next_record]))
                record = next_record
                ticks += 1
                notify(events)
        if following_keys & 128 and not keys & 128:
            paused ^= 1
        if not paused:
            identity, generation, raw = record
            kind, = struct.unpack_from('<H', raw)
            edges = following_keys & ~keys
            operations = [(0, slot * 2) for slot in range(COUNTS[kind]) if edges & (256 << slot)]
            if edges & 8192:
                operations.append((1, 0))
            for operation, argument in operations:
                updated, events = weapon_update(raw, operation, argument)
                updated = take_control(updated)
                if oracle:
                    observed = oracle.select((identity, generation, raw), operation, argument)
                    if observed != (identity, generation, updated):
                        raise AssertionError('Original player weapon input/control-refresh difference')
                raw = updated
                notify(events)
            record = identity, generation, raw
        keys = following_keys
        output += 'advance 0\n' + state()
    return output


class DrivingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-driving-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = pathlib.Path(cls.temp.name)
        synthetic_inputs(cls.directory, flat=True)
        fixture_models(cls.directory)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_driving_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_driving_probe.js')])

    def scenario(self, kind=0, *, selected=0, loaded=0, recoil=0, **fields):
        raw = bytearray(start(kind, **fields))
        raw[0x91], raw[0xa5], raw[0x3c] = selected, loaded, recoil
        raw[22] |= 32
        raw[0xa9:0xac] = b'\x80\0\0'
        path = self.directory / 'PLAYER.FSG'
        path.write_bytes(scenario_data([(9, 12, bytes(raw))]))
        return path

    def run_scene(self, intervals, *, scenario=None, directory=None, side=512, pixels=None):
        scenario = scenario or self.scenario()
        directory = directory or self.directory
        records = records_from_scenario(scenario.read_bytes())
        _, _, _, roster = expected(records)
        record = records[roster[0]]
        states, _ = initialized([record], (0, 0, 0, 0), 0, 0)
        pixels = pixels if pixels is not None else bytes([32]) * side * side
        # This session selects the delivered manual turret stage. Typed actor
        # state deliberately has no recovered targeting contract yet; supply
        # that explicit untargeted boundary to the original stage oracle too.
        identity, generation, raw = states[0]
        raw = bytearray(raw)
        struct.pack_into('<H', raw, 0x97, 0)
        state = installed(side, pixels, [(identity, generation, bytes(raw))])[0]
        expected_output = trace(state, intervals, side, pixels)
        request = self.directory / 'intervals.bin'
        request.write_bytes(struct.pack('<II', side, len(intervals)) + b''.join(
            struct.pack('<IH', *interval) for interval in intervals))
        results = []
        for command in self.commands:
            result = subprocess.run([*command, str(scenario), str(directory), str(request)],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, expected_output)
            results.append(result.stdout)
        return results

    def test_timed_actions_all_classes_and_complete_state(self):
        intervals = [(0, 1), (2200000, 9), (1000000, 33), (1000000, 18),
                     (1600000, 66), (300000, 2), (900000, 0), (1000000, 0)]
        for kind in range(4):
            with self.subTest(kind=kind):
                self.run_scene(intervals, scenario=self.scenario(kind, x=2147483637, y=-2147483640))

    def test_clock_partition_and_input_boundary_independent_of_frame_rate(self):
        for keys in (1, 9, 33, 18, 0):
            coarse = self.run_scene([(0, keys), (1234567, keys)])
            fine = self.run_scene([(0, keys)] + [(12345, keys)] * 100 + [(67, keys)])
            for a, b in zip(coarse, fine):
                self.assertEqual(a.split('advance 0\n')[-1], b.split('advance 0\n')[-1])
        # A press at the end cannot influence the interval preceding it.
        self.run_scene([(16665, 1), (1, 0), (16666, 9), (0, 33), (50001, 0)])

    def test_pause_rising_edges_repeats_releases_and_no_catchup(self):
        intervals = [(0, 1), (33334, 129), (1000000, 129), (2000000, 1),
                     (0xffffffff, 129), (33334, 129), (0, 0), (33334, 0)]
        result = self.run_scene(intervals)[0]
        rows = [line for line in result.splitlines() if line.startswith('clock ')]
        self.assertEqual([line.split()[1:3] for line in rows[2:5]], [rows[2].split()[1:3]] * 3)

    def test_conflicting_inputs_throttle_limits_out_of_range_and_altitude_lanes(self):
        self.run_scene([(0, 63), (100000, 0), (0, 1), (100000, 2), (100000, 64), (33334, 0)],
                       scenario=self.scenario(throttle=254))
        for throttle in (-32768, -255, -254, 254, 255, 32767):
            self.run_scene([(0, 1), (33334, 2), (33334, 0)],
                           scenario=self.scenario(throttle=throttle))
        raw = bytearray(start()); raw[22] |= 32
        raw[0x91] = raw[0xa5] = raw[0x3c] = 0
        struct.pack_into('<I', raw, 0x0c, 0xdeadbeef)
        path = self.directory / 'ALT.FSG'
        path.write_bytes(scenario_data([(7, 3, bytes(raw))]))
        output = self.run_scene([(16667, 0)], scenario=path)[0]
        altitude = int(output.split('advance 0\n')[1].splitlines()[1].split()[6])
        self.assertEqual(altitude & 0xffffffff, 0xdead20ef)

    def test_invalid_key_mask_preserves_complete_clock_player_and_controls(self):
        self.run_scene([(0, 1), (50000, 16384), (50000, 32768), (50000, 0)])

    def test_weapon_edges_repeats_pause_and_class_capabilities(self):
        intervals = [(0, 512), (0, 512), (100000, 512), (0, 0), (0, 8192),
                     (100000, 8192), (0, 4096), (100000, 0), (0, 128), (1000000, 256),
                     (0, 384), (0, 0), (0, 256), (0, 16128), (6000000, 0)]
        for kind in range(4):
            self.run_scene(intervals, scenario=self.scenario(kind, phase=254, recoil=255))

    def test_weapon_reload_cadence_and_clock_partition(self):
        for kind in range(4):
            scenario = self.scenario(kind)
            coarse = self.run_scene([(0, 512), (5500000, 512)], scenario=scenario)
            fine = self.run_scene([(0, 512)] + [(55000, 512)] * 100, scenario=scenario)
            for left, right in zip(coarse, fine):
                self.assertEqual(left.split('advance 0\n')[-1], right.split('advance 0\n')[-1])
            feedback = [line.split() for line in coarse[0].splitlines() if line.startswith('feedback ')]
            self.assertEqual(feedback[-1][1:3], ['1', '1'])

    def test_missing_assets_player_and_invalid_installed_detail_fail_without_output(self):
        scenario = self.scenario()
        request = self.directory / 'bad.bin'
        for side in (0, 3, 768, 0xffffffff):
            request.write_bytes(struct.pack('<II', side, 0))
            for command in self.commands:
                result = subprocess.run([*command, str(scenario), str(self.directory), str(request)],
                                        capture_output=True, timeout=20)
                self.assertEqual((result.returncode, result.stdout), (1, b''))
        request.write_bytes(struct.pack('<II', 512, 0))
        # A malformed selected station cannot silently become a usable HUD or
        # acquire a default weapon. Only T80 accepts the original fifth code.
        for kind in range(4):
            for selected in (1, 10, 255, *((8,) if kind != 2 else ())):
                invalid = self.scenario(kind, selected=selected)
                for command in self.commands:
                    result = subprocess.run([*command, str(invalid), str(self.directory), str(request)],
                                            capture_output=True, timeout=20)
                    self.assertEqual((result.returncode, result.stdout), (1, b''))
        scenario = self.scenario()
        missing = self.directory / 'M1_C.M32'; saved = missing.read_bytes(); missing.unlink()
        try:
            for command in self.commands:
                result = subprocess.run([*command, str(scenario), str(self.directory), str(request)],
                                        capture_output=True, timeout=20)
                self.assertEqual((result.returncode, result.stdout), (1, b''))
        finally:
            missing.write_bytes(saved)
        empty = self.directory / 'EMPTY.FSG'; empty.write_bytes(scenario_data([]))
        for command in self.commands:
            result = subprocess.run([*command, str(empty), str(self.directory), str(request)],
                                    capture_output=True, timeout=20)
            self.assertEqual((result.returncode, result.stdout), (1, b''))

    def test_all_original_players_installed_fields_and_timed_controls(self):
        if not ORIGINALS:
            self.skipTest('Full original corpus is an explicit additional gate')
        from original_asset_oracle import OriginalAssetOracle
        decoder = OriginalAssetOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        maps = {}
        for name, info in manifest.items():
            scenario = ROOT / 'armoredfist/FISTDATA' / name
            data = scenario.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            probe = subprocess.run([str(BUILD / 'native/fist_scenario_probe'), str(scenario)],
                                   check=True, capture_output=True, text=True)
            height = next(line.split(' ', 2)[2] for line in probe.stdout.splitlines()
                          if line.startswith('asset ') and line.endswith('.KLC') and ' D' in line)
            if height not in maps:
                decoded = decoder.klc((scenario.parent / height).read_bytes())
                dimensions, body = decoded.split(b'\n', 1)
                width, _ = map(int, dimensions.split())
                maps[height] = resize(width, body[768:], [2048])[1]
            self.run_scene([(0, 521), (166670, 33), (166670, 8210), (166670, 64), (166670, 0)],
                           scenario=scenario, directory=scenario.parent, side=2048, pixels=maps[height])
        print(f'Original driving: {len(manifest)} player starts, {len(maps)} installed maps, '
              f'full timed state traces on {len(self.commands)} targets', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS, ORACLE = args.build_root, args.target, args.native_probe, args.originals, args.oracle
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS))
