"""Original reaching PUSH CS contract and complete segment-push controlled coverage."""
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
from capture_cpu_segment_push import capture as capture_controlled

class PushCsPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-push-cs-unit-');cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE);resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'source'
        cls.case=capture(ROOT,cls.source,through_push_cs=True);cls.rows=cls.case['events'][:23]
        cls.initial=initial_packet(cls.rows[0],cls.source/'source');cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_vga_callbacks.c',clock_source=ROOT/'tests/cpu_vga_clock.c',
                     include_dirs=includes,extra_flags=('-DFIST_VGA_PUSH_CS_CONTINUE',))

    def test_one_seed_complete23_boundaries2972_fetches50_lines(self):
        results=replay(self.directory,self.source,self.commands,rows=self.rows,initial=self.initial,extra_parts=extra_parts)
        expected=[observation(q) for q in self.case['fetches']]
        self.assertEqual(len(results),2);self.assertEqual(len(expected),2972)
        self.assertEqual([q['kind'] for q in self.rows[-2:]],['before-push-cs','after-push-cs'])
        for q in results:
            self.assertEqual((self.directory/(q['target']+'.fetches')).read_text().splitlines(),expected)
            self.assertEqual((self.directory/(q['target']+'.draw-requests')).read_bytes(),draw_requests(self.case))
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_previous_decoder_reaches_actual_missing_push_cs(self):
        directory=self.directory/'previous';directory.mkdir()
        text=subprocess.check_output(['git','show','cbaee6d9e31fec74ffc8436b18bdd3b98d8fdd72:re_out/fist_exec.h'],cwd=ROOT,text=True)
        (directory/'fist_exec.h').write_text(text)
        results=replay(directory,self.source,self.build(directory,(directory,)),strict=False,rows=self.rows[:22],
                       initial=self.initial,extra_parts=extra_parts,compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0);self.assertEqual(q['memory_context_pic_errors'],[])
            self.assertEqual((directory/(q['target']+'.log')).read_text().splitlines()[:22],
                             [observation(v) for v in self.rows[:22]])
            self.assertEqual((directory/(q['target']+'.fetches')).read_text().splitlines(),
                             [observation(v) for v in self.case['fetches'][:2971]])
        last=self.case['fetches'][2970]
        self.assertEqual(last['segments'][1]['value'],0x2082)
        self.assertEqual(last['cpu_regs.ip.dword[0]'],0x3abb);self.assertTrue(last['code_hex'].startswith('0e'))

class SegmentPushProgramsTest(unittest.TestCase):
    def test_complete3072_original_programs_and12_causal_faults(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-segment-push-unit-') as directory:
            proof=capture_controlled(ROOT,Path(directory)/'source')
            self.assertEqual(proof['cases'],3072);self.assertEqual(proof['valid_source_lazy_tags'],65)
            self.assertEqual(len(proof['causal_negatives']),12)
            outputs=proof['outputs_sha256']
            self.assertEqual(outputs['original'],outputs['native']);self.assertEqual(outputs['original'],outputs['wasm'])

if __name__=='__main__':unittest.main()
