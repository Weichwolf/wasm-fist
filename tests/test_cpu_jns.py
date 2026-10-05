"""Original reaching JNS and complete release sign-query/branch contracts."""
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
from capture_cpu_jns import capture as capture_instructions


class JnsPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-jns-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        limit = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, limit[1]))
        cls.addClassCleanup(resource.setrlimit, resource.RLIMIT_CORE, limit)
        cls.source = cls.directory / 'source'
        cls.case = capture(ROOT, cls.source, through_jns=True)
        cls.rows = cls.case['events'][:29]
        cls.initial = initial_packet(cls.rows[0], cls.source / 'source')
        cls.commands = cls.build(cls.directory)

    @classmethod
    def build(cls, directory, includes=()):
        return build(directory, driver=ROOT / 'tests/cpu_vga_callbacks.c',
                     clock_source=ROOT / 'tests/cpu_vga_clock.c', include_dirs=includes,
                     extra_flags=('-DFIST_VGA_JNS_CONTINUE',))

    def test_one_seed_complete29_boundaries2982_fetches50_lines(self):
        results = replay(self.directory, self.source, self.commands, rows=self.rows,
                         initial=self.initial, extra_parts=extra_parts)
        expected = [observation(q) for q in self.case['fetches']]
        self.assertEqual(len(results), 2)
        self.assertEqual(len(expected), 2982)
        self.assertEqual([q['kind'] for q in self.rows[-2:]],
                         ['before-jns', 'after-jns'])
        for q in results:
            self.assertEqual((self.directory / (q['target'] + '.fetches')).read_text().splitlines(), expected)
            self.assertEqual((self.directory / (q['target'] + '.draw-requests')).read_bytes(), draw_requests(self.case))
        self.assertEqual(self.case['frames'], 39)
        self.assertEqual(self.case['samples'], 27518)

    def test_previous_decoder_reaches_actual_missing_jns(self):
        directory = self.directory / 'previous'
        directory.mkdir()
        text = subprocess.check_output(['git', 'show',
            'aa4b1ddd5dbbe50299ff466316031fc23d43ac60:re_out/fist_exec.h'], cwd=ROOT, text=True)
        (directory / 'fist_exec.h').write_text(text)
        results = replay(directory, self.source, self.build(directory, (directory,)),
            strict=False, rows=self.rows[:28], initial=self.initial,
            extra_parts=extra_parts, compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'], 0)
            self.assertEqual(q['memory_context_pic_errors'], [])
            self.assertEqual((directory / (q['target'] + '.log')).read_text().splitlines()[:28],
                             [observation(v) for v in self.rows[:28]])
            self.assertEqual((directory / (q['target'] + '.fetches')).read_text().splitlines(),
                             [observation(v) for v in self.case['fetches'][:2981]])
            self.assertEqual((directory / (q['target'] + '.draw-requests')).read_bytes(), draw_requests(self.case))
        last = self.case['fetches'][2980]
        self.assertEqual(last['segments'][1]['value'], 0x2082)
        self.assertEqual(last['cpu_regs.ip.dword[0]'], 0x3b48)
        self.assertTrue(last['code_hex'].startswith('7919'))


class JnsProgramsTest(unittest.TestCase):
    def test_complete2236720_sign_records8448_original_programs8_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-jns-encoded-unit-') as directory:
            proof = capture_instructions(ROOT, Path(directory) / 'source')
            self.assertEqual([q['records'] for q in proof['sign_queries']['results']], [2236720]*3)
            self.assertEqual(len({q['sha256'] for q in proof['sign_queries']['results']}), 1)
            self.assertEqual(len(proof['source_contract']['enum']), 66)
            self.assertEqual(proof['programs']['cases'], 8448)
            self.assertEqual(len(set(proof['programs']['outputs_sha256'].values())), 1)
            self.assertEqual(len(proof['programs']['causal_negatives']), 8)


if __name__ == '__main__':
    unittest.main()
