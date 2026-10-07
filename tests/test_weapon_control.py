#!/usr/bin/env python3
"""Shared original station selection, gun/recoil pose and phase-driven reload."""
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
from test_vehicle_start import COMPONENT_OFFSET, COMPONENTS, initialized, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
COUNTS = (4, 4, 5, 4)
CONTINUOUS = (4, 6, 6, 6)
TIMES = ((20, 20, 0, 20), (2, 2, 2, 0), (20, 20, 40, 0, 8), (2, 2, 2, 0))
LOAD_CUES = ((0, 1, 255, 2), (5, 7, 6, 255), (2, 0, 1, 255, 255), (6, 7, 5, 255))
READY_CUES = ((9, 10, 255, 11), (), (11, 9, 10, 255, 255), ())
DIRTY = ((0xdb, 0xdc, 0xde, 0xe0, 0xe2), (0xcd, 0xce, 0xd0, 0xd2, 0xd4),
         (0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xcf), (0xd8, 0xda, 0xdc, 0xde, 0xd7))
RELOAD_DIRTY = (0xdb, 0xcd, 0xcf, 0xd7)
STOCK_STATION = (255, 0, 255, 4)


def weapon_start(kind=0, *, selected=0, loaded=0, countdown=0, stock=None, rounds=None,
                 phase=0, recoil=0, elevation=0, trigger=0):
    raw = bytearray(start(kind, phase=phase))
    raw[0x91] = selected
    raw[0xa5] = loaded
    raw[0xa8] = countdown
    raw[0x3c] = recoil
    raw[0x92] = trigger
    struct.pack_into('<H', raw, 0x38, elevation % 65536)
    if stock is not None:
        raw[0xbb] = stock
    if rounds is not None:
        struct.pack_into('<' + 'H' * len(rounds), raw, 0xac if kind == 2 else 0xad, *rounds)
    # Nonzero original component markers expose accidental clobbering/no-op writes.
    offset = COMPONENT_OFFSET[kind]
    for index in range(len(COMPONENTS[kind])):
        raw[offset + index] = (index * 7 + 19) % 256
    return bytes(raw)


