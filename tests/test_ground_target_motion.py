#!/usr/bin/env python3
"""Compare complete owned target motion with the independently proved model."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import struct
import subprocess
import tempfile

from ground_target_motion_contract import motion
from roster_promotion_contract import store, word
from target_discovery_contract import TABLES
from test_target_acquisition import fixture
from test_target_discovery import NONE, physical, pointer
from test_vehicle_motion import start
from test_vehicle_start import state_lines


def expected(case):
    slot = case['actor']
    original = case['raw'][slot]
    objects = {pointer(other): raw for other, raw in case['raw'].items()}
    supplied = bytearray(original)
    target = word(supplied, 0x97)
    if target and physical(target) not in case['raw']:
        store(supplied, 0x97, 0)
    after, events = motion(TABLES, bytes(supplied), objects, case['coarse'], pointer(slot))
    target_slot = physical(word(after, 0x97))
    normalized = bytearray(after)
    store(normalized, 0x97, 0)
    store(normalized, 0x9d, 0)
    return (state_lines([(0, 1, bytes(normalized))]) +
            'ground_motion ' + ' '.join(map(str, (slot, target_slot, word(after, 0x9b),
                                                 word(after, 0x99), *events))) + '\n')


def encode(case):
    slot, raw = case['actor'], case['raw']
    header = struct.pack('<6H3B4HH', slot, physical(case['selected']), case['clock'],
                         case['gate'], case['last'], physical(word(raw[slot], 0x97)),
                         case['link'], case['coarse'], case['cursor'], *case['seeds'], len(raw))
    header += struct.pack('<B3H', 70, 0, NONE, NONE)
    return (header + b''.join(struct.pack('<HH', *entry) for entry in case['registry']) +
            b''.join(struct.pack('<H', other) + state for other, state in sorted(raw.items())))


def cases():
    angles = (0, 1, 31, 32, 33, 63, 64, 8192, 16384, 32767, 32768, 49152, 65535)
    for kind, target_kind, coarse, present in itertools.product(range(4), range(28), (0, 1, 255), (False, True)):
        case = fixture(kind, target_kind, old=-1 if present else NONE, operation=70)
        raw = bytearray(start(kind, hull=65000, request=2000, offset=17000,
                              turret_request=31000, speed=51, throttle=121))
        store(raw, 0x97, word(case['raw'][150], 0x97))
        store(raw, 0x9b, 9000)
        store(raw, 0x99, 65000)
        case['raw'][150] = bytes(raw)
        case['coarse'] = coarse
        yield case
    for kind, coarse, heading, speed, direction, self_target in itertools.product(
            range(4), (0, 1, 255), angles, (-32768, -1, 0, 1, 321, 32767),
            (0, 2, 4, 6, 16), (False, True)):
        case = fixture(kind, 26, old=-1, operation=70)
        raw = bytearray(start(kind, hull=heading, request=heading, speed=speed,
                              x=2147483647, y=-2147483648, phase=1, flags=direction))
        store(raw, 0x97, pointer(150) if self_target else pointer(case['candidate']))
        store(raw, 0x9b, 9000)
        case['raw'][150] = bytes(raw)
        case['coarse'] = coarse
        yield case


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=Path, default=Path('/tmp/wasm-fist-rewrite'))
    parser.add_argument('--native-probe', type=Path)
    args = parser.parse_args()
    commands = []
    if args.target in ('native', 'all'):
        commands.append([str(args.native_probe or args.build_root / 'native/fist_target_discovery_probe')])
    if args.target in ('wasm', 'all'):
        commands.append(['node', str(args.build_root / 'wasm/fist_target_discovery_probe.js')])
    digest = hashlib.sha256()
    count = 0
    with tempfile.TemporaryDirectory(prefix='fist-ground-target-motion-', dir='/tmp') as temporary:
        path = Path(temporary) / 'request.bin'
        batch = []
        def check():
            nonlocal count
            if not batch:
                return
            wanted = ''.join(expected(case) for case in batch)
            path.write_bytes(struct.pack('<II', 1, len(batch)) + b'\0' + b''.join(map(encode, batch)))
            for command in commands:
                result = subprocess.run([*command, '--ground-motion', str(path)], capture_output=True, text=True, timeout=180)
                if result.returncode or result.stderr or result.stdout != wanted:
                    actual, desired = result.stdout.splitlines(), wanted.splitlines()
                    index = next((i for i, (a, b) in enumerate(zip(actual, desired)) if a != b), min(len(actual), len(desired)))
                    raise AssertionError((command, result.returncode, result.stderr, index, actual[index:index+1], desired[index:index+1]))
            count += len(batch)
            digest.update(wanted.encode())
            batch.clear()
        for case in cases():
            batch.append(case)
            if len(batch) == 512:
                check()
        check()
    assert count == 10032, count
    print(json.dumps({'success': True, 'cases_per_target': count, 'skips': 0,
                      'output_sha256': digest.hexdigest(), 'scope': 'Target-motion development; lifetime/failure/canonical gates remain required'}, sort_keys=True))


if __name__ == '__main__':
    main()
