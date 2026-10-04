"""Instruction-pointer width and segmented REP chunks against original core bodies."""
import itertools
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from test_port_io import ROOT, tool
sys.path.insert(0,str(ROOT/'tools/oracle'))
from cpu_execute_probe import build as build_original


def packet(code, big=0, stack=0, ip=0xfffe, flags=0x3206, ecx=2,
           esi=0x8000, edi=0x8010, direction=1, budget=7):
    registers=(0x76543210,ecx,0x12345678,0xabcdef01,0x28000,0x89abcdef,esi,edi)
    flags=(flags&~0x400)|(0x400 if direction<0 else 0)
    return struct.pack('<16I',big,stack,ip,flags,*registers,direction&0xffffffff,budget,0,len(code))+code


class CpuExecuteTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage=tempfile.TemporaryDirectory(prefix='wasm-fist-execute-test-');cls.addClassCleanup(cls.storage.cleanup)
        cls.directory=Path(cls.storage.name);cls.original=build_original(ROOT,cls.directory)
        cls.commands=cls.build(cls.directory)

    @classmethod
    def build(cls,directory,header=None):
        directory.mkdir(parents=True,exist_ok=True)
        source=directory/'controlled.c';source.write_bytes((ROOT/'tests/cpu_execute_controlled.c').read_bytes())
        if header is not None:(directory/'fist_exec.h').write_text(header)
        flags=['-O2','-DNDEBUG','-I'+str(directory),'-I'+str(ROOT/'re_out')]
        native,wasm=directory/'native',directory/'wasm.js'
        result=[]
        for target,build,run in (
            ('native',['gcc','-m32',*flags,str(source),'-o',str(native)],[str(native)]),
            ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),*flags,str(source),'-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1','-o',str(wasm)],
             [tool('node','Git/emsdk/node/*/bin/node'),str(wasm)])):
            p=subprocess.run(build,capture_output=True,text=True,timeout=120)
            if p.returncode:raise RuntimeError(p.stderr)
            result.append((target,run))
        return result

    def compare(self,cases):
        for start in range(0,len(cases),128):
            batch=cases[start:start+128];inputs=b''.join(batch)
            expected=subprocess.run([self.original],input=inputs,capture_output=True,check=True,timeout=30).stdout
            self.assertEqual(len(expected),len(batch)*(19*4+0x40000))
            for target,run in self.commands:
                with self.subTest(target=target,start=start,cases=len(batch)):
                    p=subprocess.run(run,input=inputs,capture_output=True,timeout=30)
                    self.assertEqual(p.returncode,0,p.stderr[:1000])
                    self.assertTrue(p.stdout==expected,'Complete register/flags/budget/physical memory differs')

    def test_operand_sized_jumps_and_full_sequential_eip_match_original(self):
        cases=[]
        for big,width,stack,ip in itertools.product((0,1),(2,4),(0,1),(0xfffd,0xfffe,0xffff,0x1fffe)):
            prefix=b'\x66' if (4 if big else 2)!=width else b''
            for op in (0xb8,0xe8,0xe9,0xeb,0x74,0x75,0xe1,0xe2,0x184,0x185,0x186):
                displacement=width if op in (0xb8,0xe8,0xe9) or op>0xff else 1
                for delta in (0,1,-2):
                    code=prefix+(bytes([0x0f,op&255]) if op>0xff else bytes([op]))+(delta&((1<<(8*displacement))-1)).to_bytes(displacement,'little')
                    for flags in (0x3206,0x3246):cases.append(packet(code,big,stack,ip,flags))
        self.compare(cases)

    def test_rep_movs_chunks_address_wrap_direction_and_overlap_match_original(self):
        cases=[]
        for big,address,width,direction in itertools.product((0,1),(2,4),(1,2,4),(-1,1)):
            default=4 if big else 2
            prefix=(b'\x67' if address!=default else b'')+(b'\x66' if width!=1 and width!=default else b'')
            for count,budget in ((0,1),(0,7),(1,1),(1,7),(2,1),(7,3),(3,7)):
                for rep in (b'\xf2',b'\xf3'):
                    code=prefix+rep+bytes([0xa4 if width==1 else 0xa5])
                    # Upper CX/SI/DI words are preserved only with 16-bit addressing.
                    ecx=(0xabcd0000 if address==2 else 0)|count
                    si=(0x12340000 if address==2 else 0)+0xfffe
                    di=(0x56780000 if address==2 else 0)+0xffff
                    cases.append(packet(code,big=big,ip=0x2000,ecx=ecx,esi=si,edi=di,direction=direction,budget=budget))
        self.compare(cases)

    def test_code_segment_sized_eip_substitution_reaches_original_failure(self):
        header=(ROOT/'re_out/fist_exec.h').read_text()
        old='e->bus->cpu->eip=ip;'
        # Only the ordinary fall-through assignment, not callback SAVEIP.
        self.assertEqual(header.count('\n  '+old),1)
        mutant=header.replace('\n  '+old,'\n  e->bus->cpu->eip=e->bus->cpu->code_big?ip:ip&0xffff;')
        runs=self.build(self.directory/'segment-sized-eip',mutant)
        inputs=packet(b'\xb8\x34\x12',ip=0xffff)
        expected=subprocess.run([self.original],input=inputs,capture_output=True,check=True).stdout
        self.assertEqual(struct.unpack_from('<I',expected,8*4)[0],0x10002)
        for target,run in runs:
            with self.subTest(target=target):
                p=subprocess.run(run,input=inputs,capture_output=True,check=True)
                self.assertEqual(struct.unpack_from('<I',p.stdout,8*4)[0],2)
                self.assertNotEqual(p.stdout,expected)
                self.assertEqual(p.stdout[:32]+p.stdout[36:],expected[:32]+expected[36:])

    def test_whole_rep_substitution_reaches_original_chunk_failure(self):
        header=(ROOT/'re_out/fist_exec.h').read_text()
        old='take=count<budget?count:budget;'
        self.assertEqual(header.count(old),1)
        runs=self.build(self.directory/'whole-rep',header.replace(old,'take=count;'))
        inputs=packet(b'\xf3\xa4',ip=0x2000,ecx=7,esi=0x8000,edi=0x8001,budget=3)
        expected=subprocess.run([self.original],input=inputs,capture_output=True,check=True).stdout
        self.assertEqual(struct.unpack_from('<I',expected,4)[0],4)
        self.assertEqual(struct.unpack_from('<I',expected,8*4)[0],0x2000)
        for target,run in runs:
            with self.subTest(target=target):
                p=subprocess.run(run,input=inputs,capture_output=True,check=True)
                self.assertEqual(len(p.stdout),len(expected))
                self.assertEqual(struct.unpack_from('<I',p.stdout,4)[0],0)
                self.assertEqual(struct.unpack_from('<I',p.stdout,8*4)[0],0x2002)
                self.assertNotEqual(p.stdout,expected)


if __name__=='__main__':unittest.main()
