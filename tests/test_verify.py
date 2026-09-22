import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]


class VerifyTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name) / 'repo'
        for folder in ('tools', 'ref', 'armoredfist', 're_out'):
            (self.root / folder).mkdir(parents=True)
        for name in ('verify.sh', 'compare_output.py'):
            shutil.copyfile(ROOT / 'tools' / name, self.root / 'tools' / name)
        (self.root / 're_out/fist_image.bin').write_bytes(b'image')
        def chunk(name, data):
            return struct.pack('>I', len(data)) + name + data + struct.pack('>I', zlib.crc32(name + data))
        png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 320, 200, 8, 2, 0, 0, 0))
        png += chunk(b'IDAT', zlib.compress((b'\0' + b'\x42' * 960) * 200)) + chunk(b'IEND', b'')
        self.reference = self.root / 'ref/main_menu_native320.png'
        self.reference.write_bytes(png)
        self.fake = Path(self.tmp.name) / 'capture'
        self.fake.write_text('''#!/usr/bin/env python3
import os
from pathlib import Path
import sys
import wave
mode = os.environ.get('CAPTURE_MODE_WASM' if len(sys.argv) > 1 else 'CAPTURE_MODE', 'frame')
if mode != 'missing':
    out = os.environ.get('FIST_MISSFB') or os.environ.get('FIST_FBDUMP')
    if out:
        data = b'P6\\n320 200\\n255\\n' + b'\\x42' * (320 * 200 * 3)
        if mode == 'truncated': data = data[:-1]
        if mode == 'extra': data += b'x'
        if mode == 'different': data = data[:-1] + b'\\x43'
        Path(out).write_bytes(data)
    audio = os.environ.get('FIST_AUDIO_WAV')
    if audio:
        with wave.open(audio, 'wb') as f:
            f.setparams((1, 2, 44100, 0, 'NONE', 'not compressed'))
            f.writeframes(b'\\x12\\x34' * (9 if mode == 'extra' else 8))
sys.exit(7 if mode == 'failed' else 0)
''')
        self.fake.chmod(0o755)

    def run_flow(self, flow, mode, target='native', wasm_mode='frame'):
        env = dict(os.environ, NATIVE=str(self.fake), NODE=str(self.fake), OUTJS='wasm',
                   FIST_FLOWS=flow, CAPTURE_MODE=mode, CAPTURE_MODE_WASM=wasm_mode)
        env.pop('FIST_VERIFY_OUT', None)
        return subprocess.run(['bash', str(self.root / 'tools/verify.sh'), target], env=env,
                              capture_output=True, text=True, timeout=15)

    def test_empty_selection_fails(self):
        self.assertNotEqual(self.run_flow('^not-a-real-flow$', 'frame').returncode, 0)

    def test_missing_frame_fails(self):
        self.assertNotEqual(self.run_flow('^mainmenu$', 'missing').returncode, 0)

    def test_failed_producer_with_frame_fails(self):
        self.assertNotEqual(self.run_flow('^terrain-azer1$', 'failed').returncode, 0)

    def test_truncated_frame_fails(self):
        self.assertNotEqual(self.run_flow('^terrain-azer1$', 'truncated').returncode, 0)

    def test_valid_frame_passes(self):
        result = self.run_flow('^terrain-azer1$', 'frame')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_missing_required_reference_fails(self):
        self.reference.unlink()
        self.assertNotEqual(self.run_flow('^mainmenu$', 'frame').returncode, 0)

    def test_editor_roundtrip_marker_is_not_an_image_reference(self):
        data = self.root / 'armoredfist/FISTDATA'
        data.mkdir()
        (data / 'AZER1.FSG').write_bytes(b'DCBS\x02\x00\x00\x00')
        result = self.run_flow('^editor-fsg-roundtrip$', 'frame', 'both')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_invalid_target_fails(self):
        self.assertNotEqual(self.run_flow('^terrain-azer1$', 'frame', 'bogus').returncode, 0)

    def test_cross_target_suffix_or_pixel_difference_fails(self):
        for mode in ('extra', 'different', 'missing', 'failed'):
            with self.subTest(mode=mode):
                self.assertNotEqual(self.run_flow('^terrain-azer1$', 'frame', 'both', mode).returncode, 0)

    def test_cross_target_equal_frame_passes(self):
        result = self.run_flow('^mainmenu$', 'frame', 'both')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_audio_length_and_producer_status_are_checked(self):
        for mode in ('extra', 'missing', 'failed'):
            with self.subTest(mode=mode):
                self.assertNotEqual(self.run_flow('^audio-opl-init$', 'frame', 'both', mode).returncode, 0)
        result = self.run_flow('^audio-opl-init$', 'frame', 'both')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
