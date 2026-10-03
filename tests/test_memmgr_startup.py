import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from test_port_io import ROOT, tool


class MemmgrStartupTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='wasm-fist-memmgr-test-')
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        (cls.directory/'production/wasm').mkdir(parents=True)
        (cls.directory/'objects').mkdir()
        env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k not in ('NATIVE','NODE','OUTJS','OBJDIR')}
        env.update(FIST_WORKDIR=str(cls.directory), OBJDIR=str(cls.directory/'objects'))
        native = cls.directory/'production/native'
        wasm = cls.directory/'production/wasm/fistrun.js'
        for target, output in [('native', 'NATIVE='+str(native)), ('wasm', 'OUTJS='+str(wasm))]:
            result = subprocess.run(['make', target, output], cwd=ROOT, env=env,
                                    capture_output=True, text=True, timeout=180)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
        command = [sys.executable, '-B', str(ROOT/'tools/capture_memmgr_startup_ports.py'),
                   '--native', str(native), '--wasm', str(wasm),
                   '--node', tool('node', 'Git/emsdk/node/*/bin/node'),
                   '--output', str(cls.directory/'observation')]
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, timeout=180)
        if result.returncode:
            raise RuntimeError(result.stdout+result.stderr)
        cls.proof = json.loads((cls.directory/'observation/proof.json').read_text())
        cls.source = json.loads((ROOT/'tools/oracle/memmgr_startup_case.json').read_text())

    def assert_original_code(self):
        image = (ROOT/'re_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), self.source['image_sha256'])
        for code in self.source['code']:
            expected = bytes.fromhex(code['bytes'])
            self.assertEqual(image[code['image_offset']:code['image_offset']+len(expected)], expected)

    def test_constructor_runs_original_allocations_before_kdv(self):
        self.assert_original_code()
        native = next(row for row in self.proof['cases'] if row['target'] == 'native')
        initial = native['constructor']
        expected = self.source['allocations']
        self.assertEqual(initial['block_count'], len(expected))
        self.assertEqual(len(initial['blocks']), len(expected))
        for actual, original in zip(initial['blocks'], expected):
            self.assertEqual(actual['slot']-initial['g_mem_host']-initial['module_offset'], original['slot'])
            for key in ('size', 'alignment', 'flags'):
                self.assertEqual(actual[key], original[key])
            self.assertEqual(actual['address'] % actual['alignment'], 0)
        self.assertEqual(initial['checkpoint'], initial['blocks'][-1]['address'])

    def test_normal_kdv_uses_shared_task_without_false_memory_error(self):
        self.assert_original_code()
        for case in self.proof['cases']:
            with self.subTest(target=case['target']):
                self.assertTrue(case['shared_task'], case)
                self.assertEqual(case['actual_task_status'], self.source['normal_KDV_task_status'], case)
                self.assertEqual(case['frames']['records'], self.source['frames'])
                self.assertEqual(case['endpoint_ms'], self.source['endpoint_ms'])
                if case['target'] == 'native':
                    self.assertEqual(case['actual_native_errors'], [])
                    entry = case['kdv_entry']
                    self.assertEqual(entry['current_TCB'], entry['engine_task_far']['address'])
        self.assertEqual(self.proof['cases'][0]['frame_sha256'], self.proof['cases'][1]['frame_sha256'])
        self.assertEqual(self.proof['cases'][0]['end_sha256'], self.proof['cases'][1]['end_sha256'])
