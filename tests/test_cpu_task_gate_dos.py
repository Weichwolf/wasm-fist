"""Actual reset/DTA/FindFirst path with complete source CPU and host states."""
import json
from pathlib import Path
import resource
import tempfile
import unittest
from test_port_io import ROOT
from capture_cpu_task_gate import capture,observer,verify,source_inputs
from cpu_task_gate_fixture import build,replay
from cpu_execute_fixture import layout
import subprocess

FIXTURES=ROOT
OWNER=ROOT/'re_out'
INCLUDES=()


class TaskGateDosTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-task-gate-dos-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name)
        limit=resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,limit)
        cls.source=cls.directory/'source'
        cls.case=capture(ROOT,cls.source,make_observer=lambda repo:observer(repo,True),
            check_capture=lambda repo,root:verify(repo,root,True),
            additional_inputs=source_inputs(ROOT),source_wall_seconds=180)
        cls.rows=cls.case['events'][:29]
        cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,includes=(),flags=()):
        return build(directory,driver=FIXTURES/'tests/cpu_task_gate.c',
                     include_dirs=(*includes,*INCLUDES),extra_flags=flags)

    def run_variant(self,name,header,*,rows=None,find=True,flags=()):
        directory=self.directory/name;directory.mkdir()
        includes=()
        if header is not None:
            (directory/'fist_dos_cpu.h').write_text(header);includes=(directory,)
        return replay(directory,self.source,self.build(directory,includes,flags),rows or self.rows,
                      dos=True,dos_events=self.case['find_events'] if find else (),
                      strict=False,compare_artifacts=False)

    def assert_cause(self,results,*,kind,first,states=False,host=False):
        for q in results:
            self.assertEqual((q['terminal_exit'],q['boundaries'],q['fetches']),(0,39,1125))
            errors=[a for a in q['artifacts'] if not a['equal']]
            if kind:
                self.assertTrue(errors);self.assertEqual({a['kind'] for a in errors},{kind})
                self.assertEqual(errors[0]['boundary'],first)
            else:self.assertFalse(errors)
            self.assertEqual(bool(q['state_errors']),states)
            self.assertEqual(bool(q['host_errors']),host)
            self.assertEqual(q['trace_equal'],not states)
            if states:self.assertEqual(q['state_errors'][0]['boundary'],first)
            if host:self.assertEqual(q['host_errors'][0]['boundary'],first)

    def test_complete39_states1125_fetches_and40_host_packets(self):
        results=replay(self.directory,self.source,self.commands,self.rows,
                       dos=True,dos_events=self.case['find_events'])
        for q in results:
            self.assertEqual((q['terminal_exit'],q['boundaries'],q['fetches']),(0,39,1125))
            self.assertTrue(q['CPU_time_equal'] and q['trace_equal'] and q['complete_context_equal'])
            self.assertFalse(q['host_errors'])
        self.assertEqual(len(self.case['events']),31)
        self.assertEqual(len(self.case['find_events']),10)

    def test_previous_DTA_only_reaches_and_rejects_actual_FindFirst(self):
        directory=self.directory/'previous-service';directory.mkdir()
        results=replay(directory,self.source,
            self.build(directory,flags=('-DFIST_TASK_GATE_DTA_ONLY',)),self.rows,
            complete=False,expected_rows=self.rows[:27],dos=True)
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0)
            self.assertEqual((q['boundaries'],q['fetches']),(27,1042))
            self.assertTrue(q['CPU_time_equal']);self.assertFalse(q['host_errors'])

    def test_FindFirst_PSP_access_links_original_guest_page(self):
        header=(OWNER/'fist_dos_cpu.h').read_text()
        at=header.index('static inline void fist_dos_cpu_find_first(')
        body=header[at:];marker=' fist_dos_cpu_save_stack(bus);'
        self.assertEqual(body.count(marker),1)
        results=self.run_variant('find-no-PSP',header[:at]+body.replace(marker,''))
        self.assert_cause(results,kind='context',first='find-before-FindFirst')

    def replace_cause(self,name,marker,replacement,*,kind,first,states=False,host=False):
        header=(OWNER/'fist_dos_cpu.h').read_text();self.assertEqual(header.count(marker),1)
        results=self.run_variant(name,header.replace(marker,replacement))
        self.assert_cause(results,kind=kind,first=first,states=states,host=host)

    def test_FindFirst_directory_ID_is_written_before_SetResult(self):
        self.replace_cause('find-no-ID',' fist_ram_write(bus,pt+offsetof(FistDosDta,dirID),2,id);','',
                           kind='memory',first='find-before-SetResult')

    def test_FindFirst_size_uses_original_DWORD_store(self):
        self.replace_cause('find-WORD-size','fist_ram_write(bus,pt+offsetof(FistDosDta,size),4,size);',
            'fist_ram_write(bus,pt+offsetof(FistDosDta,size),2,size);',kind='memory',first='find-after-SetResult')

    def test_FindFirst_date_uses_original_WORD_store(self):
        self.replace_cause('find-DWORD-date','fist_ram_write(bus,pt+offsetof(FistDosDta,date),2,date);',
            'fist_ram_write(bus,pt+offsetof(FistDosDta,date),4,date);',kind='memory',first='find-after-SetResult')

    def test_FindFirst_advances_original_directory_cache_cursor(self):
        self.replace_cause('find-no-cursor','host->next_free=(host->next_free+1)%2048;\n if(scans==2048)',
            '\n if(scans==2048)',kind=None,first='find-after-directory',host=True)

    def test_FindFirst_clears_WORD_AX_after_success(self):
        self.replace_cause('find-unchanged-AX',' cpu->eax=fist_cpu_low(cpu->eax,0,16);','',
            kind=None,first='find-after-DOS21',states=True)

    def check_DTA_cause(self,name,marker,replacement,kinds=('memory',)):
        header=(OWNER/'fist_dos_cpu.h').read_text();self.assertEqual(header.count(marker),1)
        results=self.run_variant(name,header.replace(marker,replacement),rows=self.rows[:25],find=False)
        for q in results:
            self.assertEqual((q['terminal_exit'],q['boundaries'],q['fetches']),(0,25,586))
            self.assertTrue(q['CPU_time_equal'] and q['trace_equal'])
            self.assertFalse(q['host_errors'])
            errors=[a for a in q['artifacts'] if not a['equal']]
            self.assertTrue(errors);self.assertEqual({a['kind'] for a in errors},set(kinds))
            self.assertEqual(errors[0]['boundary'],'after-dos-callback')

    def test_DTA_entry_saves_original_PSP_stack(self):
        self.check_DTA_cause('DTA-no-PSP',' fist_dos_cpu_save_stack(bus);\n fist_ram_write(bus,FIST_DOS_SDA',
                             '\n fist_ram_write(bus,FIST_DOS_SDA',kinds=('memory','context'))

    def test_DTA_saved_SP_uses_original_eighteen_byte_decrement(self):
        self.check_DTA_cause('DTA-wrong-SP','(uint16_t)(cpu->esp-18)','(uint16_t)(cpu->esp-20)')

    def test_DTA_stores_real_DS_selector_not_segment_base(self):
        self.check_DTA_cause('DTA-wrong-DS','(cpu->segments[3].value<<16)|(uint16_t)cpu->edx',
                             '(cpu->segments[3].base<<16)|(uint16_t)cpu->edx')

    def test_layout_matches_original_packed_DTA_PSP_and_SDA(self):
        directory=self.directory/'layout';directory.mkdir()
        callback=next(q for q in self.rows if q['kind']=='before-dos-callback')
        (directory/'software-fetches.jsonl').write_text(json.dumps(dict(
            segments=callback['segments'],fetched_code_hex=callback['opcode_hex']))+'\n')
        layout(directory,directory)
        text='#include "fist_dos_cpu.h"\n#include "source_layout.h"\n'
        for a,b in [('FIST_DOS_SDA','SOURCE_SDA'),('FIST_DOS_SDA_DTA','SOURCE_SDA_DTA'),
                    ('FIST_DOS_SDA_PSP','SOURCE_SDA_PSP'),('FIST_DOS_PSP_STACK','SOURCE_PSP_STACK')]:
            text+='_Static_assert(%s==%s,"%s");\n'%(a,b,a)
        for field in ('sdrive','sname','sext','sattr','dirID','dirCluster','fill','attr','time','date','size','name'):
            text+='_Static_assert(offsetof(FistDosDta,%s)==SOURCE_DTA_%s,"%s");\n'%(field,field,field)
        text+='_Static_assert(sizeof(FistDosDta)==43,"packed original DTA size");\n'
        unit=directory/'check.c';unit.write_text(text)
        result=subprocess.run(['gcc','-m32','-std=c11','-I'+str(OWNER),'-I'+str(ROOT/'re_out'),
                               '-I'+str(directory),'-c',str(unit),'-o',str(directory/'check.o')],
                              capture_output=True,text=True,timeout=30)
        self.assertEqual(result.returncode,0,result.stderr)
