import hashlib
import json
import os
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class FileLoaderTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        cls.game = cls.directory/'game'
        (cls.game/'FISTDATA').mkdir(parents=True)
        for name in ('HIGH.DTL', 'DSOUNDS.BIN'):
            shutil.copyfile(ROOT/'armoredfist/FISTDATA'/name, cls.game/'FISTDATA'/name)
        patched_unit(cls.directory, 'fist_ext.c')
        cls.case = json.loads((ROOT / 'tools/oracle/file_loader_6032_case.json').read_text())
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-Wno-int-conversion', '-Wno-incompatible-pointer-types',
                 '-Wno-implicit-function-declaration', '-Wno-return-mismatch', '-w']
        sources = [str(ROOT / 'tests/file_loader.c'), str(cls.directory / 'fist_ext.c'),
                   str(ROOT / 're_out/fist_dos.c'), str(ROOT / 're_out/fist_vga.c'), str(ROOT / 're_out/fist_pic.c')]
        native, wasm = (str(cls.directory / name) for name in ('loader', 'loader.js'))
        targets = [('native', ['gcc', '-m32', '-O0', *flags, *sources,
                               '-Wl,--gc-sections', '-lm', '-o', native], [native]),
                   ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2',
                             *flags, *sources, '-sNODERAWFS=1', '-sASSERTIONS=1',
                             '-sEXIT_RUNTIME=1', '--pre-js', str(ROOT/'tools/wasm_pre.js'), '-o', wasm],
                    [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]
        cls.commands = []
        for target, build, run in targets:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append((target, run))

    def check_original_file(self, name, original_dta=False):
        case = next(row for row in self.case['files'] if row['name'] == name)
        image = ROOT / 're_out/fist_image.bin'
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(), self.case['image_sha256'])
        expected = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
        self.assertEqual(len(expected), case['size'])
        self.assertEqual(hashlib.sha256(expected).hexdigest(), case['sha256'])
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / (target + '.bin')
                args = [*run, str(image), name.lower(), str(case['size']), str(output)]
                if original_dta:
                    source = json.loads((ROOT/'tools/oracle/detail_loader_case.json').read_text())
                    self.assertEqual(source['image_sha256'], self.case['image_sha256'])
                    for row in source['code']:
                        if row['image']=='ext':
                            raw=bytes.fromhex(row['bytes'])
                            self.assertEqual(image.read_bytes()[row['image_offset']:row['image_offset']+len(raw)],raw)
                    offset=source['dta_init']['states']['after-store']['dta_value']
                    args.append(str(offset))
                result = subprocess.run(args,
                                        env=dict(os.environ, FIST_DATADIR=str(self.game)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual(result.stdout.splitlines(), [
                    f"query {case['size']} size {case['size']} error 0",
                    f"load {case['size']} size {case['size']} error 0 handle 5"])
                self.assertEqual(output.read_bytes(), bytes([0xa5])*16 + expected + bytes([0xa5])*16)

    def test_small_file_returns_complete_original_count_and_content(self):
        self.check_original_file('HIGH.DTL')

    def test_dsounds_returns_complete_original_dword_count_and_content(self):
        self.check_original_file('DSOUNDS.BIN')

    def test_original_module_dta_queries_and_loads_complete_high_dtl(self):
        self.check_original_file('HIGH.DTL', original_dta=True)

    def test_original_module_dta_queries_and_loads_complete_dsounds(self):
        self.check_original_file('DSOUNDS.BIN', original_dta=True)
