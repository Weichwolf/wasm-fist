#!/usr/bin/env python3
"""Capture first and protected-IRQ repeated original resident system bootstrap."""
import argparse
import copy
from pathlib import Path
import json
import re
from capture_pit_events import digest, format_case
from capture_pit_irq_frames import ROOT, capture, verify
from resident_image import load, relocate


FETCH_META = {'kind', 'ordinal', 'context', 'group', 'architecture', 'auto_determine',
              'normal_core', 'memory_file', 'fetched_code_physical', 'fetched_code_hex'}


def bootstrap_contract(repo, folder, boot, system, events, irq_fetches, physical):
    """Check actual opcodes, complete AND CPU/RAM effects and original time fields.

    Other bootstrap instructions retain complete observed RAM boundaries, but their
    full instruction transition interpretation remains separate work.
    """
    contexts = [0]+[q['index'] for q in events if q['kind']=='before-hardware' and q['cpu.pmode']]
    assert len(contexts)==2 and len(boot)==18, 'incomplete bootstrap fetch output'
    model=load(repo)
    lazy=(repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h').read_text()
    types=re.findall(r'\bt_[A-Za-z0-9_]+\b',lazy.split('//Types of Flag changing instructions',1)[1].split('enum {',1)[1].split('};',1)[0])
    contracts=[]
    for group,context in enumerate(contexts,1):
        rows=[q for q in boot if q['group']==group]
        assert len(rows)==9 and [q['ordinal'] for q in rows]==list(range(1,10)), 'incomplete bootstrap group'
        assert all(q['context']==context and q['normal_core'] and q['auto_determine']==0 and q['architecture']==rows[0]['architecture'] for q in rows)
        for q in rows:
            memory=(folder/q['memory_file']).read_bytes();assert len(memory)==16777216
            linear=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
            address=physical(linear,memory,q)
            assert address==q['fetched_code_physical'] and memory[address:address+32].hex()==q['fetched_code_hex'], 'bootstrap code/RAM differs'
            code=relocate(model,q['segments'][1]['base']);ip=q['cpu_regs.ip.dword[0]']
            assert code[ip:ip+32].hex()==q['fetched_code_hex'], 'bootstrap bytes differ from relocated resident'
            if context:
                comparable={k:v for k,v in q.items() if k not in FETCH_META}
                matches=[r for r in irq_fetches if r['index']==context and all(r.get(k)==v for k,v in comparable.items())]
                assert len(matches)==1, 'bootstrap differs from complete reached IRQ fetch'
        # Decode the reached SS memory AND instruction, including its actual displacement/immediate.
        pairs=[(a,b) for a,b in zip(rows,rows[1:]) if bytes.fromhex(a['fetched_code_hex']).startswith(b'\x36\x80\x26')]
        assert len(pairs)==1
        a,b=pairs[0];opcode=bytes.fromhex(a['fetched_code_hex']);offset=int.from_bytes(opcode[3:5],'little');imm=opcode[5]
        before=bytearray((folder/a['memory_file']).read_bytes());after=(folder/b['memory_file']).read_bytes()
        linear=(a['segments'][2]['base']+offset)&0xffffffff;address=physical(linear,before,a)
        left=before[address];result=left&imm;before[address]=result
        assert before==after, 'complete bootstrap AND RAM transition'
        expected={k:copy.deepcopy(v) for k,v in a.items() if k not in FETCH_META}
        expected['cpu_regs.ip.dword[0]']+=6;expected['CPU_Cycles']-=1
        for key,value in [('lflags.var1.dword[0]',left),('lflags.var2.dword[0]',imm),('lflags.res.dword[0]',result)]:
            expected[key]=(expected[key]&0xffffff00)|value
        expected['lflags.type']=types.index('t_ANDb')
        assert expected=={k:v for k,v in b.items() if k not in FETCH_META}, 'complete bootstrap AND CPU transition'
        ltr=next(q for q in system if q.get('operation')=='CPU_LTR' and q['context']==context)
        selector=ltr['arguments']['selector'];descriptor=ltr['cpu.gdt.table_base']+(selector&~7)
        assert linear==descriptor+5 and physical(descriptor+5,after,b)==address
        assert left&0x1f in (9,11) and result&0x1f==9
        assert ltr['cpu_tss.desc.saved.fill[1]']&0x1f00==(0xb00 if context else 0)
        # MOV SS retains the following instruction in the current normal-core slice.
        stack_pairs=[(a,b) for a,b in zip(rows,rows[1:]) if bytes.fromhex(a['fetched_code_hex']).startswith(b'\x2e\x8e\x16')]
        assert len(stack_pairs)==1
        stack_before,stack_after=stack_pairs[0]
        assert stack_before['CPU_Cycles']==stack_after['CPU_Cycles']
        assert stack_after['segments'][2]!=stack_before['segments'][2]
        contracts.append(dict(group=group,context=context,architecture=rows[0]['architecture'],
                              and_before_ordinal=a['ordinal'],and_after_ordinal=b['ordinal'],
                              and_linear=linear,and_physical=address,and_input=left,and_immediate=imm,and_result=result,
                              tss_selector=selector,raw_cached_tss_kind=ltr['cpu_tss.is386'],
                              cached_descriptor_high_before_ltr=ltr['cpu_tss.desc.saved.fill[1]'],
                              mov_ss_budget_credit=1,clock_fields=['CPU_Cycles','CPU_CycleLeft','PIC_Ticks','CPU_CycleMax']))
    assert contracts[0]['and_input']==contracts[0]['and_result']
    assert contracts[1]['and_input']!=contracts[1]['and_result'], 'repeated busy clear not reached'
    return dict(fetches=boot,contracts=contracts,resident_asset_sha256=digest(repo/'armoredfist/FIST.RUN'))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    proof=verify(repo,root,bootstrap=True) if args.verify_only else capture(repo,root,bootstrap=True)
    # The shared IRQ fixture owns hardware/handler/API contracts; record only this extension.
    proof['scope']='Original first and actual protected-IRQ repeated LGDT-to-LTR bootstrap. Eighteen full16MiB fetch boundaries and eight system API pairs reuse the original IRQ owner. Exact reached AND busy-bit CPU/RAM transition, MOV-SS budget credit, relocated code and complete unchanged original output. Other bootstrap instruction interpretations, general paging and production startup/IRQ integration remain open.'
    proof['reproduction']=['python3 -B tools/oracle/capture_resident_bootstrap.py --repo . --output /tmp/wasm-fist-resident-bootstrap-replay', 'python3 -B tools/oracle/capture_resident_bootstrap.py --repo . --output /tmp/wasm-fist-resident-bootstrap-replay --verify-only']
    proof['original']=proof.pop('bootstrap_original')
    (root/'bootstrap-proof.json').write_text(format_case(proof)+'\n')
    if not args.verify_only:
        case=repo/'tools/oracle/resident_bootstrap_case.json'
        if case.exists():verify(repo,root,bootstrap=True)
    print('PASS: complete first/repeated original bootstrap source boundaries and reaching busy clear')


if __name__=='__main__':main()
