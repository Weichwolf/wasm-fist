#!/usr/bin/env python3
"""Shared bounded object identities, allocator admission, imports and exhaustion."""
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
from test_vehicle_start import initialized

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
NONE = 65535
SHORT, LONG, COUNT = 150, 32, 182
EXTENDED = {0, 1, 2, 3, 19}


def allocate(kind=8, *, low=0):
    return 1, low, kind, 0, 0


def bind(kind, index, value=1):
    return 2, 0, kind, index, value


def release(index):
    return 3, 0, 0, index, 0


def trace(commands):
    slots = [(0, 0)] * COUNT
    registry = [(NONE, 0)] * COUNT
    allocation = (28, NONE, COUNT, NONE)
    def state():
        counts = sum(used for used, _ in slots[:SHORT]), sum(used for used, _ in slots[SHORT:])
        return (f'counts {counts[0]} {counts[1]}\nslots' + ''.join(f' {used}:{kind}' for used, kind in slots) +
                '\nregistry' + ''.join(f' {slot}:{value}' for slot, value in registry) + '\n')
    output = state()
    for operation, low, kind, index, value in commands:
        result = 0
        if operation == 0:
            slots = [(0, 0)] * COUNT
            registry = [(NONE, 0)] * COUNT
        elif operation in (1, 2):
            if kind >= 28 or (operation == 1 and low > 1) or (operation == 2 and index >= COUNT):
                result = -1
            else:
                physical = range(SHORT, COUNT) if kind in EXTENDED else range(SHORT)
                slot = next((slot for slot in physical if slots[slot][0] == 0), None)
                if operation == 1:
                    index = next((index for index, binding in enumerate(registry) if binding == (NONE, 0)), None)
                    value = 1
                limited = operation == 1 and low and sum(used for used, _ in slots[:SHORT]) >= 120
                if slot is None or index is None or limited:
                    result = 1
                else:
                    slots[slot] = 1, kind
                    registry[index] = slot, value
                    allocation = kind, slot, index, value
        elif operation == 3:
            if index >= COUNT:
                result = -1
            elif registry[index][0] == NONE:
                result = 1
            else:
                slot, value = registry[index]
                allocation = slots[slot][1], slot, index, value
                slots[slot] = 0, 0
                registry[index] = NONE, (value - 1) % 65536
        else:
            result = -1
        output += f'result {result} ' + ' '.join(map(str, allocation)) + '\n' + state()
    return output


class ObjectPoolTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-object-pool-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.request = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_object_pool_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_object_pool_probe.js')])

    def check(self, commands, *, original_safe=True):
        expected = trace(commands)
        if ORACLE and original_safe:
            self.assertEqual(ORACLE.trace(commands), expected)
        self.request.write_bytes(struct.pack('<I', len(commands)) + b''.join(
            struct.pack('<BBHHH', *command) for command in commands))
        for command in self.commands:
            result = subprocess.run([*command, str(self.request)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, expected)
        return expected

    def test_all_type_classes_ordered_holes_duplicates_and_reset(self):
        commands = [allocate(kind) for kind in range(28)]
        commands += [release(index) for index in (7, 0, 27, 19, 4)]
        commands += [allocate(kind) for kind in (0, 18, 3, 26, 8)]
        commands += [bind(8, 2, 12), bind(19, 2, 1), release(2), (0, 0, 0, 0, 0), allocate(18)]
        self.check(commands)

    def test_short_admission_limit_normal_capacity_and_muzzle_boundary(self):
        for count in (118, 119, 120, 149, 150):
            commands = [allocate()] * count + [allocate(), allocate(18, low=1)]
            output = self.check(commands)
            events = [line.split() for line in output.splitlines() if line.startswith('result ')]
            self.assertEqual(int(events[-2][1]), int(count == 150))
            self.assertEqual(int(events[-1][1]), int(count >= 119))
        self.check([allocate()] * SHORT + [allocate(), release(41), allocate(18), release(149), allocate(4)])
        self.check([allocate()] * 120 + [release(2), allocate(18, low=1), allocate(18, low=1)])
        commands = [allocate()] * 119
        for kind in range(28):
            commands += [allocate(kind, low=1), release(119)]
        commands += [allocate()] + [allocate(kind, low=1) for kind in range(28)]
        self.check(commands)

    def test_saved_registry_values_skip_reservations_and_wrap_on_release(self):
        for kind in (0, 8):
            commands = []
            for index, value in enumerate((0, 1, 2, 12, 65535)):
                commands += [bind(kind, index, value), release(index), release(index)]
            commands += [allocate(18), allocate(19), allocate(8)]
            output = self.check(commands)
            events = [line.split() for line in output.splitlines() if line.startswith('result ')]
            self.assertEqual(events[-3][4], '1', 'Only a vacant entry with saved value zero can be reused')
            self.assertEqual(events[-2][4], '5')
            self.assertEqual(events[-1][4], '6')

    def test_extended_and_registry_exhaustion_fail_without_mutation(self):
        # The first 32 constructors match the actual original arena. Its 33rd
        # write is a proved corruption, so port exhaustion is checked separately.
        self.check([allocate(0)] * LONG)
        output = self.check([allocate(0)] * LONG + [allocate(0), allocate(8)], original_safe=False)
        events = [line.split() for line in output.splitlines() if line.startswith('result ')]
        self.assertEqual(events[-2][1], '1')
        self.assertEqual(events[-1][1:5], ['0', '8', '0', '32'])
        commands = [bind(8 if index < SHORT else 0, index, 2) for index in range(COUNT)]
        commands += [release(index) for index in range(COUNT)]
        self.check(commands)
        output = self.check(commands + [allocate(), bind(18, 3, 1), allocate()], original_safe=False)
        events = [line.split() for line in output.splitlines() if line.startswith('result ')]
        self.assertEqual([event[1] for event in events[-3:]], ['1', '0', '1'])

    def test_invalid_requests_preserve_pool_and_output_and_incomplete_files_fail(self):
        commands = [allocate(), bind(0, 42, 12)]
        commands += [allocate(28), allocate(65535), allocate(low=2), allocate(low=255), bind(28, 0),
                     bind(8, COUNT), bind(8, NONE), release(COUNT), release(NONE), (255, 0, 0, 0, 0)]
        self.check(commands, original_safe=False)
        for contents in (b'', b'\0', struct.pack('<I', 1), struct.pack('<I', 0) + bytes(8),
                         struct.pack('<I', 1) + bytes(7)):
            self.request.write_bytes(contents)
            for command in self.commands:
                result = subprocess.run([*command, str(self.request)], capture_output=True, timeout=10)
                self.assertEqual((result.returncode, result.stdout), (1, b''))

    def test_original_corruptions_and_complete_reaching_m1_primary_handlers(self):
        if not ORACLE:
            self.skipTest('Actual original instructions are an explicit additional gate')
        self.assertEqual(ORACLE.type_flags, bytes(int(kind in EXTENDED) for kind in range(28)))
        pointer, index = ORACLE.exhaustion_corruption()
        self.assertEqual(pointer, 0xdfbc)
        self.assertEqual(index, 184)
        states, _ = initialized([(9, 12, start())], (0, 0, 0, 0), 0, 0)
        raw = bytearray(states[0][2]); struct.pack_into('<H', raw, 0x97, 0)
        raw[0x91] = raw[0xa5] = raw[0xa8] = raw[0x3c] = 0
        raw[0x92] = 48  # Actual player fire command at an eligible station-0 boundary.
        for occupied in (118, 119, 149, 150):
            actor, objects, carry = ORACLE.m1_primary_shots([bytes(raw)], short_fill=occupied)[0]
            expected = bytearray(raw)
            struct.pack_into('<H', expected, 0xad, 14)
            expected[0xe4] = 3
            if occupied < 150:
                expected[0xa8], expected[0x3c], expected[0x92] = 20, 16, 0
            self.assertEqual(actor, bytes(expected), 'Pool failure happens after the actual ammunition decrement')
            self.assertEqual(carry, int(occupied == 150))
            self.assertEqual(len(objects), occupied + (2 if occupied == 118 else int(occupied < 150)))
            if occupied < 150:
                self.assertEqual(int.from_bytes(objects[occupied][2][:2], 'little'), 8)
            if occupied == 118:
                self.assertEqual(int.from_bytes(objects[-1][2][:2], 'little'), 18)
        print('Original pool: both exhaustion corruptions and complete M1 capacity/muzzle/ammunition handlers proved', flush=True)

    def test_all_original_snapshot_bindings_and_runtime_reallocation(self):
        if not ORIGINALS:
            self.skipTest('Complete original snapshot corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        total = 0
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            commands = [bind(int.from_bytes(raw[:2], 'little'), index, value) for index, value, raw in records]
            commands += [release(index) for index in sorted({index for index, _, _ in records})]
            commands += [allocate(8), allocate(18, low=1), allocate(0)]
            self.check(commands)
            total += len(records)
        self.assertEqual(total, 4213)
        print(f'Original pool: {total} complete snapshot imports in {len(manifest)} missions and runtime reuse on {len(self.commands)} targets', flush=True)


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
        from original_object_pool_oracle import OriginalObjectPoolOracle
        ORACLE = OriginalObjectPoolOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (not ORIGINALS) + (not ORACLE))
