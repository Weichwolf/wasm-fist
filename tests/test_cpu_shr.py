"""Original reaching SHR, complete flags and encoded RAM/fetch contracts."""
import resource
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build,replay,observation
from cpu_vga_fixture import initial_packet,extra_parts,draw_requests
sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_vga_callbacks import capture
from capture_cpu_shr_flags import capture as capture_flags
from capture_cpu_shr_instructions import capture as capture_instructions

class ShrPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-shr-unit-');cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE);resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'source'
        cls.case=capture(ROOT,cls.source,through_shr_word=True);cls.rows=cls.case['events'][:25]
        cls.initial=initial_packet(cls.rows[0],cls.source/'source');cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_vga_callbacks.c',clock_source=ROOT/'tests/cpu_vga_clock.c',
                     include_dirs=includes,extra_flags=('-DFIST_VGA_SHR_WORD_CONTINUE',))

    def test_one_seed_complete25_boundaries2976_fetches50_lines(self):
        results=replay(self.directory,self.source,self.commands,rows=self.rows,initial=self.initial,extra_parts=extra_parts)
        expected=[observation(q) for q in self.case['fetches']]
        self.assertEqual(len(results),2);self.assertEqual(len(expected),2976)
        self.assertEqual([q['kind'] for q in self.rows[-2:]],['before-shr-word-1','after-shr-word-1'])
        for q in results:
            self.assertEqual((self.directory/(q['target']+'.fetches')).read_text().splitlines(),expected)
            self.assertEqual((self.directory/(q['target']+'.draw-requests')).read_bytes(),draw_requests(self.case))
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_previous_decoder_reaches_actual_missing_shr_word(self):
        directory=self.directory/'previous';directory.mkdir()
        text=subprocess.check_output(['git','show','07b34ffc87d64adcc6d5a8571456306a802a9900:re_out/fist_exec.h'],cwd=ROOT,text=True)
        (directory/'fist_exec.h').write_text(text)
        results=replay(directory,self.source,self.build(directory,(directory,)),strict=False,rows=self.rows[:24],
                       initial=self.initial,extra_parts=extra_parts,compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0);self.assertEqual(q['memory_context_pic_errors'],[])
            self.assertEqual((directory/(q['target']+'.log')).read_text().splitlines()[:24],
                             [observation(v) for v in self.rows[:24]])
            self.assertEqual((directory/(q['target']+'.fetches')).read_text().splitlines(),
                             [observation(v) for v in self.case['fetches'][:2975]])
        last=self.case['fetches'][2974]
        self.assertEqual(last['segments'][1]['value'],0x2082)
        self.assertEqual(last['cpu_regs.ip.dword[0]'],0x3b38);self.assertTrue(last['code_hex'].startswith('d12e5004'))

class ShrProgramsTest(unittest.TestCase):
    def test_complete148672_original_flag_programs_and10_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-shr-flags-unit-') as directory:
            proof=capture_flags(ROOT,Path(directory)/'source')
            self.assertEqual({q['programs'] for q in proof['results']},{148672})
            self.assertEqual({q['records'] for q in proof['results']},{594688})
            self.assertEqual(len({q['sha256'] for q in proof['results']}),1)
            self.assertEqual(len(proof['causal_negatives']),10)

    def test_complete7936_original_encoded_programs_and16_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-shr-encoded-unit-') as directory:
            proof=capture_instructions(ROOT,Path(directory)/'source')
            self.assertEqual(proof['cases'],7936)
            self.assertEqual(len(set(proof['outputs_sha256'].values())),1)
            self.assertEqual(len(proof['causal_negatives']),16)

if __name__=='__main__':unittest.main()
