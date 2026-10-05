"""Actual original byte-string I/O and full controlled observation contracts."""
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build, replay, observation
from cpu_dac_fixture import initial_packet, extra_parts, io_observations, io_palettes
from cpu_vga_fixture import draw_requests

sys.path.insert(0, str(ROOT / 'tools/oracle'))
from capture_cpu_outsb import capture
from capture_cpu_vga_callbacks import capture as capture_source


class OutsbPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-outsb-reaching-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        limit = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, limit[1]))
        cls.addClassCleanup(resource.setrlimit, resource.RLIMIT_CORE, limit)
        cls.source = cls.directory / 'source'
        cls.case = capture_source(ROOT, cls.source, through_outsb=True)
        cls.rows = cls.case['events'][:33]
        cls.initial = initial_packet(cls.rows[0], cls.source / 'source')
        cls.commands = cls.build(cls.directory)

    @classmethod
    def build(cls, directory, includes=()):
        return build(directory, driver=ROOT / 'tests/cpu_vga_callbacks.c',
                     clock_source=ROOT / 'tests/cpu_vga_clock.c', include_dirs=includes,
                     extra_flags=('-DFIST_VGA_OUTSB_CONTINUE', '-DFIST_CPU_DAC_CLOCK'))

    def check_streams(self, directory, target, fetches, io):
        self.assertEqual((directory / (target + '.fetches')).read_text().splitlines(),
                         [observation(q) for q in fetches])
        self.assertEqual((directory / (target + '.draw-requests')).read_bytes(), draw_requests(self.case))
        self.assertEqual((directory / (target + '.io-cpu')).read_text().splitlines(), io_observations(io))
        self.assertEqual((directory / (target + '.io-dac')).read_bytes(), io_palettes(io))

    def test_one_seed_complete33_boundaries2998_fetches1544_io_states(self):
        results = replay(self.directory, self.source, self.commands, rows=self.rows,
                         initial=self.initial, extra_parts=extra_parts)
        self.assertEqual(len(results), 2)
        self.assertEqual(len(self.case['fetches']), 2998)
        self.assertEqual(len(self.case['io_writes']), 1544)
        self.assertEqual([q['kind'] for q in self.rows[-2:]], ['before-outsb', 'after-outsb'])
        for q in results:
            self.check_streams(self.directory, q['target'], self.case['fetches'], self.case['io_writes'])
        self.assertEqual(self.case['frames'], 39)
        self.assertEqual(self.case['samples'], 27518)

    def test_previous_decoder_reaches_actual_missing_outsb(self):
        directory = self.directory / 'previous'
        directory.mkdir()
        text = subprocess.check_output(['git', 'show',
            'e18a05b95838cada8ac9a017e6dc0581f1bf8dd7:re_out/fist_exec.h'], cwd=ROOT, text=True)
        (directory / 'fist_exec.h').write_text(text)
        results = replay(directory, self.source, self.build(directory, (directory,)),
                         strict=False, rows=self.rows[:32], initial=self.initial,
                         extra_parts=extra_parts, compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'], 0)
            self.assertEqual(q['memory_context_pic_errors'], [])
            self.assertEqual((directory / (q['target'] + '.log')).read_text().splitlines()[:32],
                             [observation(v) for v in self.rows[:32]])
            self.check_streams(directory, q['target'], self.case['fetches'][:2997], self.case['io_writes'][:8])
        last = self.case['fetches'][2996]
        self.assertEqual(last['segments'][1]['value'], 0x4ec3)
        self.assertEqual(last['cpu_regs.ip.dword[0]'], 0xbff)
        self.assertTrue(last['code_hex'].startswith('f36e'))

    def test_duplicated_production_dac_delay_fails_even_with_identical_palettes(self):
        positive = self.directory / 'delay-positive'
        positive.mkdir()
        results = replay(positive, self.source, self.commands, rows=self.rows,
                         initial=self.initial, extra_parts=extra_parts)
        for q in results:
            self.check_streams(positive, q['target'], self.case['fetches'], self.case['io_writes'])
        directory = self.directory / 'duplicated-delay'
        directory.mkdir()
        source = (ROOT / 're_out/fist_vga.c').read_text()
        anchor = '        fist_dac_write(g_dac_context,dac_mode(),g_dac_render_context,fist_render_set_pal,port,val);'
        self.assertEqual(source.count(anchor), 1)
        source = source.replace(anchor, '        cpu_io_delay(1); /* Deliberate fixture fault. */\n' + anchor)
        source = source.replace('"../tools/oracle/fist_sequence_endpoint.h"',
                                '"' + str(ROOT / 'tools/oracle/fist_sequence_endpoint.h') + '"')
        (directory / 'fist_vga.c').write_text(source)
        results = replay(directory, self.source, self.build(directory, (directory,)),
                         strict=False, rows=self.rows, initial=self.initial,
                         extra_parts=extra_parts, compare_failed=True)
        expected = io_observations(self.case['io_writes'])
        for q in results:
            self.assertEqual(q['terminal_exit'], 0)
            self.assertFalse(q['complete_CPU_time_equal'])
            actual = (directory / (q['target'] + '.io-cpu')).read_text().splitlines()
            self.assertEqual(len(actual), len(expected))
            first = next(i for i, (a, b) in enumerate(zip(actual, expected)) if a != b)
            self.assertEqual(first, 7)
            a, b = actual[first].split(), expected[first].split()
            self.assertEqual([i for i, (x, y) in enumerate(zip(a, b)) if x != y], [1, 3])
            self.assertEqual(int(a[1]) - int(b[1]), 21)
            self.assertEqual(int(a[3]) - int(b[3]), -21)
            self.assertEqual((directory / (q['target'] + '.io-dac')).read_bytes(),
                             io_palettes(self.case['io_writes']))


class OutsbProgramsTest(unittest.TestCase):
    def test_complete6048_original_programs12_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-outsb-encoded-unit-') as directory:
            proof = capture(ROOT, Path(directory) / 'source')
            self.assertEqual(proof['programs']['cases'], 6048)
            self.assertEqual(len(set(proof['programs']['outputs_sha256'].values())), 1)
            results = proof['causal_negatives']
            self.assertEqual(len(results), 12)
            self.assertEqual({q['target'] for q in results}, {'native', 'wasm'})
            self.assertTrue(all(q['unmodified_complete_positive_equal'] for q in results))
            self.assertTrue(all(q['all2MiB_RAM_equal'] for q in results))
            for q in results:
                if q['fault'] in ('cost-after-io', 'si-committed-per-byte'):
                    self.assertEqual(q['differing_final_CPU_words'], [])
                    self.assertEqual(q['first_differing_IO_record'],
                                     0 if q['fault'] == 'cost-after-io' else 1)


if __name__ == '__main__':
    unittest.main()
