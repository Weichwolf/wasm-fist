#!/usr/bin/env python3
"""Complete original directional sprite models, owned data and invalid-input behavior."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
SUFFIXES = ('MAL', 'M00', 'M08', 'M16', 'M32')


def make_record(faces=(0, 0), *, base=False, shared=False, parts=2, variant_count=2):
    data = bytearray(16)
    tables = []
    for orientation in range(0 if base else 2):
        if shared and orientation:
            tables.append(tables[0])
            continue
        table = len(data)
        tables.append(table)
        data += bytes(parts * 2)
        variants = []
        for part in range(parts):
            position = len(data)
            struct.pack_into('<H', data, table + part * 2, position)
            count = variant_count if part == 0 else 1
            variants.append((position, count))
            data += bytes(count * 2)
        for part, (position, count) in enumerate(variants):
            for variant in range(count):
                struct.pack_into('<H', data, position + variant * 2, len(data))
                pieces = [(0, -128, 127), (0xc001, 127, -128)] if variant == 0 else [(0x8000, -1, 0)]
                data += struct.pack('<H', ((50 + part * 100 + orientation * 20) << 8) | len(pieces))
                data += b''.join(struct.pack('<Hbb', *piece) for piece in pieces)
    atlas = len(data)
    sprites = [(3, 2, bytes([0, 1, 2, 3, 4, 5])), (1, 3, bytes([255, 128, 64]))]
    pixel_start = atlas + (len(sprites) + 1) * 4
    descriptors = bytearray()
    pixels = bytearray()
    for width, height, texels in sprites:
        descriptors += struct.pack('<HH', pixel_start + len(pixels), (height << 8) | width)
        pixels += texels
    descriptors += struct.pack('<HH', pixel_start + len(pixels), 0)
    data += descriptors + pixels
    first, second = (65534, 12345) if base else faces  # Base second-facing/header fields are ignored.
    a, b = (0, 0) if base else tables
    struct.pack_into('<8H', data, 0, len(data), first, second, a, b, 0xbead, 0x901a, atlas)
    return bytes(data)


def stream_records(data):
    records = []
    offset = 0
    while offset + 2 <= len(data):
        length, = struct.unpack_from('<H', data, offset)
        if not length:
            assert offset + 2 == len(data)
            return records
        records.append(data[offset:offset + length])
        offset += length
    raise ValueError('No complete EOF')


def parse_record(record):
    header = struct.unpack_from('<8H', record)
    base = header[1] == 65534
    facing = (65534, 65534) if base else tuple(((value + 32) % 64) // 2 for value in header[1:3])
    atlas = header[7]
    first_pixel, = struct.unpack_from('<H', record, atlas)
    sprites = []
    for offset in range(atlas, first_pixel - 4, 4):
        start, width, height = struct.unpack_from('<HBB', record, offset)
        sprites.append((start, width, height, record[start:start + width * height]))
    tables = []
    if not base:
        for table in header[3:5]:
            first, = struct.unpack_from('<H', record, table)
            starts = struct.unpack_from('<' + str((first - table) // 2) + 'H', record, table)
            first_list = min(struct.unpack_from('<H', record, start)[0] for start in starts)
            parts = []
            for start in starts:
                end = min([candidate for candidate in starts if candidate > start] + [first_list])
                variants = []
                for offset in range(start, end, 2):
                    pointer, = struct.unpack_from('<H', record, offset)
                    count, priority = struct.unpack_from('<BB', record, pointer)
                    pieces = []
                    for index in range(count):
                        reference, x, y = struct.unpack_from('<Hbb', record, pointer + 2 + index * 4)
                        pieces.append((reference & 0x3fff, int(bool(reference & 0x4000)),
                                       int(bool(reference & 0x8000)), x, y))
                    variants.append((priority, pieces))
                parts.append(variants)
            tables.append(parts)
    return facing, sprites, tables


def describe_file(data, base=None):
    records = stream_records(data)
    lines = [f'records {len(records)}']
    for ordinal, record in enumerate(records):
        facing, sprites, tables = parse_record(record)
        count = len(tables[0]) if tables else 0
        if ORACLE is not None:
            assert facing == ORACLE.facing(record), 'Original facing rotation differs'
            assert sprites == ORACLE.sprites(record, len(sprites)), 'Original complete sprite atlas differs'
            if tables:
                observed = ORACLE.record(record, base, tables)
                expected = []
                base_sprites = parse_record(base)[1]
                for orientation, parts in enumerate(tables):
                    for part, variants in enumerate(parts):
                        for variant, (priority, pieces) in enumerate(variants):
                            selected = []
                            for sprite, use_base, mirrored, x, y in pieces:
                                offset, width, height, pixels = (base_sprites if use_base else sprites)[sprite]
                                selected.append((offset, bool(use_base), bool(mirrored), x, y, width, height, pixels))
                            expected.append((orientation, part, variant, priority, selected))
                assert observed == expected, 'Original complete part/sprite selection differs'
        lines.append(f'record {ordinal} {facing[0]} {facing[1]} {count} {len(sprites)} {len(record)} ' + record.hex(' '))
        for sprite, (_, width, height, pixels) in enumerate(sprites):
            lines.append(f'sprite {ordinal} {sprite} {width} {height} ' + pixels.hex(' '))
        for orientation, parts in enumerate(tables):
            for part, variants in enumerate(parts):
                for variant, (priority, pieces) in enumerate(variants):
                    lines.append(f'variant {ordinal} {orientation} {part} {variant} {priority} {len(pieces)}')
                    for index, piece in enumerate(pieces):
                        lines.append(f'piece {ordinal} {orientation} {part} {variant} {index} ' + ' '.join(map(str, piece)))
    return '\n'.join(lines) + '\n'


def family_files():
    faces = [[(65534, 65534)], [(0, 0), (8, 56), (16, 48), (24, 40), (32, 32)],
             [(4, 60), (12, 52), (20, 44), (28, 36)],
             [(index, 64 - index) for index in range(2, 32, 4)]]
    result = {'MAL': bytes(index % 64 for index in range(768))}
    for file, directions in zip(SUFFIXES[1:], faces):
        result[file] = b''.join(make_record(pair, base=file == 'M00', shared=pair[0] == pair[1])
                                for pair in directions) + b'\0\0'
    return result


def describe_model(files):
    base = stream_records(files['M00'])[0]
    count = len(parse_record(stream_records(files['M08'])[0])[2][0])
    lines = [f'model {count} ' + files['MAL'].hex(' ')]
    faces = [None] * 32
    parsed_files = []
    for index, suffix in enumerate(SUFFIXES[1:]):
        lines.append(f'file {index}')
        lines.append(describe_file(files[suffix], base).rstrip('\n'))
        records = stream_records(files[suffix])
        parsed_files.append(records)
        if index:
            for ordinal, record in enumerate(records):
                for face in parse_record(record)[0]:
                    faces[face] = (index, ordinal)
    assert all(face is not None for face in faces)
    if ORACLE is not None:
        assert faces == ORACLE.family_faces(parsed_files), 'Original loader directional map differs'
    lines += [f'face {index} {face[0]} {face[1]}' for index, face in enumerate(faces)]
    return '\n'.join(lines) + '\n'


class ModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-model-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = pathlib.Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_model_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_model_probe.js')])

    def run_command(self, args, expected=None):
        for command in self.commands:
            with self.subTest(target=command[0]):
                result = subprocess.run([*command, *map(str, args)], capture_output=True,
                                        text=True, timeout=20)
                self.assertEqual(result.returncode, 0 if expected is not None else 1, result.stderr)
                self.assertEqual(result.stdout, expected or '')

    def run_file(self, data, valid=True, mode='file', base_count=2):
        path = self.directory / 'record.m08'
        path.write_bytes(data)
        expected = describe_file(data, make_record(base=True)) if valid and mode == 'file' else ('' if valid else None)
        self.run_command([mode, path, base_count], expected)

    def write_family(self, files, name='TEST'):
        directory = self.directory / name
        directory.mkdir(exist_ok=True)
        for suffix in SUFFIXES:
            path = directory / (name + '.' + suffix)
            path.unlink(missing_ok=True)
            if suffix in files:
                path.write_bytes(files[suffix])
        return directory

    def test_complete_model_sprites_variants_signed_offsets_and_all_directions(self):
        files = family_files()
        self.run_command(['model', self.write_family(files), 'test'], describe_model(files))
        self.run_file(make_record(shared=True) + b'\0\0')
        self.run_file(make_record(variant_count=128) + b'\0\0')
        self.run_file(make_record(base=True) + b'\0\0', base_count=0)
        self.run_file(b'\0\0')

    def test_every_truncated_prefix_and_bad_termination_fails_atomically(self):
        data = make_record() + make_record((8, 56)) + b'\0\0'
        self.run_file(data, mode='prefixes')
        for bad in (b'', data[:-1], data + b'x', b'\0\0x', b'\x0f\0' + bytes(13)):
            self.run_file(bad, valid=False)

    def test_zero_piece_lists_and_last_record_direction_replacement(self):
        data = bytearray(make_record())
        table, = struct.unpack_from('<H', data, 6)
        variants, = struct.unpack_from('<H', data, table)
        first_list, = struct.unpack_from('<H', data, variants)
        data[first_list] = 0
        self.run_file(bytes(data) + b'\0\0')
        files = family_files()
        files['M32'] = files['M32'][:-2] + make_record((2, 62), shared=True) + b'\0\0'
        self.run_command(['model', self.write_family(files), 'TEST'], describe_model(files))

    def test_bad_facing_tables_lists_atlas_dimensions_and_references_fail(self):
        original = make_record()
        header = struct.unpack_from('<8H', original)
        table = header[3]
        variant_table, = struct.unpack_from('<H', original, table)
        first_list, = struct.unpack_from('<H', original, variant_table)
        atlas = header[7]
        for offset, value in [(0, len(original) + 1), (2, 1), (4, 64), (6, 65535),
                              (14, len(original)), (table, table), (table, len(original)),
                              (variant_table, 65535), (first_list, 65535),
                              (first_list + 2, 2), (first_list + 2, 0x4002),
                              (atlas, atlas + 1), (atlas + 2, 65535),
                              (atlas + 8, len(original) - 1)]:
            with self.subTest(offset=offset, value=value):
                data = bytearray(original)
                struct.pack_into('<H', data, offset, value)
                self.run_file(bytes(data) + b'\0\0', valid=False)
        self.run_file(original + b'\0\0', valid=False, base_count=0)
        self.run_file(make_record(variant_count=129) + b'\0\0', valid=False)
        self.run_file(make_record(base=True) + original[:-1] + b'\0\0', valid=False)

    def test_missing_or_invalid_family_files_and_incomplete_directions_fail(self):
        original = family_files()
        for suffix in SUFFIXES:
            with self.subTest(missing=suffix):
                files = {key: value for key, value in original.items() if key != suffix}
                self.run_command(['model', self.write_family(files), 'TEST'])
        for suffix, data in [('MAL', bytes(767)), ('MAL', bytes([64]) * 768),
                             ('M00', b'\0\0'), ('M08', b'\0\0'),
                             ('M32', original['M32'][:-1]), ('M32', make_record((2, 62)) + b'\0\0'),
                             ('M32', make_record((2, 62), parts=1) + b'\0\0')]:
            with self.subTest(suffix=suffix):
                self.run_command(['model', self.write_family(dict(original, **{suffix: data})), 'TEST'])
        directory = self.write_family(original)
        for name in ['', '../TEST', 'TEST.M00', 'NINEBYTES', 'A/B', 'A B']:
            self.run_command(['model', directory, name])

    def test_all_pinned_original_model_families(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned original model coverage requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/model_originals.json').read_text())
        directory = ROOT / 'armoredfist/FISTDATA'
        paths = sorted(path for path in directory.iterdir() if path.suffix[1:] in SUFFIXES)
        self.assertEqual([path.name for path in paths], sorted(manifest))
        count = 0
        texels = 0
        for name in sorted(path.stem for path in directory.glob('*.MAL')):
            with self.subTest(model=name):
                files = {}
                for suffix in SUFFIXES:
                    path = directory / (name + '.' + suffix)
                    data = path.read_bytes()
                    self.assertEqual(len(data), manifest[path.name]['size'])
                    self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[path.name]['sha256'])
                    files[suffix] = data
                    if suffix != 'MAL':
                        records = stream_records(data)
                        count += len(records)
                        texels += sum(len(sprite[3]) for record in records for sprite in parse_record(record)[1])
                self.run_command(['model', directory, name], describe_model(files))
                for suffix in SUFFIXES:
                    path = directory / (name + '.' + suffix)
                    self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), manifest[path.name]['sha256'])
        self.assertEqual(count, 612)
        self.assertEqual(texels, 1679860)


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
        from original_model_oracle import OriginalModelOracle
        ORACLE = OriginalModelOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
