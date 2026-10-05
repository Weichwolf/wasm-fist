"""Complete original hardware and renderer palettes, including host calls."""
from pathlib import Path
import sys
import tempfile
import unittest
from test_port_io import ROOT

sys.path.insert(0, str(ROOT / 'tools/oracle'))
from capture_dac import capture as capture_dac
from capture_render_palette import capture as capture_renderer


class PaletteContractsTest(unittest.TestCase):
    def compare_contract(self, capture, programs):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-palette-unit-') as directory:
            proof = capture(ROOT, Path(directory) / 'source')
            results = proof['results']
            self.assertEqual([q['target'] for q in results], ['original', 'native', 'wasm'])
            self.assertEqual([q['programs'] for q in results], [programs] * 3)
            self.assertEqual(len({q['bytes'] for q in results}), 1)
            self.assertEqual(len({q['sha256'] for q in results}), 1)
            negatives = proof['causal_negatives']
            self.assertEqual(len(negatives), 8)
            self.assertEqual({q['target'] for q in negatives}, {'native', 'wasm'})
            self.assertEqual(len({q['fault'] for q in negatives}), 4)

    def test_complete671224_dac_programs8_causal_results(self):
        self.compare_contract(capture_dac, 671224)

    def test_complete259316_renderer_programs8_causal_results(self):
        self.compare_contract(capture_renderer, 259316)


if __name__ == '__main__':
    unittest.main()
