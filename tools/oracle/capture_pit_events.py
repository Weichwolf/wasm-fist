#!/usr/bin/env python3
"""Capture actual original PIT0/PIC producers; retain strict port budget failures."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
PIT_FIELDS=('cntr','delay_float_bits','start_double_bits','read_latch','write_latch',
            'mode','latch_mode','read_state','write_state','bcd','go_read_latch',
            'new_mode','counterstatus_set','counting','update_count','output')
LIFECYCLES={
    'default-periodic':[('F',1,0),('F',3300000,0)],
    'mode2-deferred-count':[('F',1,0),('O',0x43,0x34),('O',0x40,0),('O',0x40,0x20),
                            ('F',100000,0),('O',0x40,0),('O',0x40,0x40),('F',200000,0),('F',500000,0)],
    'mode0-one-shot-and-reload':[('F',1,0),('O',0x43,0x30),('O',0x40,0xf4),('O',0x40,1),
                                 ('F',30000,0),('F',30000,0),('O',0x40,0xe8),('O',0x40,3),('F',60000,0)],
    'mode3-reload-without-control':[('F',1,0),('O',0x43,0x36),('O',0x40,0xe8),('O',0x40,3),
                                    ('F',5000,0),('O',0x40,0xd0),('O',0x40,7),('F',100000,0)],
    'mode-aliases-and-partial-write':[('F',1,0),('O',0x43,0x3c),('O',0x40,0),('F',50,0),
                                      ('O',0x40,0x20),('F',250000,0),('O',0x43,0x3e),
                                      ('O',0x40,0),('O',0x40,0x40),('F',1000000,0)],
    'counter-and-status-latches':[('F',1,0),('F',100000,0),('O',0x43,0),('R',0x40,0),
                                  ('F',100,0),('R',0x40,0),('O',0x43,0xc2),('R',0x40,0),
                                  ('R',0x40,0),('R',0x40,0),('F',1600000,0)],
    'BCD-zero-count':[('F',1,0),('O',0x43,0x35),('O',0x40,0),('O',0x40,0),('F',600000,0)],
    'mode0-to-mode2-control':[('F',1,0),('O',0x43,0x30),('O',0x40,0xe8),('O',0x40,3),
                               ('F',100,0),('O',0x43,0x34),('O',0x40,0),('O',0x40,0x20),('F',400000,0)],
    'mode3-low-output-control':[('F',1,0),('O',0x43,0x36),('O',0x40,0xe8),('O',0x40,3),
                                ('F',25120,0),('O',0x43,0x34),('O',0x40,0),('O',0x40,0x20),('F',400000,0)]}

def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream,'sha256').hexdigest()

def read(path):
    return json.loads(Path(path).read_text())

def lines(path):
    return [json.loads(line) for line in Path(path).read_text().splitlines()]

def command(root,name,args,env,input_=None):
    result=subprocess.run(args,input=input_,capture_output=True,env=env,timeout=120)
    (root/(name+'.stdout')).write_bytes(result.stdout)
    (root/(name+'.stderr')).write_bytes(result.stderr)
    (root/(name+'.exit')).write_text(str(result.returncode)+'\n')
    assert result.returncode==0,(name,result.returncode,result.stderr[-2000:])
    return result.stdout

def source_paths(repo):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    return sorted({Path(__file__).resolve(),repo/'tests/test_port_io.py',repo/'tests/device_cpu_fixture.py',
                   *(repo/'tools/oracle'/n for n in ('pit_event_pic_probe.cpp','pit_event_timer_probe.cpp',
                       'pic_slice_probe.cpp','io_delay_probe.cpp','dos_delay_probe.cpp','rep_probe.cpp')),
                   *tree.rglob('*.h'),
                   *(tree/'src'/n for n in ('hardware/pic.cpp','hardware/timer.cpp',
                       'hardware/iohandler.cpp','dos/dos.cpp'))})

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp')),'disposable evidence must be under /tmp'
    root.mkdir(parents=True,exist_ok=False)
    sys.path.insert(0,str(repo/'tests'))
    from test_port_io import build_pic_probe,tool
    from device_cpu_fixture import words
    env={k:v for k,v in os.environ.items() if not k.startswith('FIST_')}
    originals={str(p.relative_to(repo)):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()}
    (root/'original-hashes.json').write_text(json.dumps(originals,indent=2)+'\n')
    # Reuse the existing actual-source DOS helper extraction and compiler owner.
    build_pic_probe(root)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    flags=['-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),
           '-I'+str(tree/'include'),'-I'+str(tree),'-I'+str(root),'-ffunction-sections','-fdata-sections']
    sources=[str(repo/'tools/oracle'/n) for n in ('pit_event_pic_probe.cpp','pit_event_timer_probe.cpp',
              'io_delay_probe.cpp','dos_delay_probe.cpp','rep_probe.cpp')]
    producers={str(p):digest(p) for p in source_paths(repo)}
    for variant,extra in (('baseline',[]),('trace',['-DFIST_TIMER_TRACE'])):
        binary=root/variant
        command(root,variant+'-build',['g++',*flags,*extra,*sources,'-Wl,--gc-sections','-lm','-o',str(binary)],env)
        producers[str(binary)]=digest(binary)
        command(root,variant+'-budget',[str(binary),'budget'],env)
        for name,ops in LIFECYCLES.items():
            script=root/(name+'.txt')
            script.write_text(''.join('%s %x %x\n'%op for op in ops))
            command(root,name+'-'+variant,[str(binary),'lifecycle',str(script)],env)
    checkpoint=read(repo/'tools/oracle/device_checkpoint_case.json')
    assert len(checkpoint['fetches'])==368
    inputs=b''.join(struct.pack('<37I',*words(q)) for q in checkpoint['fetches'])
    (root/'cpu-input.bin').write_bytes(inputs)
    flags=['-I'+str(repo/'re_out'),'-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-w']
    sources=[str(repo/'tests/pit_event_clock.c'),
             *(str(repo/'re_out'/n) for n in ('fist_dos.c','fist_vga.c','fist_pic.c','fist_sb.c'))]
    targets=(('native',['gcc','-m32','-O0',*flags,*sources,'-Wl,--gc-sections','-lm','-o',str(root/'clock-native')],
              [str(root/'clock-native')]),
             ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),'-O2',*flags,*sources,
                      '-sNODERAWFS=1','-sASSERTIONS=1','-sEXIT_RUNTIME=1','-o',str(root/'clock.js')],
              [tool('node','Git/emsdk/node/*/bin/node'),str(root/'clock.js')]))
    for target,build,run in targets:
        command(root,target+'-clock-build',build,env)
        command(root,target+'-clock',run,env,inputs)
    for p in (repo/'tests/pit_event_clock.c',repo/'tests/sb_clock_fixture.h',
              repo/'tools/oracle/device_checkpoint_case.json',
              repo/'tools/oracle/fist_sequence_endpoint.h',*(repo/'re_out').glob('*.h'),
              *(repo/'re_out'/n for n in ('fist_cpu.h','ghidra_compat.h','fist_dos.c','fist_vga.c','fist_pic.c',
                                        'fist_pic.h','fist_sb.c','fist_sb.h')),
              root/'dos_modify_cycles.h',root/'cpu-input.bin',root/'clock-native',root/'clock.js',root/'clock.wasm'):
        producers[str(p)]=digest(p)
    producers[str(Path(sys.executable).resolve())]=digest(Path(sys.executable).resolve())
    for name in ('g++','gcc','emcc','node'):
        if name=='emcc':path=Path(tool(name,'Git/emsdk/upstream/emscripten/emcc')).resolve()
        elif name=='node':path=Path(tool(name,'Git/emsdk/node/*/bin/node')).resolve()
        else:path=Path(shutil.which(name)).resolve()
        producers[str(path)]=digest(path)
    (root/'producers.json').write_text(json.dumps(producers,indent=2)+'\n')
    return verify(repo,root,reference=False)

def verify(repo,root,reference=True):
    for p,h in read(root/'producers.json').items():assert digest(p)==h,p
    originals=read(root/'original-hashes.json')
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()}==set(originals)
    for p,h in originals.items():assert digest(repo/p)==h,p
    original={}
    for name,ops in [('budget',None),*LIFECYCLES.items()]:
        stem=lambda variant:variant+'-budget' if ops is None else name+'-'+variant
        for variant in ('baseline','trace'):
            assert (root/(stem(variant)+'.exit')).read_text()=='0\n'
        baseline=lines(root/(stem('baseline')+'.stdout'))
        trace=lines(root/(stem('trace')+'.stdout'))
        assert [q for q in trace if q['kind']!='irq0-activate']==baseline,name
        if ops is None:
            assert len(baseline)==736 and [q['label'] for q in baseline]==list(range(736)), 'incomplete original budget output'
            assert all(q['kind']=='budget' for q in baseline)
        else:
            assert (root/(name+'.txt')).read_text()==''.join('%s %x %x\n'%op for op in ops)
            states=[q for q in baseline if q['kind']=='state']
            reads=[q for q in baseline if q['kind']=='read']
            assert len(states)==len(ops)+1 and [q['label'] for q in states]==list(range(len(states)))
            assert len(reads)==sum(op[0]=='R' for op in ops)
        for q in trace:
            if q['kind']=='read':continue
            assert len(q['pit0'])==len(PIT_FIELDS) and len(q['irq0'])==4
            assert len(q['queue'])+q['free_entries']+q['service']==512
            for index,deadline,value in q['queue']:
                actual=struct.unpack('<f',struct.pack('<I',index))[0]*30000
                assert struct.unpack('<I',struct.pack('<f',actual))[0]==deadline, 'queue float deadline does not match its original index'
        original[name]=dict(operations=None if ops is None else [list(op) for op in ops],
                            complete_baseline=baseline,
                            irq_activations=[dict(trace_record=index,state=q)
                                for index,q in enumerate(trace) if q['kind']=='irq0-activate'])
    if reference:
        golden=read(repo/'tools/oracle/pit_event_case.json')
        assert golden['original']==original,'complete source reference changed'
    budget=original['budget']['complete_baseline']
    diagnostics=[]
    for target in ('native','wasm'):
        assert (root/(target+'-clock.exit')).read_text()=='0\n'
        rows=(root/(target+'-clock.stdout')).read_text().splitlines()
        end=re.fullmatch(r'cases 368 pumps 0 legacy-int8 (\d+)',rows[-1]) if rows else None
        assert len(rows)==737 and end, 'incomplete port clock output'
        differences=[]
        for index,(line,source) in enumerate(zip(rows[:-1],budget)):
            actual=[int(v) for v in line.split()]
            assert len(actual)==4 and actual[:2]==[index//2,index%2]
            expected=[index//2,index%2,source['tick']*30000+source['index_nd'],source['cycles']]
            if actual!=expected:differences.append(dict(row=index,port=actual,original=expected))
        diagnostics.append(dict(target=target,complete_budget_parity=not differences,
                                strict_budget_verdict_exit=1 if differences else 0,
                                legacy_INT8_delivery_attempts=int(end[1]),differences=differences))
    artifacts={p.name:digest(p) for p in root.iterdir() if p.is_file() and p.suffix in ('.stdout','.stderr','.exit','.txt')}
    proof=dict(scope='Actual original default PIT0/PIC queue and nine synthetic counter/control/latch lifecycles. Complete baseline/IRQ-observer state and read sequences agree. Current Native/WASM budget failures remain explicit diagnostics. No matched guest caller/IRQ/IF/IRET replay or complete frame/audio acceptance.',
               commit_parent=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
               PIT_state_fields=list(PIT_FIELDS),IRQ_state_fields=['active','masked','inservice','vector'],
               queue_entry_fields=['index_float_bits','deadline_float_bits','value'],original=original,
               original_source_inputs={str(p.relative_to(repo)):digest(p) for p in source_paths(repo)},
               producer_manifest_sha256=digest(root/'producers.json'),artifact_hashes=artifacts,
               original_files=len(originals),originals_unchanged=True,
               CPU_input_case='tools/oracle/device_checkpoint_case.json',
               CPU_input_case_sha256=digest(repo/'tools/oracle/device_checkpoint_case.json'),
               CPU_input_sha256=digest(root/'cpu-input.bin'),diagnostics=diagnostics,
               reference_scope='The original timing producer has rawFLAGS0 and uses normal-core fetch debt with guest IRQ construction disabled. O/R exercise the real I/O delay plus timer handler, without guest instruction/register effects. Port diagnostic input contains all37 original CPU words and records every legacy INT8 delivery attempt instead of implementing missing guest delivery. These are device/time producer properties, not a synchronized full original caller replay. The port driver verifies budgets only; full flag verification remains with test_cpu_flags.py.',
               complete_original_acceptance=False,
               reproduction=['python3 -B tools/oracle/capture_pit_events.py --repo . --output /tmp/wasm-fist-pit-events-replay',
                             'python3 -B tools/oracle/capture_pit_events.py --repo . --output /tmp/wasm-fist-pit-events-replay --verify-only'])
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS:736 complete original budget rows and all nine original lifecycle baseline/trace sequences; strict port budget verdicts:',
          [(q['target'],q['strict_budget_verdict_exit'],len(q['differences'])) for q in diagnostics])
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=ROOT)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    parser.add_argument('--require-budget-parity',action='store_true',help='fail on any complete original/port budget difference')
    parser.add_argument('--record-case',action='store_true',help='record a new original source fixture; refuses an existing case')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    case=repo/'tools/oracle/pit_event_case.json'
    if args.record_case:assert not case.exists() and not args.verify_only
    if args.verify_only:
        proof=verify(repo,root)
    else:
        proof=capture(repo,root)
        if args.record_case:
            case.write_text(format_case(proof)+'\n')
        else:
            proof=verify(repo,root)
    if args.require_budget_parity:
        for row in proof['diagnostics']:
            assert row['complete_budget_parity'], (row['target'],'complete budget parity failed',row['differences'][0])

def format_case(value,level=0):
    """Keep each complete observer row on one line in the durable source fixture."""
    if isinstance(value,dict):
        if 'kind' in value:return json.dumps(value,separators=(',',':'))
        return '{\n'+',\n'.join('  '*(level+1)+json.dumps(k)+': '+format_case(v,level+1)
                               for k,v in value.items())+'\n'+'  '*level+'}'
    if isinstance(value,list) and value:
        if all(not isinstance(v,(dict,list)) for v in value):return json.dumps(value)
        return '[\n'+',\n'.join('  '*(level+1)+format_case(v,level+1) for v in value)+'\n'+'  '*level+']'
    return json.dumps(value)

if __name__=='__main__':main()
