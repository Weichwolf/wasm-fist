import importlib.util
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('sequence_format', Path(__file__).resolve().parents[1] / 'tools/oracle/sequence_format.py')
FORMAT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(FORMAT)


def frame(time, pixel):
    palette = bytearray(768)
    palette[pixel * 3] = pixel
    return b'F' + struct.pack('<QII', time, 1, 1) + palette + bytes((pixel,))


def audio(time, position, frames):
    return b'A' + struct.pack('<QQII', time, position, 44100, frames) + bytes(frames * 4)


def sequence(kind, events, records, samples):
    return b'FISTSEQ1' + kind.encode() + events + b'E' + struct.pack('<QQ', records, samples)


class SequenceFormatTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / 'stream'

    def check(self, kind, data):
        self.path.write_bytes(data)
        return FORMAT.validate(self.path, kind)

    def test_complete_frame_sequence(self):
        result = self.check('F', sequence('F', frame(3, 1) + frame(4, 1), 2, 0))
        self.assertEqual((result['records'], result['first'], result['last']), (2, 3, 4))
        output = Path(self.tmp.name) / 'last.ppm'
        self.assertEqual(FORMAT.extract_frame(self.path, -1, output), (4, 1, 1))
        self.assertEqual(output.read_bytes(), b'P6\n1 1\n255\n\1\0\0')

    def test_complete_audio_contiguous_across_chunks(self):
        result = self.check('A', sequence('A', audio(1, 0, 2) + audio(2, 2, 3), 2, 5))
        self.assertEqual(result['samples'], 5)

    def test_incomplete_or_extra_record_fails(self):
        full = sequence('F', frame(3, 1), 1, 0)
        for data in (full[:-1], full + b'x', full[:-17], sequence('F', frame(3, 1), 2, 0)):
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                self.check('F', data)

    def test_missing_audio_samples_fail(self):
        for data in (sequence('A', audio(1, 0, 2) + audio(2, 3, 1), 2, 3),
                     sequence('A', audio(1, 0, 2)[:-1], 1, 2)):
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                self.check('A', data)

    def test_reordered_events_fail(self):
        with self.assertRaisesRegex(ValueError, 'non-monotonic'):
            self.check('F', sequence('F', frame(4, 1) + frame(3, 2), 2, 0))

    def test_empty_or_wrong_kind_fails(self):
        for kind, data in (('F', b''), ('A', sequence('F', frame(1, 1), 1, 0)),
                           ('F', sequence('F', b'', 0, 0))):
            with self.subTest(kind=kind, length=len(data)), self.assertRaises(ValueError):
                self.check(kind, data)

    def test_frame_comparator_reports_first_difference(self):
        other = Path(self.tmp.name) / 'other'
        script = Path(__file__).resolve().parents[1] / 'tools/oracle/compare_frames.py'
        self.path.write_bytes(sequence('F', frame(3, 1) + frame(4, 1), 2, 0))
        for data, expected in ((sequence('F', frame(3, 1) + frame(4, 1), 2, 0), 'equal 2'),
                               (sequence('F', frame(3, 1) + frame(5, 1), 2, 0), 'time/dimensions'),
                               (sequence('F', frame(3, 1) + frame(4, 2), 2, 0), 'palette entry=1 lane=0'),
                               (sequence('F', frame(3, 1), 1, 0), 'ends before')):
            with self.subTest(expected=expected):
                other.write_bytes(data)
                result = subprocess.run([sys.executable, str(script), str(self.path), str(other)],
                                        capture_output=True, text=True)
                self.assertIn(expected, result.stdout + result.stderr)
                self.assertEqual(result.returncode, 0 if expected.startswith('equal') else 1)

    def test_unreferenced_palette_entries_still_fail(self):
        other = Path(self.tmp.name) / 'other'
        script = Path(__file__).resolve().parents[1] / 'tools/oracle/compare_frames.py'
        a = bytearray(sequence('F', frame(3, 0), 1, 0))
        b = bytearray(a)
        b[9 + 1 + 16 + 3] = 255
        self.path.write_bytes(a)
        other.write_bytes(b)
        result = subprocess.run([sys.executable, str(script), str(self.path), str(other)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('palette entry=1 lane=0', result.stderr)

    def test_equal_rgb_with_different_indices_still_fails(self):
        other = Path(self.tmp.name) / 'other'
        script = Path(__file__).resolve().parents[1] / 'tools/oracle/compare_frames.py'
        a = bytearray(sequence('F', frame(3, 0), 1, 0))
        b = bytearray(a)
        b[9 + 1 + 16 + 768] = 1
        self.path.write_bytes(a)
        other.write_bytes(b)
        result = subprocess.run([sys.executable, str(script), str(self.path), str(other)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('pixel x=0 y=0, indices 0 != 1', result.stderr)


if __name__ == '__main__':
    unittest.main()
