#!/usr/bin/env python3
"""Capture a complete same-run IRET/VGA/PIT/JCXZ prefix without changing output."""
import argparse
import json
import sys
from pathlib import Path
from capture_cpu_core_exit import capture as capture_core,verify as verify_core
from capture_cpu_pit_writes import observer as pit_observer,KINDS

CALLBACK_KINDS=tuple(k for name in ('draw-part','vert-interrupt','display-start')
                     for k in ('before-'+name,'after-'+name))
ADDITIONAL=(*CALLBACK_KINDS,*KINDS,'before-jcxz','after-jcxz','before-panning','after-panning')

def observer(repo,*,through_push_cs=False):
    text=pit_observer(repo)
    old='  if not started:save(\'handler-fetch\');started=True\n'
    assert text.count(old)==1
    text=text.replace(old,old+'''  q=state();assert not q['paging.enabled']
  address=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
  q.update(kind='fetch',code_physical=address,code_hex=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase'))+address,16)).hex())
  with (root/'handler-fetches.jsonl').open('a') as output:output.write(json.dumps(q)+'\\n')
''')
    old='if write_count==3:self.enabled=False;return False'
    assert text.count(old)==1;text=text.replace(old,'if write_count==3:return False')
    old='  address=(base+ip)&0xffffffff\n'
    assert text.count(old)==1
    text=text.replace(old,old+'''  if write_count==3:
   if ip==0x3aaa:save('before-jcxz')
   elif ip==0x3aac:save('after-jcxz');self.enabled=False;return False
''')
    if through_push_cs:
        old="elif ip==0x3aac:save('after-jcxz');self.enabled=False;return False"
        new="""elif ip==0x3aac:save('after-jcxz');return False
   elif ip==0x3abb:save('before-push-cs')
   elif ip==0x3abc:save('after-push-cs');self.enabled=False;return False"""
        assert text.count(old)==1;text=text.replace(old,new)
    extra=(repo/'tools/oracle/vga_callbacks.gdb.inc').read_text()
    assert text.count('\nend\nrun')==1
    return text.replace('\nend\nrun','\n'+extra+'\nend\nrun')

def verify(repo,folder,*,through_push_cs=False):
    kinds=ADDITIONAL[:-2]+('before-push-cs','after-push-cs')+ADDITIONAL[-2:] if through_push_cs else ADDITIONAL
    result=verify_core(repo,folder,kinds)
    fetches=[json.loads(s) for s in (folder/'source/handler-fetches.jsonl').read_text().splitlines()]
    lines=[json.loads(s) for s in (folder/'source/draw-lines.jsonl').read_text().splitlines()]
    assert len(fetches)==(2972 if through_push_cs else 2968) and len(lines)==50
    rows=result['events'];assert len(rows)==(25 if through_push_cs else 23)
    assert fetches[0]['segments'][1]['value']==fetches[-1]['segments'][1]['value']==0x2082
    assert fetches[0]['cpu_regs.ip.dword[0]']==0x3a68 and fetches[-1]['cpu_regs.ip.dword[0]']==(0x3abc if through_push_cs else 0x3aac)
    for before,after in zip(rows[7:13:2],rows[8:13:2]):
        assert before['CPU_Cycles']==after['CPU_Cycles']==0
        assert before['CPU_CycleLeft']==after['CPU_CycleLeft']
        assert before['PIC_event_service']==after['PIC_event_service']
        assert before['PIC_event_service']['active']==1
    before,after=rows[7:9]
    assert before['lines']==50 and before['vga_draw']['parts_left']==1
    assert after['vga_draw']['parts_left']==0
    assert after['vga_draw']['lines_done']==200
    assert before['vga_draw']['renderer']['updating']==1 and after['vga_draw']['renderer']['updating']==0
    before,after=rows[19:21]
    assert before['cpu_regs.ip.dword[0]']==0x3aaa and after['cpu_regs.ip.dword[0]']==0x3aac
    assert before['CPU_Cycles']==after['CPU_Cycles']+1
    if through_push_cs:
        sys.path.insert(0,str(repo/'tests'))
        from device_cpu_fixture import words,clock
        before,after=rows[21:23]
        assert before['cpu.code.big']==before['cpu.stack.big']==0
        assert next(q for q in fetches if q['cpu_regs.ip.dword[0]']==0x3abb)['code_hex'].startswith('0e')
        assert after['registers'][4]==before['registers'][4]-2
        memory=bytearray((folder/'source'/before['memory_file']).read_bytes())
        address=before['segments'][2]['base']+after['registers'][4]
        memory[address:address+2]=before['segments'][1]['value'].to_bytes(2,'little')
        assert bytes(memory)==(folder/'source'/after['memory_file']).read_bytes()
        a,b=words(before),words(after)
        assert [i for i,(x,y) in enumerate(zip(a,b)) if x!=y]==[4,8]
        assert b[8]==a[8]+1 and clock(after)==clock(before)+1
    scope='Actual original one-seed IRET/core/PIC/IRQ prefix, complete CPU/RAM/device/calendar/drawing/service observations, every fetch and50 complete linear-line outputs. Full39frame/27518mixedPCM/end600 output equals unobserved original. Renderer metadata is diagnostic; port renderer/scaler/startup/whole handler/full-sequence acceptance remains open.'
    result.update(scope=scope,through_push_cs=through_push_cs,fetches=fetches,draw_lines=lines,complete_original_acceptance=False)
    (folder/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS original',len(rows),'boundaries/',len(fetches),'fetches/50 drawn lines and unchanged complete output',flush=True)
    return result

def capture(repo,root,baseline=None,*,through_push_cs=False):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    return capture_core(repo,root,baseline,make_observer=lambda repo:observer(repo,through_push_cs=through_push_cs),
                        check_capture=lambda repo,root:verify(repo,root,through_push_cs=through_push_cs),
                        additional_inputs=(Path(__file__),repo/'tools/oracle/capture_cpu_pit_writes.py',repo/'tools/oracle/vga_callbacks.gdb.inc',
                        *[tree/p for p in ('src/hardware/timer.cpp','src/hardware/vga_draw.cpp','src/hardware/vga_misc.cpp','include/vga.h','src/gui/render.cpp')]))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--baseline',type=Path);p.add_argument('--verify-only',action='store_true')
    p.add_argument('--through-push-cs',action='store_true')
    a=p.parse_args();repo=a.repo.resolve();root=a.output.resolve()
    if a.verify_only:verify(repo,root,through_push_cs=a.through_push_cs)
    else:capture(repo,root,a.baseline.resolve() if a.baseline else None,through_push_cs=a.through_push_cs)
