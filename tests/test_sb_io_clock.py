import hashlib
import json
import os
from pathlib import Path
import subprocess
import struct
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
        cls.original_sb = build_pic_probe(cls.directory, sb_events=True)
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion']
        sources = [str(ROOT / 'tests/sb_io_clock.c'), str(ROOT / 're_out/fist_vga.c'), str(ROOT / 're_out/fist_pic.c'),
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

    def check_phase(self, start, reset=False):
        filename = 'sb_reset_clock_case.json' if reset else 'sb_io_clock_case.json'
        case = json.loads((ROOT / 'tools/oracle' / filename).read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        code = bytes.fromhex(case['code_bytes']); at = case['code_offset']
        self.assertEqual(image[at:at+len(code)], code)
        rows = case['original_rows']
        self.assertEqual(len(rows), 218 if reset else 68)
        script = self.directory / 'input.txt'
        script.write_text(''.join(f"{r['op']} {r['port']:x} {r['value']:x}\n" for r in rows))
        expected = subprocess.run([self.original_sb if reset else self.original,
                                   'device-sb' if reset else 'device-io', str(start), str(script)],
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

    def test_complete_reached_reset_polling_matches_original_device_and_pic(self):
        case = json.loads((ROOT / 'tools/oracle/sb_reset_clock_case.json').read_text())
        self.check_phase(case['start_cycle_before_fetch'], reset=True)

    def check_script(self, start, script):
        path = self.directory / 'events.txt'
        path.write_text(script)
        expected = subprocess.run([self.original_sb, 'device-sb', str(start), str(path)],
                                  check=True, capture_output=True, text=True, timeout=30).stdout
        for target, run in self.commands:
            with self.subTest(target=target, start=start):
                result = subprocess.run([*run, str(start), str(path)], capture_output=True, text=True,
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory/'events.wav')),
                                        timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected + 'pumps 0\n')
        return [line for line in expected.splitlines() if line.startswith('event ')]

    def test_reset_bit_zero_cancels_pending_ready_and_preserves_empty_data_latch(self):
        script = ('w 226 3\nr 22c ff\nw 226 2\nt 0 190\nw 226 5\n'
                  'r 22e 7f\nt 0 320\nr 22e 7f\nw 226 4\nr 22c ff\n'
                  't 0 320\nr 22e ff\nr 22a aa\nr 22e 7f\nr 22a aa\n')
        # DSP write status advances its original busy counter only in NORMAL.
        script += ''.join(f"r 22c {('ff' if i & 8 else '7f')}\n" for i in range(1, 17))
        for index in (100, 28900, 29998):
            self.check_script(525*30000 + index, script)

    def test_pic_event_order_specific_cancel_and_callback_relative_rearming(self):
        def add(handler, delay, value):
            bits = struct.unpack('<I', struct.pack('<f', delay))[0]
            return f'a {handler:x} {bits:x} {value:x}\n'
        scripts = [
            (add(2, .020, 100) + 't 0 3e8\n', [2, 0, 0], [100, 100, 101]),
            (add(0, 2.125, 10) + add(0, 2.125, 11) + 'k 0 a\nt 0 186a0\n', [0], [11]),
            (add(0, 2.125, 12) + add(0, 2.125, 13) + 'd 0 0\nt 0 186a0\n', [], []),
            (add(1, .020, 4) + 't 0 bb8\n', [1]*5, [4, 3, 2, 1, 0]),
            (add(0, 10.75, 20) + 't 0 55730\n', [0], [20]),
        ]
        for index in (100, 29969, 29998):
            for script, handlers, values in scripts:
                with self.subTest(index=index, script=script):
                    events = self.check_script(525*30000 + index, script)
                    self.assertEqual([int(e.split()[1]) for e in events], handlers)
                    self.assertEqual([int(e.split()[2]) for e in events], values)

    def test_pic_queue_capacity_matches_original_without_losing_accepted_events(self):
        bits = struct.unpack('<I', struct.pack('<f', 1.0))[0]
        script = ''.join(f'a 0 {bits:x} {value:x}\n' for value in range(513))
        script += 't 0 c350\n'
        events = self.check_script(525*30000 + 100, script)
        self.assertEqual([int(e.split()[2]) for e in events], list(range(512)))
