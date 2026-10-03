import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class InitialMixerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        patched_unit(cls.directory, 'fist_ext.c')
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion',
                 '-Wno-incompatible-pointer-types', '-Wno-implicit-function-declaration',
                 '-Wno-return-mismatch', '-w']
        sources = [str(ROOT / 'tests/initial_mixer.c'), str(cls.directory / 'fist_ext.c'),
                   *(str(ROOT / 're_out' / name) for name in
                     ('fist_dos.c', 'fist_vga.c', 'fist_pic.c'))]
        native, wasm = (str(cls.directory / name) for name in ('initial', 'initial.js'))
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

    def initial_memory(self):
        case = json.loads((ROOT / 'tools/oracle/initial_mixer_call_case.json').read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        code = bytes.fromhex(case['code_bytes']); offset = case['code_offset']
        self.assertEqual(image[offset:offset + len(code)], code)
        for region in case['byte_parameter_code']:
            code = bytes.fromhex(region['hex']); offset = region['offset']
            self.assertEqual(image[offset:offset + len(code)], code)
        memory = bytearray(b'\xa5' * (0x100000 + 0x800))
        memory[:len(image)] = image
        memory[0x1621:0x1821] = bytes([128]) * 512
        memory[0x100000:] = bytes([case['ring_initial_fill']]) * case['ring_length']
        for field in case['before']:
            data = bytes.fromhex(field['hex'])
            memory[field['offset']:field['offset'] + len(data)] = data
        expected = memory.copy()
        for field in case['after']:
            data = bytes.fromhex(field['hex'])
            expected[field['offset']:field['offset'] + len(data)] = data
        return case, memory, expected

    def check_memory(self, memory, expected, eax, entries=None):
        source = self.directory / 'initial.memory'; source.write_bytes(memory)
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}.memory'
                args = [str(source), str(output), str(eax)]
                if entries is not None:
                    args.append(str(entries))
                result = subprocess.run([*run, *args],
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0,
                                 '\n'.join(line[:500] for line in
                                           (result.stdout + result.stderr).splitlines()[:4]))
                actual = output.read_bytes()
                self.assertEqual(len(actual), len(expected))
                differences = [index for index, (found, wanted) in
                               enumerate(zip(actual, expected)) if found != wanted]
                self.assertFalse(differences,
                                 f'Unequal bytes (offset, actual, original): '
                                 f'{[(hex(i), actual[i], expected[i]) for i in differences[:24]]}')

    def test_mode1_setup_runs_original_initial_mixer_before_entering_device(self):
        case, memory, expected = self.initial_memory()
        self.check_memory(memory, expected, case['param_ax'])
        for target, _ in self.commands:
            actual = (self.directory / f'{target}.memory').read_bytes()
            with self.subTest(target=target):
                self.assertEqual(hashlib.sha256(actual[0x100000:]).hexdigest(),
                                 case['ring_sha256'])

    def effects_source(self):
        source = json.loads((ROOT / 'tools/oracle/effects_mode_case.json').read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), source['image_sha256'])
        code = bytes.fromhex(source['code_hex'])
        self.assertEqual(image[0x76fd:0x76fd + len(code)], code)
        self.assertEqual(int.from_bytes(image[0xcb3+0x68:0xcb3+0x68+4], 'little'), 0x76fd)
        return source['states']

    def test_effects_ready_zero_preserves_neighboring_instructions(self):
        _, memory, _ = self.initial_memory()
        source = self.effects_source()['01-76fd']
        self.assertEqual(source['ready'], 0)
        memory[0x77e0] = source['ready']
        memory[0x77e1] = source['mode']
        self.check_memory(memory, memory.copy(), source['registers'][0], 0)

    def test_effects_ready_one_reaches_real_initial_mixer_and_device_boundary(self):
        _, memory, expected = self.initial_memory()
        states = self.effects_source()
        source = states['02-76fd']
        self.assertEqual((source['ready'], source['mixer_active']), (1, 0))
        self.assertEqual(states['02-138d']['mixer_active'], 1)
        for data in (memory, expected):
            data[0x77e0] = source['ready']
            data[0x77e1] = source['mode']
        self.check_memory(memory, expected, source['registers'][0], 1)
