"""Require actual first-IRQ0 instructions, narrow lazy flags and continuous state."""
import json
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from cpu_core_exit_fixture import build,replay,observation
from test_port_io import ROOT

sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_cpu_irq_prefix import capture

class CpuIrqPrefixTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-irq-prefix-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name);cls.source=cls.directory/'original'
        # Unsupported-opcode substitutions must fail without producing large cores.
        original_limit=resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE,(0,original_limit[1]))
        cls.addClassCleanup(resource.setrlimit,resource.RLIMIT_CORE,original_limit)
        cls.case=capture(ROOT,cls.source)
        cls.commands=build(cls.directory,driver=ROOT/'tests/cpu_irq_prefix.c')

    def test_continuous_original_iret_pic_frame_and_first_irq0_prefix(self):
        result=replay(self.directory,self.source,self.commands,fetches=self.case['fetches'])
        self.assertEqual(len(result),2);self.assertEqual(len(self.case['events']),8)
        self.assertEqual(len(self.case['fetches']),21)
        self.assertEqual(self.case['frames'],39);self.assertEqual(self.case['samples'],27518)

    def reject_missing_instruction(self,name,headers,fetch_count,last_ip):
        directory=self.directory/name;directory.mkdir();include=directory/'headers';include.mkdir()
        for filename,text in headers.items():(include/filename).write_text(text)
        commands=build(directory,driver=ROOT/'tests/cpu_irq_prefix.c',include_dirs=(include,))
        results=replay(directory,self.source,commands,strict=False,fetches=self.case['fetches'])
        for q in results:
            self.assertNotEqual(q['terminal_exit'],0)
            lines=(directory/(q['target']+'.log')).read_text().splitlines()
            observed=[line for line in lines if line.startswith('fetch ')]
            self.assertEqual(len(observed),fetch_count)
            self.assertEqual(lines[:len(self.case['events'])-1],
                             [observation(row) for row in self.case['events'][:-1]])
            self.assertEqual(observed,[observation(row) for row in self.case['fetches'][:fetch_count]])
            self.assertEqual(int(observed[-1].split()[13],16),last_ip)
            self.assertFalse(q['complete_CPU_time_equal'])

    def test_previous_shared_decoder_fails_at_actual_byte_add(self):
        headers={name:subprocess.check_output(['git','show','5f1362450eab9d6b8a4e7a22d62f8190bd2517e8:re_out/'+name],cwd=ROOT,text=True)
                 for name in ('fist_cpu.h','fist_exec.h')}
        self.reject_missing_instruction('previous-decoder',headers,16,0x3a82)

    def test_byte_add_alone_still_fails_at_actual_nop(self):
        text=(ROOT/'re_out/fist_exec.h').read_text()
        old='  if(op==0x90){}\n  else if(op==0xfc)'
        self.assertEqual(text.count(old),1)
        text=text.replace(old,'  if(op==0xfc)')
        self.reject_missing_instruction('missing-nop',{'fist_exec.h':text},20,0x3a8f)

class CpuByteAddTest(unittest.TestCase):
    def test_exhaustive_original_byte_add_fill_flags_and_inc_carry(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-byte-add-unit-') as temp:
            directory=Path(temp);tree=ROOT/'third_party/dosbox-build/dosbox-0.74-3'
            original=directory/'original'
            compile=['g++','-O2','-std=gnu++11',
                     *subprocess.check_output(['sdl-config','--cflags'],text=True).split(),
                     '-I'+str(tree/'include'),'-I'+str(tree),'-ffunction-sections','-fdata-sections',
                     str(ROOT/'tools/oracle/cpu_byte_add_probe.cpp'),'-Wl,--gc-sections','-o',str(original)]
            p=subprocess.run(compile,capture_output=True,text=True,timeout=60)
            self.assertEqual(p.returncode,0,p.stderr)
            reference=directory/'original.raw'
            subprocess.run([str(original),str(reference)],check=True,capture_output=True,timeout=30)
            expected=reference.read_bytes();self.assertEqual(len(expected),131072*4*10*4)
            reference.unlink()
            for target,run in build(directory,driver=ROOT/'tests/cpu_byte_add.c'):
                with self.subTest(target=target):
                    output=directory/(target+'.raw')
                    p=subprocess.run([*run,str(output)],capture_output=True,timeout=30)
                    self.assertEqual(p.returncode,0,p.stderr)
                    self.assertEqual(output.read_bytes(),expected)
                    output.unlink()

if __name__=='__main__':unittest.main()
