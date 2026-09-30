import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


def tool(name, pattern):
    return os.environ.get(name.upper()) or shutil.which(name) or str(next(Path.home().glob(pattern)))


class PortIoTest(unittest.TestCase):
    def test_unrelated_ports_preserve_speaker_and_pit2(self):
        with tempfile.TemporaryDirectory() as tmp:
            sources = [str(ROOT / 'tests/port_io.c'), str(ROOT / 're_out/fist_vga.c')]
            flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections']
            native = str(Path(tmp) / 'ports')
            wasm = str(Path(tmp) / 'ports.js')
            targets = [('native', ['gcc', '-m32', *flags, *sources, '-Wl,--gc-sections', '-o', native], [native]),
                       ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags, *sources,
                                 '-sASSERTIONS=1', '-o', wasm],
                        [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]
            for target, build, run in targets:
                with self.subTest(target=target):
                    subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
                    result = subprocess.run(run, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
