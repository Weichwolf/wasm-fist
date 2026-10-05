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

def observer(repo,*,through_push_cs=False,through_shr_word=False,through_moffs=False,through_jns=False,through_outsb=False):
    through_jns=through_jns or through_outsb
    through_moffs=through_moffs or through_jns
    through_shr_word=through_shr_word or through_moffs
    through_push_cs=through_push_cs or through_shr_word
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
    if through_shr_word:
        old="elif ip==0x3abc:save('after-push-cs');self.enabled=False;return False"
        new="""elif ip==0x3abc:save('after-push-cs');return False
   elif ip==0x3b38:save('before-shr-word-1')
   elif ip==0x3b3c:save('after-shr-word-1');self.enabled=False;return False"""
        assert text.count(old)==1;text=text.replace(old,new)
    if through_moffs:
        old="elif ip==0x3b3c:save('after-shr-word-1');self.enabled=False;return False"
        new="""elif ip==0x3b3c:save('after-shr-word-1');return False
   elif ip==0x3b43:save('before-moffs-byte')
   elif ip==0x3b46:save('after-moffs-byte');self.enabled=False;return False"""
        assert text.count(old)==1;text=text.replace(old,new)
    if through_jns:
        old="elif ip==0x3b46:save('after-moffs-byte');self.enabled=False;return False"
        new="""elif ip==0x3b46:save('after-moffs-byte');return False
   elif ip==0x3b48:save('before-jns')
   elif ip==0x3b63:save('after-jns');self.enabled=False;return False"""
        assert text.count(old)==1;text=text.replace(old,new)
    if through_outsb:
        old="elif ip==0x3b63:save('after-jns');self.enabled=False;return False"
        assert text.count(old)==1;text=text.replace(old,"elif ip==0x3b63:save('after-jns')")
        old="  with (root/'handler-fetches.jsonl').open('a') as output:output.write(json.dumps(q)+'\\n')\n"
        new=old+"""  if q['segments'][1]['value']==0x4ec3:
   if q['cpu_regs.ip.dword[0]']==0xbe6:save('before-shr-byte')
   elif q['cpu_regs.ip.dword[0]']==0xbea:save('after-shr-byte')
   elif q['cpu_regs.ip.dword[0]']==0xbff:save('before-outsb')
   elif q['cpu_regs.ip.dword[0]']==0xc01:save('after-outsb');self.enabled=False;return False
"""
        assert text.count(old)==1;text=text.replace(old,new)
    extra=(repo/'tools/oracle/vga_callbacks.gdb.inc').read_text()
    if through_outsb:
        extra+='\n'+(repo/'tools/oracle/dac_state.gdb.inc').read_text()+'\n'+(repo/'tools/oracle/dac_io.gdb.inc').read_text()
    assert text.count('\nend\nrun')==1
    return text.replace('\nend\nrun','\n'+extra+'\nend\nrun')

