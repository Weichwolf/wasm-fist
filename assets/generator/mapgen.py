#!/usr/bin/env python3
"""Generate owned periodic terrain and RGB maps from explicit JSON recipes."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import zlib

import numpy as np


def keys(value, expected, label):
    if not isinstance(value, dict) or set(value) != set(expected):
        raise ValueError(f'{label}: expected fields {sorted(expected)}')


def number(value, low, high, label):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or not low <= value <= high:
        raise ValueError(f'{label}: expected finite number in [{low}, {high}]')


def integer(value, low, high, label):
    number(value, low, high, label)
    if not isinstance(value, int):
        raise ValueError(f'{label}: expected integer')


def validate_octaves(octaves):
    if not isinstance(octaves, list) or len(octaves) > 32:
        raise ValueError('octaves: expected list of at most 32 layers')
    for layer in octaves:
        keys(layer, ('frequency', 'amplitude', 'seed_offset', 'kind', 'ridge_power'), 'octave')
        integer(layer['frequency'], 1, 4096, 'frequency')
        number(layer['amplitude'], 0, 1024, 'amplitude')
        integer(layer['seed_offset'], 0, 2**32 - 1, 'seed_offset')
        number(layer['ridge_power'], .1, 16, 'ridge_power')
        if layer['kind'] not in ('smooth', 'ridge'):
            raise ValueError('octave kind: smooth or ridge required')


def validate(recipe):
    keys(recipe, ('version', 'name', 'seed', 'resolution', 'height_range', 'base_height',
                  'octaves', 'features', 'domain_warp', 'hydraulic_erosion', 'water', 'temperature',
                  'moisture', 'palette', 'materials', 'lighting', 'color_noise'), 'recipe')
    if type(recipe['version']) is not int or recipe['version'] != 2:
        raise ValueError('Unsupported recipe version')
    if not isinstance(recipe['name'], str) or not recipe['name'] or not recipe['name'].isascii() or not all(c.isalnum() or c in '_-' for c in recipe['name']):
        raise ValueError('name: ASCII letters, digits, underscore and dash required')
    integer(recipe['seed'], 0, 2**32 - 1, 'seed')
    integer(recipe['resolution'], 16, 4096, 'resolution')
    if not isinstance(recipe['height_range'], list) or len(recipe['height_range']) != 2:
        raise ValueError('height_range: two numbers required')
    lo, hi = recipe['height_range']
    number(lo, -10000, 10000, 'height minimum'); number(hi, lo + .0001, 20000, 'height maximum')
    number(recipe['base_height'], lo, hi, 'base_height')
    validate_octaves(recipe['octaves']); validate_octaves(recipe['color_noise'])
    if not isinstance(recipe['features'], list) or len(recipe['features']) > 2048:
        raise ValueError('features: expected list of at most 2048 authored hills/valleys')
    for feature in recipe['features']:
        keys(feature, ('x', 'y', 'radius_x', 'radius_y', 'angle_degrees', 'height'), 'feature')
        for axis in ('x', 'y'): number(feature[axis], 0, 1, axis)
        for axis in ('radius_x', 'radius_y'): number(feature[axis], .001, 1, axis)
        number(feature['angle_degrees'], -360, 360, 'angle_degrees')
        number(feature['height'], -10000, 10000, 'feature height')
    keys(recipe['domain_warp'], ('x_octaves', 'y_octaves'), 'domain_warp')
    for layers in recipe['domain_warp'].values():
        validate_octaves(layers)
        for layer in layers: number(layer['amplitude'], 0, .25, 'normalized warp amplitude')
    erosion = recipe['hydraulic_erosion']
    fields = ('iterations', 'time_step', 'rainfall', 'evaporation', 'flow_rate',
              'sediment_capacity', 'erosion_rate', 'deposition_rate', 'bedrock_height')
    keys(erosion, fields, 'hydraulic_erosion')
    integer(erosion['iterations'], 0, 2000, 'iterations')
    for field in fields[1:-1]: number(erosion[field], 0, 1 if field in ('time_step', 'evaporation', 'erosion_rate', 'deposition_rate') else 100, field)
    number(erosion['time_step'], .0001, 1, 'time_step')
    number(erosion['bedrock_height'], -10000, lo, 'bedrock_height')
    keys(recipe['water'], ('level', 'shore_width', 'depth_scale'), 'water')
    number(recipe['water']['level'], -10000, 20000, 'water level')
    for field in ('shore_width', 'depth_scale'): number(recipe['water'][field], .0001, 10000, field)
    keys(recipe['temperature'], ('base_celsius', 'lapse_per_height', 'latitude_gradient', 'octaves'), 'temperature')
    for field in ('base_celsius', 'latitude_gradient'): number(recipe['temperature'][field], -100, 100, field)
    number(recipe['temperature']['lapse_per_height'], 0, 10, 'lapse_per_height')
    validate_octaves(recipe['temperature']['octaves'])
    keys(recipe['moisture'], ('base', 'water_gain', 'octaves'), 'moisture')
    number(recipe['moisture']['base'], 0, 1, 'moisture base'); number(recipe['moisture']['water_gain'], 0, 1, 'water_gain')
    validate_octaves(recipe['moisture']['octaves'])
    keys(recipe['palette'], ('lowland', 'upland', 'rock', 'sand', 'snow', 'water_shallow', 'water_deep'), 'palette')
    for rgb in recipe['palette'].values():
        if not isinstance(rgb, list) or len(rgb) != 3: raise ValueError('Palette color: three components required')
        for channel in rgb: integer(channel, 0, 255, 'RGB')
    fields = ('upland_start', 'upland_end', 'rock_slope_start', 'rock_slope_end',
              'snow_temperature', 'snow_transition', 'dry_moisture', 'dry_transition')
    keys(recipe['materials'], fields, 'materials')
    for field in fields: number(recipe['materials'][field], -10000, 20000, field)
    if recipe['materials']['upland_end'] <= recipe['materials']['upland_start'] or recipe['materials']['rock_slope_end'] <= recipe['materials']['rock_slope_start'] or recipe['materials']['snow_transition'] <= 0 or recipe['materials']['dry_transition'] <= 0:
        raise ValueError('Material transitions must have positive widths')
    keys(recipe['lighting'], ('azimuth_degrees', 'elevation_degrees', 'ambient', 'diffuse', 'normal_scale'), 'lighting')
    light = recipe['lighting']; number(light['azimuth_degrees'], -360, 360, 'azimuth'); number(light['elevation_degrees'], 0, 90, 'elevation')
    for field in ('ambient', 'diffuse'): number(light[field], 0, 2, field)
    number(light['normal_scale'], .0001, 10000, 'normal_scale')


def noise(size, seed, octaves):
    y, x = np.mgrid[:size, :size] / size
    result = np.zeros((size, size))
    for layer in octaves:
        frequency = layer['frequency']
        random = np.random.Generator(np.random.PCG64(seed + layer['seed_offset']))
        angles = random.uniform(0, math.tau, (frequency, frequency))
        gradient_x, gradient_y = np.cos(angles), np.sin(angles)
        gx, gy = x * frequency, y * frequency
        ix, iy = gx.astype(int), gy.astype(int)
        local_x, local_y = gx - ix, gy - iy
        fx = local_x**3 * (local_x * (local_x * 6 - 15) + 10)
        fy = local_y**3 * (local_y * (local_y * 6 - 15) + 10)
        def dot(offset_x, offset_y):
            at = ((iy + offset_y) % frequency, (ix + offset_x) % frequency)
            return gradient_x[at] * (local_x - offset_x) + gradient_y[at] * (local_y - offset_y)
        a, b, c, d = dot(0, 0), dot(1, 0), dot(0, 1), dot(1, 1)
        value = (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy
        value *= math.sqrt(2)
        if layer['kind'] == 'ridge': value = 2 * (1 - np.abs(value))**layer['ridge_power'] - 1
        result += value * layer['amplitude']
    return result


def feature_surface(size, features, coordinates=None):
    if coordinates is None:
        y, x = np.mgrid[:size, :size] / size
    else:
        y, x = coordinates
    result = np.zeros((size, size))
    for feature in features:
        dx = (x - feature['x'] + .5) % 1 - .5
        dy = (y - feature['y'] + .5) % 1 - .5
        angle = math.radians(feature['angle_degrees']); c, s = math.cos(angle), math.sin(angle)
        # A positive periodic quadratic agrees with an oriented Gaussian near
        # its center. Unlike wrapping a rotated Euclidean ellipse, its cross
        # term has no discontinuity at a map edge.
        sx, sy = np.sin(math.pi * dx) / math.pi, np.sin(math.pi * dy) / math.pi
        ix, iy = 1 / feature['radius_x']**2, 1 / feature['radius_y']**2
        cross = np.sin(math.tau * dx) * np.sin(math.tau * dy) / (4 * math.pi**2)
        distance = (c*c*ix + s*s*iy) * sx*sx + (s*s*ix + c*c*iy) * sy*sy + 2*c*s*(ix-iy)*cross
        result += feature['height'] * np.exp(-.5 * distance)
    return result


def hydraulic(height, config):
    """Periodic conservative four-neighbor water/sediment transport; no pixel data input."""
    height = height.copy(); water = np.zeros_like(height); sediment = np.zeros_like(height)
    shifts = ((0, 1), (0, -1), (1, 1), (1, -1))
    dt = config['time_step']
    initial_mass = float(height.sum())
    for _ in range(config['iterations']):
        water += config['rainfall'] * dt
        surface = height + water
        differences = [np.maximum(surface - np.roll(surface, step, axis), 0) for axis, step in shifts]
        potential = sum(differences) * config['flow_rate'] * dt
        scale = np.minimum(water / np.maximum(potential, 1e-12), 1)
        flows = [d * config['flow_rate'] * dt * scale for d in differences]
        outgoing = sum(flows)
        concentration = sediment / np.maximum(water, 1e-12)
        sediments = [flow * concentration for flow in flows]
        # Subtract before adding incoming flux. Fully drained cells can round
        # a few ulps below zero; never carry negative water/sediment forward.
        if np.any(outgoing - water > 1e-10) or np.any(sum(sediments) - sediment > 1e-10):
            raise ValueError('Hydraulic transport exceeds available material')
        water = np.maximum(water - outgoing, 0) + sum(np.roll(flow, -step, axis) for flow, (axis, step) in zip(flows, shifts))
        sediment = np.maximum(sediment - sum(sediments), 0) + sum(np.roll(flow, -step, axis) for flow, (axis, step) in zip(sediments, shifts))
        slope = np.sqrt(sum((height - np.roll(height, 1, axis))**2 for axis in (0, 1)))
        capacity = config['sediment_capacity'] * outgoing * slope
        eroded = np.minimum(np.maximum(capacity - sediment, 0) * config['erosion_rate'] * dt,
                            np.maximum(height - config['bedrock_height'], 0))
        deposited = np.maximum(sediment - capacity, 0) * config['deposition_rate'] * dt
        height += deposited - eroded; sediment += eroded - deposited
        water *= 1 - config['evaporation'] * dt
    # Settle all remaining sediment: no material silently disappears on exit.
    height += sediment
    mass_error = float(height.sum()) - initial_mass
    if abs(mass_error) > max(1, abs(initial_mass)) * 1e-10 or not np.isfinite(height).all():
        raise ValueError('Hydraulic erosion lost material or produced invalid output')
    return height, water, mass_error


def blend(a, b, amount):
    return a * (1 - amount[..., None]) + b * amount[..., None]


def ramp(value, start, end):
    amount = np.clip((value - start) / (end - start), 0, 1)
    return amount * amount * (3 - 2 * amount)


def generate(recipe):
    validate(recipe); size = recipe['resolution']; seed = recipe['seed']
    lo, hi = recipe['height_range']
    y, x = np.mgrid[:size, :size] / size
    y += noise(size, seed + 5000, recipe['domain_warp']['y_octaves'])
    x += noise(size, seed + 4000, recipe['domain_warp']['x_octaves'])
    height = np.clip(recipe['base_height'] + noise(size, seed, recipe['octaves']) + feature_surface(size, recipe['features'], (y, x)), lo, hi)
    height, runoff, mass_error = hydraulic(height, recipe['hydraulic_erosion'])
    if height.min() < lo - 1e-8 or height.max() > hi + 1e-8:
        raise ValueError('Eroded terrain exceeds configured height_range; expand the recipe range')
    temperature = recipe['temperature']; moisture = recipe['moisture']; water = recipe['water']; materials = recipe['materials']
    y = np.arange(size)[:, None] / size
    temp = temperature['base_celsius'] - height * temperature['lapse_per_height'] + np.cos(y * math.tau) * temperature['latitude_gradient'] + noise(size, seed + 1000, temperature['octaves'])
    wet = np.clip(moisture['base'] + noise(size, seed + 2000, moisture['octaves']) + moisture['water_gain'] * np.exp(-np.maximum(height - water['level'], 0) / water['shore_width']), 0, 1)
    dx = (np.roll(height, -1, 1) - np.roll(height, 1, 1)) * size / 2
    dy = (np.roll(height, -1, 0) - np.roll(height, 1, 0)) * size / 2
    slope = np.hypot(dx, dy); palette = {key: np.array(value, float) for key, value in recipe['palette'].items()}
    rgb = blend(palette['lowland'], palette['upland'], ramp(height, materials['upland_start'], materials['upland_end']))
    rgb = blend(rgb, palette['sand'], 1 - ramp(wet, materials['dry_moisture'], materials['dry_moisture'] + materials['dry_transition']))
    rgb = blend(rgb, palette['rock'], ramp(slope, materials['rock_slope_start'], materials['rock_slope_end']))
    rgb = blend(rgb, palette['snow'], 1 - ramp(temp, materials['snow_temperature'], materials['snow_temperature'] + materials['snow_transition']))
    rgb *= 1 + noise(size, seed + 3000, recipe['color_noise'])[..., None]
    light = recipe['lighting']; az, el = math.radians(light['azimuth_degrees']), math.radians(light['elevation_degrees'])
    nx, ny = -dx / light['normal_scale'], -dy / light['normal_scale']
    normal_length = np.sqrt(nx*nx + ny*ny + 1)
    illumination = np.maximum((nx * math.cos(el) * math.cos(az) + ny * math.cos(el) * math.sin(az) + math.sin(el)) / normal_length, 0)
    rgb *= (light['ambient'] + light['diffuse'] * illumination)[..., None]
    water_rgb = blend(palette['water_shallow'], palette['water_deep'], np.clip((water['level'] - height) / water['depth_scale'], 0, 1))
    rgb = np.where((height <= water['level'])[..., None], water_rgb, rgb)
    return height, np.rint(np.clip(rgb, 0, 255)).astype(np.uint8), {'mass_error': mass_error, 'height_min': float(height.min()), 'height_max': float(height.max()), 'temperature_min': float(temp.min()), 'temperature_max': float(temp.max()), 'water_fraction': float(np.mean(height <= water['level'])), 'remaining_water': float(runoff.sum())}


def png_bytes(pixels):
    height, width = pixels.shape[:2]; gray = pixels.ndim == 2
    depth = 16 if gray else 8; color = 0 if gray else 2
    pixels = np.ascontiguousarray(pixels, dtype='>u2' if gray else np.uint8)
    rows = b''.join(b'\0' + row.tobytes() for row in pixels)
    def chunk(tag, data): return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data))
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, depth, color, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(rows, 9)) + chunk(b'IEND', b'')


def unique_object(pairs):
    value = {}
    for key, item in pairs:
        if key in value: raise ValueError(f'Duplicate JSON key: {key}')
        value[key] = item
    return value


def load_recipe(path):
    return json.loads(path.read_text(), object_pairs_hook=unique_object,
                      parse_constant=lambda value: (_ for _ in ()).throw(ValueError(f'Nonfinite JSON: {value}')))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('recipe', type=Path); parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args(); recipe = load_recipe(args.recipe)
    height, colors, stats = generate(recipe)
    lo, hi = recipe['height_range']; encoded = np.rint((height - lo) / (hi - lo) * 65535).astype(np.uint16)
    outputs = {recipe['name'] + '-height.png': png_bytes(encoded), recipe['name'] + '-color.png': png_bytes(colors)}
    manifest = {'version': 1, 'name': recipe['name'], 'generator_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(), 'recipe_sha256': hashlib.sha256(args.recipe.read_bytes()).hexdigest(), 'numpy_version': np.__version__, 'resolution': recipe['resolution'], 'height_range': [lo, hi], 'height_encoding': 'PNG unsigned 16-bit; world_height=min+sample/65535*(max-min)', 'periodic': True, 'statistics': stats, 'outputs': {name: {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()} for name, data in outputs.items()}}
    args.output_dir.mkdir(parents=True, exist_ok=True)
    for name, data in outputs.items(): (args.output_dir / name).write_bytes(data)
    (args.output_dir / (recipe['name'] + '-manifest.json')).write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n')
    print(json.dumps(manifest, sort_keys=True))


if __name__ == '__main__':
    main()
