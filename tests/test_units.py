#!/usr/bin/env python3
"""Typed mission identities, owned snapshots and normal-side roster on both C targets."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_scenario import envelope, synthetic_chunks

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
ABSENT = 65535


def snapshot(kind=0, *, pool=99, pose=(-2147483648, 2147483647, -65537), heading=65535,
             flags=32, platoon=0, member=0, size=None):
    length = size if size is not None else (251 if kind in (0, 1, 2, 3, 19) else 55)
    data = bytearray((offset * 73 + 19) % 256 for offset in range(length))
    struct.pack_into('<HHiiiH', data, 0, kind, pool, *pose, heading)
    data[22] = flags
    data[23] = 138
    data[27:30] = bytes((platoon, member, 237))
    data[35:37] = bytes((platoon, member))
    return bytes(data)


def scenario_data(records, *, orders=None):
    chunks = synthetic_chunks()
    chunks[1] = (b'DCBS', struct.pack('<H', len(records)) + b''.join(
        struct.pack('<3H', len(state), index, generation) + state
        for index, generation, state in records))
    if orders is not None:
        chunks[2], chunks[4] = (b'PATH', orders[0]), (b'PINF', orders[1])
    return envelope(chunks)


def records_from_scenario(data):
    offset = 0
    units = None
    while offset < len(data):
        tag, length = struct.unpack_from('<4sH', data, offset)
        if tag == b'DCBS':
            units = data[offset + 6:offset + 6 + length]
        offset += 6 + length
    assert units is not None and offset == len(data)
    count, = struct.unpack_from('<H', units)
    records = []
    offset = 2
    for _ in range(count):
        length, index, generation = struct.unpack_from('<3H', units, offset)
        records.append((index, generation, units[offset + 6:offset + 6 + length]))
        offset += 6 + length
    assert offset == len(units)
    return records


def expected(records):
    registry = [ABSENT] * 182
    roster = [ABSENT] * 32
    lines = [f'units {len(records)}']
    fields = []
    for ordinal, (index, generation, state) in enumerate(records):
        kind, pool, map_x, map_y, altitude, heading = struct.unpack_from('<HHiiiH', state)
        platoon, member = state[35:37] if kind == 23 else state[27:29]
        flags, secondary_flags = state[22], state[23]
        numbers = [ordinal, kind, index, generation, pool, map_x, map_y, altitude, heading,
                   flags, secondary_flags, platoon, member, len(state)]
        lines.append('unit ' + ' '.join(map(str, numbers)) + ' ' + state.hex(' '))
        fields.append((kind, pool, (map_x, map_y, altitude, heading), flags, secondary_flags,
                       len(state)))
        registry[index] = ordinal
        if kind == 23 or flags & 32:
            slot = platoon * 4 + member
            if kind != 23 or slot:
                roster[slot] = ordinal
    lines += [f'registry {index} {ordinal}' for index, ordinal in enumerate(registry)]
    lines += [f'roster {slot} {ordinal}' for slot, ordinal in enumerate(roster)]
    return '\n'.join(lines) + '\n', fields, registry, roster


class UnitTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-units-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_unit_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_unit_probe.js')])

    def run_records(self, records, valid=True):
        return self.run_data(scenario_data(records), valid)

    def run_data(self, data, valid=True):
        filename = pathlib.Path(self.temp.name) / 'fixture.fsg'
        filename.write_bytes(data)
        transcript = None
        if valid:
            records = records_from_scenario(data)
            transcript, fields, registry, roster = expected(records)
            if ORACLE is not None:
                observed, actual_registry, actual_roster = ORACLE.definitions(records)
                self.assertEqual(observed, fields)
                self.assertEqual(actual_roster, roster)
                expected_registry = [(ordinal, records[ordinal][1]) if ordinal != ABSENT
                                     else (ABSENT, 0) for ordinal in registry]
                self.assertEqual(actual_registry, expected_registry)
        for command in self.commands:
            with self.subTest(target=command[0]):
                result = subprocess.run([*command, str(filename)], capture_output=True,
                                        text=True, timeout=15)
                # 2 means the probe detected a failed ownership/API invariant.
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, transcript if valid else '')
        return transcript

    def test_all_type_sizes_pose_widths_and_complete_owned_bytes(self):
        records = [(kind, 65535 - kind, snapshot(kind, flags=0, pool=65535 - kind))
                   for kind in range(28)]
        self.run_records(records)
        for pose in ((0, 0, 0), (-1, 1, -1), (2147483647, -2147483648, 2147483647)):
            with self.subTest(pose=pose):
                self.run_records([(181, 0, snapshot(pose=pose, heading=0))])

    def test_all_roster_slots_and_generation_are_independent_of_saved_pool(self):
        records = [(181 - slot, 1000 + slot, snapshot(kind=slot % 4, pool=60000 + slot,
                                                    platoon=slot // 4, member=slot % 4))
                   for slot in range(32)]
        transcript = self.run_records(records)
        self.assertIn('roster 0 0\n', transcript)
        self.assertIn('roster 31 31\n', transcript)

    def test_placeholders_use_alternate_fields_and_exclude_slot_zero(self):
        alternate = bytearray(snapshot(23, flags=0, platoon=7, member=3))
        alternate[27:29] = b'\xff\xff'  # Unused regular-vehicle fields.
        records = [(9, 8, snapshot(platoon=0, member=0)), (12, 7, bytes(alternate)),
                   (13, 6, snapshot(23, flags=255, platoon=0, member=0))]
        transcript = self.run_records(records)
        self.assertIn('roster 0 0\n', transcript)
        self.assertIn('roster 31 1\n', transcript)

    def test_nonparticipants_and_empty_scenarios_have_no_fabricated_player(self):
        self.run_records([])
        records = [(32, 99, snapshot(26, flags=0, platoon=255, member=255)),
                   (0, 1, snapshot(23, flags=0, platoon=0, member=0))]
        transcript = self.run_records(records)
        self.assertIn('roster 0 65535\n', transcript)

    def test_duplicate_registry_and_roster_assignments_are_last_record_wins(self):
        records = [(7, 1, snapshot()), (7, 65535, snapshot(1, pool=60000)),
                   (9, 5, snapshot(23, platoon=0, member=1)),
                   (10, 6, snapshot(2, platoon=0, member=1))]
        transcript = self.run_records(records)
        self.assertIn('registry 7 1\n', transcript)
        self.assertIn('roster 0 1\n', transcript)
        self.assertIn('roster 1 3\n', transcript)

    def test_invalid_types_sizes_registry_and_participating_roster_fail_atomically(self):
        for records in (
            [(0, 1, snapshot(28))], [(0, 1, snapshot(65535))],
            [(0, 1, snapshot(0, size=55))], [(0, 1, snapshot(19, size=55))],
            [(0, 1, snapshot(26, size=251))], [(0, 1, snapshot()[:-1])],
            [(0, 1, snapshot() + b'x')], [(182, 1, snapshot())],
            [(65535, 1, snapshot())], [(0, 1, snapshot(platoon=8))],
            [(0, 1, snapshot(member=4))], [(0, 1, snapshot(23, flags=0, member=4))],
            [(1, 2, snapshot()), (2, 3, snapshot(platoon=255))],
        ):
            with self.subTest(records=records):
                self.run_records(records, valid=False)
        data = scenario_data([(0, 1, snapshot())])
        for malformed in (data[:-1], data + b'trailing'):
            self.run_data(malformed, valid=False)

    def test_all_pinned_original_scenarios_and_player_roster(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned original coverage requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([file.name for file in files], sorted(manifest))
        count = 0
        for file in files:
            with self.subTest(file=file.name):
                data = file.read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[file.name]['sha256'])
                self.assertEqual(len(data), manifest[file.name]['size'])
                transcript = self.run_data(data)
                records = records_from_scenario(data)
                count += len(records)
                _, _, _, roster = expected(records)
                self.assertNotEqual(roster[0], ABSENT)
                kind, = struct.unpack_from('<H', records[roster[0]][2])
                self.assertEqual(kind, 1 if file.name == 'INDIA3.FSG' else 0)
                if file.name == 'TRAIN1.FSG':
                    self.assertEqual(records[roster[0]][0], 32)
                    self.assertIn('-1061797 1816527 1280 51700', transcript)
                self.assertEqual(hashlib.sha256(file.read_bytes()).hexdigest(),
                                 manifest[file.name]['sha256'])
        self.assertEqual(count, 4213)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true', help='Require pinned original instructions')
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, ORIGINALS = args.native_probe, args.originals
    if args.oracle:
        from original_unit_oracle import OriginalUnitOracle
        ORACLE = OriginalUnitOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