def update(original, operation, argument):
    raw = bytearray(original)
    kind, = struct.unpack_from('<H', raw)
    events = [0, 0, 255, 255, 0]
    if operation == 1:
        advanced = (raw[0x91] + 2) % 256
        argument = advanced if advanced <= 6 else 0
        operation = 0
    if operation == 0 and argument != raw[0x91]:
        raw[0x91] = argument
        events[0] = 1
        if argument != CONTINUOUS[kind] and argument != raw[0xa5]:
            raw[0xa5] = argument
            slot = argument // 2
            raw[0xa8] = TIMES[kind][slot]
            events[2] = LOAD_CUES[kind][slot]
            rounds, = struct.unpack_from('<H', raw, (0xac if kind == 2 else 0xad) + argument)
            if rounds == 0:
                if kind == 0:
                    raw[0xa8] = 255
                    events[2] = 13
                elif argument == STOCK_STATION[kind]:
                    if raw[0xbb] == 0:
                        events[2] = 13
                    else:
                        events[3:5] = [25, 90]
        for offset in DIRTY[kind]:
            raw[offset] = 3
    if operation == 2:
        raw[0x92] = 48
    if operation in (3, 5):
        raw[0xa7] = raw[0x39]
        if raw[0x3c]:
            raw[0x3c] -= 1
    if operation == 5:
        raw[0x3d] = (raw[0x3d] + 2) % 256
    if operation in (4, 5) and raw[0x3d] & 0x1e == 0 and raw[0xa8]:
        raw[0xa8] -= 1
        if raw[0xa8] == 0:
            events[1] = 1
            if kind in (0, 2):
                rounds, = struct.unpack_from('<H', raw, (0xad if kind == 0 else 0xac) + raw[0x91])
                if rounds:
                    raw[RELOAD_DIRTY[kind]] = 3
                    events[2] = READY_CUES[kind][raw[0x91] // 2]
            else:
                raw[RELOAD_DIRTY[kind]] = 3
    return bytes(raw), tuple(events)


class WeaponTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-weapons-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        if ORACLE is not None and ORACLE.tables() != list(zip(TIMES, LOAD_CUES, READY_CUES)):
            raise AssertionError('Weapon/reload request tables differ from the pinned original')
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_weapon_control_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_weapon_control_probe.js')])

    def run_probe(self, data, expected, valid=True):
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(data)
        for command in self.commands:
            with self.subTest(target=command[0]):
                result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, expected)

    def transitions(self, cases):
        expected = []
        for raw, steps, operation, argument in cases:
            for _ in range(steps):
                raw, events = update(raw, operation, argument)
                expected.append((raw, events))
        if ORACLE is not None:
            observed = ORACLE.transitions(cases)
            self.assertEqual(len(observed), len(expected))
            for (raw, hud, notice), (wanted, events) in zip(observed, expected):
                self.assertEqual(raw, wanted)  # Complete 251-byte write footprint, not a filter.
                self.assertEqual(hud, bytes([3, 0, 0, 0] * 4) if events[0] else bytes(16))
                self.assertEqual(notice, (events[3], events[4]))
        text = ''.join(state_lines([(0, 0, raw)]) + 'weapon_events ' +
                       ' '.join(map(str, events)) + '\n' for raw, events in expected)
        data = struct.pack('<I', len(cases)) + b''.join(raw + struct.pack('<HBB', steps, operation, arg)
                                                      for raw, steps, operation, arg in cases)
        self.run_probe(data, text)
        return expected

    def test_all_class_station_selected_loaded_empty_and_stock_boundaries(self):
        for kind in range(4):
            self.transitions([(weapon_start(kind, selected=selected, loaded=loaded,
                                            countdown=countdown, stock=stock, rounds=rounds),
                               2, 0, station)
                              for station in range(0, COUNTS[kind] * 2, 2)
                              for selected in range(0, COUNTS[kind] * 2, 2)
                              for loaded in range(0, COUNTS[kind] * 2, 2)
                              for countdown in (0, 1, 255) for stock in (0, 1, 255)
                              for rounds in ([0] * COUNTS[kind], [1] * COUNTS[kind])])

    def test_cycle_order_wrap_and_fifth_t80_station(self):
        for kind in range(4):
            self.transitions([(weapon_start(kind, selected=selected, loaded=selected), 10, 1, 0)
                              for selected in range(0, COUNTS[kind] * 2, 2)])
        results = self.transitions([(weapon_start(2, selected=6), 1, 0, 8)])
        self.assertEqual((results[0][0][0x91], results[0][0][0xa8]), (8, 8))

    def test_every_reload_counter_and_phase_for_each_class(self):
        for kind in range(4):
            self.transitions([(weapon_start(kind, countdown=countdown, phase=phase), 1, 4, 0)
                              for countdown in range(256) for phase in range(256)])
            self.transitions([(weapon_start(kind, selected=station, countdown=1, rounds=rounds),
                               1, 4, 0)
                              for station in range(0, COUNTS[kind] * 2, 2)
                              for rounds in ([0] * COUNTS[kind], [1] * COUNTS[kind])])

    def test_timed_reload_recoil_pose_and_trigger_preservation(self):
        for kind in range(4):
            values = self.transitions([(weapon_start(kind, selected=station, countdown=20,
                                                      phase=phase, recoil=16, trigger=48,
                                                      elevation=-1), 321, 5, 0)
                                       for phase in (0, 1, 30, 31, 254, 255)
                                       for station in range(0, COUNTS[kind] * 2, 2)])
            for ordinal in range(len(values) // 321):
                block = values[ordinal * 321:(ordinal + 1) * 321]
                self.assertEqual(sum(events[1] for _, events in block), 1)
                self.assertEqual((block[-1][0][0x3c], block[-1][0][0x92], block[-1][0][0xa7]),
                                 (0, 48, 255))

    def test_every_signed_elevation_and_recoil_byte(self):
        for kind in range(4):
            self.transitions([(weapon_start(kind, elevation=elevation, recoil=elevation % 256),
                               1, 3, 0) for elevation in range(-32768, 32768)])

    def test_fire_request_replaces_every_pending_byte_without_consuming_ammunition(self):
        self.transitions([(weapon_start(kind, trigger=trigger, countdown=255,
                                        rounds=[0] * COUNTS[kind]), 2, 2, 0)
                          for kind in range(4) for trigger in range(256)])

    def test_invalid_truncated_missing_and_empty_requests(self):
        record = weapon_start() + struct.pack('<HBB', 1, 0, 2)
        valid = struct.pack('<I', 1) + record
        for data in (b'', valid[:-1], valid + b'x', struct.pack('<I', 2) + record,
                     struct.pack('<I', 0xffffffff) + record, valid[:-4] + struct.pack('<HBB', 0, 0, 2),
                     valid[:-4] + struct.pack('<HBB', 1, 6, 0),
                     valid[:-4] + struct.pack('<HBB', 1, 1, 2),
                     struct.pack('<I', 2) + record + record[:-1] + b'\x01'):
            self.run_probe(data, '', valid=False)
        # Every invalid station byte on every class is checked inside the probe
        # with complete state/event preservation. Sample CLI failures separately.
        for station in (1, 3, 5, 7, 8, 9, 254, 255):
            self.run_probe(valid[:-1] + bytes([station]), '', valid=False)
        for kind in range(4):
            if kind != 2:
                self.run_probe(struct.pack('<I', 1) + weapon_start(kind) +
                               struct.pack('<HBB', 1, 0, 8), '', valid=False)
        self.run_probe(struct.pack('<I', 0), '')
        missing = pathlib.Path(self.temp.name) / 'missing'
        for command in self.commands:
            result = subprocess.run([*command, str(missing)], capture_output=True, text=True, timeout=30)
            self.assertEqual((result.returncode, result.stdout), (1, ''))

    def test_complete_original_ground_corpus(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([path.name for path in files], sorted(manifest))
        count = 0
        for path in files:
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
            states, _ = initialized(records_from_scenario(data), (0, 0, 0, 0), 0, 0)
            count += len(states)
            cases = []
            for _, _, raw in states:
                kind, = struct.unpack_from('<H', raw)
                for station in range(0, COUNTS[kind] * 2, 2):
                    cases.append((raw, 1, 0, station))
                cases.extend(((raw, 1, 2, 0), (raw, 32, 5, 0)))
            self.transitions(cases)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), manifest[path.name]['sha256'])
        self.assertEqual(count, 960)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, ORIGINALS = args.native_probe, args.originals
    if args.oracle:
        from original_weapon_control_oracle import OriginalWeaponControlOracle
        ORACLE = OriginalWeaponControlOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