def verify(repo,folder,*,through_push_cs=False,through_shr_word=False,through_moffs=False,through_jns=False,through_outsb=False):
    through_jns=through_jns or through_outsb
    through_moffs=through_moffs or through_jns
    through_shr_word=through_shr_word or through_moffs
    through_push_cs=through_push_cs or through_shr_word
    kinds=ADDITIONAL[:-2]+('before-push-cs','after-push-cs')+ADDITIONAL[-2:] if through_push_cs else ADDITIONAL
    if through_shr_word:kinds=kinds[:-2]+('before-shr-word-1','after-shr-word-1')+kinds[-2:]
    if through_moffs:kinds=kinds[:-2]+('before-moffs-byte','after-moffs-byte')+kinds[-2:]
    if through_jns:kinds=kinds[:-2]+('before-jns','after-jns')+kinds[-2:]
    if through_outsb:kinds=kinds[:-2]+('before-shr-byte','after-shr-byte','before-outsb','after-outsb')+kinds[-2:]
    result=verify_core(repo,folder,kinds)
    fetches=[json.loads(s) for s in (folder/'source/handler-fetches.jsonl').read_text().splitlines()]
    lines=[json.loads(s) for s in (folder/'source/draw-lines.jsonl').read_text().splitlines()]
    assert len(fetches)==(2998 if through_outsb else 2982 if through_jns else 2980 if through_moffs else 2976 if through_shr_word else 2972 if through_push_cs else 2968) and len(lines)==50
    rows=result['events'];assert len(rows)==(35 if through_outsb else 31 if through_jns else 29 if through_moffs else 27 if through_shr_word else 25 if through_push_cs else 23)
    assert fetches[0]['segments'][1]['value']==0x2082
    assert fetches[-1]['segments'][1]['value']==(0x4ec3 if through_outsb else 0x2082)
    assert fetches[0]['cpu_regs.ip.dword[0]']==0x3a68 and fetches[-1]['cpu_regs.ip.dword[0]']==(0xc01 if through_outsb else 0x3b63 if through_jns else 0x3b46 if through_moffs else 0x3b3c if through_shr_word else 0x3abc if through_push_cs else 0x3aac)
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
    if through_shr_word:
        before,after=rows[23:25]
        assert before['cpu.code.big']==0 and not before['paging.enabled']
        assert next(q for q in fetches if q['cpu_regs.ip.dword[0]']==0x3b38)['code_hex'].startswith('d12e5004')
        memory=bytearray((folder/'source'/before['memory_file']).read_bytes())
        address=before['segments'][3]['base']+0x450
        operand=int.from_bytes(memory[address:address+2],'little');result_word=operand>>1
        memory[address:address+2]=result_word.to_bytes(2,'little')
        assert bytes(memory)==(folder/'source'/after['memory_file']).read_bytes()
        expected=words(before);actual=words(after)
        expected[8]+=4
        expected[22]=(expected[22]&0xffff0000)|operand
        expected[23]=(expected[23]&0xffffff00)|1
        expected[24]=(expected[24]&0xffff0000)|result_word
        expected[25]=38
        assert actual==expected and clock(after)==clock(before)+1
    if through_moffs:
        before,after=rows[25:27]
        assert before['cpu.code.big']==0 and not before['paging.enabled']
        assert next(q for q in fetches if q['cpu_regs.ip.dword[0]']==0x3b43)['code_hex'].startswith('a03b07')
        memory=(folder/'source'/before['memory_file']).read_bytes()
        value=memory[before['segments'][3]['base']+0x73b]
        assert memory==(folder/'source'/after['memory_file']).read_bytes()
        expected=words(before);expected[0]=(expected[0]&0xffffff00)|value;expected[8]+=3
        assert words(after)==expected and clock(after)==clock(before)+1
    if through_jns:
        before,after=rows[27:29]
        assert before['cpu.code.big']==0 and not before['paging.enabled']
        assert before['lflags.type']==4  # Original t_ORb sign is bit7 of lf_resb.
        memory=(folder/'source'/before['memory_file']).read_bytes()
        address=before['segments'][1]['base']+before['cpu_regs.ip.dword[0]']
        assert memory[address:address+2]==bytes.fromhex('7919')
        assert memory==(folder/'source'/after['memory_file']).read_bytes()
        expected=words(before);delta=int.from_bytes(memory[address+1:address+2],'little',signed=True)
        take=not(expected[24]&0x80)
        expected[8]=(expected[8]&0xffff0000)|((expected[8]+2+(delta if take else 0))&0xffff)
        assert words(after)==expected and clock(after)==clock(before)+1
    if through_outsb:
        io=[json.loads(s) for s in (folder/'source/io-writes.jsonl').read_text().splitlines()]
        writes=[q for q in io if q['phase']=='before'];after=[q for q in io if q['phase']=='after']
        assert len(writes)==len(after)==772 and [q['serial'] for q in writes]==list(range(772))
        assert [q['port'] for q in writes]==[0x43,0x40,0x40,0x3c8]+[0x3c9]*768
        before,end=rows[31:33]
        assert (before['segments'][1]['value'],before['cpu_regs.ip.dword[0]'])==(0x4ec3,0xbff)
        assert bytes.fromhex(fetches[-2]['code_hex'])[:2]==bytes.fromhex('f36e')
        memory=(folder/'source'/before['memory_file']).read_bytes()
        assert memory==(folder/'source'/end['memory_file']).read_bytes()
        start=before['segments'][3]['base']+(before['registers'][6]&0xffff)
        assert [q['value'] for q in writes[4:]]==list(memory[start:start+768])
        # Original reserves768 REP cycles before any DAC I/O and keeps SI/CX local until cleanup.
        assert writes[4]['cpu']['CPU_Cycles']==before['CPU_Cycles']+1-768
        for q in io[8:]:
         expected=words(before)
         assert words(q['cpu'])==expected
        a,b=words(before),words(end);a[1]&=0xffff0000;a[6]=(a[6]&0xffff0000)|768;a[8]+=2
        assert a==b
        rgb=bytes.fromhex(end['dac']['rgb_hex'])
        assert rgb==bytes(value&0x3f for value in memory[start:start+768])
        assert end['dac']['fields']['write_index']==0 and end['dac']['fields']['pel_index']==0
        result['io_writes']=io
    scope='Actual original one-seed IRET/core/PIC/IRQ prefix, complete CPU/RAM/device/calendar/drawing/service observations, every fetch and50 complete linear-line outputs. Full39frame/27518mixedPCM/end600 output equals unobserved original. Renderer metadata is diagnostic; port renderer/scaler/startup/whole handler/full-sequence acceptance remains open.'
    result.update(scope=scope,through_push_cs=through_push_cs,through_shr_word=through_shr_word,through_moffs=through_moffs,through_jns=through_jns,through_outsb=through_outsb,fetches=fetches,draw_lines=lines,complete_original_acceptance=False)
    (folder/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS original',len(rows),'boundaries/',len(fetches),'fetches/50 drawn lines and unchanged complete output',flush=True)
    return result

def capture(repo,root,baseline=None,*,through_push_cs=False,through_shr_word=False,through_moffs=False,through_jns=False,through_outsb=False):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    return capture_core(repo,root,baseline,make_observer=lambda repo:observer(repo,through_push_cs=through_push_cs,through_shr_word=through_shr_word,through_moffs=through_moffs,through_jns=through_jns,through_outsb=through_outsb),
                        check_capture=lambda repo,root:verify(repo,root,through_push_cs=through_push_cs,through_shr_word=through_shr_word,through_moffs=through_moffs,through_jns=through_jns,through_outsb=through_outsb),
                        source_wall_seconds=300 if through_outsb else 120,
                        additional_inputs=(Path(__file__),repo/'tools/oracle/capture_cpu_pit_writes.py',repo/'tools/oracle/vga_callbacks.gdb.inc',repo/'tools/oracle/dac_state.gdb.inc',repo/'tools/oracle/dac_io.gdb.inc',
                        *[tree/p for p in ('src/hardware/vga_dac.cpp','src/hardware/iohandler.cpp','src/cpu/core_normal/string.h','include/render.h','src/hardware/timer.cpp','src/hardware/vga_draw.cpp','src/hardware/vga_misc.cpp','include/vga.h','src/gui/render.cpp','src/cpu/instructions.h','src/cpu/lazyflags.h','src/cpu/flags.cpp','include/logging.h','config.h','src/cpu/core_normal/prefix_none.h','src/cpu/core_normal/table_ea.h')]))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--baseline',type=Path);p.add_argument('--verify-only',action='store_true')
    p.add_argument('--through-push-cs',action='store_true')
    p.add_argument('--through-shr-word',action='store_true')
    p.add_argument('--through-moffs',action='store_true')
    p.add_argument('--through-jns',action='store_true')
    p.add_argument('--through-outsb',action='store_true')
    a=p.parse_args();repo=a.repo.resolve();root=a.output.resolve()
    if a.verify_only:verify(repo,root,through_push_cs=a.through_push_cs,through_shr_word=a.through_shr_word,through_moffs=a.through_moffs,through_jns=a.through_jns,through_outsb=a.through_outsb)
    else:capture(repo,root,a.baseline.resolve() if a.baseline else None,through_push_cs=a.through_push_cs,through_shr_word=a.through_shr_word,through_moffs=a.through_moffs,through_jns=a.through_jns,through_outsb=a.through_outsb)
