set pagination off
set confirm off
python
# Reuse the original error-body/CF observer and its paged state/RAM owner.
from pathlib import Path
import os
original_probe=Path(os.environ['FIST_DETAIL_REPO'])/'tools/oracle/file_error.gdb'
program=original_probe.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
exec(compile(program,str(original_probe),'exec'),globals())
caller_labels=set();engine_cs=None;service_cs=None;caller_limit=0
class Continuation(gdb.Breakpoint):
 def stop(self):
  global engine_cs,service_cs,caller_limit
  q=state()
  with (root/'caller-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  ip=q['cpu_regs.ip.dword[0]'];cs=q['segments'][1]['value']
  # Segment roles are recovered from the original transition, not fixed load segments.
  if engine_cs is None and ip==0xe34c:engine_cs=cs
  if engine_cs is not None and cs!=engine_cs and ip==0x314:service_cs=cs
  label=None
  if engine_cs is not None and cs==engine_cs:
   label={0xe34c:'main-service-return',0xe366:'main-abort-jump',0xe0:'main-loop-resume',0xe5:'main-loop-carry-test',0xd8:'main-loop-next',0xe7:'main-loop-exit'}.get(ip)
  elif service_cs is not None and cs==service_cs:
   label={0x314:'crt-restart-entry',0x32b:'crt-restart-jump',0x65cf:'main-loop-poll-entry'}.get(ip)
  if label and label not in caller_labels:
   caller_labels.add(label);m=memory()
   code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
   q['fetched_code_physical']=code;q['fetched_code_hex']=m[code:code+32].hex()
   stack=(q['segments'][2]['base']+q['registers'][4])&0xffffffff
   q['stack_linear']=stack;q['stack_physical']=physical(stack,m,q)
   q['stack_hex']=m[q['stack_physical']:q['stack_physical']+64].hex()
   (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n')
   (root/(label+'.memory')).write_bytes(m)
  caller_limit+=1
  if 'main-loop-next' in caller_labels or 'main-loop-exit' in caller_labels:self.enabled=False
  if caller_limit>10000:raise RuntimeError('Missing original main-loop carry decision within traced continuation')
  return False
continuation=Continuation('fist_cpu_trace');continuation.enabled=False
original_stop=CodeRead.stop
def extended_stop(self):
 result=original_stop(self)
 if self.expected==0xf5d and not self.enabled:
  continuation.enabled=True
 return result
CodeRead.stop=extended_stop
end
run
