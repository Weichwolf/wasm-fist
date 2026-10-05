"""Full original reaching VGA-device prefix and complete controlled contracts."""
import json
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build,replay,observation
from cpu_vga_fixture import initial_packet,extra_parts,draw_requests
sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_vga_callbacks import capture
from capture_vga_draw_programs import capture as capture_drawing
from capture_cpu_byte_instructions import capture as capture_instructions
from capture_vga_status_period import capture as capture_status
from capture_vga_pic_rearm import capture as capture_rearm

class CpuVgaPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-vga-prefix-unit-');cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE);resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'original';cls.case=capture(ROOT,cls.source);cls.rows=cls.case['events'][:21]
        cls.initial=initial_packet(cls.rows[0],cls.source/'source');cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_vga_callbacks.c',clock_source=ROOT/'tests/cpu_vga_clock.c',include_dirs=includes)

    def check(self,directory,commands,strict=True,rows=None,compare_failed=False):
        return replay(directory,self.source,commands,strict=strict,rows=self.rows if rows is None else rows,
                      initial=self.initial,extra_parts=extra_parts,compare_failed=compare_failed)

    def fetches(self,directory,target):return (directory/(target+'.fetches')).read_text().splitlines()

    def test_one_seed_complete21_boundaries2968_fetches50_line_bytes(self):
        results=self.check(self.directory,self.commands)
        self.assertEqual(len(results),2);self.assertEqual(len(self.rows),21)
        expected=[observation(q) for q in self.case['fetches']]
        self.assertEqual(len(expected),2968)
        for q in results:
            self.assertEqual(self.fetches(self.directory,q['target']),expected)
            self.assertEqual((self.directory/(q['target']+'.draw-requests')).read_bytes(),draw_requests(self.case))
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_previous_decoder_reaches_first_missing_and_al(self):
        directory=self.directory/'previous-decoder';directory.mkdir()
        text=subprocess.check_output(['git','show','f6b1b5db245c2a38d029dd2e57b0a9e83d8fd42d:re_out/fist_exec.h'],cwd=ROOT,text=True)
        (directory/'fist_exec.h').write_text(text)
        results=self.check(directory,self.build(directory,(directory,)),strict=False,rows=self.rows[:7],compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0)
            self.assertEqual(q['memory_context_pic_errors'],[])
            self.assertEqual(self.fetches(directory,q['target']),[observation(v) for v in self.case['fetches'][:22]])
            last=self.case['fetches'][21]
            self.assertEqual(last['cpu_regs.ip.dword[0]'],0x3a91);self.assertTrue(last['code_hex'].startswith('24'))

    def test_old_status_has_first_unmasked_eax_difference(self):
        directory=self.directory/'old-status';directory.mkdir()
        text=(ROOT/'re_out/fist_vga.c').read_text()
        old='if(g_vga_status_context)return fist_vga_read_status(g_vga_status_context,fist_clock_full_index());'
        self.assertEqual(text.count(old),1)
        (directory/'fist_vga.c').write_text(text.replace(old,'return vga_status(g_clock);'))
        results=self.check(directory,self.build(directory,(directory,)),strict=False)
        expected=[observation(q) for q in self.case['fetches']]
        for q in results:
            self.assertEqual(q['terminal_exit'],0)
            actual=self.fetches(directory,q['target'])
            first=next(i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b)
            self.assertEqual(first,90);self.assertEqual(actual[:first],expected[:first])
            a,b=actual[first].split(),expected[first].split()
            self.assertEqual([i for i,(x,y) in enumerate(zip(a,b)) if x!=y],[5])
            self.assertEqual(a[5],'00000000');self.assertEqual(b[5],'00000001')

    def test_pic_service_query_must_return_original_zero_budget(self):
        directory=self.directory/'old-service-budget';directory.mkdir()
        text=(ROOT/'re_out/fist_vga.c').read_text()
        old='return (g_pic_service || clock_equal(g_cpu_time, clock_now())) ? g_cpu_remaining : cpu_next_slice(cpu);'
        self.assertEqual(text.count(old),1)
        (directory/'fist_vga.c').write_text(text.replace(old,'return clock_equal(g_cpu_time, clock_now()) ? g_cpu_remaining : cpu_next_slice(cpu);'))
        results=self.check(directory,self.build(directory,(directory,)),strict=False)
        expected=[observation(q) for q in self.rows]
        for q in results:
            self.assertEqual(q['terminal_exit'],0);self.assertEqual(q['memory_context_pic_errors'],[])
            actual=(directory/(q['target']+'.log')).read_text().splitlines()
            self.assertEqual(len(actual),len(expected))
            differences=[i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b]
            self.assertEqual(differences,list(range(7,13)))
            for i in differences:
                self.assertEqual([k for k,(a,b) in enumerate(zip(actual[i].split(),expected[i].split())) if a!=b],[3])
                self.assertEqual(expected[i].split()[3],'0')
            self.assertEqual(self.fetches(directory,q['target']),[observation(v) for v in self.case['fetches']])
            self.assertEqual((directory/(q['target']+'.draw-requests')).read_bytes(),draw_requests(self.case))

    def test_every_cpu_quantum_of_complete_original_status_period(self):
        proof=capture_status(ROOT,self.directory/'status',self.case)
        self.assertEqual(len(proof['results']),3)
        self.assertEqual({q['cases'] for q in proof['results']},{428044})
        self.assertEqual(len({q['sha256'] for q in proof['results']}),1)

class VgaDrawProgramsTest(unittest.TestCase):
    def test_complete_bound_pic_rearms_and_owner_retirement(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-vga-pic-rearm-') as directory:
            proof=capture_rearm(ROOT,Path(directory)/'source')
            self.assertEqual(len(proof['programs']),5)
            self.assertEqual(len(proof['causal_negatives']),6)

    def test_complete3474_original_programs_and14_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-vga-draw-programs-') as directory:
            proof=capture_drawing(ROOT,Path(directory)/'source')
            self.assertEqual(proof['cases'],3474)
            self.assertEqual(len(set(proof['outputs_sha256'].values())),1)
            self.assertEqual(len(proof['causal_negatives']['results']),14)

class CpuByteInstructionsTest(unittest.TestCase):
    def test_exhaustive_and_flags8192_and5472_jcxz_full_outputs(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-byte-instructions-') as directory:
            proof=capture_instructions(ROOT,Path(directory)/'source')
            self.assertEqual({q['records'] for q in proof['byte_results']},{524288})
            self.assertEqual(proof['AND_cases'],8192);self.assertEqual(proof['JCXZ_cases'],5472)
            self.assertEqual(len(proof['causal_negatives']),10)
            for outputs in proof['instruction_outputs_sha256'].values():
                self.assertEqual(outputs['original'],outputs['native']);self.assertEqual(outputs['original'],outputs['wasm'])

if __name__=='__main__':unittest.main()
