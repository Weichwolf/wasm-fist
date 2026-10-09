#!/usr/bin/env python3
"""Original-free FMAP integrity, lossless ownership and periodic triangle behavior."""
import argparse
import copy
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
spec = importlib.util.spec_from_file_location('pack_maps', ROOT / 'assets/generator/pack_maps.py')
PACK = importlib.util.module_from_spec(spec)
spec.loader.exec_module(PACK)


def bundle(side, heights, colors, bounds=(-1.25, 1000.375), water=1200.5):
    plane = struct.pack('<' + 'H' * len(heights), *heights)
    payload = plane + colors
    prefix = struct.pack('<8s6I3dI', b'FISTMAP\0', 1, 64, side, len(plane),
                         len(colors), 0, *bounds, water, 0)
    return prefix + struct.pack('<I', zlib.crc32(prefix + payload)) + payload


def repaired_crc(data):
    data = bytearray(data)
    struct.pack_into('<I', data, 60, zlib.crc32(data[:60] + data[64:]))
    return bytes(data)


def png(side, pixels, depth, color_type, row_filter=0):
    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data))
    stride = side * (2 if depth == 16 else 3)
    rows = b''.join(bytes([row_filter]) + pixels[start:start + stride]
                    for start in range(0, len(pixels), stride))
    return (b'\x89PNG\r\n\x1a\n' +
            chunk(b'IHDR', struct.pack('>IIBBBBB', side, side, depth, color_type, 0, 0, 0)) +
            chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b''))


def restrict_memory():
    import resource
    resource.setrlimit(resource.RLIMIT_AS, (12 * 1024 * 1024, 12 * 1024 * 1024))


class OwnedTerrainTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-owned-terrain-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_owned_terrain_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_owned_terrain_probe.js')])
        if not cls.commands:
            raise AssertionError('No requested runtime target')

    def run_bundle(self, data, coordinates=None, valid=True, sampled=True, limited=False,
                   commands=None, allocation=False):
        path = self.directory / 'input.fmap'
        path.write_bytes(data)
        args = [str(path)]
        if allocation:
            args.append('allocation')
        if coordinates is not None:
            requests = self.directory / 'coordinates.bin'
            requests.write_bytes(struct.pack('<I', len(coordinates)) +
                                 b''.join(struct.pack('<dd', *pair) for pair in coordinates))
            args.append(str(requests))
        expected_planes = b''
        if valid:
            expected_planes = data[16:20] + data[32:56] + data[64:]
        results = []
        for command in commands or self.commands:
            with self.subTest(target=command[0], bytes=len(data), valid=valid):
                result = subprocess.run([*command, *args], capture_output=True, timeout=60,
                                        preexec_fn=restrict_memory if limited else None)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr.decode())
                if valid:
                    self.assertEqual(result.stdout[:len(expected_planes)], expected_planes)
                    count = len(coordinates) if coordinates is not None else 0
                    self.assertEqual(len(result.stdout), len(expected_planes) + count * 8)
                    results.append(struct.unpack('<' + 'd' * count,
                                                 result.stdout[len(expected_planes):]))
                else:
                    self.assertEqual(result.stdout, b'')
                    self.assertEqual(result.stderr, b'sample rejected\n' if not sampled
                                     else b'decode rejected\n')
        return results

    def fixture(self, side=16):
        heights = [(row * 4097 + column * 1009) % 65536
                   for row in range(side) for column in range(side)]
        colors = bytes((index * 37 + index // 3) % 256 for index in range(side * side * 3))
        return heights, colors, bundle(side, heights, colors)

    def test_null_empty_destroy_and_failure_contracts(self):
        for command in self.commands:
            result = subprocess.run([*command, 'contracts'], capture_output=True, timeout=30)
            self.assertEqual((result.returncode, result.stdout, result.stderr), (0, b'', b''))
        self.run_bundle(b'', valid=False)

    def test_all_height_words_rgb_bytes_fractional_metadata_and_source_release(self):
        # Each possible 16-bit height appears once; an 8-bit intermediate cannot pass.
        heights = list(range(65536))
        colors = bytes(range(256)) * 768
        self.run_bundle(bundle(256, heights, colors))

    def test_header_dimensions_lengths_version_flags_reserved_and_truncation(self):
        _, _, original = self.fixture()
        invalid = [original[:length] for length in (1, 7, 31, 60, 63, 64, 65, len(original) - 1)]
        invalid += [original + b'\0', original + original]
        for offset, values in ((8, (0, 2, 0xffffffff)), (12, (0, 63, 65)),
                               (16, (0, 1, 15, 17, 8192, 0xffffffff)),
                               (20, (0, 511, 513, 0xffffffff)),
                               (24, (0, 767, 769, 0xffffffff)),
                               (28, (1, 0xffffffff)), (56, (1, 0xffffffff))):
            for value in values:
                changed = bytearray(original)
                struct.pack_into('<I', changed, offset, value)
                invalid.append(repaired_crc(changed))
        for offset in range(8):
            changed = bytearray(original); changed[offset] ^= 1
            invalid.append(repaired_crc(changed))
        for data in invalid:
            self.run_bundle(data, valid=False)

    def test_nonfinite_and_invalid_elevation_metadata_under_production_fast_math(self):
        _, _, original = self.fixture()
        for offset in (32, 40, 48):
            for value in (math.inf, -math.inf, math.nan, -10000.01, 20000.01):
                changed = bytearray(original); struct.pack_into('<d', changed, offset, value)
                self.run_bundle(repaired_crc(changed), valid=False)
            for bits in (0x7ff0000000000001, 0xfff8000000000001):
                changed = bytearray(original); struct.pack_into('<Q', changed, offset, bits)
                self.run_bundle(repaired_crc(changed), valid=False)
        for bounds in ((0, 0), (100, 99)):
            self.run_bundle(bundle(16, [0] * 256, bytes(768), bounds=bounds), valid=False)

    def test_complete_header_height_and_color_crc_protection(self):
        _, _, original = self.fixture()
        for offset in (32, 39, 40, 47, 48, 55, 60, 63, 64, 65, 575, 576, 577, len(original)-1):
            changed = bytearray(original); changed[offset] ^= 1
            self.run_bundle(bytes(changed), valid=False)

    def test_triangles_diagonal_knots_and_both_periodic_seams(self):
        side = 16
        heights, _, data = self.fixture(side)
        coordinates = [(0, 0), (1/side, 0), (0, 1/side), (1/side, 1/side),
                       (.25/side, .25/side), (.75/side, .75/side), (.25/side, .75/side),
                       (.75/side, .25/side), (1-.25/side, 1-.25/side),
                       (1-.75/side, .25/side), (.25/side, 1-.75/side)]
        randomizer = random.Random(133)
        coordinates += [(randomizer.random(), randomizer.random()) for _ in range(128)]
        coordinates += [(x + dx, y + dy) for x, y in coordinates[:11]
                        for dx, dy in ((-1, 0), (0, -1), (1, 1), (-4, 7), (32, -32))]
        coordinates += [(1e300, -1e300), (-0.0, 0.0), (-1, 1)]
        expected = []
        for x, y in coordinates:
            column, row = x % 1 * side, y % 1 * side
            left, top = math.floor(column), math.floor(row)
            u, v = column - left, row - top
            corners = [heights[(r % side)*side + c % side]
                       for r, c in ((top, left), (top, left+1), (top+1, left), (top+1, left+1))]
            a, b, c, d = corners
            encoded = (a*(1-u-v) + b*u + c*v if u+v <= 1
                       else d*(u+v-1) + c*(1-u) + b*(1-v))
            expected.append(-1.25 + encoded / 65535 * 1001.625)
        for output in self.run_bundle(data, coordinates):
            for actual, reference in zip(output, expected):
                self.assertAlmostEqual(actual, reference, delta=1e-9)

    def test_nonfinite_sample_coordinates_fail_atomically(self):
        _, _, data = self.fixture()
        for value in (math.inf, -math.inf, math.nan):
            for pair in ((value, .5), (.5, value)):
                self.run_bundle(data, [pair], valid=False, sampled=False)

    def test_allocation_failure_preserves_output_and_frees_partial_planes(self):
        for command in self.commands:
            # Input fits either the native12-MiB address limit or WASM32-MiB heap;
            # both independently allocated output planes do not. The complete
            # input must be read before an atomic decode failure is observed.
            wasm = command[0] == 'node'
            side = 2048 if wasm else 1024
            data = bundle(side, [0] * (side*side), bytes(side*side*3))
            self.run_bundle(data, valid=False, limited=not wasm, commands=[command], allocation=True)

    def test_all_eight_actual_owned_maps_pack_and_decode_complete_planes(self):
        required = {'arid-ridges', 'dry-mountains', 'limestone-valleys', 'rocky-highlands',
                    'sandy-desert', 'snowy-alpine', 'temperate-forest', 'training-valley'}
        recipes = sorted((ROOT / 'assets/maps').glob('*.json'))
        self.assertEqual({p.stem for p in recipes}, required)
        for recipe in recipes:
            with self.subTest(map=recipe.stem):
                data, metadata = PACK.pack_map(recipe, ROOT / 'assets/generated/maps')
                self.assertEqual(len(data), 5242944)
                self.assertEqual(metadata['outputs'][f'{recipe.stem}.fmap'],
                                 {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
                runtime = ROOT / 'assets/generated/runtime/maps'
                self.assertEqual((runtime / f'{recipe.stem}.fmap').read_bytes(), data)
                self.assertEqual(json.loads((runtime / f'{recipe.stem}-package.json').read_text()),
                                 metadata)
                self.run_bundle(data, [(0, 0), (.5, .5), (1, 1), (-.25, 2.75)])

    def test_png_crc_encoding_stream_row_and_manifest_corruption_rejected(self):
        heights, colors, _ = self.fixture()
        encoded = struct.pack('>' + 'H'*len(heights), *heights)
        correct = png(16, encoded, 16, 0)
        self.assertEqual(PACK.png_plane(correct, 16, 16, 0),
                         struct.pack('<'+'H'*len(heights), *heights))
        bad_crc = bytearray(correct); bad_crc[-1] ^= 1
        for data in (bytes(bad_crc), correct[:-1], correct+b'\0', png(16, encoded, 16, 0, 1),
                     png(16, encoded+b'xx', 16, 0), png(16, encoded[:-2], 16, 0),
                     png(16, colors, 8, 2)):
            with self.assertRaises((ValueError, zlib.error)):
                PACK.png_plane(data, 16, 16, 0)
        with self.assertRaises(ValueError):
            PACK.png_plane(correct, 32, 16, 0)
        compressed_size, = struct.unpack_from('>I', correct, 33)
        compressed = correct[41:41+compressed_size]
        for stream in (compressed[:-1], compressed + zlib.compress(b'extra'), b''):
            chunk = (struct.pack('>I', len(stream)) + b'IDAT' + stream +
                     struct.pack('>I', zlib.crc32(b'IDAT' + stream)))
            changed = correct[:33] + chunk + correct[-12:]
            with self.assertRaises((ValueError, zlib.error)):
                PACK.png_plane(changed, 16, 16, 0)
        original_path = ROOT / 'assets/maps/training-valley.json'
        recipe_path = self.directory / original_path.name
        recipe = json.loads(original_path.read_text())
        recipe['resolution'] = 16
        recipe_path.write_text(json.dumps(recipe))
        outputs = {'training-valley-height.png': correct,
                   'training-valley-color.png': png(16, colors, 8, 2)}
        manifest = {'version': 1, 'name': recipe['name'], 'resolution': 16,
                    'height_range': recipe['height_range'], 'periodic': True,
                    'height_encoding': PACK.HEIGHT_ENCODING,
                    'recipe_sha256': hashlib.sha256(recipe_path.read_bytes()).hexdigest(),
                    'outputs': {name: {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
                                for name, data in outputs.items()}}
        for name, data in outputs.items():
            (self.directory / name).write_bytes(data)
        manifest_path = self.directory / 'training-valley-manifest.json'
        manifest_path.write_text(json.dumps(manifest))
        data, _ = PACK.pack_map(recipe_path, self.directory)
        self.run_bundle(data)
        for field, value in (('recipe_sha256', '0'*64), ('periodic', False),
                             ('resolution', 32), ('height_encoding', '8-bit'), ('outputs', {})):
            changed = copy.deepcopy(manifest); changed[field] = value
            manifest_path.write_text(json.dumps(changed))
            with self.assertRaises(ValueError):
                PACK.pack_map(recipe_path, self.directory)
        manifest_path.write_text(json.dumps(manifest))
        (self.directory / 'training-valley-color.png').write_bytes(b'corrupt')
        with self.assertRaises(ValueError):
            PACK.pack_map(recipe_path, self.directory)

    def test_actual_batch_preflight_failure_and_repeated_complete_success(self):
        root = self.directory / 'batch'
        recipes = root / 'recipes'
        recipes.mkdir(parents=True)
        for name in ('rocky-highlands', 'training-valley'):
            (recipes / f'{name}.json').write_bytes((ROOT / f'assets/maps/{name}.json').read_bytes())
        output = root / 'output'
        command = ['python3', str(ROOT / 'assets/generator/pack_maps.py'),
                   '--recipes-dir', str(recipes), '--output-dir', str(output)]
        broken = recipes / 'training-valley.json'
        original = broken.read_bytes()
        changed = json.loads(original); changed['name'] = 'wrong-name'
        broken.write_text(json.dumps(changed))
        result = subprocess.run(command, capture_output=True, timeout=60)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(output.exists(), 'A late invalid recipe must not publish a partial batch')
        broken.write_bytes(original)
        result = subprocess.run(command, capture_output=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        self.assertEqual(json.loads(result.stdout),
                         {'success': True, 'maps': ['rocky-highlands', 'training-valley']})
        first = {path.name: path.read_bytes() for path in output.iterdir()}
        self.assertEqual(set(first), {'rocky-highlands.fmap', 'rocky-highlands-package.json',
                                     'training-valley.fmap', 'training-valley-package.json'})
        result = subprocess.run(command, capture_output=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        self.assertEqual(first, {path.name: path.read_bytes() for path in output.iterdir()})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default='all')
    parser.add_argument('--build-root', type=Path, default=BUILD)
    parser.add_argument('--native-probe', type=Path)
    args, remaining = parser.parse_known_args()
    BUILD, TARGET, NATIVE_PROBE = args.build_root, args.target, args.native_probe
    unittest.main(argv=[__file__, *remaining])
