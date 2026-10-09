#!/usr/bin/env python3
"""Original-free actual thread starts, four-sample coverage and context lifecycle."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
WIDTH, HEIGHT = 640, 360
TRIANGLES = (
    ((24.25, 24.25), (280.25, 24.25), (24.25, 216.25)),
    ((400.25, 100), (400.5, 100), (400.375, 100.25)),
    ((460, 180.5), (460.25, 180.5), (460.125, 180.75)),
)


def inside(x, y, triangle):
    """Independent oriented geometry with the documented top-left boundary rule."""
    for start, end in zip(triangle, triangle[1:] + triangle[:1]):
        dx, dy = end[0] - start[0], end[1] - start[1]
        area = dx * (y - start[1]) - dy * (x - start[0])
        if area < 0 or (area == 0 and not (dy < 0 or (dy == 0 and dx < 0))):
            return False
    return True


def expected_frame(samples):
    positions = ((.375, .125), (.875, .375), (.125, .625), (.625, .875)) if samples else ((.5, .5),)
    # Background remains opaque for every sample. Geometry is constant white.
    pixels = bytearray(bytes((0, 0, 0, 255)) * WIDTH * HEIGHT)
    for triangle in TRIANGLES:
        for y in range(int(min(point[1] for point in triangle)),
                       int(max(point[1] for point in triangle)) + 1):
            for x in range(int(min(point[0] for point in triangle)),
                           int(max(point[0] for point in triangle)) + 1):
                count = sum(inside(x + sx, y + sy, triangle) for sx, sy in positions)
                gray = (255 * count + len(positions) // 2) // len(positions)
                offset = (y * WIDTH + x) * 4
                pixels[offset:offset + 4] = bytes((gray, gray, gray, 255))
    return bytes(pixels)


class RenderProfileTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-render-profile-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append(('native', [str(NATIVE_PROBE or BUILD / 'native/fist_render_profile_probe')]))
        if TARGET in ('all', 'wasm'):
            cls.commands.append(('wasm', ['node', str(ROOT / 'tests/check_render_profile_wasm.cjs'),
                                          str(BUILD / 'wasm/fist_render_profile_probe.js')]))
        if not cls.commands:
            raise AssertionError('No requested runtime target')

    def run_probe(self, command, args, success=True):
        result = subprocess.run([*command, *args], capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0 if success else 1, result.stderr)
        if not success:
            self.assertEqual(result.stdout, '', 'Failure must not report success')
            return None
        return json.loads(result.stdout) if result.stdout else None

    def render(self, name, command, samples):
        output = self.directory / f'{name}-{samples}.rgba'
        output.unlink(missing_ok=True)
        profile = self.run_probe(command, [str(samples), str(output)])
        self.assertEqual(profile, dict(width=WIDTH, height=HEIGHT, sample_buffers=int(samples != 0),
                                       samples=samples, multisample_enabled=1,
                                       configured_helpers=3, started_helpers=3, total_threads=4))
        pixels = output.read_bytes()
        self.assertEqual(len(pixels), WIDTH * HEIGHT * 4)
        return pixels

    def test_complete_single_sample_and_four_sample_frames(self):
        for samples in (0, 4):
            expected = expected_frame(samples)
            for name, command in self.commands:
                with self.subTest(target=name, samples=samples):
                    pixels = self.render(name, command, samples)
                    self.assertEqual(pixels, expected, 'Every RGBA byte must match geometric coverage')
                    if samples:
                        self.assertEqual(set(pixels[::4]), {0, 64, 128, 191, 255})
                    else:
                        self.assertEqual(set(pixels[::4]), {0, 255})
                    for x, y in ((400, 100), (460, 180)):
                        offset = (y * WIDTH + x) * 4
                        gray = 64 if samples else 0
                        self.assertEqual(pixels[offset:offset + 4], bytes((gray, gray, gray, 255)))

    def test_repeated_contexts_sizes_samples_and_frame_reads(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                self.assertEqual(self.run_probe(command, ['cycles']), {'cycles': 12})

    def test_invalid_dimensions_samples_and_atomic_profile_output(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                self.assertIsNone(self.run_probe(command, ['contracts']))

    def test_all_thread_starts_fail_then_actual_recovery(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                result = self.run_probe(command, ['failure-all'])
                self.assertGreaterEqual(result['failed_starts'], 3)
                self.assertEqual(result['partial_starts'], 0)
                self.assertEqual(result['recovered_helpers'], 3)

    def test_partial_thread_starts_fail_then_actual_recovery(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                result = self.run_probe(command, ['failure-partial'])
                self.assertGreater(result['failed_starts'], 0)
                self.assertGreater(result['partial_starts'], 0)
                self.assertEqual(result['recovered_helpers'], 3)

    def test_invalid_command_is_failure(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                self.run_probe(command, ['unknown'], success=False)
                self.run_probe(command, ['4'], success=False)

    def test_missing_output_directory_is_failure(self):
        for name, command in self.commands:
            with self.subTest(target=name):
                path = self.directory / 'absent' / 'frame.rgba'
                args = ['4', str(path)]
                if name == 'wasm':
                    args.append('/absent/frame.rgba')
                self.run_probe(command, args, success=False)
                self.assertFalse(path.exists())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root', type=Path, default=BUILD)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default=TARGET)
    parser.add_argument('--native-probe', type=Path)
    args, remaining = parser.parse_known_args()
    BUILD, TARGET, NATIVE_PROBE = args.build_root, args.target, args.native_probe
    unittest.main(argv=[__file__, *remaining])
