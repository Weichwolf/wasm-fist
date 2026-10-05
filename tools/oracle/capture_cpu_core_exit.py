#!/usr/bin/env python3
"""Observe a reaching IRET/core/PIC/IRQ chain and unchanged complete original output."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
from capture_pit_irq_frames import SYSTEM_FIELDS
from memory_context import validate_context
from sequence_format import validate,validate_endpoint

def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def observer(repo):
 text=(repo/'tools/oracle/core_exit.gdb').read_text()
 cache=(repo/'tools/oracle/paging_control.gdb.inc').read_text()
 extra='SYSTEM_FIELDS='+repr(SYSTEM_FIELDS)+'\n'
 extra+=(repo/'tools/oracle/physical_provider.gdb.inc').read_text()
 extra+=cache[cache.index('def cache_state('):cache.index('\ndef crx_record(')]
 extra+=(repo/'tools/oracle/memory_context.gdb.inc').read_text()
 assert text.count('# CORE_MEMORY_CONTEXT')==1
 return text.replace('# CORE_MEMORY_CONTEXT is composed by the capture tool from the existing owner.',extra)

def verify(repo,root,additional_kinds=()):
 assert (root/'source.exit').read_text()=='0\n'
 for p,h in json.loads((root/'inputs.json').read_text()).items():assert digest(p)==h,p
 originals=json.loads((root/'originals.json').read_text())
 assert set(originals)=={str(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()}
 for p,h in originals.items():assert digest(p)==h,p
 folder=root/'source';log=(folder/'dosbox.log').read_text();assert 'Python Exception' not in log
 assert validate_endpoint(folder/'sequence',600)==600
 frames=validate(str(folder/'sequence.frames'),'F');audio=validate(str(folder/'sequence.pcm'),'A')
 assert frames['records']==39 and audio['samples']==27518
 baseline=Path(json.loads((root/'baseline.json').read_text())['path'])
 for suffix,h in json.loads((root/'baseline.json').read_text())['sha256'].items():assert digest(baseline/('sequence.'+suffix))==h,suffix
 for suffix in ('frames','pcm','end'):
  assert (folder/('sequence.'+suffix)).read_bytes()==(baseline/('sequence.'+suffix)).read_bytes(),suffix
 rows=[json.loads(s) for s in (folder/'events.jsonl').read_text().splitlines()]
 kinds=['before-iret','after-iret','after-core','before-hardware','after-hardware','after-queue','handler-fetch']
 assert [q['kind'] for q in rows]==kinds+list(additional_kinds)
 for q in rows:
  validate_context(q['memory_context'],folder,repo)
  assert (folder/q['memory_file']).stat().st_size==16777216
  assert not q['trap_decoder'] and q['CPU_CycleMax']==30000
 before,iret,core,hardware,after,queue,fetch=rows[:len(kinds)]
 assert before['opcode_hex'].startswith('cf') and before['use32']==0
 assert before['PIC_IRQCheck'] and iret['PIC_IRQCheck'] and iret['cpu_regs.flags']&0x200
 for q in (iret,core,hardware,after,queue):
  assert q['PIC_Ticks']==before['PIC_Ticks'] and q['CPU_Cycles']==before['CPU_Cycles'] and q['CPU_CycleLeft']==before['CPU_CycleLeft']
 assert core['cpu_regs.ip.dword[0]']==iret['cpu_regs.ip.dword[0]']
 assert core['lflags.type']==0 and hardware['num']==8 and hardware['type']==0
 assert hardware['PIC_IRQCheck']==0 and hardware['PIC_IRQActive']==255
 assert hardware['pic']['irqs'][0]['inservice']==after['pic']['irqs'][0]['inservice']==0
 assert queue['PIC_IRQActive']==0 and queue['pic']['irqs'][0]['inservice']==1
 assert fetch['CPU_Cycles']==queue['CPU_Cycles']-1
 assert fetch['cpu_regs.ip.dword[0]']==queue['cpu_regs.ip.dword[0]']
 result=dict(scope='Actual original first reaching pending-IRQ IRET, normal-core return, PIC hardware delivery and first handler fetch. All58 CPU/system words and complete RAM/provider/cache/VGA/PIC states are retained from one run. Complete600ms39-frame/27518-PCM/end output equals unprobed original; no complete port runtime/video/audio acceptance.',events=rows,frames=frames['records'],samples=audio['samples'],endpoint_ms=600,source_manifest_sha256=digest(root/'inputs.json'),memory_sha256={q['memory_file']:digest(folder/q['memory_file']) for q in rows},complete_original_acceptance=False)
 (root/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS full original pending-IRQ IRET/core/PIC/first-fetch chain and unchanged complete600ms output',flush=True)
 return result

def capture(repo,root,baseline=None,*,make_observer=None,check_capture=None,additional_inputs=(),source_wall_seconds=120):
 assert root.is_relative_to(Path('/tmp'));root.mkdir(exist_ok=False,parents=True)
 if baseline is None:
  baseline=root/'baseline';baseline.mkdir()
  env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
  env.update(DOSBOX=str(repo/'third_party/dosbox-fist'),FIST_SEQUENCE_END_MS='600')
  with (root/'baseline.log').open('w') as out:p=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(baseline)],cwd=repo,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=90)
  (root/'baseline.exit').write_text(str(p.returncode)+'\n');assert p.returncode==0
  assert validate_endpoint(baseline/'sequence',600)==600
  assert validate(str(baseline/'sequence.frames'),'F')['records']==39
  assert validate(str(baseline/'sequence.pcm'),'A')['samples']==27518
 folder=root/'source';folder.mkdir();probe=root/'observer.gdb';probe.write_text((make_observer or observer)(repo))
 paths=[Path(__file__),probe,*[repo/'tools/oracle'/name for name in ('core_exit.gdb','capture_sequence.sh','file_error.gdb','physical_provider.gdb.inc','paging_control.gdb.inc','memory_context.gdb.inc','memory_context.py','capture_pit_irq_frames.py')],repo/'third_party/dosbox-fist']
 tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
 paths += [tree/name for name in ('src/cpu/core_normal.cpp','src/cpu/core_normal/prefix_none.h','src/cpu/core_normal/prefix_66.h','src/cpu/cpu.cpp','src/cpu/flags.cpp','src/cpu/paging.cpp','src/hardware/pic.cpp','include/cpu.h','include/regs.h','include/mem.h')]
 paths += list(additional_inputs)
 (root/'inputs.json').write_text(json.dumps({str(p):digest(p) for p in paths},indent=2)+'\n')
 (root/'originals.json').write_text(json.dumps({str(p):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()},indent=2)+'\n')
 (root/'baseline.json').write_text(json.dumps(dict(path=str(baseline),sha256={s:digest(baseline/('sequence.'+s)) for s in ('frames','pcm','end')}),indent=2)+'\n')
 wrapper=root/'dosbox-gdb';wrapper.write_text('#!/usr/bin/env bash\nexec '+shlex.join(['gdb','-q','-batch','-x',str(probe),'--args',str(repo/'third_party/dosbox-fist')])+' "$@"\n');wrapper.chmod(0o755)
 env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
 assert isinstance(source_wall_seconds,int) and source_wall_seconds>0
 env.update(DOSBOX=str(wrapper),FIST_SEQUENCE_END_MS='600',FIST_ORACLE_WALL_SECONDS=str(source_wall_seconds),FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder))
 with (root/'source.log').open('w') as out: p=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(folder)],cwd=repo,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=source_wall_seconds+10)
 (root/'source.exit').write_text(str(p.returncode)+'\n');assert p.returncode==0,p.returncode
 return (check_capture or verify)(repo,root)

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--baseline',type=Path);parser.add_argument('--verify-only',action='store_true')
 args=parser.parse_args();repo=args.repo.resolve();root=args.output.resolve()
 if args.verify_only:verify(repo,root)
 else:
  capture(repo,root,args.baseline.resolve() if args.baseline else None)
