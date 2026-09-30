import subprocess
import tempfile
import unittest
from pathlib import Path
import struct


COMPARE = Path(__file__).resolve().parents[1] / 'tools/oracle/compare_sequences.py'


def write_capture(prefix, kind, records):
    body = bytearray(b'FISTSEQ1' + kind.encode())
    samples = 0
    for time, payload in records:
        body += kind.encode() + struct.pack('<Q', time) + payload
        if kind == 'A':
            samples += struct.unpack_from('<I', payload, 12)[0]
    body += b'E' + struct.pack('<QQ', len(records), samples)
    Path(str(prefix) + ('.frames' if kind == 'F' else '.pcm')).write_bytes(body)


def frame(pixel=0):
    return struct.pack('<II', 1, 1) + bytes(768) + bytes([pixel])


def pcm(sample=0):
    return struct.pack('<QIIhh', 0, 44100, 1, sample, sample)


class CompareSequencesTest(unittest.TestCase):
    def test_endpoint_is_required_matched_and_complete(self):
        with tempfile.TemporaryDirectory() as directory:
            left, right = (Path(directory) / name for name in ('left', 'right'))
            for prefix in (left, right):
                write_capture(prefix, 'F', [(200, frame())])
                write_capture(prefix, 'A', [(100, pcm())])
            command = ['python3', str(COMPARE), str(left), str(right), '--end-ms', '3000']
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            Path(str(left) + '.end').write_bytes(b'FISTEND1\n3000\n')
            result = subprocess.run(command[:-2], capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            for data in (b'FISTEND1\n4000\n', b'FISTEND1\n0\n', b'FISTEND1\n4294967296\n', b'FISTEND1\n3000\njunk'):
                Path(str(right) + '.end').write_bytes(data)
                result = subprocess.run(command, capture_output=True, text=True)
                self.assertEqual(result.returncode, 1, result.stdout)
            Path(str(right) + '.end').write_bytes(b'FISTEND1\n3000\n')
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout)

    def test_complete_and_first_chronological_difference(self):
        with tempfile.TemporaryDirectory() as directory:
            left, right = (Path(directory) / name for name in ('left', 'right'))
            write_capture(left, 'F', [(200, frame())])
            write_capture(right, 'F', [(200, frame())])
            write_capture(left, 'A', [(100, pcm())])
            write_capture(right, 'A', [(100, pcm())])
            command = ['python3', str(COMPARE), str(left), str(right)]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout)

            write_capture(right, 'F', [(200, frame(1))])
            write_capture(right, 'A', [(100, pcm(1))])
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            self.assertIn('A event 0 PCM byte 0', result.stdout)

            with Path(str(right) + '.pcm').open('ab') as stream:
                stream.write(b'junk')
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            self.assertIn('invalid completion record', result.stdout)


if __name__ == '__main__':
    unittest.main()
