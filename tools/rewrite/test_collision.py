#!/usr/bin/env python3
"""Ordered live unit collision, complete type rules and deterministic random consumption."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_object_pool import EXTENDED, NONE
from test_units import records_from_scenario
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
HEIGHTS = {0: 2048, 1: 2304, 2: 2048, 3: 1792, 23: 1280, 27: 3072}
TYPE26_HEIGHTS = (4096, 4608, 2816, 3328) * 2
CHANCES = {7: 128, 8: 38, 9: 38, 10: 38, 11: 76, 12: 38, 13: 38, 15: 253}


def body(kind=0, *, index=0, value=1, x=0, y=0, altitude=0, heading=0, scale=1024, flags=64, mode=0):
    return kind, index, value, x, y, altitude, heading, scale, flags, mode


def fixture(bodies=None, queries=(0,), *, seeds=(1, 2, 32768, 65535), cursor=0):
    return bodies or [body(8, index=1), body()], queries, seeds, cursor


def from_raw(index, value, raw):
    kind, = struct.unpack_from('<H', raw)
    x, y, altitude, heading = struct.unpack_from('<3iH', raw, 4)
    scale, = struct.unpack_from('<H', raw, 0x14)
    return body(kind, index=index, value=value, x=x, y=y, altitude=altitude, heading=heading,
                scale=scale, flags=raw[0x16], mode=raw[0x19])


def delta(first, second):
    return (first - second + 2**31) % 2**32 - 2**31


def expected(case):
    bodies, queries, seeds, cursor = case
    words = list(seeds)
    slots = []
    registry = {}
    short = extended = 0
    for kind, index, value, *_ in bodies:
        slot = 150 + extended if kind in EXTENDED else short
        if kind in EXTENDED:
            extended += 1
        else:
            short += 1
        slots.append(slot)
        registry[index] = len(slots) - 1
    output = ''
    for ordinal in queries:
        kind, _, _, x, y, altitude, heading, _, _, mode = bodies[ordinal]
        hit = NONE, NONE, 0, 0
        for index, target_ordinal in sorted(registry.items()):
            target_kind, _, value, tx, ty, tz, target_heading, scale, flags, target_mode = bodies[target_ordinal]
            bound = (scale + 256) % 65536
            if target_ordinal == ordinal or not flags & 64 or abs(delta(x, tx)) > bound or abs(delta(y, ty)) > bound:
                continue
            height = (altitude - tz) % 2**32
            if target_kind in (5, 6):
                random, cursor = step(words, cursor)
                collides = random % 256 < CHANCES.get(kind, 0) and -512 <= delta(altitude, tz) <= 512
            else:
                limit = HEIGHTS.get(target_kind, 0)
                if target_kind == 26:
                    limit = TYPE26_HEIGHTS[target_mode] // (4 if mode & 4 else 1)
                collides = height < limit
            if collides:
                hit = slots[target_ordinal], index, value, ((-heading - target_heading) % 65536) // 4096
                break
        output += 'hit ' + ' '.join(map(str, hit)) + '\nrandom ' + ' '.join(map(str, [cursor, *words])) + '\n'
    return output


def encode(cases):
    output = struct.pack('<I', len(cases))
    for bodies, queries, seeds, cursor in cases:
        output += struct.pack('<4HBHHH', *seeds, cursor, len(bodies), len(queries), 0)
        output += b''.join(struct.pack('<HHHiiiHHBB', *item) for item in bodies)
        output += struct.pack('<' + 'H' * len(queries), *queries)
    return output


class CollisionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-collision-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'requests.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_collision_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_collision_probe.js')])

    def run_probe(self, data, wanted, valid=True):
        self.path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
            self.assertEqual(result.stdout, wanted)

    def check(self, cases):
        wanted = ''.join(expected(case) for case in cases)
        if ORACLE:
            original = ''.join(ORACLE.queries(*case) for case in cases)
            self.assertEqual(original, wanted)
        self.run_probe(encode(cases), wanted)
        return wanted

    def test_all_type_dispatches_vertical_boundaries_and_type26_modes(self):
        cases = []
        for kind in range(28):
            for mode in range(8) if kind == 26 else (0,):
                for source_mode in (0, 4, 255):
                    limit = TYPE26_HEIGHTS[mode] // (4 if source_mode & 4 else 1) if kind == 26 else HEIGHTS.get(kind, 0)
                    for altitude in (-2147483648, -513, -512, -1, 0, 1, 511, 512, 513,
                                     limit - 1, limit, limit + 1, 2147483647):
                        cases.append(fixture([body(8, index=1, altitude=altitude, mode=source_mode),
                                              body(kind, mode=mode)], seeds=(2, 0, 0, 0)))
        self.check(cases)
        # Trees point to the exact clc/ret, rather than their adjacent height code.
        self.assertTrue(self.check([fixture([body(8, index=1), body(21)])]).startswith('hit 65535 65535 0 0'))

    def test_square_inclusive_range_word_wrap_and_dword_subtraction(self):
        cases = []
        for scale in (0, 1, 1024, 65279, 65280, 65535):
            bound = (scale + 256) % 65536
            offsets = (-65536, -bound - 1, -bound, -1, 0, 1, bound, bound + 1, 65536)
            for dx in offsets:
                for dy in offsets:
                    cases.append(fixture([body(8, index=1, x=dx, y=dy), body(scale=scale)]))
        self.check(cases)
        # Subtractions wrap before sign/absolute conversion: opposite int32
        # endpoints are adjacent, rather than billions of units apart.
        self.check([fixture([body(8, index=1, x=2147483647, y=-2147483648, altitude=-2147483648),
                             body(x=-2147483648, y=2147483647, altitude=2147483647)])])

    def test_probabilistic_types_every_random_byte_source_type_cursor_and_failed_height(self):
        for cursor in range(4):
            cases = []
            for kind in range(28):
                for sample in range(256):
                    seeds = [0] * 4
                    seeds[cursor] = 2 * (sample + 1)
                    cases.append(fixture([body(kind, index=1), body(5)], seeds=tuple(seeds), cursor=cursor))
            # Keep peak owned case storage bounded; each batch is complete.
            for offset in range(0, len(cases), 512):
                self.check(cases[offset:offset + 512])
        cases = []
        for altitude in (-513, -512, 0, 512, 513):
            for target_kind in (5, 6):
                cases.append(fixture([body(8, index=1, altitude=altitude), body(target_kind)], seeds=(2, 0, 0, 0)))
        self.check(cases)

    def test_registry_order_flags_overwritten_bindings_origin_and_rng_encounters(self):
        # Storage order differs from registry order. A hit on its firer is still
        # the query's first result; only the consuming flight owner ignores it.
        self.check([fixture([body(8, index=10), body(index=9), body(index=2, value=0)])])
        self.check([fixture([body(8, index=10), body(index=1), body(index=1, x=100000)])])
        self.check([fixture([body(8, index=1), body(flags=flags)]) for flags in range(256)])
        # First type-5 body passes XY but fails Z, consuming RNG. The second
        # consumes another stream; the later ground hit does not consume RNG.
        bodies = [body(8, index=20), body(5, index=0, altitude=1000),
                  body(6, index=1), body(5, index=2, x=100000), body(index=3)]
        self.check([fixture(bodies, (0, 0, 0, 0), seeds=(2, 76, 78, 1))])
        # Overwritten source binding leaves a valid physical orphan.
        self.check([fixture([body(8, index=0), body(8, index=0, x=100000), body(index=2)])])

    def test_hit_aspect_word_subtraction_boundaries(self):
        headings = (0, 1, 4095, 4096, 4097, 16384, 32768, 49152, 65535)
        self.check([fixture([body(8, index=1, heading=source), body(heading=target)])
                    for source in headings for target in headings])

    def test_invalid_requests_world_and_incomplete_runs_fail(self):
        good = encode([fixture()])
        invalid = [b'', good[:-1], good + b'x', struct.pack('<I', 2) + good[4:]]
        for offset, data in ((4 + 8, b'\x04'), (4 + 13, b'\x01'), (4 + 11, b'\0\0'),
                             (4 + 15, struct.pack('<H', 28)), (len(good) - 2, struct.pack('<H', 2))):
            raw = bytearray(good); raw[offset:offset + len(data)] = data; invalid.append(bytes(raw))
        invalid.append(encode([fixture([body(8, index=1), body(26, mode=8)])]))
        invalid.append(encode([fixture([body(8, index=1), body(index=182)])]))
        # Invalid later case must not publish the preceding valid query.
        invalid.append(encode([fixture(), fixture([body(8, index=1), body(26, mode=255)])]))
        for data in invalid:
            self.run_probe(data, '', valid=False)
        self.run_probe(struct.pack('<I', 0), '')

    def test_complete_original_snapshot_worlds_and_m1_shell_queries(self):
        if not ORIGINALS:
            self.skipTest('Complete original corpus is an explicit additional gate')
        manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        total = shots = 0
        for name, info in manifest.items():
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records = records_from_scenario(data)
            bodies = [from_raw(index, value, raw) for index, value, raw in records]
            cases = [fixture(bodies, tuple(range(len(bodies))))]
            total += len(bodies)
            registry = {item[1] for item in bodies}
            free = next(index for index in range(182) if index not in registry)
            for kind, _, _, x, y, altitude, heading, *_ in bodies:
                if kind != 0:
                    continue
                source = body(8, index=free, x=x, y=y,
                              altitude=(altitude + 2048 + 2**31) % 2**32 - 2**31,
                              heading=heading, scale=0, flags=0)
                cases.append(fixture([*bodies, source], (len(bodies),)))
                shots += 1
            self.check(cases)
        self.assertEqual((len(manifest), total, shots), (47, 4213, 179))
        print(f'Collision corpus: all {total} source queries and {shots} M1 launch-position shell queries in {len(manifest)} snapshot worlds on {len(self.commands)} targets', flush=True)


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
        from original_collision_oracle import OriginalCollisionOracle
        ORACLE = OriginalCollisionOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != (0 if ORIGINALS else 1))
