#!/usr/bin/env python3
"""Capture complete original three-channel PIT producers and speaker requests."""
import argparse
import json
import os
from pathlib import Path
import subprocess
from capture_pit_events import LIFECYCLES,digest,source_paths

ROOT=Path(__file__).resolve().parents[2]

def programs():
    result=dict(LIFECYCLES);controls=[('F',1,0)]
    for value in range(256):
        controls.extend([('O',0x43,value),('R',0x40,0),('R',0x41,0),('R',0x42,0),('F',23,0),('G',value&1,0)])
    result['all-control-values']=controls;gate=[]
    for bcd in (0,1):
        for mode in range(8):
            for access in (1,2,3):
                for count in (0,1000,1320,0x1234,0xffff):
                    gate.extend([('G',0,0),('O',0x43,0x80+(access<<4)+(mode<<1)+bcd),('O',0x42,count&255),('O',0x42,count>>8),('G',1,0),('G',1,0),('F',123,0),('R',0x42,0),('R',0x42,0),('G',0,0),('G',0,0),('F',200,0),('R',0x42,0),('O',0x43,0x80),('R',0x42,0),('R',0x42,0),('O',0x43,0xce),('R',0x40,0),('R',0x41,0),('R',0x42,0),('O',0x43,0xe8),('R',0x42,0),('G',1,0),('R',0x42,0)])
    result['all-gate-counter2-modes']=gate
    result['retained-timer-construction']=[('F',1,0),('O',0x43,0xb7),('O',0x42,0x34),('O',0x42,0x12),('G',1,0),('F',100000,0),('O',0x43,0xe8),('I',0,0),('R',0x42,0),('O',0x43,0x38),('O',0x40,0),('O',0x40,0x80),('F',3300000,0),('I',0,0),('F',3300000,0)]
    result['timer-detach']=[('F',1,0),('O',0x43,0x36),('O',0x40,0xe8),('O',0x40,3),('F',60000,0),('D',0,0),('F',3300000,0)]
    ppi=[('F',1,0)]
    for previous in range(4):
        for value in range(256):
            ppi.extend([('O',0x61,previous),('O',0x61,value),('R',0x61,0),('R',0x61,0),('O',0x61,value),('F',123,0)])
    result['all-port61-values']=ppi
    ppi_gate=[]
    for bcd in (0,1):
        for mode in range(8):
            for access in (1,2,3):
                for count in (0,1000,1320,0x1234,0xffff):
                    ppi_gate.extend([('O',0x61,0),('O',0x43,0x80+(access<<4)+(mode<<1)+bcd),('O',0x42,count&255),('O',0x42,count>>8),('O',0x61,3),('F',123,0),('R',0x61,0),('R',0x61,0),('O',0x61,3),('O',0x61,2),('F',200,0),('R',0x42,0),('O',0x61,0),('O',0x61,1),('R',0x42,0),('O',0x61,0)])
    result['port61-timer-gates']=ppi_gate
    return result

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    import sys
    sys.path.insert(0,str(repo/'tests'))
    from test_port_io import build_pic_probe
    build_pic_probe(root)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3';ops=programs();rows={}
    flags=['-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),'-I'+str(root),'-ffunction-sections','-fdata-sections']
    sources=[repo/'tools/oracle'/n for n in ('pit_channels_pic_probe.cpp','pit_channels_timer_probe.cpp','pit_channels_keyboard_probe.cpp','io_delay_probe.cpp','dos_delay_probe.cpp','rep_probe.cpp')]
    inputs={str(p):digest(p) for p in (*source_paths(repo),tree/'src/hardware/keyboard.cpp',*sources,Path(__file__),root/'dos_modify_cycles.h')}
    originals={str(p):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()}
    env={k:v for k,v in os.environ.items() if not k.startswith('FIST_')}
    for name,sequence in ops.items():(root/(name+'.txt')).write_text(''.join('%s %x %x\n'%op for op in sequence))
    for variant,extra in (('source',[]),('source-silent',['-DSILENT_SPEAKER_OBSERVER'])):
        output=root/variant
        p=subprocess.run(['g++',*flags,*extra,*map(str,sources),'-Wl,--gc-sections','-lm','-o',str(output)],capture_output=True,text=True,timeout=90)
        (root/(variant+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
        inputs[str(output)]=digest(output)
        for name in ('budget',*ops):
            p=subprocess.run([str(output),'budget' if name=='budget' else str(root/(name+'.txt'))],env=env,capture_output=True,text=True,timeout=30)
            (root/(variant+'-'+name+'.stdout')).write_text(p.stdout)
            (root/(variant+'-'+name+'.stderr')).write_text(p.stderr)
            (root/(variant+'-'+name+'.exit')).write_text(str(p.returncode)+'\n');assert p.returncode==0,p.stderr
            observed=[json.loads(line) for line in p.stdout.splitlines()]
            if variant=='source':rows[name]=observed
            else:assert observed==[q for q in rows[name] if q['kind'] not in ('speaker-request','speaker-type-request')],name
            states=[q for q in observed if q['kind']=='state']
            expected=736 if name=='budget' else len(ops[name])+1
            assert len(states)==expected and [q['label'] for q in states]==list(range(expected)),name
            for q in states:
                assert len(q['counters'])==3 and all(len(p)==16 for p in q['counters'])
                assert len(q['irqs'])==16 and all(len(p)==4 for p in q['irqs'])
                assert len(q['queue'])+q['free_entries']+q['service']==512
            if name!='budget':assert sum(q['kind']=='read' for q in observed)==sum(op[0]=='R' for op in ops[name])
    for p,h in inputs.items():assert digest(p)==h,p
    for p,h in originals.items():assert digest(p)==h,p
    result=dict(scope='Controlled actual original PIT0/1/2 fields/outputs, gate/status, port61 read/write byte and ordered gate/speaker-type requests, complete16 IRQs and PIC calendar/budgets/IODelayRemoved. All256 controls and240 channel2 mode/access/BCD/count combinations, repeated gates and retained TIMER construction;736 complete budget states. All256 port61 bytes from each prior low-bit value, repeat writes/read toggles and all240 counter2 combinations via actual port61. Speaker observers record only actual request parameters; their suppressed variant preserves every complete state/read. Speaker bodies/PCM, other keyboard ports, guest IRQ/IF/handler/startup and complete original sequence acceptance remain open.',source_inputs_sha256=inputs,originals_sha256=originals,programs={name:[list(op) for op in sequence] for name,sequence in ops.items()},source_rows={name:len(sequence) for name,sequence in rows.items()},source_stdout_sha256={name:digest(root/('source-'+name+'.stdout')) for name in rows},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS complete original736-budget and15 three-channel PIT/port61 programs; speaker observation preserves all states/reads',flush=True)
    return result,rows

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,default=ROOT)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
