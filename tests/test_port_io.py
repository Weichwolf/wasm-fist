import os
import importlib.util
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('sequence_format', ROOT / 'tools/oracle/sequence_format.py')
FORMAT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(FORMAT)


def tool(name, pattern):
    return os.environ.get(name.upper()) or shutil.which(name) or str(next(Path.home().glob(pattern)))


class PortIoTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        sources = [str(ROOT / 'tests/port_io.c'), str(ROOT / 're_out/fist_vga.c')]
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections']
        native, wasm = (str(cls.directory / name) for name in ('ports', 'ports.js'))
        targets = [('native', ['gcc', '-m32', *flags, *sources, '-Wl,--gc-sections', '-o', native], [native]),
                   ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags, *sources,
                             '-sASSERTIONS=1', '-sNODERAWFS=1', '-sEXIT_RUNTIME=1',
                             '--pre-js', str(ROOT / 'tools/wasm_pre.js'), '-o', wasm],
                    [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]
        cls.commands = []
        for target, build, run in targets:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append((target, run))

    def test_unrelated_ports_preserve_speaker_and_pit2(self):
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run(run, capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_capture_ends_on_the_requested_clock_boundary(self):
        for end_ms in (31, 32, 999, 1000, 3000):
            captures = []
            for target, run in self.commands:
                with self.subTest(target=target, end_ms=end_ms):
                    prefix = self.directory / f'{target}-{end_ms}'
                    env = dict(os.environ, FIST_SEQUENCE=str(prefix), FIST_SEQUENCE_END_MS=str(end_ms))
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual(Path(str(prefix) + '.end').read_bytes(), f'FISTEND1\n{end_ms}\n'.encode())
                    frames = Path(str(prefix) + '.frames')
                    info = FORMAT.validate(frames, 'F')
                    self.assertLess(info['last'], end_ms * 1000)
                    captures.append(frames.read_bytes())
            if len(captures) == 2:
                self.assertEqual(*captures)

    def test_invalid_capture_endpoint_fails(self):
        for value in ('', '0', '-1', '3.5', '4294967296', 'x'):
            for target, run in self.commands:
                with self.subTest(target=target, value=value):
                    prefix = self.directory / f'invalid-{target}-{value}'
                    env = dict(os.environ, FIST_SEQUENCE=str(prefix), FIST_SEQUENCE_END_MS=value)
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn('FIST_SEQUENCE:', result.stderr)
                    self.assertFalse(Path(str(prefix) + '.end').exists())


if __name__ == '__main__':
    unittest.main()
