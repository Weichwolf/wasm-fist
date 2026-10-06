#!/usr/bin/env python3
"""Complete authored-resolution model assembly on native and WASM."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest
import zlib

from test_models import SUFFIXES, family_files, make_record, parse_record, stream_records

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
REVIEW = None


def top(y):
    return -128 if y == -128 else -y


def family(data):
    files = [[parse_record(record) for record in stream_records(data[suffix])]
             for suffix in SUFFIXES[1:]]
    faces = [None] * 32
    for records in files[1:]:
        for record in records:
            for face in record[0]:
                faces[face] = record
    assert all(face is not None for face in faces)
    return files[0][0][1], faces, files


def compose(base, faces, poses):
    selected = []
    all_pieces = []
    for part, (facing, variant) in enumerate(poses):
        record = faces[facing]
        orientation = 1 if facing < 16 else 0
        priority, pieces = record[2][orientation][part][variant]
        selected.append((priority, pieces, record))
        all_pieces += [(x, top(y), (base if use_base else record[1])[sprite])
                       for sprite, use_base, _, x, y in pieces]
    if not all_pieces:
        return (0, 0, 0, 0, b'')
    left = min(x for x, _, _ in all_pieces)
    upper = min(y for _, y, _ in all_pieces)
    right = max(x + sprite[1] for x, _, sprite in all_pieces)
    bottom = max(y + sprite[2] for _, y, sprite in all_pieces)
    width, height = right - left, bottom - upper
    pixels = bytearray(width * height)
    order = sorted(range(len(selected)), key=lambda index: selected[index][0])
    if ORACLE is not None:
        assert ORACLE.order([part[0] for part in selected]) == order
    for index in order:
        _, pieces, record = selected[index]
        for sprite, use_base, mirrored, x, y in pieces:
            actual = (base if use_base else record[1])[sprite]
            _, columns, rows, texels = actual
            expected = bytes(texels[(columns - column - 1 if mirrored else column) * rows + row]
                             for row in range(rows) for column in range(columns))
            if ORACLE is not None:
                assert ORACLE.sprite(actual, mirrored) == expected
            for row in range(rows):
                for column in range(columns):
                    value = expected[row * columns + column]
                    if value:
                        destination = (top(y) - upper + row) * width + (x - left + column)
                        pixels[destination] = value
    return left, upper, width, height, bytes(pixels)


def bitmap_line(bitmap):
    left, upper, width, height, pixels = bitmap
    return f'bitmap {left} {upper} {width} {height}' + ''.join(f' {byte:02x}' for byte in pixels) + '\n'


def expected(data):
    base, faces, files = family(data)
    count = len(faces[0][2][0])
    lines = [f'parts {count}\n']
    review = []
    cases = 0
    for facing in range(32):
        poses = [(facing, 0)] * count
        first = compose(base, faces, poses)
        lines += [f'case {facing} -1 0\n', bitmap_line(first)]
        cases += 1
        if facing in (0, 8, 16, 24):
            review.append(first)
        secondary = (facing + 16) % 32
        orientation = 1 if secondary < 16 else 0
        for part in range(count):
            for variant in range(len(faces[secondary][2][orientation][part])):
                poses = [(facing, 0)] * count
                poses[part] = (secondary, variant)
                lines += [f'case {facing} {part} {variant}\n', bitmap_line(compose(base, faces, poses))]
                cases += 1
    lines += ['retained\n', ''.join(f' {byte:02x}' for byte in data['MAL']) + '\n',
              bitmap_line(compose(base, faces, [(0, 0)] * count))]
    if ORACLE is not None:
        # Include complete unused and lower-LOD atlas sprites, both mirror paths.
        for records in files:
            for _, sprites, _ in records:
                for sprite in sprites:
                    for mirrored in (False, True):
                        ORACLE.sprite(sprite, mirrored)
    return ''.join(lines), review, cases


def custom_record(priorities=(150, 50), *, empty=False, maximum=False):
    # Two overlapping pieces whose column-major ordering/zero transparency
    # produce observably different outcomes under part/piece reorder or flips.
    record = bytearray(make_record(shared=True, variant_count=1))
    table, = struct.unpack_from('<H', record, 6)
    for part, priority in enumerate(priorities):
        variants, = struct.unpack_from('<H', record, table + part * 2)
        pointer, = struct.unpack_from('<H', record, variants)
        record[pointer + 1] = priority
        if empty:
            record[pointer] = 0
        else:
            struct.pack_into('<Hbb', record, pointer + 2, 0x8000 if part else 0, 0, 0)
            struct.pack_into('<Hbb', record, pointer + 6, 0x4001, 1, 0)
    if maximum:
        atlas, = struct.unpack_from('<H', record, 14)
        start, = struct.unpack_from('<H', record, atlas)
        # Retain 3x2 sprite 0, replace base/local sprite 1 by a 255x255 atlas.
        record = record[:start] + bytes((0, 1, 2, 3, 4, 5)) + bytes([211]) * (255 * 255)
        struct.pack_into('<HBB', record, atlas + 4, start + 6, 255, 255)
        struct.pack_into('<H', record, atlas + 8, len(record))
        struct.pack_into('<H', record, 0, len(record))
    return bytes(record)


class BitmapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-composition-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = pathlib.Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append(('native', [str(NATIVE_PROBE or BUILD / 'native/fist_model_bitmap_probe')]))
        if TARGET in ('all', 'wasm'):
            cls.commands.append(('wasm', ['node', str(BUILD / 'wasm/fist_model_bitmap_probe.js')]))
        cls.reviews = {}

    def run_family(self, data, name='TEST', directory=None, valid=True):
        if directory is None:
            directory = self.directory / name
            directory.mkdir(exist_ok=True)
            for suffix in SUFFIXES:
                path = directory / f'{name}.{suffix}'
                path.unlink(missing_ok=True)
                if suffix in data:
                    path.write_bytes(data[suffix])
        transcript, review, cases = expected(data) if valid else ('', [], 0)
        for target, command in self.commands:
            with self.subTest(target=target, name=name):
                result = subprocess.run([*command, 'all', str(directory), name], capture_output=True,
                                        text=True, timeout=45)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
                self.assertEqual(result.stdout, transcript)
                if REVIEW and name in ('M1_C', 'M3_C', 'T80_C', 'BMP_C'):
                    # Read the actual C bitmap output, independently of the Python reference.
                    actual = []
                    lines = result.stdout.splitlines()
                    for facing in (0, 8, 16, 24):
                        index = lines.index(f'case {facing} -1 0')
                        fields = lines[index + 1].split()
                        actual.append((*map(int, fields[1:5]), bytes.fromhex(' '.join(fields[5:]))))
                    self.reviews.setdefault(target, []).append((name, data['MAL'], actual))
        return cases

    def test_full_family_and_independent_owned_output(self):
        self.run_family(family_files())

    def test_stable_priority_piece_order_transparency_mirror_and_base_atlas(self):
        for priorities in ((150, 50), (50, 150), (150, 150), (0, 255), (255, 0)):
            record = custom_record(priorities)
            data = family_files()
            for suffix in ('M08', 'M16', 'M32'):
                data[suffix] = b''.join(record[:2] + struct.pack('<HH', face, face) + record[6:]
                                       for face in range(0, 64, 2)) + b'\0\0'
            self.run_family(data)

    def test_signed_extrema_empty_frames_and_maximum_sprite_dimensions(self):
        self.run_family(family_files())  # All four signed extrema in the fixture.
        for empty, maximum in ((True, False), (False, True)):
            record = custom_record(empty=empty, maximum=maximum)
            data = family_files()
            if maximum:
                base = bytearray(custom_record(maximum=True))
                struct.pack_into('<HH', base, 2, 65534, 65534)
                data['M00'] = bytes(base) + b'\0\0'
            for suffix in ('M08', 'M16', 'M32'):
                data[suffix] = b''.join(record[:2] + struct.pack('<HH', face, face) + record[6:]
                                       for face in range(0, 64, 2)) + b'\0\0'
            self.run_family(data)

    def test_missing_malformed_assets_and_selection_errors(self):
        for changed in ('MAL', 'M00', 'M08', 'M16', 'M32'):
            data = family_files()
            del data[changed]
            self.run_family(data, valid=False)
        data = family_files()
        data['M08'] = data['M08'][:-1]
        self.run_family(data, valid=False)

    def test_original_anchor_sign_and_byte_extrema(self):
        if ORACLE is None:
            # The fixture/composition gates remain useful without optional Unicorn.
            return
        def trunc(numerator, denominator):
            return (abs(numerator) // denominator) * (-1 if numerator < 0 else 1)
        for value in range(-128, 128):
            self.assertEqual(ORACLE.anchor(value, 0), (trunc((value + 1), 2), 0))
            self.assertEqual(ORACLE.anchor(0, value), (0, trunc(top(value) * 65536 + 32768, 65536)))

    def test_complete_original_families_directions_variants_and_all_sprite_texels(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned originals requested separately')
        manifest = json.loads((ROOT / 'tools/rewrite/model_originals.json').read_text())
        names = sorted(name[:-4] for name in manifest if name.endswith('.MAL'))
        total = 0
        for name in names:
            with self.subTest(model=name):
                data = {}
                for suffix in SUFFIXES:
                    path = ROOT / 'armoredfist/FISTDATA' / f'{name}.{suffix}'
                    value = path.read_bytes()
                    self.assertEqual(len(value), manifest[path.name]['size'])
                    self.assertEqual(hashlib.sha256(value).hexdigest(), manifest[path.name]['sha256'])
                    data[suffix] = value
                total += self.run_family(data, name, ROOT / 'armoredfist/FISTDATA')
                for suffix in SUFFIXES:
                    path = ROOT / 'armoredfist/FISTDATA' / f'{name}.{suffix}'
                    self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), manifest[path.name]['sha256'])
        self.assertEqual(len(names), 34)
        self.assertEqual(total, 6432)
        if REVIEW:
            self.write_reviews()
        print(f'complete composition: {len(names)} families, {total} full bitmaps', flush=True)

    def write_reviews(self):
        # Diagnostic contact sheets from actual C bitmap transcripts. PNG uses
        # only the standard library; no runtime/Pillow dependency is introduced.
        def chunk(kind, data):
            return (struct.pack('>I', len(data)) + kind + data +
                    struct.pack('>I', zlib.crc32(kind + data)))
        REVIEW.mkdir(parents=True, exist_ok=True)
        for target, rows in self.reviews.items():
            width, height = 640, 400
            background = bytes((40, 44, 50))
            image = bytearray(background * width * height)
            for row, (_, palette, frames) in enumerate(sorted(rows)):
                for column, (_, _, columns, lines, pixels) in enumerate(frames):
                    scale = min(3, 140 // columns, 90 // lines)
                    if scale < 1:
                        raise ValueError('Ground review tile does not fit without cropping')
                    left = column * 160 + (160 - columns * scale) // 2
                    upper = row * 100 + (100 - lines * scale) // 2
                    for y in range(lines * scale):
                        for x in range(columns * scale):
                            value = pixels[(y // scale) * columns + x // scale]
                            color = bytes((palette[value * 3 + channel] * 255 + 31) // 63
                                          for channel in range(3)) if value else background
                            offset = ((upper + y) * width + left + x) * 3
                            image[offset:offset + 3] = color
            scanlines = b''.join(b'\0' + image[row * width * 3:(row + 1) * width * 3]
                                 for row in range(height))
            png = (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>2I5B', width, height, 8, 2, 0, 0, 0)) +
                   chunk(b'IDAT', zlib.compress(scanlines)) + chunk(b'IEND', b''))
            (REVIEW / f'{target}.png').write_bytes(png)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--target', choices=['all', 'native', 'wasm'], default=TARGET)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--originals', action='store_true')
    parser.add_argument('--oracle', action='store_true')
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    BUILD, TARGET = args.build_root, args.target
    NATIVE_PROBE, ORIGINALS, REVIEW = args.native_probe, args.originals, args.review_dir
    if args.oracle:
        from original_sprite_oracle import OriginalSpriteOracle
        ORACLE = OriginalSpriteOracle()
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
