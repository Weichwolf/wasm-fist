"""Original reaching A0 and complete A0/A1 memory-load contracts."""
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build, replay, observation
from cpu_vga_fixture import initial_packet, extra_parts, draw_requests
sys.path.insert(0, str(ROOT / 'tools/oracle'))
from capture_cpu_vga_callbacks import capture
from capture_cpu_moffs import capture as capture_instructions


class MoffsPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-moffs-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        limit = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, limit[1]))
        cls.addClassCleanup(resource.setrlimit, resource.RLIMIT_CORE, limit)
        cls.source = cls.directory / 'source'
        cls.case = capture(ROOT, cls.source, through_moffs=True)
        cls.rows = cls.case['events'][:27]
        cls.initial = initial_packet(cls.rows[0], cls.source / 'source')
        cls.commands = cls.build(cls.directory)

    @classmethod
    def build(cls, directory, includes=()):
        return build(directory, driver=ROOT / 'tests/cpu_vga_callbacks.c',
                     clock_source=ROOT / 'tests/cpu_vga_clock.c', include_dirs=includes,
                     extra_flags=('-DFIST_VGA_MOFFS_CONTINUE',))

    def test_one_seed_complete27_boundaries2980_fetches50_lines(self):
        results = replay(self.directory, self.source, self.commands, rows=self.rows,
                         initial=self.initial, extra_parts=extra_parts)
        expected = [observation(q) for q in self.case['fetches']]
        self.assertEqual(len(results), 2)
        self.assertEqual(len(expected), 2980)
        self.assertEqual([q['kind'] for q in self.rows[-2:]],
                         ['before-moffs-byte', 'after-moffs-byte'])
        for q in results:
            self.assertEqual((self.directory / (q['target'] + '.fetches')).read_text().splitlines(), expected)
            self.assertEqual((self.directory / (q['target'] + '.draw-requests')).read_bytes(), draw_requests(self.case))
        self.assertEqual(self.case['frames'], 39)
        self.assertEqual(self.case['samples'], 27518)

    def test_previous_decoder_reaches_actual_missing_a0(self):
        directory = self.directory / 'previous'
        directory.mkdir()
        text = subprocess.check_output(['git', 'show',
            '8f13c93e5cba655c143fe8c2be8cdd62998ccd37:re_out/fist_exec.h'], cwd=ROOT, text=True)
        (directory / 'fist_exec.h').write_text(text)
        results = replay(directory, self.source, self.build(directory, (directory,)),
            strict=False, rows=self.rows[:26], initial=self.initial,
            extra_parts=extra_parts, compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'], 0)
            self.assertEqual(q['memory_context_pic_errors'], [])
            self.assertEqual((directory / (q['target'] + '.log')).read_text().splitlines()[:26],
                             [observation(v) for v in self.rows[:26]])
            self.assertEqual((directory / (q['target'] + '.fetches')).read_text().splitlines(),
                             [observation(v) for v in self.case['fetches'][:2979]])
            self.assertEqual((directory / (q['target'] + '.draw-requests')).read_bytes(), draw_requests(self.case))
        last = self.case['fetches'][2978]
        self.assertEqual(last['segments'][1]['value'], 0x2082)
        self.assertEqual(last['cpu_regs.ip.dword[0]'], 0x3b43)
        self.assertTrue(last['code_hex'].startswith('a03b07'))


class MoffsProgramsTest(unittest.TestCase):
    def test_complete1120_original_programs_and8_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-moffs-encoded-unit-') as directory:
            proof = capture_instructions(ROOT, Path(directory) / 'source')
            self.assertEqual(proof['cases'], 1120)
            self.assertEqual(len(set(proof['outputs_sha256'].values())), 1)
            self.assertEqual(len(proof['causal_negatives']), 8)


if __name__ == '__main__':
    unittest.main()
