"""Original reaching CMPB and complete encoded byte comparison contracts."""
from pathlib import Path
import resource,subprocess,sys,tempfile,unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build,replay,observation
from cpu_dac_fixture import initial_packet,extra_parts,io_observations,io_palettes
from cpu_vga_fixture import draw_requests
sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_cmp_byte import capture
from capture_cpu_vga_callbacks import capture as capture_source

PREVIOUS_EXEC_SHA='7dcfbb5920c4b9a6398b0ecd94c3ce2737583eab'

class CmpBytePrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-cmp-byte-reaching-unit-')
        cls.addClassCleanup(cls.storage.cleanup);cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'source';cls.case=capture_source(ROOT,cls.source,through_cmp_byte=True)
        cls.rows=cls.case['events'][:39];cls.initial=initial_packet(cls.rows[0],cls.source/'source')
        cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_vga_callbacks.c',
            clock_source=ROOT/'tests/cpu_vga_clock.c',include_dirs=includes,
            extra_flags=('-DFIST_VGA_CMP_BYTE_CONTINUE','-DFIST_CPU_DAC_CLOCK'))

    def check_streams(self,directory,target,fetches):
        self.assertEqual((directory/(target+'.fetches')).read_text().splitlines(),
                         [observation(q) for q in fetches])
        self.assertEqual((directory/(target+'.draw-requests')).read_bytes(),draw_requests(self.case))
        self.assertEqual((directory/(target+'.io-cpu')).read_text().splitlines(),io_observations(self.case['io_writes']))
        self.assertEqual((directory/(target+'.io-dac')).read_bytes(),io_palettes(self.case['io_writes']))

    def test_one_seed_complete39_boundaries3054_fetches1544_io_states(self):
        results=replay(self.directory,self.source,self.commands,rows=self.rows,
                       initial=self.initial,extra_parts=extra_parts)
        self.assertEqual(len(results),2)
        self.assertEqual(len(self.case['fetches']),3054);self.assertEqual(len(self.case['io_writes']),1544)
        self.assertEqual([q['kind'] for q in self.rows[-2:]],['before-cmp-byte','after-cmp-byte'])
        for q in results:self.check_streams(self.directory,q['target'],self.case['fetches'])
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_previous_decoder_reaches_actual_missing_cmp_byte(self):
        directory=self.directory/'previous';directory.mkdir()
        text=subprocess.check_output(['git','show',PREVIOUS_EXEC_SHA+':re_out/fist_exec.h'],cwd=ROOT,text=True)
        (directory/'fist_exec.h').write_text(text)
        results=replay(directory,self.source,self.build(directory,(directory,)),strict=False,
            rows=self.rows[:38],initial=self.initial,extra_parts=extra_parts,compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0);self.assertEqual(q['memory_context_pic_errors'],[])
            self.assertEqual((directory/(q['target']+'.log')).read_text().splitlines()[:38],
                             [observation(v) for v in self.rows[:38]])
            self.check_streams(directory,q['target'],self.case['fetches'][:3053])
        last=self.case['fetches'][3052]
        self.assertEqual(last['segments'][1]['value'],0x4ec3)
        self.assertEqual(last['cpu_regs.ip.dword[0]'],0x2f55)
        self.assertTrue(last['code_hex'].startswith('3a06b415'))

    def test_eager_flags_first_differs_only_after_actual_cmp_byte(self):
        positive=self.directory/'eager-positive';positive.mkdir()
        for q in replay(positive,self.source,self.commands,rows=self.rows,initial=self.initial,extra_parts=extra_parts):
            self.check_streams(positive,q['target'],self.case['fetches'])
        directory=self.directory/'eager-flags';directory.mkdir()
        source=(ROOT/'re_out/fist_exec.h').read_text()
        branch='else if(op==0x3b||op==0x3a)fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),fist_exec_read_op(e,q,w));'
        self.assertEqual(source.count(branch),1)
        (directory/'fist_exec.h').write_text(source.replace(branch,
            'else if(op==0x3b||op==0x3a){fist_exec_alu(e,7,w,fist_exec_reg_read(e,i,w),'
            'fist_exec_read_op(e,q,w));if(op==0x3a)fist_cpu_fill_flags(e->bus->cpu);}'))
        results=replay(directory,self.source,self.build(directory,(directory,)),strict=False,
            rows=self.rows,initial=self.initial,extra_parts=extra_parts,compare_failed=True)
        expected=[observation(q) for q in self.rows];fetches=[observation(q) for q in self.case['fetches']]
        for q in results:
            target=q['target'];self.assertEqual(q['terminal_exit'],0)
            self.assertFalse(q['complete_CPU_time_equal']);self.assertEqual(q['memory_context_pic_errors'],[])
            actual=(directory/(target+'.log')).read_text().splitlines()
            self.assertEqual(len(actual),39);self.assertEqual(actual[:38],expected[:38])
            a,b=actual[-1].split(),expected[-1].split()
            self.assertEqual([i for i,(x,y) in enumerate(zip(a,b)) if x!=y],[26,30])
            self.assertEqual((a[26],b[26]),('00003286','00003246'))
            self.assertEqual((a[30],b[30]),('00000000','00000016'))
            actual=(directory/(target+'.fetches')).read_text().splitlines()
            self.assertEqual(len(actual),3054);self.assertEqual(actual[:3053],fetches[:3053])
            self.assertEqual([i for i,(x,y) in enumerate(zip(actual[-1].split(),fetches[-1].split())) if x!=y],[26,30])
            self.assertEqual((directory/(target+'.draw-requests')).read_bytes(),draw_requests(self.case))
            self.assertEqual((directory/(target+'.io-cpu')).read_text().splitlines(),io_observations(self.case['io_writes']))
            self.assertEqual((directory/(target+'.io-dac')).read_bytes(),io_palettes(self.case['io_writes']))

class CmpByteProgramsTest(unittest.TestCase):
    def test_complete11840_original_programs16_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-cmp-byte-encoded-unit-') as directory:
            proof=capture(ROOT,Path(directory)/'source')
            self.assertEqual(proof['cases'],11840);self.assertEqual(len(set(proof['outputs_sha256'].values())),1)
            self.assertEqual(len(proof['causal_negatives']),16)
            self.assertTrue(all(q['unmodified_complete_positive_equal'] for q in proof['causal_negatives']))

if __name__=='__main__':unittest.main()
