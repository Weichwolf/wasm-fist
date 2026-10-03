#!/usr/bin/env python3
"""Recover original op44 dispatcher registers, saved service stack and near/far return frames."""
import argparse
import ast
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys

from capture_file_error import digest
from capture_file_error_main import verify as verify_continuation

ROOT = Path(__file__).resolve().parents[2]


def verify(root, repo):
    producers=json.loads((root/'producers.json').read_text())
    for name,sha in producers.items():assert digest(name)==sha,name
    body=verify_continuation(root,repo)
    program=(repo/'tools/oracle/file_error.gdb').read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    functions=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical'];assert len(functions)==1
    ns={'struct':struct};exec(compile(ast.fix_missing_locations(ast.Module(body=functions,type_ignores=[])),'shared original paging owner','exec'),ns);physical=ns['physical']
    image=(repo/'re_out/fist_image.bin').read_bytes()
    labels={'service-entry':0xf30,'service-stack-saved':0xf36,'service-index':0xf39,'service-target':0xf3f,'service-task-read':0xf45,'service-inbox-read':0xf4b,'service-handler-call':0xf51,'service-handler-entry':0x10da,'service-body-entry':0x7660}
    results=[]
    for stage in ('find','open'):
     folder=root/stage/'error';states={p.stem:json.loads(p.read_text()) for p in folder.glob('service*.json')};assert set(states)==set(labels)
     memory={name:(folder/(name+'.memory')).read_bytes() for name in states};assert all(len(m)==16777216 for m in memory.values())
     def flat(q,m,off):return physical((q['segments'][3]['base']+off)&0xffffffff,m,q)
     def u32(q,m,off):return struct.unpack_from('<I',m,flat(q,m,off))[0]
     def clock(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
     for name,ip in labels.items():
      q=states[name];m=memory[name];assert q['cpu_regs.ip.dword[0]']==ip
      code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
      # f60 is mutable saved-stack data next to code; check each complete
      # fetched instruction, using the lengths decoded from the original ASM.
      size={0xf30:6,0xf36:3,0xf39:6,0xf3f:6,0xf45:6,0xf4b:6,0xf51:6,0x10da:5,0x7660:6}[ip]
      assert m[code:code+size]==image[ip:ip+size],name
     def transition(a_name,b_name,regs,writes=()):
      a,b=states[a_name],states[b_name];assert b['registers']==regs,(a_name,b_name,'registers')
      expected=bytearray(memory[a_name])
      for address,value in writes:struct.pack_into('<I',expected,address,value)
      assert memory[b_name]==expected,(a_name,b_name,'whole RAM')
      fixed=[k for k in a if k.startswith(('cpu.','cpu_regs.','paging.','lflags.')) and k!='cpu_regs.ip.dword[0]']+['segments','PIC_Ticks','CPU_CycleMax','CPU_CycleLeft']
      for key in fixed:assert a[key]==b[key],(a_name,b_name,key)
      assert clock(b)-clock(a)==1,(a_name,b_name,'one actual fetch')
     a=states['service-entry'];m=memory['service-entry'];selector=a['registers'][3]&65535;assert selector==0x44
     entry_regs=a['registers'].copy();transition('service-entry','service-stack-saved',entry_regs,[(flat(a,m,0xf60),entry_regs[4])])
     regs=entry_regs.copy();regs[3]=selector;transition('service-stack-saved','service-index',regs)
     index=states['service-index'];im=memory['service-index'];target=u32(index,im,0xcb3+regs[3]);assert target==labels['service-handler-entry']
     assert target==struct.unpack_from('<I',image,0xcb3+selector)[0]
     regs[3]=target;transition('service-index','service-target',regs)
     t=states['service-target'];transition('service-target','service-task-read',regs,[(flat(t,memory['service-target'],0xcaf),target)])
     regs[3]=states['service-task-read']['tcb_logical'];transition('service-task-read','service-inbox-read',regs)
     q=states['service-inbox-read'];inbox=u32(q,memory['service-inbox-read'],q['tcb_logical']+0x3f2);regs[3]=inbox;transition('service-inbox-read','service-handler-call',regs)
     for a_name,b_name,return_ip in [('service-handler-call','service-handler-entry',0xf57),('service-handler-entry','service-body-entry',0x10df)]:
      regs=regs.copy();regs[4]=(regs[4]-4)&0xffffffff;b=states[b_name]
      address=physical((b['segments'][2]['base']+regs[4])&0xffffffff,memory[a_name],states[a_name])
      transition(a_name,b_name,regs,[(address,return_ip)])
     loader=json.loads((folder/'at-6032.json').read_text());exit_state=json.loads((folder/'at-0f5d.json').read_text());exit_memory=(folder/'at-0f5d.memory').read_bytes()
     assert loader['registers'][4]==(entry_regs[4]-12)&0xffffffff
     assert exit_state['registers'][4]==exit_state['saved_service_ESP']==entry_regs[4]
     stack=physical((a['segments'][2]['base']+entry_regs[4])&0xffffffff,m,a)
     frame=m[stack:stack+8];assert exit_memory[stack:stack+8]==frame
     return_ip,return_cs=struct.unpack('<II',frame)
     fetch=json.loads((folder/'caller-fetches.jsonl').read_text().splitlines()[0])
     assert fetch['cpu_regs.ip.dword[0]']==return_ip and fetch['segments'][1]['value']==return_cs&65535
     expected=exit_state['registers'].copy();expected[4]+=8;assert fetch['registers']==expected
     results.append(dict(stage=stage,states=states,memory_sha256={n:hashlib.sha256((folder/(n+'.memory')).read_bytes()).hexdigest() for n in states},selector=selector,full_incoming_EBX=entry_regs[3],handler=target,inbox=inbox,service_ESP=entry_regs[4],far_return_frame=frame.hex(),near_stack_words=[0xf57,0x10df],loader_stack_depth_bytes=12,whole_RAM_transitions_exact=True,eight_prefix_instructions=8,first_caller_fetch=fetch))
    proof=dict(scope='Source-only actual0f30 service stack save, MOVZX EBX/BX selector, near callback table/caf, current task/inbox and two nested near CALL frames for op44. Nine complete GP/segments/raw+lazyflags/control/time/whole16MiB boundaries per find/open failure. Every prefix transition is one real fetch; exact whole RAM DWORD stores/pushes and unchanged flags/segments are checked. Actual6032 has12 bytes of near frames; f57 restores the entry ESP, and f5d consumes the original8-byte far return frame. No guessed stack pointer, ABI scalar/normal return or port acceptance.',cases=results,main_continuation=body,script_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__),Path(__file__).with_name('file_service_stack.gdb'),repo/'tools/oracle/file_error.gdb',repo/'tools/oracle/file_error_main.gdb']},producers=producers,reproduction='python3 -B tools/oracle/capture_file_service_stack.py --output /tmp/wasm-fist-file-service-stack-source',kernel_image_sha256=hashlib.sha256(image).hexdigest(),code=[dict(image_offset=x,bytes=image[x:y].hex()) for x,y in [(0xf23,0xf77),(0xcb3,0xd13),(0x10da,0x10e0),(0x7660,0x76fd)]],frames=38,mixed_samples=27518,endpoint_ms=600,complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: both original service prefixes/near frames/whole RAM stores and restored8-byte far return; complete error/main continuation pairs unchanged.')

    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    repo, root = args.repo.resolve(strict=True), args.output.resolve()
    if args.verify_only:
        return verify(root, repo)
    if not root.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True, exist_ok=False)
    binary = repo/'third_party/dosbox-fist'
    probe = Path(__file__).resolve().with_name('file_service_stack.gdb')
    files=(binary,probe,Path(__file__).resolve(),repo/'tools/oracle/capture_file_error.py',repo/'tools/oracle/file_error.gdb',repo/'tools/oracle/capture_file_error_main.py',repo/'tools/oracle/file_error_main.gdb',repo/'tools/oracle/file_error_case.json',repo/'tools/oracle/capture_sequence.sh',repo/'tools/oracle/sequence_format.py',repo/'re_out/fist_image.bin',repo/'re_out/fist_dat_image.bin',repo/'armoredfist/FISTDATA/HIGH.DTL')
    (root/'producers.json').write_text(json.dumps({str(p):digest(p) for p in files},indent=2)+'\n')
    for stage in ('find', 'open'):
        case = root/stage
        case.mkdir()
        for name, observe in [('baseline', False), ('error', True)]:
            folder = case/name
            folder.mkdir()
            wrapper = case/('dosbox-'+name)
            command = (['gdb', '-q', '-batch', '-x', str(probe), '--args']
                       if observe or stage == 'open' else [])+[str(binary)]
            removal = ('' if stage == 'open' else
                       'mv '+shlex.quote(str(folder/'game/FISTDATA/HIGH.DTL'))+' '+
                       shlex.quote(str(folder/'removed.HIGH.DTL'))+'\n')
            wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\n'+removal+
                               'exec '+shlex.join(command)+' "$@"\n')
            wrapper.chmod(0o755)
            env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
            env.update(FIST_SEQUENCE_END_MS='600', DOSBOX=str(wrapper),
                       FIST_DETAIL_OPERANDS_DIR=str(folder), FIST_DETAIL_REPO=str(repo),
                       FIST_FILE_ERROR_STAGE=stage, FIST_FILE_ERROR_OBSERVE=str(int(observe)))
            with (case/(name+'.log')).open('w') as log:
                result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(folder)],
                                        cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
            (case/(name+'.exit')).write_text(str(result.returncode)+'\n')
            assert result.returncode == 0, (stage, name, result.returncode)
    return verify(root, repo)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, TypeError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
