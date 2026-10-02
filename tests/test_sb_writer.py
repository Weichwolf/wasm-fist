import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, build_pic_probe, patched_unit, tool


class SoundBlasterWriterTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        patched_unit(cls.directory, 'fist_ext.c')
        wrapper = cls.directory / 'writer.c'
        # Instrument I/O boundaries while retaining the complete actual module.
        wrapper.write_text('#define in fist_writer_test_in\n'
                           '#define out fist_writer_test_out\n#include "fist_ext.c"\n')
        cls.original = build_pic_probe(cls.directory, sb_events=True)
        cls.proof = json.loads((ROOT / 'tools/oracle/sb_writer_132f_cases.json').read_text())
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion', '-Wno-incompatible-pointer-types',
                 '-Wno-implicit-function-declaration', '-Wno-return-mismatch', '-w']
        sources = [str(ROOT / 'tests/sb_writer.c'), str(wrapper),
                   *(str(ROOT / 're_out' / name) for name in ('fist_vga.c', 'fist_pic.c', 'fist_sb.c', 'fist_dos.c'))]
        native, wasm = (str(cls.directory / name) for name in ('writer', 'writer.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-Wl,--gc-sections', '-lm',
                        '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags,
                      *sources, '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1',
                      '-o', wasm], [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout + result.stderr)
            cls.commands.append((target, run))

    def check_case(self, original, index, start=None, eax=None):
        case = original['cases'][index]
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), self.proof['image_sha256'])
        code = bytes.fromhex(self.proof['code_bytes']); at = self.proof['code_offset']
        self.assertEqual(image[at:at+len(code)], code)
        if start is None:
            start = case['start_cycle_before_fetch']
        if eax is None:
            eax = case['eax']
        initial = ''.join(f"v 22c {c['io'][-1]['value']:x}\n" for c in original['cases'][:index])
        instructions = case['instructions']
        source = self.directory / 'source.txt'
        source.write_text(initial + f"u 22c {case['initial_write_busy']:x}\n" +
                          ''.join(f"{r['op']} {r['port']:x} {r['value']:x}\n" for r in instructions))
        result = subprocess.run([self.original, 'device-sb', str(start), str(source)],
                                check=True, capture_output=True, text=True, timeout=30)
        clocks = [list(map(int, line.split())) for line in result.stdout.splitlines()]
        self.assertEqual(len(clocks), len(instructions))
        io = [dict(direction=r['op'], port=r['port'], value=r['value'],
                   before_cycle=clock[0], after_cycle=clock[1], remaining=clock[2])
              for r, clock in zip(instructions, clocks) if r['op'] in ('r', 'w')]
        if start == case['start_cycle_before_fetch']:
            self.assertEqual(io, case['io'])
            self.assertEqual(clocks[-1][1:], [case['ret_cycle'], case['ret_remaining']])
        expected = ''.join(f"io {r['direction']} {r['port']:x} {r['value']:x} "
                           f"{r['before_cycle']} {r['after_cycle']} {r['remaining']}\n" for r in io)
        expected += f'return {eax:08x} {clocks[-1][1]} {clocks[-1][2]}\npumps 0\n'
        script = self.directory / 'writer.txt'
        script.write_text(initial + f"p {start} {eax:x} {case['port']:x}\n")
        for target, run in self.commands:
            with self.subTest(target=target, original=original['name'], call=index, start=start, eax=eax):
                result = subprocess.run([*run, str(case['initial_write_busy']), str(script)],
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory/'device.wav')),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual(result.stdout, expected)

    def test_all_reached_ready_and_controlled_busy_calls_match_original_byte_io_and_clock(self):
        for original in self.proof['originals']:
            for index in range(len(original['cases'])):
                self.check_case(original, index)

    def test_busy_polling_preserves_full_eax_across_thin_budgets_and_tick_boundaries(self):
        for original in self.proof['originals']:
            for index in (29900, 29969, 29998, 29999):
                self.check_case(original, 0, start=535*30000+index, eax=0xf1a58040)
