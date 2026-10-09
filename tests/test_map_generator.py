#!/usr/bin/env python3
"""Owned terrain generation: repeatability, controls, conservation and file contracts."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import zlib

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('mapgen', ROOT / 'assets/generator/mapgen.py')
MAP = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MAP)


def fixture():
    recipe = json.loads((ROOT / 'assets/maps/training-valley.json').read_text())
    recipe['resolution'] = 32
    recipe['features'] = []
    recipe['base_height'] = 80
    recipe['octaves'] = [{'frequency': 4, 'amplitude': 30, 'seed_offset': 3,
                         'kind': 'smooth', 'ridge_power': 1}]
    # This coarse, high-gradient test world has its own erosion settings;
    # high-resolution visual recipe tuning must not change its input contract.
    recipe['hydraulic_erosion'] = {'iterations': 12, 'time_step': .2,
        'rainfall': .035, 'evaporation': .12, 'flow_rate': .3,
        'sediment_capacity': 2, 'erosion_rate': .03, 'deposition_rate': .2,
        'bedrock_height': 0}
    return recipe


def png(data):
    if data[:8] != b'\x89PNG\r\n\x1a\n': raise AssertionError('Missing PNG signature')
    offset = 8; header = None; compressed = b''; ended = False
    while offset < len(data):
        size, = struct.unpack_from('>I', data, offset); kind = data[offset+4:offset+8]
        payload = data[offset+8:offset+8+size]
        crc, = struct.unpack_from('>I', data, offset+8+size)
        if zlib.crc32(kind + payload) != crc: raise AssertionError('Bad PNG CRC')
        if kind == b'IHDR': header = struct.unpack('>IIBBBBB', payload)
        if kind == b'IDAT': compressed += payload
        if kind == b'IEND': ended = True
        offset += size + 12
    if not ended or offset != len(data): raise AssertionError('Incomplete PNG')
    return header, zlib.decompress(compressed)


class MapGeneration(unittest.TestCase):
    def test_all_map_family_recipes_are_complete(self):
        expected = ['arid-ridges', 'dry-mountains', 'limestone-valleys',
                    'rocky-highlands', 'sandy-desert', 'snowy-alpine',
                    'temperate-forest', 'training-valley']
        paths = sorted((ROOT / 'assets/maps').glob('*.json'))
        self.assertEqual([path.stem for path in paths], expected)
        seeds = set()
        for path in paths:
            recipe = MAP.load_recipe(path); MAP.validate(recipe)
            self.assertEqual(recipe['name'], path.stem)
            self.assertEqual(recipe['resolution'], 1024)
            self.assertNotIn(recipe['seed'], seeds); seeds.add(recipe['seed'])

    def test_repeatable_without_original_assets(self):
        recipe = fixture()
        first = MAP.generate(recipe); second = MAP.generate(copy.deepcopy(recipe))
        self.assertTrue(np.array_equal(first[0], second[0]))
        self.assertTrue(np.array_equal(first[1], second[1]))
        self.assertEqual(first[2], second[2])
        self.assertEqual(recipe, fixture(), 'Generation must not mutate authoring inputs')

    def test_hydraulic_mass_and_actual_transport(self):
        recipe = fixture(); initial = recipe['base_height'] + MAP.noise(32, recipe['seed'], recipe['octaves'])
        eroded, water, error = MAP.hydraulic(initial, recipe['hydraulic_erosion'])
        self.assertAlmostEqual(float(initial.sum()), float(eroded.sum()), places=8)
        self.assertLess(abs(error), 1e-8)
        self.assertTrue(np.isfinite(eroded).all()); self.assertTrue((water >= 0).all())
        self.assertFalse(np.array_equal(initial, eroded), 'Enabled erosion must move material')
        disabled = copy.deepcopy(recipe['hydraulic_erosion']); disabled['iterations'] = 0
        self.assertTrue(np.array_equal(initial, MAP.hydraulic(initial, disabled)[0]))

    def test_json_controls_reach_visible_outputs(self):
        recipe = fixture(); height, color, _ = MAP.generate(recipe)
        variants = [('seed', 999), ('water', dict(recipe['water'], level=90)),
                    ('temperature', dict(recipe['temperature'], base_celsius=-40)),
                    ('palette', dict(recipe['palette'], rock=[255, 0, 255], lowland=[255, 0, 255])),
                    ('lighting', dict(recipe['lighting'], ambient=.1)),
                    ('moisture', dict(recipe['moisture'], base=0)),
                    ('octaves', [dict(recipe['octaves'][0], amplitude=5)]),
                    ('hydraulic_erosion', dict(recipe['hydraulic_erosion'], iterations=0))]
        for name, value in variants:
            modified = copy.deepcopy(recipe); modified[name] = value
            h, c, _ = MAP.generate(modified)
            with self.subTest(control=name):
                self.assertFalse(np.array_equal(height, h) and np.array_equal(color, c))

    def test_water_palette_and_cold_snow(self):
        recipe = fixture(); recipe['octaves'] = []; recipe['hydraulic_erosion']['iterations'] = 0
        recipe['water']['level'] = 100
        _, rgb, stats = MAP.generate(recipe)
        self.assertEqual(stats['water_fraction'], 1)
        self.assertTrue(np.array_equal(rgb[0, 0], recipe['palette']['water_deep']))
        recipe['water']['level'] = -1; recipe['temperature']['base_celsius'] = -40
        _, snow, stats = MAP.generate(recipe)
        self.assertEqual(stats['water_fraction'], 0)
        self.assertGreater(float(snow.mean()), float(rgb.mean()))

    def test_periodic_authored_features(self):
        feature = {'x': 0, 'y': .3, 'radius_x': .7, 'radius_y': .1, 'angle_degrees': 37, 'height': 30}
        at_zero = MAP.feature_surface(64, [feature]); feature['x'] = 1
        self.assertTrue(np.allclose(at_zero, MAP.feature_surface(64, [feature]), atol=1e-12))
        feature['x'] = .5
        self.assertTrue(np.allclose(np.roll(at_zero, 32, axis=1), MAP.feature_surface(64, [feature]), atol=1e-12))

    def test_dry_climate_preserves_elevation_materials(self):
        recipe = fixture(); recipe['octaves'] = []; recipe['features'] = []
        recipe['hydraulic_erosion']['iterations'] = 0
        recipe['domain_warp'] = {'x_octaves': [], 'y_octaves': []}
        recipe['moisture'].update(base=0, water_gain=0, octaves=[])
        recipe['color_noise'] = []; recipe['lighting'].update(ambient=1, diffuse=0)
        recipe['materials'].update(upland_start=40, upland_end=80)
        recipe['palette'].update(sand=[200, 100, 50], upland=[50, 50, 100])
        recipe['base_height'] = 20
        self.assertTrue(np.all(MAP.generate(recipe)[1] == [200, 100, 50]))
        recipe['base_height'] = 80
        self.assertTrue(np.all(MAP.generate(recipe)[1] == [50, 50, 100]))

    def test_cold_climate_exposes_steep_rock(self):
        recipe = fixture(); recipe['octaves'] = []
        recipe['domain_warp'] = {'x_octaves': [], 'y_octaves': []}
        recipe['hydraulic_erosion']['iterations'] = 0
        recipe['features'] = [{'x': .5, 'y': .5, 'radius_x': .1, 'radius_y': .1,
                               'angle_degrees': 0, 'height': 30}]
        recipe['temperature'].update(base_celsius=-40, latitude_gradient=0, octaves=[])
        recipe['materials'].update(rock_slope_start=50, rock_slope_end=100)
        recipe['color_noise'] = []; recipe['lighting'].update(ambient=1, diffuse=0)
        rgb = MAP.generate(recipe)[1]
        self.assertTrue(np.any(np.all(rgb == recipe['palette']['rock'], axis=2)))
        self.assertTrue(np.any(np.all(rgb == recipe['palette']['snow'], axis=2)))
        self.assertTrue(np.array_equal(rgb[16, 16], recipe['palette']['snow']))

    def test_domain_warp_controls_and_periodicity(self):
        recipe = fixture(); recipe['resolution'] = 64
        recipe['octaves'] = []; recipe['hydraulic_erosion']['iterations'] = 0
        feature = {'x': .2, 'y': .4, 'radius_x': .1, 'radius_y': .07,
                   'angle_degrees': 25, 'height': 30}
        recipe['features'] = [feature]
        recipe['domain_warp'] = {'x_octaves': [], 'y_octaves': []}
        plain = MAP.generate(recipe)[0]
        y, x = np.mgrid[:64, :64] / 64
        self.assertTrue(np.array_equal(MAP.feature_surface(64, [feature]),
            MAP.feature_surface(64, [feature], (y, x))))
        for axis in ('x', 'y'):
            modified = copy.deepcopy(recipe)
            modified['domain_warp'][axis + '_octaves'] = [
                {'frequency': 4, 'amplitude': .025, 'seed_offset': 77,
                 'kind': 'smooth', 'ridge_power': 1}]
            first = MAP.generate(modified)[0]
            self.assertGreater(float(np.max(np.abs(first - plain))), .1)
            self.assertTrue(np.array_equal(first, MAP.generate(modified)[0]))
            wx = x + MAP.noise(64, modified['seed'] + 4000, modified['domain_warp']['x_octaves'])
            wy = y + MAP.noise(64, modified['seed'] + 5000, modified['domain_warp']['y_octaves'])
            self.assertTrue(np.allclose(MAP.feature_surface(64, [feature], (wy, wx)),
                MAP.feature_surface(64, [feature], (wy + 1, wx + 1)), atol=1e-11))

    def test_invalid_recipes_fail_before_outputs(self):
        for key, value in [('seed', True), ('resolution', 0), ('version', 1.0), ('version', 1),
                           ('height_range', [1, 1]), ('octaves', [{'unused': 1}]),
                           ('base_height', float('nan')),
                           ('domain_warp', {'unknown': []}),
                           ('domain_warp', {'x_octaves': [{'frequency': 4, 'amplitude': 1,
                              'seed_offset': 0, 'kind': 'smooth', 'ridge_power': 1}], 'y_octaves': []})]:
            recipe = fixture(); recipe[key] = value
            with self.subTest(field=key), self.assertRaises(ValueError): MAP.generate(recipe)
        with tempfile.TemporaryDirectory(dir='/tmp', prefix='wasm-fist-map-invalid-') as temporary:
            path = Path(temporary) / 'bad.json'; path.write_text('{"version":1,"version":1}')
            with self.assertRaises(ValueError): MAP.load_recipe(path)

    def test_erosion_outside_export_range_is_rejected(self):
        recipe = fixture()
        recipe['hydraulic_erosion'].update(time_step=.4, rainfall=.25,
            flow_rate=.6, sediment_capacity=12, erosion_rate=.35,
            deposition_rate=.1, evaporation=.05)
        with self.assertRaisesRegex(ValueError, 'height_range'):
            MAP.generate(recipe)

    def test_batch_complete_maps_and_preflight_rejection(self):
        with tempfile.TemporaryDirectory(dir='/tmp', prefix='wasm-fist-map-batch-') as temporary:
            directory = Path(temporary); recipes = directory / 'recipes'; recipes.mkdir()
            for name in ('first-map', 'second-map'):
                recipe = fixture(); recipe['name'] = name
                (recipes / (name + '.json')).write_text(json.dumps(recipe))
            batch = ROOT / 'assets/generator/generate_maps.py'
            output = directory / 'output'
            command = ['python3', str(batch), '--recipes-dir', str(recipes), '--output-dir', str(output)]
            result = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
            rows = [json.loads(row) for row in result.stdout.splitlines()]
            self.assertEqual(rows[-1], {'success': True, 'maps': ['first-map', 'second-map']})
            self.assertEqual(len(rows), 3)
            for name in rows[-1]['maps']:
                manifest = json.loads((output / (name + '-manifest.json')).read_text())
                self.assertEqual(len(manifest['outputs']), 2)
                for asset, expected in manifest['outputs'].items():
                    self.assertEqual(hashlib.sha256((output / asset).read_bytes()).hexdigest(), expected['sha256'])
            bad = fixture(); bad['name'] = 'wrong-name'
            (recipes / 'third-map.json').write_text(json.dumps(bad))
            rejected = directory / 'rejected'
            command[-1] = str(rejected)
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('match recipe filenames', result.stderr)
            self.assertFalse(rejected.exists(), 'Validate every recipe before generating any output')

    def test_cli_complete_png_and_manifest(self):
        with tempfile.TemporaryDirectory(dir='/tmp', prefix='wasm-fist-map-contract-') as temporary:
            directory = Path(temporary); recipe = fixture(); source = directory / 'recipe.json'
            source.write_text(json.dumps(recipe))
            command = ['python3', str(ROOT / 'assets/generator/mapgen.py'), str(source), '--output-dir', str(directory / 'output')]
            result = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
            manifest = json.loads(result.stdout); self.assertEqual(manifest['numpy_version'], np.__version__)
            self.assertEqual(manifest['recipe_sha256'], hashlib.sha256(source.read_bytes()).hexdigest())
            for name, expected in manifest['outputs'].items():
                data = (directory / 'output' / name).read_bytes()
                self.assertEqual(len(data), expected['bytes']); self.assertEqual(hashlib.sha256(data).hexdigest(), expected['sha256'])
                header, pixels = png(data); self.assertEqual(header[:2], (32, 32))
                channels = 2 if header[2] == 16 else 3
                self.assertEqual(len(pixels), 32 * (1 + 32 * channels))
                self.assertEqual(set(pixels[row * (1 + 32 * channels)] for row in range(32)), {0})
            repeated = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
            self.assertEqual(result.stdout, repeated.stdout)


if __name__ == '__main__':
    unittest.main()
