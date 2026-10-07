#!/usr/bin/env python3
"""Ground-class start state, owned component data and original four-stream RNG."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_units import records_from_scenario, scenario_data, snapshot

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
SEEDS = (1, 2, 32768, 65535)
COMPONENTS = [bytes.fromhex(value) for value in (
    '00 ff 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 '
    '01 00 02 00 04 00 00 00 00 00 00 00 00 00 00 00 '
    '02 00 04 00 06 00 00 00 02 00 04 00 06 00 00 01 '
    '00 02 00 06 00 00 00 00 00',
    '00 ff 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 '
    '00 00 00 00 00 02 00 04 00 06 00 00 00 02 00 04 '
    '00 06 00 00 00 00 00 01 00 00 00 01 00 00 00 00 '
    '00 00 00 00 00 01 00 02 00 06 00 00 00 00',
    '00 ff 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 '
    '00 00 00 00 00 00 00 02 00 04 00 06 00 08 00 00 '
    '00 02 00 04 00 06 00 08 00 00 01 00 02 00 06 00 '
    '00 00 00 00 00 00 00 00 00 00 00',
    '00 ff 3f 00 00 00 00 00 00 00 00 00 00 01 00 00 '
    '00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 02 '
    '00 04 00 06 00 00 00 02 00 04 00 06 00 00 00 00 '
    '00 00 01 00 02 00 06 00 00 00 00 00 00')]
ROUNDS = ((15, 20, 2000, 5), (2, 500, 400, 2000),
          (15, 16, 5, 2000), (300, 400, 4, 2000))
CAMERA_HEIGHT = (2048, 2560, 2048, 1920)
COMPONENT_OFFSET = (0xbf, 0xbc, 0xbe, 0xbc)


def step(words, cursor):
    previous = words[cursor]
    words[cursor] = (previous >> 1) ^ (0xb400 if previous & 1 else 0)
    return (words[cursor] - 1) % 65536, (cursor + 1) % 4


def random_line(words, cursor):
    return 'random ' + ' '.join(map(str, [cursor, *words])) + '\n'


def initialized(records, seeds, cursor, link):
    words = list(seeds)
    result = []
    for index, generation, original in records:
        kind, = struct.unpack_from('<H', original)
        if kind >= 4:
            continue
        raw = bytearray(original)
        value, cursor = step(words, cursor)
        raw[0x6d] = value % 256
        value, cursor = step(words, cursor)
        raw[0x42] = value % 256
        for offset, value in ((0x90, 0), (0x86, 1), (0x1a, 17 if link == 2 else 1),
                              (0x8d, 1), (0xa8, 0), (0x3e, 0)):
            raw[offset] = value
        for offset, value in ((0x8b, 0), (0x12, 40), (0x5d, 65535),
                              (0x87, CAMERA_HEIGHT[kind]), (0x14, 1024)):
            struct.pack_into('<H', raw, offset, value)
        raw[0x16] |= 0x66
        raw[0x17] |= 0x34
        flags, = struct.unpack_from('<H', raw, 0x40)
        struct.pack_into('<H', raw, 0x40, flags | 1)
        raw[0x16] = raw[0x16] | 8 if kind >= 2 else raw[0x16] & 247
        struct.pack_into('<4H', raw, 0xac if kind == 2 else 0xad, *ROUNDS[kind])
        if kind == 2:
            struct.pack_into('<H', raw, 0xb4, 20)
        else:
            raw[(0xb5, 0xfa, 0xb4, 0xf9)[kind]] = 20
        if kind in (1, 3):
            raw[0xb5:0xb7] = b'\x01\x01'
            raw[0xbb] = 10 if kind == 1 else 12
        offset = COMPONENT_OFFSET[kind]
        raw[offset:offset + len(COMPONENTS[kind])] = COMPONENTS[kind]
        result.append((index, generation, bytes(raw)))
    return result, (words, cursor)


def state_lines(records):
    lines = []
    for index, generation, raw in records:
        def word(offset):
            return struct.unpack_from('<H', raw, offset)[0]
        def signed(offset):
            return struct.unpack_from('<h', raw, offset)[0]
        kind = word(0)
        pose = struct.unpack_from('<3i', raw, 4)
        lines.append('state ' + ' '.join(map(str, [kind, index, generation, *pose,
                     signed(0x55), signed(0x57), signed(0x34), signed(0x59), signed(0x5b),
                     word(0x26), word(0x30), word(0x5d), raw[0x19], raw[0x3d],
                     word(0x10), word(0x89), word(0x8b)])))
        lines.append('ground ' + ' '.join(map(str, [raw[0x1d], signed(0x32),
                     signed(0x22), signed(0x24)])))
        lines.append('control ' + ' '.join(map(str, [word(0x12), word(0x14), word(0x87),
                     word(0x40), raw[0x16], raw[0x17], raw[0x1a], raw[0x6d], raw[0x42],
                     raw[0x90], raw[0x86], raw[0x8d], raw[0x3e], raw[0xa8],
                     len(COMPONENTS[kind])])))
        offset = 0xac if kind == 2 else 0xad
        rounds = struct.unpack_from('<4H', raw, offset)
        parameter = word(0xb4) if kind == 2 else raw[(0xb5, 0xfa, 0xb4, 0xf9)[kind]]
        cycle = raw[0xb5:0xb7] if kind in (1, 3) else (0, 0)
        stock = raw[0xbb] if kind in (1, 3) else 0
        lines.append('weapons ' + ' '.join(map(str, [*rounds, parameter, *cycle, stock])))
        lines.append('weapon_control ' + ' '.join(map(str, [raw[0x91], raw[0xa5], raw[0x92],
                                                           raw[0x3c], signed(0x38), raw[0xa7]])))
        offset = COMPONENT_OFFSET[kind]
        lines.append('components ' + raw[offset:offset + len(COMPONENTS[kind])].hex(' '))
        lines.append('selectors ' + ' '.join(map(str, raw[0xa9:0xac])))
        lines.append('behavior_flags ' + str(raw[0x63]))
        lines.append('speed_counter ' + str(raw[0x5f]))
        lines.append('command ' + ' '.join(map(str, [raw[0x43], raw[0x45], word(0x97),
                     *struct.unpack_from('<2i', raw, 0x49), *struct.unpack_from('<4H', raw, 0x28), word(0x53)])))
        lines.append('position_history ' + ' '.join(map(str, struct.unpack_from('<12H', raw, 0x6e))))
    return '\n'.join(lines) + ('\n' if lines else '')


class StartTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-start-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_vehicle_start_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_vehicle_start_probe.js')])

    def request(self, seeds=SEEDS, cursor=0, link=0):
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(struct.pack('<4HBB', *seeds, cursor, link))
        return path

    def run_probe(self, arguments, expected, valid=True):
        for command in self.commands:
            with self.subTest(target=command[0], mode=arguments[0]):
                result = subprocess.run([*command, *map(str, arguments)], capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, expected)

    def run_data(self, data, *, seeds=SEEDS, cursor=0, link=0):
        records = records_from_scenario(data)
        states, (words, end) = initialized(records, seeds, cursor, link)
        if ORACLE is not None:
            actual, random = ORACLE.initialize(records, seeds, cursor, link)
            # Complete 251-byte results include all preserved bytes and every
            # original class write, even fields not yet given runtime semantics.
            self.assertEqual(actual, states)
            self.assertEqual(random, (words, end))
        path = pathlib.Path(self.temp.name) / 'input.fsg'
        path.write_bytes(data)
        self.run_probe(['units', self.request(seeds, cursor, link), path],
                       state_lines(states) + random_line(words, end))

    def test_all_random_input_words_in_each_stream(self):
        for selected in range(4):
            words = list(SEEDS)
            values = []
            for previous in range(65536):
                words[selected] = previous
                value, end = step(words, selected)
                values.append(value)
            if ORACLE is not None:
                self.assertEqual(ORACLE.random(SEEDS, selected, sweep=True),
                                 (values, (words, end)))
            self.run_probe(['sweep', self.request(cursor=selected)],
                           ''.join(f'{value}\n' for value in values) + random_line(words, end))

    def test_random_sequences_cursor_wrap_and_zero_seeds(self):
        for seeds, cursor in ((SEEDS, 3), ((0, 0, 0, 0), 0),
                              ((65535, 32768, 32767, 1), 1)):
            words = list(seeds)
            end = cursor
            values = []
            for _ in range(65536):
                value, end = step(words, end)
                values.append(value)
            if ORACLE is not None:
                self.assertEqual(ORACLE.random(seeds, cursor), (values, (words, end)))
            self.run_probe(['random', self.request(seeds, cursor)],
                           ''.join(f'{value}\n' for value in values) + random_line(words, end))

    def test_all_class_flag_bytes_and_separate_preserved_motion_fields(self):
        for kind in range(4):
            for start in range(0, 256, 64):
                records = []
                for value in range(start, start + 64):
                    raw = bytearray(snapshot(kind, flags=value, heading=65535 - value))
                    raw[0x17] = 255 - value
                    raw[0x19] = value
                    raw[0x43], raw[0x45] = value, 255 - value
                    struct.pack_into('<H', raw, 0x97, value * 257)
                    struct.pack_into('<H', raw, 0x53, (255 - value) * 257)
                    struct.pack_into('<4H', raw, 0x28, value * 257, (255 - value) * 257,
                                     value * 911 % 65536, value * 379 % 65536)
                    struct.pack_into('<ii', raw, 0x49, -value * 8388608, value * 8388608)
                    raw[0x3d] = 255 - value
                    for offset, number in ((0x26, value * 257), (0x30, 65535 - value * 257),
                                           (0x34, value * 257), (0x55, 65535 - value * 257),
                                           (0x57, value * 257), (0x59, 32768 - value),
                                           (0x5b, 32767 + value), (0x89, value * 256),
                                           (0x40, value * 257)):
                        struct.pack_into('<H', raw, offset, number)
                    records.append((value % 182, 65535 - value, bytes(raw)))
                self.run_data(scenario_data(records), cursor=kind, link=2 if start & 64 else 0)

    def test_every_link_byte_all_classes_and_random_cursors(self):
        for link in range(256):
            self.run_data(scenario_data([(kind, kind, snapshot(kind, flags=0))
                                         for kind in range(4)]),
                          seeds=(0, 1, 32768, 65535), cursor=link % 4, link=link)

    def test_non_ground_records_do_not_consume_random_and_empty_input_is_valid(self):
        self.run_data(scenario_data([]), cursor=3)
        self.run_data(scenario_data([(kind, kind, snapshot(kind, flags=0))
                                    for kind in range(28)]), cursor=2)
        self.run_data(scenario_data([(kind, kind, snapshot(kind, flags=0))
                                    for kind in range(4, 28)]), seeds=(0, 0, 0, 0), cursor=1)

    def test_invalid_and_missing_request_scenario_and_api_inputs(self):
        request = self.request()
        original = request.read_bytes()
        for raw in (b'', original[:-1], original + b'x', original[:8] + b'\x04\x00',
                    original[:8] + b'\xff\x00'):
            request.write_bytes(raw)
            self.run_probe(['random', request], '', valid=False)
        request = self.request()
        missing = pathlib.Path(self.temp.name) / 'missing'
        self.run_probe(['random', missing], '', valid=False)
        self.run_probe(['units', request, missing], '', valid=False)
        self.run_probe(['bad', request], '', valid=False)
        self.run_probe(['units', request], '', valid=False)
        path = pathlib.Path(self.temp.name) / 'bad.fsg'
        for raw in (b'', scenario_data([(0, 0, snapshot())])[:-1],
                    scenario_data([(0, 0, snapshot(0, size=55))])):
            path.write_bytes(raw)
            self.run_probe(['units', request, path], '', valid=False)

    def test_complete_pinned_original_ground_corpus(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        files = sorted((ROOT / 'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([path.name for path in files], sorted(manifest))
        count = 0
        types = set()
        for ordinal, path in enumerate(files):
            with self.subTest(file=path.name):
                data = path.read_bytes()
                self.assertEqual(len(data), manifest[path.name]['size'])
                self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
                self.run_data(data, cursor=ordinal % 4, link=ordinal % 3)
                for _, _, state in records_from_scenario(data):
                    kind, = struct.unpack_from('<H', state)
                    if kind < 4:
                        count += 1
                        types.add(kind)
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(),
                                 manifest[path.name]['sha256'])
        self.assertEqual(count, 960)
        self.assertEqual(types, set(range(4)))


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
        from original_vehicle_start_oracle import OriginalVehicleStartOracle
        ORACLE = OriginalVehicleStartOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
