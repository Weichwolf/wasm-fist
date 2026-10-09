#!/usr/bin/env python3
"""Pack verified owned generator PNGs into lossless, dependency-free C11 FMAP inputs."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[2]
HEIGHT_ENCODING = 'PNG unsigned 16-bit; world_height=min+sample/65535*(max-min)'


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f'Duplicate JSON field: {key}')
        result[key] = value
    return result


def read_json(path):
    def reject(value):
        raise ValueError(f'Nonfinite JSON number: {value}')
    return json.loads(path.read_text(), object_pairs_hook=unique_object, parse_constant=reject)


def checksum(data):
    return hashlib.sha256(data).hexdigest()


def png_plane(data, side, depth, color_type):
    """Strict generated PNG subset; complete CRC-checked unfiltered rows only."""
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('Invalid PNG signature')
    offset = 8
    state = 'header'
    streams = []
    while offset < len(data):
        if len(data) - offset < 12:
            raise ValueError('Truncated PNG chunk')
        size, = struct.unpack_from('>I', data, offset)
        end = offset + size + 12
        if end > len(data):
            raise ValueError('Truncated PNG payload')
        tag = data[offset + 4:offset + 8]
        payload = data[offset + 8:end - 4]
        crc, = struct.unpack_from('>I', data, end - 4)
        if zlib.crc32(tag + payload) != crc:
            raise ValueError('PNG CRC mismatch')
        if tag == b'IHDR' and state == 'header':
            expected = struct.pack('>IIBBBBB', side, side, depth, color_type, 0, 0, 0)
            if payload != expected:
                raise ValueError('PNG dimensions/encoding do not match the owned map')
            state = 'data'
        elif tag == b'IDAT' and state in ('data', 'stream'):
            streams.append(payload)
            state = 'stream'
        elif tag == b'IEND' and state == 'stream' and not payload and end == len(data):
            state = 'complete'
        else:
            raise ValueError('Unexpected PNG chunk/order/trailing bytes')
        offset = end
    if state != 'complete':
        raise ValueError('Incomplete PNG')
    stride = side * (2 if depth == 16 else 3)
    expected_size = side * (stride + 1)
    decoder = zlib.decompressobj()
    rows = decoder.decompress(b''.join(streams), expected_size + 1)
    if (len(rows) != expected_size or not decoder.eof or decoder.unused_data or
            decoder.unconsumed_tail):
        raise ValueError('Incomplete/oversized/trailing PNG deflate stream')
    if any(rows[row * (stride + 1)] != 0 for row in range(side)):
        raise ValueError('Owned generator PNGs must have unfiltered rows')
    pixels = b''.join(rows[row * (stride + 1) + 1:(row + 1) * (stride + 1)]
                      for row in range(side))
    if depth == 16:
        # PNG words are big endian; FMAP words have a fixed little-endian wire order.
        little = bytearray(len(pixels))
        little[0::2] = pixels[1::2]
        little[1::2] = pixels[0::2]
        pixels = bytes(little)
    return pixels


def pack_map(recipe_path, generated_dir):
    recipe_bytes = recipe_path.read_bytes()
    recipe = read_json(recipe_path)
    name = recipe.get('name')
    if (recipe.get('version') != 2 or not isinstance(name, str) or
            name != recipe_path.stem or not name or
            any(char not in 'abcdefghijklmnopqrstuvwxyz0123456789-' for char in name)):
        raise ValueError('Invalid recipe identity/version')
    side = recipe.get('resolution')
    if type(side) is not int or not 16 <= side <= 4096 or side & (side - 1):
        raise ValueError('Owned runtime maps require power-of-two side 16..4096')
    bounds = recipe.get('height_range')
    if not isinstance(bounds, list) or len(bounds) != 2:
        raise ValueError('Missing height range')
    minimum, maximum = bounds
    water = recipe.get('water', {}).get('level')
    for value in (minimum, maximum, water):
        if (type(value) not in (int, float) or not math.isfinite(value) or
                not -10000 <= value <= 20000):
            raise ValueError('Invalid elevation/water metadata')
    if minimum >= maximum:
        raise ValueError('Empty height range')
    manifest_path = generated_dir / f'{name}-manifest.json'
    manifest = read_json(manifest_path)
    if (type(manifest.get('version')) is not int or manifest.get('version') != 1 or manifest.get('name') != name or
            manifest.get('resolution') != side or manifest.get('height_range') != bounds or
            manifest.get('periodic') is not True or
            manifest.get('height_encoding') != HEIGHT_ENCODING or
            manifest.get('recipe_sha256') != checksum(recipe_bytes)):
        raise ValueError('Generator manifest does not match recipe/encoding')
    expected_names = {f'{name}-height.png', f'{name}-color.png'}
    if set(manifest.get('outputs', {})) != expected_names:
        raise ValueError('Incomplete generator outputs')
    planes = []
    for suffix, depth, color_type in (('height', 16, 0), ('color', 8, 2)):
        filename = f'{name}-{suffix}.png'
        data = (generated_dir / filename).read_bytes()
        if manifest['outputs'][filename] != {'bytes': len(data), 'sha256': checksum(data)}:
            raise ValueError('Generator output size/hash mismatch')
        planes.append(png_plane(data, side, depth, color_type))
    payload = b''.join(planes)
    prefix = struct.pack('<8s6I3dI', b'FISTMAP\0', 1, 64, side,
                         len(planes[0]), len(planes[1]), 0, minimum, maximum, water, 0)
    crc = zlib.crc32(payload, zlib.crc32(prefix))
    bundle = prefix + struct.pack('<I', crc) + payload
    metadata = {'version': 1, 'name': name, 'format': 'FMAP1', 'resolution': side,
                'recipe_sha256': checksum(recipe_bytes),
                'generator_manifest_sha256': checksum(manifest_path.read_bytes()),
                'packer_sha256': checksum(Path(__file__).read_bytes()),
                'outputs': {f'{name}.fmap': {'bytes': len(bundle), 'sha256': checksum(bundle)}}}
    return bundle, metadata


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipes-dir', type=Path, default=ROOT / 'assets/maps')
    parser.add_argument('--generated-dir', type=Path, default=ROOT / 'assets/generated/maps')
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    recipes = sorted(args.recipes_dir.glob('*.json'))
    if not recipes:
        parser.error('No map recipes')
    # Validate the complete batch before writing any output.
    packed = [pack_map(path, args.generated_dir) for path in recipes]
    args.output_dir.mkdir(parents=True, exist_ok=True)
    for bundle, metadata in packed:
        name = metadata['name']
        (args.output_dir / f'{name}.fmap').write_bytes(bundle)
        (args.output_dir / f'{name}-package.json').write_text(
            json.dumps(metadata, sort_keys=True, indent=2) + '\n')
    print(json.dumps({'success': True, 'maps': [metadata['name'] for _, metadata in packed]}))


if __name__ == '__main__':
    main()
