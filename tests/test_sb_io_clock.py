import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, build_pic_probe, tool


class SoundBlasterIoClockTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        cls.original = build_pic_probe(cls.directory)
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion']
        sources = [str(ROOT / 'tests/sb_io_clock.c'), str(ROOT / 're_out/fist_vga.c'),
                   str(ROOT / 're_out/fist_sb.c'), str(ROOT / 're_out/fist_dos.c')]
        native, wasm = (str(cls.directory / name) for name in ('clock', 'clock.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-Wl,--gc-sections', '-lm',
                        '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags,
                      *sources, '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1',
                      '-o', wasm], [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append((target, run))

    def check_phase(self, start):
        case = json.loads((ROOT / 'tools/oracle/sb_io_clock_case.json').read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        code = bytes.fromhex(case['code_bytes']); at = case['code_offset']
        self.assertEqual(image[at:at+len(code)], code)
        rows = case['original_rows']
        self.assertEqual(len(rows), 68)
        script = self.directory / 'input.txt'
        script.write_text(''.join(f"{r['op']} {r['port']:x} {r['value']:x}\n" for r in rows))
        expected = subprocess.run([self.original, 'device-io', str(start), str(script)],
                                  check=True, capture_output=True, text=True, timeout=30).stdout
        if start == case['start_cycle_before_fetch']:
            self.assertEqual(expected, case['original_clock_output'])
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run, str(start), str(script)], capture_output=True, text=True,
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory/'device.wav')),
                                        timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual(result.stdout, expected + 'pumps 0\n')

    def test_reached_reset_high_phase_uses_original_io_cost_and_budget(self):
        case = json.loads((ROOT / 'tools/oracle/sb_io_clock_case.json').read_text())
        self.check_phase(case['start_cycle_before_fetch'])

    def test_sb_io_uses_original_thin_budget_and_tick_boundary_behavior(self):
        for index in (29899, 29905, 29969, 29998):
            with self.subTest(index=index):
                self.check_phase(525*30000 + index)
