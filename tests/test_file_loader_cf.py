import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class FileLoaderCarryTest(unittest.TestCase):
    OBSERVE_AFTER_STORES = False

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        text = patched_unit(cls.directory, 'fist_ext.c')
        entry = 'undefined4 __allregs FUN_0000_0f64(undefined4 param_1)\n\n{'
        if text.count(entry) != 1:
            raise RuntimeError('Missing actual error-helper entry')
        # Observe the selected original boundary and real DOS commands. No
        # adapter supplies find/open/CF/store results or a nonlocal continuation.
        if cls.OBSERVE_AFTER_STORES:
            start = text.index(entry)
            end = text.index('\n}', start)+2
            body = text[start:end]
            boundary = '  return;\n}'
            if body.count(boundary) != 1:
                raise RuntimeError('Missing actual error-helper post-store boundary')
            observed = body.replace(boundary, '  file_cf_error_entry(param_1);\n'+boundary, 1)
            text = text[:start]+observed+text[end:]
        else:
            text = text.replace(entry, entry+'\n  file_cf_error_entry(param_1);', 1)
        text = '#define fist_int_dispatch file_cf_int_dispatch\nvoid file_cf_error_entry(unsigned);\n'+text
        (cls.directory/'fist_ext.c').write_text(text)
        flags = ['-I'+str(ROOT/'re_out'), '-ffunction-sections', '-fdata-sections',
                 '-Wno-int-conversion', '-Wno-incompatible-pointer-types',
                 '-Wno-implicit-function-declaration', '-Wno-return-mismatch', '-w']
        sources = [str(ROOT/'tests/file_loader_cf.c'), str(cls.directory/'fist_ext.c'),
                   *(str(ROOT/'re_out'/name) for name in ('fist_dos.c', 'fist_vga.c', 'fist_pic.c'))]
        native, wasm = (str(cls.directory/name) for name in ('carry', 'carry.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-Wl,--gc-sections', '-lm', '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags, *sources,
                      '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1', '--pre-js',
                      str(ROOT/'tools/wasm_pre.js'), '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target, run))

    def check_failure(self, stage, initial_size):
        case_name = 'file_open_error_case.json' if stage == 'open' else 'file_error_case.json'
        case = json.loads((ROOT/'tools/oracle'/case_name).read_text())
        self.assertEqual(case['failure_stage'], stage)
        image = ROOT/'re_out/fist_image.bin';raw = image.read_bytes()
        self.assertEqual(hashlib.sha256(raw).hexdigest(), case['image_sha256'])
        for code in case['code']:
            expected = bytes.fromhex(code['bytes'])
            self.assertEqual(raw[code['image_offset']:code['image_offset']+len(expected)], expected)
        original = (ROOT/'armoredfist/FISTDATA/HIGH.DTL').read_bytes()
        self.assertEqual(hashlib.sha256(original).hexdigest(), case['removed_asset_sha256'])
        dta = case['states']['at-6032']['ext_slots']['0x927']
        for target, run in self.commands:
            with self.subTest(target=target):
                game = self.directory/(stage+'-'+target)
                (game/'FISTDATA').mkdir(parents=True)
                if stage == 'open':
                    (game/'FISTDATA/HIGH.DTL').write_bytes(original)
                output = self.directory/(stage+'-'+target+'.guard')
                result = subprocess.run([*run, str(image), 'high.dtl', str(len(original)), str(output),
                                         str(dta), stage, str(initial_size)],
                                        env=dict(os.environ, FIST_DATADIR=str(game)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                size = len(original) if stage == 'open' else initial_size
                commands = 'dos 1a 4e 1a 3d' if stage == 'open' else 'dos 1a 4e'
                status = case['task_status'] if self.OBSERVE_AFTER_STORES else 0
                self.assertEqual(result.stdout.splitlines(), [
                    f"error reason {case['reason']} size {size} cf 1 task {status}", commands])
                self.assertEqual(output.read_bytes(), bytes([0xa5])*(len(original)+32))
                self.assertEqual(Path(str(output)+'.task').read_bytes(),
                                 status.to_bytes(2, 'little')+bytes([0xa5])*2+bytes(0x1000-4))

    def test_failed_find_does_not_consume_stale_dta_or_open_or_read(self):
        self.check_failure('find', 0x89ab7654)

    def test_failed_open_does_not_issue_a_read_after_successful_find(self):
        self.check_failure('open', 0)
