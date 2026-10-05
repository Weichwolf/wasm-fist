"""Production text bootstrap matches the original DOS loader palette origin."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build
from cpu_dac_fixture import dac_packet

sys.path.insert(0, str(ROOT / 'tools/oracle'))
from capture_dac_startup import capture


class DacStartupTest(unittest.TestCase):
    def test_actual_constructor_matches_all3635_original_palette_bytes(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-dac-startup-unit-') as directory:
            root = Path(directory)
            source = root / 'source'
            proof = capture(ROOT, source)
            row = proof['events'][2]
            self.assertEqual((row['machine'], row['svga_card'], row['renderer_output']['out_mode']), (5, 1, 3))
            fmt = row['renderer_output']['pixel_format']
            self.assertEqual([fmt[k] for k in ('BitsPerPixel', 'Rloss', 'Gloss', 'Bloss',
                                             'Rshift', 'Gshift', 'Bshift', 'Amask')],
                             [32, 0, 0, 0, 16, 8, 0, 0])
            expected = dac_packet(row)
            self.assertEqual(len(expected), 3635)
            env = {k: v for k, v in os.environ.items() if not k.startswith('FIST_')}
            env['FIST_TEXT_STATE'] = str(source / 'source/start-state')
            commands = build(root, driver=ROOT / 'tests/dac_boot.c',
                             clock_source=ROOT / 'tests/dac_boot_clock.c')
            for target, command in commands:
                with self.subTest(target=target):
                    output = root / (target + '.raw')
                    result = subprocess.run([*command, str(output)], env=env,
                                            capture_output=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(output.read_bytes(), expected)


if __name__ == '__main__':
    unittest.main()
