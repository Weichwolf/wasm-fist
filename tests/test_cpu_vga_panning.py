"""Original reached panning, complete state/stream transport and latch widths."""
from pathlib import Path
import resource,subprocess,sys,tempfile,unittest
from test_port_io import ROOT
from cpu_core_exit_fixture import build,replay,observation
from cpu_dac_fixture import initial_packet,extra_parts,io_observations,io_palettes
from cpu_vga_fixture import draw_requests
from device_cpu_fixture import clock
sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_vga_callbacks import capture as capture_source,verify as verify_source
from capture_vga_panning import capture as capture_programs

class PanningPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-panning-reaching-unit-')
        cls.addClassCleanup(cls.storage.cleanup);cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE);resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'source';cls.case=capture_source(ROOT,cls.source,through_panning=True)
        cls.rows=cls.case['events'];cls.initial=initial_packet(cls.rows[0],cls.source/'source')
        cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,clock_source=None,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_vga_callbacks.c',clock_source=clock_source or ROOT/'tests/cpu_vga_clock.c',
                     include_dirs=includes,extra_flags=('-DFIST_VGA_PANNING_CONTINUE','-DFIST_CPU_DAC_CLOCK'))

    def check_streams(self,directory,target,fetches):
        self.assertEqual((directory/(target+'.fetches')).read_text().splitlines(),[observation(q) for q in fetches])
        self.assertEqual((directory/(target+'.draw-requests')).read_bytes(),draw_requests(self.case))
        self.assertEqual((directory/(target+'.io-cpu')).read_text().splitlines(),io_observations(self.case['io_writes']))
        self.assertEqual((directory/(target+'.io-dac')).read_bytes(),io_palettes(self.case['io_writes']))

    def test_one_seed_complete42_boundaries3057_fetches1544_io_states(self):
        self.assertEqual(len(self.rows),42);self.assertEqual(len(self.case['fetches']),3057)
        self.assertEqual([q['kind'] for q in self.rows[-3:]],['before-panning','after-panning','before-test-word'])
        results=replay(self.directory,self.source,self.commands,rows=self.rows,initial=self.initial,extra_parts=extra_parts)
        self.assertEqual(len(results),2)
        for q in results:self.check_streams(self.directory,q['target'],self.case['fetches'])
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_previous_calendar_binding_reaches_actual_missing_panning(self):
        directory=self.directory/'previous-binding';directory.mkdir()
        old=subprocess.check_output(['git','show','7dcfbb5920c4b9a6398b0ecd94c3ce2737583eab:tests/cpu_pit_clock.c'],cwd=ROOT,text=True)
        (directory/'cpu_pit_clock.c').write_text(old)
        # Quoted includes must select this previous clock beside its copied caller.
        (directory/'cpu_vga_clock.c').write_text((ROOT/'tests/cpu_vga_clock.c').read_text())
        results=replay(directory,self.source,self.build(directory,directory/'cpu_vga_clock.c',(directory,)),
                       strict=False,rows=self.rows[:39],initial=self.initial,extra_parts=extra_parts,compare_failed=True)
        fetches=[q for q in self.case['fetches'] if clock(q)<clock(self.rows[39])];self.assertEqual(len(fetches),3055)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0);self.assertEqual(q['memory_context_pic_errors'],[])
            actual=(directory/(q['target']+'.log')).read_text().splitlines()
            self.assertEqual(actual[:39],[observation(v) for v in self.rows[:39]])
            self.assertTrue(len(actual)==39 or (q['target']=='wasm' and actual[39]=='Aborted()'))
            self.check_streams(directory,q['target'],fetches)

    def test_source_rejects_missing_fetch_and_panning_return(self):
        for name,index in [('handler-fetches.jsonl',-2),('events.jsonl',40)]:
            path=self.source/'source'/name;original=path.read_bytes();rows=original.splitlines(keepends=True)
            del rows[index]
            try:
                path.write_bytes(b''.join(rows))
                with self.assertRaises(AssertionError):verify_source(ROOT,self.source,through_panning=True)
            finally:path.write_bytes(original)

class PanningProgramsTest(unittest.TestCase):
    def test_complete6144_original_programs6_causal_results(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-panning-encoded-unit-') as directory:
            proof=capture_programs(ROOT,Path(directory)/'source')
            self.assertEqual(proof['cases'],6144);self.assertEqual(len(set(proof['outputs_sha256'].values())),1)
            self.assertEqual(len(proof['causal_negatives']),6)
            self.assertTrue(all(q['unmodified_complete_positive_equal'] and q['full256KiB_RAM_requests_equal'] for q in proof['causal_negatives']))

if __name__=='__main__':unittest.main()
