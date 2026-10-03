import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, build_pic_probe, patched_unit, tool


class DeviceConfigurationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        patched_unit(cls.directory, 'fist_ext.c')
        cls.original = build_pic_probe(cls.directory)
        cls.case = json.loads((ROOT / 'tools/oracle/device_config_1280_case.json').read_text())
        wrapper = cls.directory / 'config.c'
        wrapper.write_text('#define fist_clock_charge_cpu_instructions fist_device_test_charge\n'
                           '#include "fist_ext.c"\n')
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion',
                 '-Wno-incompatible-pointer-types', '-Wno-implicit-function-declaration',
                 '-Wno-return-mismatch', '-w']
        sources = [str(ROOT / 'tests/device_config.c'), str(wrapper),
                   *(str(ROOT / 're_out' / name) for name in
                     ('fist_dos.c', 'fist_vga.c', 'fist_pic.c', 'fist_sb.c'))]
        native, wasm = (str(cls.directory / name) for name in ('config', 'config.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-Wl,--gc-sections',
                        '-lm', '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2',
                      *flags, *sources, '-sNODERAWFS=1', '-sASSERTIONS=1',
                      '-sEXIT_RUNTIME=1', '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout + result.stderr)
            cls.commands.append((target, run))

    def check_phase(self, start, guarded=False, case=None):
        case = self.case if case is None else case
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        for at, data in [(case['code_offset'], case['code_bytes']),
                         (case['caller_offset'], case['caller_bytes'])]:
            code = bytes.fromhex(data); self.assertEqual(image[at:at + len(code)], code)
        memory = bytearray(b'\xa5' * (0x100000 + 0x498)); memory[:len(image)] = image
        for field in case['before']:
            data = bytes.fromhex(field['hex'])
            memory[field['offset']:field['offset'] + len(data)] = data
        words = struct.pack('<HHH', *case['packet_words'])
        memory[0x100490:0x100496] = words
        expected = memory.copy()
        for field in case['after']:
            data = bytes.fromhex(field['hex'])
            expected[field['offset']:field['offset'] + len(data)] = data
        if guarded:
            expected[0x12ce:0x12d0] = b'\x5a\xc3'
        source = self.directory / 'config.memory'; source.write_bytes(memory)
        instructions = self.directory / 'instructions.txt'
        instructions.write_text('n 0 0\n' * len(case['instructions']))
        clocks = [list(map(int, line.split())) for line in subprocess.check_output(
            [self.original, 'device-io', str(start), str(instructions)], text=True).splitlines()]
        self.assertEqual(len(clocks), 8)
        if start == case['start_cycle_before_fetch']:
            self.assertEqual([clock[0] for clock in clocks],
                             [row['cycle'] for row in case['instructions']])
            self.assertEqual(clocks[-1][1:], [case['return_cycle'], case['return_remaining']])
        expected_text = ''.join(
            f"fetch {clock[0]} {clock[2]} {row['eax']:08x} {row['ebx']:08x}\n"
            for row, clock in zip(case['instructions'], clocks))
        expected_text += (f"return {case['return_eax']:08x} {case['return_ebx']:08x} "
                          f"{clocks[-1][1]} {clocks[-1][2]}\npumps 0\n")
        for target, run in self.commands:
            with self.subTest(target=target, start=start, guarded=guarded):
                output = self.directory / f'{target}.memory'
                result = subprocess.run([*run, str(source), str(output), str(start),
                                         str(case['input_eax']), str(case['input_ebx']), str(int(guarded))],
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory / 'device.wav')),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0,
                                 '\n'.join((result.stdout + result.stderr).splitlines()[:4]))
                self.assertEqual(result.stdout, expected_text)
                actual = output.read_bytes(); self.assertEqual(len(actual), len(expected))
                differences = [(hex(i), found, wanted) for i, (found, wanted) in
                               enumerate(zip(actual, expected)) if found != wanted]
                self.assertFalse(differences, f'Unequal bytes: {differences[:24]}')

    def test_reached_configuration_matches_original_registers_memory_and_fetches(self):
        self.check_phase(self.case['start_cycle_before_fetch'])

    def test_port_word_store_preserves_adjacent_bytes_across_thin_budgets(self):
        for index in (29992, 29998, 29999):
            self.check_phase(525 * 30000 + index, guarded=True)

    def test_original_controlled_words_preserve_ax_width_and_zero_extend_irq_dma(self):
        case = json.loads((ROOT / 'tools/oracle/device_config_1280_widths_case.json').read_text())
        for start in (case['start_cycle_before_fetch'],
                      *(525 * 30000 + index for index in (29992, 29998, 29999))):
            self.check_phase(start, case=case)
