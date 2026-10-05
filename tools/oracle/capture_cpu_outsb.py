#!/usr/bin/env python3
"""Compare actual original OUTSB, complete observations and deliberate causal faults.

The original fixture executes CASE_B(0x6e), CPU_IO_Exception and DoString
from local DOSBox source. These controlled real-mode cases observe I/O without
hardware delay; the continuous production-device regression owns that transport.
"""
import argparse,hashlib,itertools,json,re,resource,struct,subprocess,sys
from pathlib import Path
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build,digest


def programs(repo,root):
    header=(repo/'re_out/fist_exec.h').read_text()
    source=root/'source';source.mkdir(exist_ok=False)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3';cpu_source=(tree/'src/cpu/cpu.cpp').read_text()
    tss=cpu_source[cpu_source.index('class TaskStateSegment {'):cpu_source.index('enum TSwitchType {')]
    prepare=re.search(r'bool CPU_PrepareException\(.*?\n\}',cpu_source,re.S)[0]
    permission=re.search(r'bool CPU_IO_Exception\(.*?\n\}',cpu_source,re.S)[0]
    original='#include <stddef.h>\n'+(repo/'tools/oracle/cpu_execute_probe.cpp').read_text()
    original=original.replace('#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"','#include "'+str(tree/'src/cpu/flags.cpp')+'"')
    rep=(repo/'tools/oracle/rep_probe.cpp').read_text().replace('#define IO_WriteB(...) unused_route()','#define IO_WriteB(port,value) record_io(port,value)')
    rep=rep.replace('static void unused_route(...) { abort(); }','static void record_io(unsigned,unsigned);\nstatic void unused_route(...) { abort(); }')
    rep=rep.replace('#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal/string.h"','#include "'+str(tree/'src/cpu/core_normal/string.h')+'"')
    rep=rep.replace('ram_accesses[65]','ram_accesses[257]').replace('ram_count<16','ram_count<64')
    (source/'rep_probe.cpp').write_text(rep)
    observe=r'''static unsigned io_count,io_rows[1+16*21];
    static void record_io(unsigned port,unsigned value) {
     assert(io_count<16);unsigned *q=io_rows+1+21*io_count++;
     q[0]=port;q[1]=value;for(unsigned i=0;i<8;i++)q[2+i]=reg_32(i);
     q[10]=reg_eip;q[11]=reg_flags;q[12]=lflags.var1.dword[0];q[13]=lflags.var2.dword[0];q[14]=lflags.res.dword[0];
     q[15]=lflags.type;q[16]=lflags.prev_type;q[17]=lflags.oldcf;q[18]=(unsigned)CPU_Cycles;q[19]=(unsigned)cpu.direction;q[20]=cpu.code.big;
     io_rows[0]=io_count;
    }
    #define mem_readw(a) load(a,2)
    #define mem_readd(a) load(a,4)
    '''+tss+prepare+permission+r'''
    void CPU_Exception(Bitu,Bitu) {abort();} /* A reached exception fails this real-mode fixture. */
    '''
    original=original.replace('int main(void) {',observe+'\nint main(void) {')
    original=original.replace('  CPU_Cycles=q[13]-1;LOADIP;','  cpu.pmode=false;cpu.cpl=0;io_count=0;memset(io_rows,0,sizeof io_rows);\n  CPU_Cycles=q[13]-1;LOADIP;')
    original=original.replace('  assert(fwrite(out,sizeof out,1,stdout)==1);','  assert(fwrite(out,sizeof out,1,stdout)==1 && fwrite(io_rows,sizeof io_rows,1,stdout)==1);')
    (root/'original.cpp').write_text(original)
    original_command=original_build(repo,source,source_file=root/'original.cpp',byte_opcodes=(0x6e,),trace=True,lazy_input=True,segment_input=True,group_shift=True,ram_trace=True)
    portable=(repo/'tests/cpu_execute_controlled.c').read_text().replace('ram_accesses[65]','ram_accesses[257]').replace('ram_count<16','ram_count<64')
    observe=r'''static unsigned io_count,io_rows[1+16*21];
    static void record_io(void *opaque,unsigned port,unsigned value) {
     FistCpuState *cpu=(FistCpuState *)opaque;fist_cpu_require(io_count<16);
     unsigned *q=io_rows+1+21*io_count++;q[0]=port;q[1]=value;
     memcpy(q+2,cpu,9*sizeof *q);memcpy(q+11,&cpu->flags,sizeof cpu->flags);q[18]=remaining;
     q[19]=(cpu->flags.flags&0x400u)?0xffffffffu:1;q[20]=cpu->code_big;io_rows[0]=io_count;
    }
    '''
    portable=portable.replace('int main(void) {',observe+'\nint main(void) {')
    portable=portable.replace('  FistExec engine={.bus=&bus,.budget=budget,.credit=credit,.charge=charge};','  io_count=0;memset(io_rows,0,sizeof io_rows);\n  FistExec engine={.bus=&bus,.opaque=&cpu,.out=record_io,.budget=budget,.credit=credit,.charge=charge};')
    portable=portable.replace('  fist_cpu_require(fwrite(out,sizeof out,1,stdout)==1);','  fist_cpu_require(fwrite(out,sizeof out,1,stdout)==1 && fwrite(io_rows,sizeof io_rows,1,stdout)==1);')
    portable='#include "fist_cpu.h"\n'+portable
    # The shared controlled builder supplies fetch/lazy/segment/shift/RAM observers.
    target_folder=root/'targets';target_folder.mkdir();(target_folder/'fist_cpu.h').write_bytes((repo/'re_out/fist_cpu.h').read_bytes())
    commands=target_build(repo,target_folder,header,segment_input=True,group_shift=True,ram_trace=True,controlled_source=portable)
    corpus=[]
    styles=((0,1),(0,7),(1,1),(1,7),(2,1),(7,3),(3,7),(16,7),(8,16))
    segments=[v for selector in (0x101,0x1003,0x2005,0x3007,0x4009,0x500b) for v in (selector,selector<<4)]
    for big,address,stack,rep,operand,direction,segment,(count,budget) in itertools.product((0,1),(2,4),(0,1),(b'',b'\xf2',b'\xf3'),(2,4),(-1,1),(None,0x26,0x2e,0x36,0x3e,0x64,0x65),styles):
     default=4 if big else 2
     code=(b'\x66' if operand!=default else b'')+(b'\x67' if address!=default else b'')+(bytes([segment]) if segment is not None else b'')+rep+b'\x6e'
     ordinal=len(corpus);ip=(0xbff,0xffff,0x1fffe)[ordinal%3]
     high=0xabcd0000 if address==2 else 0
     si=(0x12340000 if address==2 else 0x10000)+(0,1,0xfffe,0xffff)[ordinal%4]
     flags=(0x3003,0xffffffff)[ordinal&1];flags=(flags&~0x400)|(0x400 if direction<0 else 0)
     q=[big,stack,ip,flags,0xfedcba98,high+count,0x9abc03c9,0x12345678,0xcafe8000,0x89abcdef,si,0x5678ffff,direction&0xffffffff,budget,ordinal%65,len(code)]
     corpus.append(struct.pack('<28I',*q,*segments)+code)
    assert len(corpus)==6048
    size=19*4+(1+16*21)*4+12*4+3*8+0x200000+49*4+257*4
    hashes={target:hashlib.sha256() for target in ('original','native','wasm')}
    for start in range(0,len(corpus),16):
     batch=corpus[start:start+16];data=b''.join(batch);expected=None
     for target,command in [('original',[original_command]),*commands]:
      p=subprocess.run(command,input=data,capture_output=True,timeout=45)
      assert p.returncode==0,(target,start,p.stderr.decode())
      assert len(p.stdout)==len(batch)*size,(target,start,len(p.stdout),len(batch)*size)
      if expected is None:expected=p.stdout
      else:assert p.stdout==expected,(target,start,next(i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b))
      hashes[target].update(p.stdout)
     if start%512==0:print('Matched full original OUTSB',min(start+16,len(corpus)),'/',len(corpus),flush=True)
    paths=[Path(__file__),root/'original.cpp',source/'rep_probe.cpp',target_folder/'controlled.c',repo/'re_out/fist_cpu.h',repo/'re_out/fist_exec.h',repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',tree/'src/cpu/cpu.cpp',tree/'src/cpu/core_normal/string.h',tree/'src/cpu/core_normal/prefix_none.h',*tree.rglob('*.h'),*sorted((repo/'re_out').glob('*.h'))]
    proof=dict(scope='6048 actual original OUTSB CASE_B/CPU_IO_Exception/DoString versus shared MOVS/OUTSB owner. All19 CPU/budget/lazy words,all12 cached segment words,three64-bit stack metadata words,all2MiB RAM,every code and RAM access and ordered port/value/full CPU-before-IO records match both release targets. All code/address/stack sizes,byte66 independence,both directions,all segment overrides,REP/F2/F3/nonREP,counts0..16/budgets1..16,address16 wrap/dirty upper words and all65 incoming lazy tags are covered. The fixture IO provider records writes without hardware delay; actual device/timing/full-DAC integration has its independent reaching evidence. Protected IO permission faults,full runtime and complete frame/audio acceptance remain separate.',cases=len(corpus),record_size=size,outputs_sha256={target:h.hexdigest() for target,h in hashes.items()},corpus_sha256=hashlib.sha256(b''.join(corpus)).hexdigest(),inputs_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS all6048 complete original OUTSB programs on both release targets',flush=True)
    return proof,original_command,commands,portable


def causal(repo,root,positive_proof,original_command,commands,driver):
    work=root/'causal';work.mkdir()
    header=(repo/'re_out/fist_exec.h').read_text();cpu=(repo/'re_out/fist_cpu.h').read_text()
    size=positive_proof['record_size'];memory_start=19*4+(1+16*21)*4+12*4+3*8
    original=[original_command]
    def packet(code,count=3,budget=7,direction=1):
        flags=0x3287|(0x400 if direction<0 else 0)
        q=[0,0,0xbff,flags,0xfedcba98,count,0x9abc03c9,0x12345678,
           0xcafe8000,0x89abcdef,0x8000,0x9000,direction&0xffffffff,budget,4,len(code)]
        segments=[v for selector in (0x101,0x1003,0x2005,0x3007,0x4009,0x500b)
                  for v in (selector,selector<<4)]
        return struct.pack('<28I',*q,*segments)+code
    credit='    e->charge(e->opaque,cost);\n';footer='   if(rep){fist_exec_reg_write(e,1,address,count-take);if(count>take)ip=e->bus->cpu->eip;}'
    assert header.count(credit)==header.count(footer)==1
    si='    si=fist_cpu_low(0,si+e->bus->system->direction*w,8*address);';assert header.count(si)==1
    load='    uint32_t v=fist_ram_resident_read(e->bus,seg<6?seg:3,si,w);';assert header.count(load)==1
    width='   unsigned w=op==0xa5?width:1;';assert header.count(width)==1
    mutations=[
     ('cost-after-io',header.replace(credit,'').replace(footer,footer+'if(rep)e->charge(e->opaque,cost);'),packet(b'\xf3\x6e')),
     ('si-committed-per-byte',header.replace(si,si+'fist_exec_reg_write(e,6,address,si);'),packet(b'\xf3\x6e')),
     ('ignored-cs-source',header.replace(load,load.replace('seg<6?seg:3','3')),packet(b'\x2e\xf3\x6e')),
     ('ignored-direction',header.replace(si,si.replace('e->bus->system->direction*w','w')),packet(b'\xf3\x6e',direction=-1)),
     ('widened-outs-byte',header.replace(width,'   unsigned w=width;'),packet(b'\x66\xf3\x6e')),
     ('whole-rep',header.replace('take=count<budget?count:budget;','take=count;'),packet(b'\xf3\x6e',count=7,budget=3)),
    ]
    def run(command,data):
     p=subprocess.run(command,input=data,capture_output=True,timeout=30)
     assert p.returncode==0 and len(p.stdout)==size,(command,p.returncode,len(p.stdout),p.stderr.decode())
     return p.stdout
    results=[]
    for name,mutant,data in mutations:
     assert mutant!=header
     expected=run(original,data)
     for target,command in commands:assert run(command,data)==expected,(name,target,'unmodified positive')
     folder=work/name;folder.mkdir();(folder/'fist_cpu.h').write_text(cpu)
     for target,command in target_build(repo,folder,mutant,segment_input=True,group_shift=True,ram_trace=True,controlled_source=driver):
      actual=run(command,data);assert actual!=expected,(name,target)
      a,b=struct.unpack_from('<19I',actual),struct.unpack_from('<19I',expected)
      fields=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
      assert actual[memory_start:memory_start+0x200000]==expected[memory_start:memory_start+0x200000]
      io_a=struct.unpack_from('<337I',actual,76);io_b=struct.unpack_from('<337I',expected,76)
      first=next((i for i in range(min(io_a[0],io_b[0])) if io_a[1+21*i:1+21*(i+1)]!=io_b[1+21*i:1+21*(i+1)]),None)
      if name in ('cost-after-io','si-committed-per-byte'):
       assert not fields and first==(0 if name=='cost-after-io' else 1),(name,target,fields,first)
       ia=io_a[1+21*first:1+21*(first+1)];ib=io_b[1+21*first:1+21*(first+1)]
       changed=[i for i,(x,y) in enumerate(zip(ia,ib)) if x!=y]
       assert changed==([18] if name=='cost-after-io' else [8]),(name,target,changed)
      results.append(dict(fault=name,target=target,differing_final_CPU_words=fields,all2MiB_RAM_equal=True,first_differing_IO_record=first,original_IO_writes=io_b[0],mutant_IO_writes=io_a[0],unmodified_complete_positive_equal=True,input_sha256=hashlib.sha256(data).hexdigest(),original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(actual).hexdigest()))
     print('PASS controlled original OUTSB distinguishes',name,'on both targets',flush=True)
    proof=dict(scope='12 causal OUTSB programs on both release targets. Every case first checks both unmodified targets against the original. Six deliberate faults isolate pre-IO REP cost,late register commit,segment source,direction,byte width andbounded chunk. Full final RAM stays equal. Cost/SI faults leave every final CPU word equal but differ only in CPU budget/ESI inside their first differing IO record. Whole runtime/frame/audio acceptance remains separate.',results=results,positive_proof_sha256=digest(root/'proof.json'),inputs_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (Path(__file__),root/'targets/controlled.c',repo/'re_out/fist_cpu.h',repo/'re_out/fist_exec.h')},complete_original_acceptance=False)
    (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    return proof


def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'))
    limit=resource.getrlimit(resource.RLIMIT_CORE)
    resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
    try:
        positive,original,commands,driver=programs(repo,root)
        negative=causal(repo,root,positive,original,commands,driver)
        proof=dict(scope='Original controlled OUTSB contracts; protected I/O faults, reaching device transport and full frame/audio parity remain separate.',
                   programs=positive,causal_negatives=negative['results'],
                   inputs_sha256=positive['inputs_sha256'],complete_original_acceptance=False)
        (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
        return proof
    finally:
        resource.setrlimit(resource.RLIMIT_CORE,limit)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
