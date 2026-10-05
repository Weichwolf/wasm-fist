#!/usr/bin/env python3
"""Observe all timer state at the first actual IRQ0 PIT control/low/high writes."""
import argparse
import json
from pathlib import Path
from capture_cpu_core_exit import capture as capture_core,observer as core_observer,verify as verify_core,digest

PIT_FIELDS=('cntr','delay','start','read_latch','write_latch','mode','latch_mode',
            'read_state','write_state','bcd','go_read_latch','new_mode',
            'counterstatus_set','counting','update_count')
KINDS=tuple(kind for stage in ('pit-control','pit-low','pit-high')
            for kind in ('before-'+stage,'after-'+stage))

def observer(repo):
    text=core_observer(repo)
    old="class Fetch(gdb.Breakpoint):\n def stop(self):save('handler-fetch');self.enabled=False;return False"
    assert text.count(old)==1
    return text.replace(old,'''write_count=0;after_write=None;started=False
original_state=state
def state():
 q=original_state();counters=[]
 for counter in range(3):
  p={}
  for key in '''+repr(PIT_FIELDS)+''':
   v=gdb.parse_and_eval("'timer.cpp'::pit[%d].%s"%(counter,key))
   p[key]=float(v) if key in ('delay','start') else int(v)
   if key in ('delay','start'):p[key+'_bits']=bytes(gdb.selected_inferior().read_memory(int(v.address),v.type.sizeof)).hex()
  counters.append(p)
 q['pit']=dict(counters=counters,gate2=int(gdb.parse_and_eval("'timer.cpp'::gate2")),status=int(gdb.parse_and_eval("'timer.cpp'::latched_timerstatus")),status_locked=int(gdb.parse_and_eval("'timer.cpp'::latched_timerstatus_locked")))
 q['vga_status_device']={key:bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('vga.draw.delay.'+key).address),8)).hex() for key in ('framestart','vrstart','vrend','hblkstart','hblkend','htotal','vdend','vtotal')}
 q['vga_status_device'].update(attrindex=int(gdb.parse_and_eval('vga.internal.attrindex')),pcjr_flipflop=int(gdb.parse_and_eval('vga.tandy.pcjr_flipflop')))
 return q
class Fetch(gdb.Breakpoint):
 def stop(self):
  global started,after_write,write_count
  if not started:save('handler-fetch');started=True
  if after_write:
   save('after-'+after_write);after_write=None
   if write_count==3:self.enabled=False;return False
  assert not int(gdb.parse_and_eval('paging.enabled'))
  ip=int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'));base=int(gdb.parse_and_eval('Segs.phys[1]'))
  address=(base+ip)&0xffffffff
  code=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase'))+address,16))
  if code[0]==0xe6 and code[1] in (0x40,0x43):
   assert write_count<3
   if write_count==0:assert code[1]==0x43;kind='pit-control'
   elif write_count==1:assert code[1]==0x40;kind='pit-low'
   else:assert code[1]==0x40;kind='pit-high'
   save('before-'+kind,dict(code_physical=address,code_hex=code.hex(),port=code[1],value=int(gdb.parse_and_eval('cpu_regs.regs[0].byte[0]'))))
   write_count+=1;after_write=kind
  return False''')

def verify(repo,root):
    result=verify_core(repo,root,KINDS)
    rows=result['events'];assert len(rows)==13
    writes=rows[7:];assert [q['value'] for q in writes[::2]]==[0x36,0x6c,0x40]
    for q in rows:
        assert len(q['pit']['counters'])==3
        for p in q['pit']['counters']:
            assert set(p)==set(PIT_FIELDS)|{'delay_bits','start_bits'}
            assert len(bytes.fromhex(p['delay_bits']))==4 and len(bytes.fromhex(p['start_bits']))==8
    for before,after in zip(writes[::2],writes[1::2]):
        assert before['cpu.pmode']==0 and before['port'] in (0x40,0x43)
        assert after['CPU_IODelayRemoved']==before['CPU_IODelayRemoved']+21
        assert after['CPU_Cycles']==before['CPU_Cycles']-22
        assert after['CPU_CycleLeft']==before['CPU_CycleLeft'] and after['PIC_Ticks']==before['PIC_Ticks']
        assert after['pit']['counters'][1:]==before['pit']['counters'][1:]
        assert after['vga_status_device']==before['vga_status_device']
        raw=(root/'source'/before['memory_file']).read_bytes();a=before['code_physical']
        assert raw[a:a+16].hex()==before['code_hex']
    before,control,low_before,low,high_before,high=writes
    assert any('PIT0_Event' in e['handler'] for e in before['calendar'])
    assert all('PIT0_Event' not in e['handler'] for q in (control,low_before,low,high_before) for e in q['calendar'])
    assert sum('PIT0_Event' in e['handler'] for e in high['calendar'])==1
    assert control['pit']['counters'][0]['new_mode']==1
    assert low['pit']['counters'][0]['write_latch']==0x6c and low['pit']['counters'][0]['write_state']==0
    assert high['pit']['counters'][0]['write_latch']==0x406c and high['pit']['counters'][0]['write_state']==3
    assert high['pit']['counters'][0]['new_mode']==0
    result['scope']='Actual first IRQ0 PIT control/low/high writes. All13 complete CPU/system/RAM/provider/cache/VGA/PIC/calendar/PIT observations include all3 counters and retained gate/status state. Complete39frames/27518PCM/end600 equals unprobed original. Source states are test inputs, never runtime initialization. Whole handler/startup/speaker body/mixedPCM and complete original sequence acceptance remain open.'
    (root/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS actual first PIT writes, all3 timer channels and complete unchanged original600ms output',flush=True)
    return result

def capture(repo,root,baseline=None):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    return capture_core(repo,root,baseline,make_observer=observer,check_capture=verify,
                        additional_inputs=(Path(__file__),*[tree/p for p in ('src/hardware/timer.cpp','src/hardware/iohandler.cpp','src/hardware/vga_misc.cpp','include/vga.h')]))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--repo',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True);p.add_argument('--baseline',type=Path)
    p.add_argument('--verify-only',action='store_true');a=p.parse_args();repo=a.repo.resolve();root=a.output.resolve()
    if a.verify_only:verify(repo,root)
    else:capture(repo,root,a.baseline.resolve() if a.baseline else None)
