"""Actual DOS software entry and outer RETF preserve complete CPU/memory."""
import copy
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

import test_cpu_interrupt as interrupt_fixture
from device_cpu_fixture import words
from memory_context_fixture import system_words,memory_packet,expected_memory_context
from test_port_io import ROOT
sys.path.insert(0,str(ROOT/'tools/oracle'))
from capture_software_dos import verify
from capture_software_startup_dos import capture,verify as verify_complete
from capture_pit_irq_frames import shared_physical


class CpuSoftwareDosTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-software-dos-test-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name);cls.original=cls.directory/'original'
        cls.case=capture(ROOT,cls.original)
        cls.events=[json.loads(l) for l in (cls.original/'source/software-events.jsonl').read_text().splitlines()]
        cls.commands=interrupt_fixture.CpuInterruptTest.build(cls.directory,ROOT/'tests/cpu_interrupt.c')

    def run_pair(self,index,commands=None,directory=None):
        directory=directory or self.directory;source=self.original/'source'
        before,after=self.events[index*2:index*2+2];args=before['arguments']
        op=[7,args['num'],args['oldeip']] if index==0 else [8,args['use32'],args['bytes']]
        packet=directory/(str(index)+'.input')
        packet.write_bytes(struct.pack('<61I',*op,*words(before),*system_words(before))+memory_packet(before,source))
        expected=' '.join('%08x'%v for v in words(after)+system_words(after))+'\ncomplete-RAM 16777216\n'
        for target,run in commands or self.commands:
            output=directory/(target+'-'+str(index)+'.memory');context=output.with_suffix('.context')
            result=subprocess.run([*run,str(packet),str(source/before['memory_file']),str(output),
                                   str(source/after['memory_file']),str(context)],capture_output=True,text=True,timeout=30)
            (directory/(target+'-'+str(index)+'.log')).write_text(result.stdout+result.stderr)
            yield target,result,expected,output,context,after

    def test_actual_software_entry_and_far_return_match_all_cpu_ram_cache_vga(self):
        self.assertEqual(len(self.events),5)
        for index in (0,1):
            for target,result,expected,output,context,after in self.run_pair(index):
                with self.subTest(target=target,pair=index):
                    self.assertEqual(result.returncode,0,result.stderr)
                    self.assertEqual(result.stdout,expected)
                    self.assertTrue(output.read_bytes()==(self.original/'source'/after['memory_file']).read_bytes(),'complete16MiB RAM differs')
                    self.assertTrue(context.read_bytes()==expected_memory_context(after,self.original/'source'),'complete provider/cache/VGA differs')
        header=(ROOT/'third_party/dosbox-build/dosbox-0.74-3/include/cpu.h').read_text()
        import re
        kind=int(re.search(r'^#define CPU_INT_SOFTWARE\s+(0x\w+)',header,re.M)[1],16)
        self.assertEqual(self.events[0]['arguments']['type'],kind)
        self.assertIn('FIST_INT_SOFTWARE=0x%xu'%kind,(ROOT/'re_out/fist_interrupt.h').read_text())

    def test_continuous_actual_byte_execution_matches_complete_startup_and_both_handlers(self):
        from cpu_execute_fixture import build, replay
        directory=self.directory/'continuous'
        source=self.original/'source'
        receipts=replay(directory,source,build(directory,source))
        self.assertEqual(len(receipts),2)
        self.assertEqual([q['CPU_time'] for q in receipts],[1034,1034])

    def test_missing_movss_fetch_credit_reaches_first_wrong_original_budget(self):
        from cpu_execute_fixture import build, inputs, read
        header=(ROOT/'re_out/fist_exec.h').read_text()
        old='if(index==2)e->credit(e->opaque);'
        self.assertEqual(header.count(old),1)
        directory=self.directory/'missing-MOVSS-credit';source=self.original/'source'
        runs=build(directory,source,header.replace(old,'/* Deliberate negative: lose CPU_Cycles++. */'))
        packet,expected,_,_=inputs(directory,source)
        rows=[q for name in ('prefix-fetches.jsonl','software-fetches.jsonl','following/fetches.jsonl','findfirst/fetches.jsonl') for q in read(source/name)]
        def movss(q):
            code=bytes.fromhex(q['fetched_code_hex']);at=0
            while code[at] in (0x66,0x67,0x2e,0x36,0x3e,0x26,0x64,0x65,0xf2,0xf3):at+=1
            return code[at]==0x8e and ((code[at+1]>>3)&7)==2
        expected_rows=expected.splitlines()
        reached=next(q for q in rows if movss(q))
        first=next(i for i,line in enumerate(expected_rows) if line.startswith('fetch ') and
                   int(line.split()[13],16)==reached['cpu_regs.ip.dword[0]'] and
                   int(line.split()[16],16)==reached['segments'][1]['value'])+1
        for target,run in runs:
            with self.subTest(target=target,first=first):
                output=directory/(target+'-output')
                try:
                    result=subprocess.run([*run,str(packet),str(source/'77e2.memory'),str(output),str(source/'game/FISTDATA')],
                                          capture_output=True,text=True,timeout=30)
                    actual=result.stdout.splitlines()
                    self.assertGreater(len(actual),first)
                    self.assertEqual(actual[:first],expected_rows[:first])
                    a,b=actual[first].split(),expected_rows[first].split()
                    self.assertEqual(int(a[1]),int(b[1])+1)
                    self.assertEqual(int(a[3]),int(b[3])-1)
                    self.assertEqual(a[5:],b[5:])
                finally:
                    for p in directory.glob(target+'-output-*'):p.unlink()

    def build_mutant(self,name,header):
        directory=self.directory/name;directory.mkdir()
        (directory/'fist_interrupt.h').write_text(header)
        source=directory/'cpu_interrupt.c';source.write_bytes((ROOT/'tests/cpu_interrupt.c').read_bytes())
        return directory,interrupt_fixture.CpuInterruptTest.build(directory,source)

    def test_hardware_substitution_reaches_wrong_saved_software_return(self):
        header=(ROOT/'re_out/fist_interrupt.h').read_text()
        old='    fist_cpu_interrupt(bus,num,FIST_INT_SOFTWARE,oldeip);'
        self.assertEqual(header.count(old),1)
        directory,commands=self.build_mutant('hardware-substitution',header.replace(old,'    fist_cpu_interrupt(bus,num,0,bus->cpu->eip);'))
        before=self.events[0];after=self.events[1];source=self.original/'source'
        expected_ram=(source/after['memory_file']).read_bytes()
        address=shared_physical(ROOT)(after['segments'][2]['base']+(after['registers'][4]&after['cpu.stack.mask']),expected_ram,after)
        self.assertEqual(struct.unpack_from('<I',expected_ram,address)[0],before['arguments']['oldeip'])
        for target,result,expected,output,context,q in self.run_pair(0,commands,directory):
            with self.subTest(target=target):
                self.assertEqual(result.returncode,10,result.stderr)
                self.assertEqual(result.stdout,expected.split('\ncomplete-RAM')[0]+'\n')
                ram=output.read_bytes();self.assertEqual(len(ram),len(expected_ram))
                corrected=bytearray(ram);struct.pack_into('<I',corrected,address,before['arguments']['oldeip'])
                self.assertEqual(struct.unpack_from('<I',ram,address)[0],before['cpu_regs.ip.dword[0]'])
                self.assertTrue(corrected==expected_ram,'defect must be exactly the saved software return IP')

    def test_near_return_substitution_preserves_ram_but_loses_cs_cpl_esp(self):
        header=(ROOT/'re_out/fist_interrupt.h').read_text()
        start=header.index('    FistCpuState *cpu=bus->cpu;',header.index('static inline void fist_cpu_far_ret'))
        mutant=header[:start]+'    bus->cpu->eip=fist_cpu_pop(bus,use32 ? 4 : 2);return;\n'+header[start:]
        directory,commands=self.build_mutant('near-substitution',mutant)
        for target,result,expected,output,context,after in self.run_pair(1,commands,directory):
            with self.subTest(target=target):
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertNotEqual(result.stdout,expected)
                self.assertTrue(output.read_bytes()==(self.original/'source'/after['memory_file']).read_bytes())
                actual=[int(v,16) for v in result.stdout.splitlines()[0].split()]
                self.assertEqual(actual[9+2*1],8) # Actual source kernel CS remains instead of caller2b.
                self.assertEqual(actual[33],0) # CPL remains0 instead of3.

    def test_missing_and_coherently_wrong_source_boundaries_fail(self):
        source=self.original/'source'
        for name in ('missing-ret-RAM','missing-VGA','wrong-saved-return','wrong-return-clock'):
            with self.subTest(case=name):
                folder=self.directory/name;folder.mkdir();(folder/'source').mkdir()
                for p in self.original.iterdir():
                    if p.name!='source':(folder/p.name).symlink_to(p)
                for p in source.iterdir():(folder/'source'/p.name).symlink_to(p)
                def replace(path,data):
                    target=folder/'source'/path;target.unlink();target.write_bytes(data)
                if name=='missing-ret-RAM':(folder/'source'/self.events[3]['memory_file']).unlink()
                elif name=='missing-VGA':(folder/'source'/self.events[3]['memory_context']['vga']['fastmem']['file']).unlink()
                elif name=='wrong-saved-return':
                    q=self.events[1];ram=bytearray((source/q['memory_file']).read_bytes())
                    address=shared_physical(ROOT)(q['segments'][2]['base']+(q['registers'][4]&q['cpu.stack.mask']),ram,q)
                    struct.pack_into('<I',ram,address,self.events[0]['cpu_regs.ip.dword[0]'])
                    replace(q['memory_file'],ram)
                else:
                    rows=copy.deepcopy(self.events);rows[3]['CPU_Cycles']-=1
                    replace('software-events.jsonl',(''.join(json.dumps(q)+'\n' for q in rows)).encode())
                with self.assertRaises((AssertionError,OSError,ValueError)):verify(ROOT,folder,reference=False)

    def test_complete_startup_source_rejects_missing_or_incorrect_findfirst_context(self):
        source=self.original/'source'
        find=[json.loads(l) for l in (source/'findfirst/events.jsonl').read_text().splitlines()]
        cases=(('missing-FindFirst-RAM',source/find[9]['memory_file']),
               ('short-FindFirst-RAM',source/find[9]['memory_file']),
               ('missing-FindFirst-VGA',source/find[9]['memory_context']['vga']['fastmem']['file']),
               ('short-following',source/'following/fetches.jsonl'),
               ('wrong-directory-slot',source/'host-events.jsonl'),
               ('wrong-host-name-tail',source/'host-events.jsonl'),
               ('wrong-FindFirst-return',source/'findfirst/events.jsonl'))
        for name,path in cases:
            with self.subTest(case=name):
                original=path.read_bytes()
                try:
                    if name.startswith('missing-'):path.unlink()
                    elif name=='short-FindFirst-RAM':path.write_bytes(original[:-1])
                    elif name=='short-following':path.write_bytes(b'\n'.join(original.splitlines()[:-1])+b'\n')
                    else:
                        rows=[json.loads(l) for l in original.decode().splitlines()]
                        if name=='wrong-directory-slot':rows[2]['occupied'][3]=True
                        elif name=='wrong-host-name-tail':
                            args=rows[3]['arguments'];args['name_raw_hex']=args['name_raw_hex'][:-2]+'ff'
                        else:rows[12]['arguments']['oldeip']-=1
                        path.write_text(''.join(json.dumps(q)+'\n' for q in rows))
                    with self.assertRaises((AssertionError,OSError,ValueError)):verify_complete(ROOT,self.original)
                finally:path.write_bytes(original)
        verify_complete(ROOT,self.original)


if __name__=='__main__':unittest.main()
