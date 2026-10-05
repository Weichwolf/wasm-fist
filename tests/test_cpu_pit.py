"""Require original PIT instruction/device boundaries and complete timer producers."""
import json
import os
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from cpu_core_exit_fixture import build,replay,observation
from cpu_pit_fixture import initial_packet,extra_parts
from test_port_io import ROOT

sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_pit_writes import capture
from capture_pit_channels import capture as capture_channels

class CpuPitWritesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-pit-writes-unit-')
        cls.addClassCleanup(cls.storage.cleanup);cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'original';cls.case=capture(ROOT,cls.source)
        cls.rows=cls.case['events'][7:];cls.initial=initial_packet(cls.rows[0],cls.source/'source')
        cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=()):
        return build(directory,driver=ROOT/'tests/cpu_pit_writes.c',
                     clock_source=ROOT/'tests/cpu_pit_clock.c',include_dirs=includes)

    def check(self,directory,commands,strict=True,rows=None,compare_failed=False):
        return replay(directory,self.source,commands,strict=strict,
                      rows=self.rows if rows is None else rows,initial=self.initial,
                      extra_parts=extra_parts,compare_failed=compare_failed)

    def test_continuous_actual_control_low_high_full_state(self):
        result=self.check(self.directory,self.commands)
        self.assertEqual(len(result),2);self.assertEqual(len(self.rows),6)
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def negative(self,name,files,wanted):
        directory=self.directory/name;directory.mkdir()
        for filename,text in files.items():(directory/filename).write_text(text)
        result=self.check(directory,self.build(directory,(directory,)),strict=False)
        for q in result:
            self.assertEqual(q['terminal_exit'],0)
            self.assertTrue(q['complete_CPU_time_equal'])
            self.assertEqual(q['memory_context_pic_errors'],wanted(q['target']))
        return result

    def test_previous_decoder_fails_before_first_pit_write(self):
        directory=self.directory/'previous-decoder';directory.mkdir()
        text=subprocess.check_output(['git','show','3238e0ced01e6cb02f299fef1862a666ecbaa6d5:re_out/fist_exec.h'],cwd=ROOT,text=True)
        (directory/'fist_exec.h').write_text(text)
        results=self.check(directory,self.build(directory,(directory,)),strict=False,
                           rows=self.rows[:1],compare_failed=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0)
            observed=[line for line in (directory/(q['target']+'.log')).read_text().splitlines()
                      if line.startswith('before-pit-control ')]
            self.assertEqual(observed,[observation(self.rows[0])])
            self.assertEqual(q['memory_context_pic_errors'],[])

    def test_control_must_remove_prior_pit_event(self):
        text=(ROOT/'re_out/fist_pit.h').read_text()
        old='  host->remove_events(host->context);\n  if(!fist_pit_output'
        self.assertEqual(text.count(old),1)
        self.negative('keep-event',{'fist_pit.h':text.replace(old,'  if(!fist_pit_output')},
                      lambda target:[(q['kind'],'calendar') for q in self.rows[1:]])

    def test_low_write_must_clear_upper_latch_bits(self):
        text=(ROOT/'re_out/fist_pit.h').read_text();old='case 3:p->write_latch=value&255;'
        self.assertEqual(text.count(old),1)
        self.negative('keep-upper',{'fist_pit.h':text.replace(old,'case 3:p->write_latch=(p->write_latch&0xff00)|(value&255);')},
                      lambda target:[('after-pit-low','pit'),('before-pit-high','pit')])

    def test_full_index_must_promote_original_binary32_phase(self):
        text=(ROOT/'re_out/fist_vga.c').read_text();old='return (double)g_pic_tick+(double)phase;'
        self.assertEqual(text.count(old),1)
        self.negative('double-phase',{'fist_vga.c':text.replace(old,'return (double)g_pic_tick+(double)pic_index(cpu)/30000.0;')},
                      lambda target:[('after-pit-high','pit')])

    def test_timer_frequency_must_round_to_binary32_on_native(self):
        text=(ROOT/'re_out/fist_pit.h').read_text();old='volatile float frequency=(float)FIST_PIT_HZ/(float)count;'
        self.assertEqual(text.count(old),1)
        self.negative('x87-frequency',{'fist_pit.h':text.replace(old,'float frequency=(float)FIST_PIT_HZ/(float)count;')},
                      lambda target:[('after-pit-high','pit'),('after-pit-high','calendar')] if target=='native' else [])

class PitChannelsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-pit-channels-unit-')
        cls.addClassCleanup(cls.storage.cleanup);cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'original';cls.case,cls.original=capture_channels(ROOT,cls.source)
        cls.commands=build(cls.directory,driver=ROOT/'tests/pit_channels.c',
                           clock_source=ROOT/'tests/pit_channels_clock.c',pic=ROOT/'tests/pit_channels_pic.c')

    def compare(self,name):
        for target,run in self.commands:
            with self.subTest(target=target,program=name):
                argument='budget' if name=='budget' else str(self.source/(name+'.txt'))
                p=subprocess.run([*run,argument],cwd=self.directory,capture_output=True,text=True,timeout=30)
                self.assertEqual(p.returncode,0,p.stderr)
                actual=[json.loads(line) for line in p.stdout.splitlines()]
                self.assertEqual(actual,self.original[name])

    def test_complete_original736_budget_states(self):
        self.compare('budget')
        self.assertEqual(len(self.original['budget']),736)

    def test_complete_original_nine_lifecycles_and_retained_constructor(self):
        for name in self.original:
            if name not in ('budget','all-control-values','all-gate-counter2-modes','all-port61-values','port61-timer-gates'):self.compare(name)

    def test_all256_control_values_and_all_three_counters(self):
        self.compare('all-control-values')

    def test_all_gate_access_mode_bcd_count_combinations(self):
        self.compare('all-gate-counter2-modes')

    def test_original_port61_values_read_toggle_and_repeated_writes(self):
        self.compare('all-port61-values')

    def test_original_port61_counter2_gate_and_speaker_type_order(self):
        self.compare('port61-timer-gates')

    def test_port61_gate_must_reach_bound_timer_before_speaker_type(self):
        directory=self.directory/'legacy-port61-gate';directory.mkdir()
        text=(ROOT/'re_out/fist_vga.c').read_text()
        old='if ((g_port61^val)&1)fist_pit_gate2(g_pit_context,&g_pit_host,val&1);'
        self.assertEqual(text.count(old),1)
        legacy='if (((g_port61^val)&1) && (val&1))g_pit_base[2]=clock_now();'
        (directory/'fist_vga.c').write_text(text.replace(old,legacy))
        commands=build(directory,driver=ROOT/'tests/pit_channels.c',clock_source=ROOT/'tests/pit_channels_clock.c',
                       pic=ROOT/'tests/pit_channels_pic.c',include_dirs=(directory,))
        expected=self.original['all-port61-values']
        for target,run in commands:
            p=subprocess.run([*run,str(self.source/'all-port61-values.txt')],cwd=directory,capture_output=True,text=True,timeout=30)
            self.assertEqual(p.returncode,0,p.stderr)
            actual=[json.loads(line) for line in p.stdout.splitlines()]
            self.assertEqual(len(actual),len(expected))
            first=next(i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b)
            self.assertEqual(actual[:first],expected[:first])
            self.assertEqual(actual[first]['kind'],'speaker-type-request')
            self.assertEqual({k for k in actual[first] if actual[first][k]!=expected[first][k]},{'gate2'})
            self.assertEqual(actual[first]['gate2'],0);self.assertEqual(expected[first]['gate2'],1)

    def test_failed_fetch_debt_uses_original_cycleleft(self):
        directory=self.directory/'missing-initial-debt';directory.mkdir()
        text=(ROOT/'re_out/fist_vga.c').read_text()
        old='if (pic_index(clock_cpu_cycles(clock_now())) < 30000)'
        self.assertEqual(text.count(old),1)
        (directory/'fist_vga.c').write_text(text.replace(old,'if (clock_cpu_cycles(clock_now()) % 30000u)'))
        commands=build(directory,driver=ROOT/'tests/pit_channels.c',clock_source=ROOT/'tests/pit_channels_clock.c',
                       pic=ROOT/'tests/pit_channels_pic.c',include_dirs=(directory,))
        for target,run in commands:
            p=subprocess.run([*run,str(self.source/'default-periodic.txt')],cwd=directory,capture_output=True,text=True,timeout=30)
            self.assertEqual(p.returncode,0,p.stderr);actual=[json.loads(line) for line in p.stdout.splitlines()]
            expected=self.original['default-periodic'];self.assertEqual(len(actual),len(expected))
            self.assertEqual(actual[0],expected[0])
            first=actual[1];self.assertEqual({k for k in first if first[k]!=expected[1][k]},{'index_nd','cycles'})
            self.assertEqual(first['index_nd'],expected[1]['index_nd']-1)
            self.assertEqual(first['cycles'],expected[1]['cycles']+1)

    def test_timer_detach_does_not_restore_synthetic_irq_clock(self):
        self.compare('timer-detach')
        directory=self.directory/'legacy-after-detach';directory.mkdir()
        text=(ROOT/'re_out/fist_vga.c').read_text()
        self.assertEqual(text.count('if (!g_pic_machine_calendar) {'),1)
        self.assertEqual(text.count('if (g_pic_machine_calendar) {'),1)
        text=text.replace('if (!g_pic_machine_calendar) {','if (!g_pit_context) {').replace('if (g_pic_machine_calendar) {','if (g_pit_context) {')
        (directory/'fist_vga.c').write_text(text)
        commands=build(directory,driver=ROOT/'tests/pit_channels.c',clock_source=ROOT/'tests/pit_channels_clock.c',
                       pic=ROOT/'tests/pit_channels_pic.c',include_dirs=(directory,))
        for target,run in commands:
            p=subprocess.run([*run,str(self.source/'timer-detach.txt')],cwd=directory,capture_output=True,text=True,timeout=30)
            self.assertNotEqual(p.returncode,0,p.stderr)
            actual=[json.loads(line) for line in p.stdout.splitlines()]
            self.assertEqual(actual,self.original['timer-detach'][:-1])

    def test_bound_calendar_preserves_complete_original_endpoint_state(self):
        for endpoint in (1,60,110):
            directory=self.directory/('endpoint-'+str(endpoint));directory.mkdir()
            source=directory/'source'
            env={k:v for k,v in os.environ.items() if not k.startswith('FIST_')}
            env.update(FIST_SEQUENCE=str(source),FIST_SEQUENCE_END_MS=str(endpoint))
            original=subprocess.run([str(self.source/'source'),str(self.source/'default-periodic.txt')],
                                    env=env,capture_output=True,text=True,timeout=30)
            self.assertEqual(original.returncode,0,original.stderr)
            expected=[json.loads(line) for line in original.stdout.splitlines()]
            self.assertEqual(expected[-1]['label'],0xffffffff)
            self.assertEqual(expected[-1]['tick'],endpoint)
            self.assertEqual(expected[-1]['index_nd'],0)
            self.assertEqual(expected[-1]['cycles'],0);self.assertEqual(expected[-1]['left'],30000)
            marker=Path(str(source)+'.end').read_bytes()
            for target,run in self.commands:
                with self.subTest(target=target,endpoint=endpoint):
                    prefix=directory/target
                    p=subprocess.run([*run,str(self.source/'default-periodic.txt'),str(endpoint),str(prefix)],cwd=directory,
                                     env=dict(env,FIST_SEQUENCE=str(prefix)),capture_output=True,text=True,timeout=30)
                    self.assertEqual(p.returncode,0,p.stderr)
                    self.assertEqual([json.loads(line) for line in p.stdout.splitlines()],expected)
                    self.assertEqual(Path(str(prefix)+'.end').read_bytes(),marker)

if __name__=='__main__':unittest.main()
