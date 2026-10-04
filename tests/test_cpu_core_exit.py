"""Require the reaching original core/PIC/frame chain and distinguish exit ordering."""
import json
import itertools
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from cpu_core_exit_fixture import build,replay
from test_port_io import ROOT

sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_core_exit import capture
from core_exit_probe import prepare

class CpuCoreExitTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-core-exit-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name);cls.source=cls.directory/'original'
        cls.case=capture(ROOT,cls.source)
        cls.commands=build(cls.directory)

    def test_complete_reaching_iret_core_pic_frame_and_fetch(self):
        result=replay(self.directory,self.source,self.commands)
        self.assertEqual(len(result),2)
        self.assertEqual(len(self.case['events']),7)
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def test_extra_fetch_before_core_return_changes_only_clock(self):
        directory=self.directory/'extra-fetch';directory.mkdir()
        unit=directory/'clock.c'
        unit.write_text('#define fist_clock_cpu_core_exit original_core_exit\n#include "cpu_core_exit_clock.c"\n#undef fist_clock_cpu_core_exit\nvoid fist_clock_cpu_core_exit(void){fist_clock_charge_cpu_instructions(1);original_core_exit();}\n')
        result=replay(directory,self.source,build(directory,clock_source=unit),strict=False)
        for row in result:
            self.assertEqual(row['terminal_exit'],0)
            self.assertFalse(row['complete_CPU_time_equal'])
            self.assertEqual(row['memory_context_pic_errors'],[])
            lines=(directory/(row['target']+'.log')).read_text().splitlines()
            expected=self.case['events'][2]
            fields=lines[2].split();self.assertEqual(fields[0],'after-core')
            original_time=expected['PIC_Ticks']*30000+30000-expected['CPU_Cycles']-expected['CPU_CycleLeft']
            self.assertEqual(int(fields[1]),original_time+1)
            self.assertEqual(int(fields[3]),expected['CPU_Cycles']-1)

    def test_vector_only_selection_cannot_mark_service_before_cpu_frame(self):
        directory=self.directory/'early-service';directory.mkdir()
        unit=directory/'pic.c'
        text=(ROOT/'re_out/fist_pic.c').read_text()
        old='        if (deliver) deliver(context,*vector);\n'
        self.assertEqual(text.count(old),1)
        text=text.replace(old,'')
        old='        return (int)i;'
        self.assertEqual(text.count(old),1)
        text=text.replace(old,'        if (deliver) deliver(context,*vector);\n'+old)
        changed=directory/'fist_pic.c';changed.write_text(text)
        fixture=(ROOT/'tests/cpu_core_exit_pic.c').read_text().replace('#include "fist_pic.c"','#include "'+str(changed)+'"')
        unit.write_text(fixture)
        result=replay(directory,self.source,build(directory,pic=unit),strict=False)
        for row in result:
            self.assertEqual(row['terminal_exit'],0)
            self.assertTrue(row['complete_CPU_time_equal'])
            self.assertEqual(row['memory_context_pic_errors'],[('before-hardware','pic'),('after-hardware','pic')])

class CpuCoreExitControlledTest(unittest.TestCase):
    def test_original_cli_sti_popf_iret_exits_widths_flags_pending_and_priority(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-core-exit-controlled-') as temp:
            directory=Path(temp)
            original=prepare(ROOT,directory)
            commands=build(directory,driver=ROOT/'tests/cpu_core_exit_controlled.c')
            cases=0
            for case in itertools.product((0xfa,0xfb,0x9d,0xcf),(0,1),(0,1),(0,1,2),
                                           (0x3003,0x3203,0x3303,0x3703),(8,64),(0,1)):
                op,big,flip,pending,flags,budget,stackbig=case
                if op not in (0x9d,0xcf) and flags!=0x3003:continue
                arguments=list(map(str,case))
                source=subprocess.run([*original,*arguments],capture_output=True,timeout=10)
                self.assertEqual(source.returncode,0,(case,source.stderr))
                self.assertEqual(source.stdout.count(b'\n'),2)
                for target,command in commands:
                    with self.subTest(target=target,case=case):
                        result=subprocess.run([*command,*arguments],capture_output=True,timeout=15)
                        self.assertEqual(result.returncode,0,result.stderr)
                        self.assertEqual(result.stdout,source.stdout)
                cases+=1
            self.assertEqual(cases,480)

if __name__=='__main__':unittest.main()
