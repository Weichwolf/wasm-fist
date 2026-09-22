import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('compare_output', Path(__file__).resolve().parents[1] / 'tools/compare_output.py')
OUTPUT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(OUTPUT)


def wav(samples=b'\x12\x34' * 8, channels=1, rate=44100, bits=16, extra=b''):
    def chunk(name, payload):
        return name + struct.pack('<I', len(payload)) + payload + b'\0' * (len(payload) & 1)
    align = channels * (bits // 8)
    body = b'WAVE' + chunk(b'fmt ', struct.pack('<HHIIHH', 1, channels, rate, rate * align, align, bits))
    body += extra + chunk(b'data', samples)
    return b'RIFF' + struct.pack('<I', len(body)) + body


class OutputTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.paths = [Path(self.tmp.name) / name for name in ('a', 'b')]

    def compare(self, kind, a, b):
        for path, data in zip(self.paths, (a, b)):
            path.write_bytes(data)
        return OUTPUT.compare(kind, self.paths)

    def test_equal_pcm_with_different_container_metadata(self):
        metadata = b'JUNK' + struct.pack('<I', 3) + b'abc\0'
        self.assertIn('equal pcm', self.compare('pcm', wav(), wav(extra=metadata)))

    def test_changed_sample_reports_position(self):
        with self.assertRaisesRegex(ValueError, 'sample frame=3'):
            self.compare('pcm', wav(), wav(b'\x12\x34' * 3 + b'\x56\x78' + b'\x12\x34' * 4))

    def test_unequal_valid_pcm_lengths(self):
        with self.assertRaisesRegex(ValueError, 'length differs'):
            self.compare('pcm', wav(), wav(b'\x12\x34' * 9))

    def test_pcm_format_is_not_normalized(self):
        for change in ({'rate': 22050}, {'channels': 2}, {'bits': 8}):
            with self.subTest(change=change), self.assertRaisesRegex(ValueError, 'format differs'):
                self.compare('pcm', wav(), wav(**change))

    def test_invalid_pcm_never_passes_even_if_both_equal(self):
        complete = wav()
        for data in (b'', complete[:-1], complete + b'\0', wav(b''), wav(b'\0'),
                     complete[:40] + struct.pack('<I', 100) + complete[44:]):
            with self.subTest(data=data), self.assertRaises(ValueError):
                self.compare('pcm', data, data)

    def test_complete_frame_compares_pixels_not_header_comments(self):
        pixels = b'\x42' * (320 * 200 * 3)
        a = b'P6\n320 200\n255\n' + pixels
        b = b'P6\n# capture\n320 200\n255\n' + pixels
        self.assertIn('equal frame', self.compare('frame', a, b))
        with self.assertRaisesRegex(ValueError, 'x=0, y=0, RGB lane=1'):
            self.compare('frame', a, b'P6\n320 200\n255\n' + b'\x42\x43' + pixels[2:])

    def test_frame_length_and_dimensions_are_strict(self):
        complete = b'P6\n320 200\n255\n' + b'\0' * (320 * 200 * 3)
        for data in (b'', complete[:-1], complete + b'\0', b'P6\n1 1\n255\n\0\0\0'):
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                self.compare('frame', data, data)

    def test_missing_capture_fails(self):
        with self.assertRaises(OSError):
            OUTPUT.compare('frame', self.paths)


if __name__ == '__main__':
    unittest.main()
