import base64
import gzip
import importlib.util
import os
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
        targets = [('native', ['gcc', '-m32', *flags, *sources, '-Wl,--gc-sections', '-lm', '-o', native], [native]),
                   ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags, *sources,
                             '-sASSERTIONS=1', '-sNODERAWFS=1', '-sEXIT_RUNTIME=1',
                             '--pre-js', str(ROOT / 'tools/wasm_pre.js'), '-o', wasm],
                    [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]
        cls.commands = []
        for target, build, run in targets:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append((target, run))
        tree = ROOT / 'third_party/dosbox-build/dosbox-0.74-3'
        cls.pit_probe = str(cls.directory / 'pit-probe')
        sdl_flags = subprocess.check_output(['sdl-config', '--cflags'], text=True).split()
        subprocess.run(['g++', '-std=gnu++11', *sdl_flags, '-I' + str(tree / 'include'), '-I' + str(tree),
                        '-ffunction-sections', '-fdata-sections', str(ROOT / 'tools/oracle/pit_latch_probe.cpp'),
                        '-Wl,--gc-sections', '-lm', '-o', cls.pit_probe], check=True, capture_output=True, text=True)
        cls.pic_probe = str(cls.directory / 'pic-probe')
        subprocess.run(['g++', '-std=gnu++11', *sdl_flags, '-I' + str(tree / 'include'), '-I' + str(tree),
                        '-ffunction-sections', '-fdata-sections', str(ROOT / 'tools/oracle/pic_slice_probe.cpp'),
                        '-Wl,--gc-sections', '-lm', '-o', cls.pic_probe], check=True, capture_output=True, text=True)

    def test_pit_latches_match_original_float_period_and_rounding(self):
        for mode in (2, 3):
            for period in (200, 8191, 17023, 65536):
                for elapsed in sorted({1, 2, 123, 17022, period // 2, period - 1, period, period + 1, 2 * period + 17}):
                    args = [str(value) for value in (mode, period, elapsed)]
                    expected = subprocess.check_output([self.pit_probe, *args], text=True)
                    for target, run in self.commands:
                        with self.subTest(target=target, mode=mode, period=period, elapsed=elapsed):
                            result = subprocess.run([*run, 'pit', *args], capture_output=True, text=True, timeout=30)
                            self.assertEqual(result.returncode, 0, result.stderr)
                            self.assertEqual(result.stdout, expected)

    def test_unrelated_ports_preserve_speaker_and_pit2(self):
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run(run, capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_pit_latches_preserve_sub_count_cpu_time(self):
        for mode in (2, 3):
            for period in (200, 8191, 17023, 65536):
                for instructions in (1, 2, 20, 25, 26, 27, 123, 30000):
                    elapsed = 1 + instructions * 1193182 / 30000000
                    expected = subprocess.check_output([self.pit_probe, str(mode), str(period), str(elapsed)], text=True)
                    for target, run in self.commands:
                        for command in ('pit-cpu', 'pit-cpu-base'):
                            with self.subTest(target=target, mode=mode, period=period, instructions=instructions, command=command):
                                result = subprocess.run([*run, command, str(mode), str(period), str(instructions)],
                                                        capture_output=True, text=True, timeout=30)
                                self.assertEqual(result.returncode, 0, result.stderr)
                                self.assertEqual(result.stdout, expected)

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

    def pic_fixture(self):
        prefix = self.directory / 'pic-start'
        for suffix in ('text', 'bda'):
            source = ROOT / f'tools/oracle/start_state.{suffix}.gz.b64'
            Path(str(prefix) + '.' + suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
        state = Path(str(prefix) + '.vga')
        state.write_bytes((ROOT / 'tools/oracle/start_state.vga').read_bytes())
        return prefix, state

    def test_cpu_slices_match_original_vga_queue(self):
        prefix, state = self.pic_fixture()
        cases = ((6354, 1198), (6354, 1546), (6354, 2972), (6354, 3322), (6355, 4221),
                 (6354, 2296), (6354, 2297), (6355, 5661), (6355, 5662),
                 (6358, 10993), (6358, 10994), (6358, 10995),
                 (76, 1000), (76, 2000), (76, 3843), (76, 3844))
        for tick, index in cases:
            scenario = ['transition'] if tick == 76 else []
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index), *scenario], text=True)
            for target, run in self.commands:
                for capture in ((False, True) if (tick, index) == (6354, 1546) else (False,)):
                    with self.subTest(target=target, tick=tick, index=index, capture=capture):
                        env = dict(os.environ, FIST_TEXT_STATE=str(prefix))
                        env.pop('FIST_SEQUENCE_END_MS', None)
                        env.pop('FIST_SEQUENCE', None)
                        if capture:
                            env['FIST_SEQUENCE'] = str(self.directory / f'pic-{target}')
                        result = subprocess.run([*run, 'pic-slice', str(tick), str(index)], env=env,
                                                capture_output=True, text=True, timeout=30)
                        self.assertEqual(result.returncode, 0, result.stderr)
                        self.assertEqual(result.stdout, expected)

    def test_initial_cpu_phase_matches_original_queue(self):
        prefix, state = self.pic_fixture()
        start_ns = int(state.read_text().splitlines()[1].split()[0])
        cycle = (start_ns * 3 + 50) // 100
        expected = subprocess.check_output([self.pic_probe, str(state), str(cycle // 30000),
                                            str(cycle % 30000)], text=True)
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run, 'start-cpu'], env=dict(os.environ, FIST_TEXT_STATE=str(prefix)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)

    def test_cpu_retirement_matches_original_queue(self):
        prefix, state = self.pic_fixture()
        cases = ((6355, 6529, '290708'), (6355, 6529, '290712'),
                 (6355, 6529, '1,290707'), (6355, 6529, '100000,190708'),
                 (6358, 10990, '3'), (6358, 10990, '3,1'), (6358, 10990, '1,1,1,1'),
                 (6354, 1198, '1098'), (6354, 1198, '1098,1'),
                 (6354, 29998, '2'), (6354, 29998, '2,1'), (6354, 29998, '1,1,1'))
        for tick, index, counts in cases:
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index),
                                                'retire', counts], text=True)
            for target, run in self.commands:
                with self.subTest(target=target, tick=tick, index=index, counts=counts):
                    env = dict(os.environ, FIST_TEXT_STATE=str(prefix))
                    env.pop('FIST_SEQUENCE_END_MS', None)
                    env.pop('FIST_SEQUENCE', None)
                    result = subprocess.run([*run, 'pic-retire', str(tick), str(index), counts], env=env,
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout, expected)

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

    def test_start_fixture_requires_its_original_vga_queue_state(self):
        prefix = self.directory / 'start-state'
        for suffix in ('text', 'bda'):
            source = ROOT / f'tools/oracle/start_state.{suffix}.gz.b64'
            Path(str(prefix) + '.' + suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
        state = Path(str(prefix) + '.vga')
        valid = (ROOT / 'tools/oracle/start_state.vga').read_bytes()
        for target, run in self.commands:
            for data in (None, b'FISTVGA1\n20960467 20168067\n20 nan\n', valid + b'junk', valid):
                with self.subTest(target=target, data=data):
                    if data is None:
                        state.unlink(missing_ok=True)
                    else:
                        state.write_bytes(data)
                    capture = self.directory / f'state-{target}'
                    env = dict(os.environ, FIST_TEXT_STATE=str(prefix), FIST_SEQUENCE=str(capture),
                               FIST_SEQUENCE_END_MS='3000')
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode == 0, data == valid, result.stderr)


if __name__ == '__main__':
    unittest.main()
