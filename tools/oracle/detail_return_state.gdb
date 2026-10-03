set pagination off
set confirm off
python
import ast,gdb,json,os,struct
from pathlib import Path
repo=Path(os.environ['FIST_DETAIL_REPO']);root=Path(os.environ['FIST_DETAIL_OPERANDS_DIR'])
# State and paging have one owner. Import its definitions without arming the
# missing-file observer or changing the successful game.
shared=repo/'tools/oracle/file_error.gdb'
program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
nodes=[n for n in ast.parse(program).body if
 isinstance(n,ast.FunctionDef) and n.name in ('state','memory','physical') or
 isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='fields' for t in n.targets)]
assert len(nodes)==4
exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),globals())
reference=json.loads((repo/'tools/oracle/detail_return_case.json').read_text())
expected=reference['handler_to_op68'];index=0;watches=[]
kernel_cs=expected[0]['cs'];engine_cs=reference['states']['op68-entry']['cs']
kernel_ips={0x76fc:'body-return',0x10df:'handler-return',0xf57:'restore-service-stack',0xf5d:'far-return',0xbf04:'far-caller-return'}
engine_ips={0xe34c:'gate-return',0xe34f:'save-eax',0xe350:'save-esi',0xe351:'task-segment-load',0xe355:'task-offset-load',0xe359:'task-test',0xe35d:'normal-branch',0xe36a:'restore-esi',0xe36b:'restore-eax',0xe36c:'gate-near-return',0xdea5:'de89-near-return',0x6dfd:'first-config-load',0x6e00:'first-config-shift',0x6e02:'first-config-call',0xc008:'config-store',0xc00b:'config-return',0x6e05:'second-config-load',0x6e08:'second-config-shift',0x6e0a:'second-config-call',0xe2df:'op68-entry'}
def observe(q):
 ip=q['cpu_regs.ip.dword[0]'];cs=q['segments'][1]['value']
 label=(kernel_ips if cs==kernel_cs else engine_ips if cs==engine_cs else {}).get(ip)
 if label:
  m=memory();code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  stack=physical((q['segments'][2]['base']+q['registers'][4])&0xffffffff,m,q)
  q.update(fetched_code_physical=code,fetched_code_hex=m[code:code+32].hex(),stack_physical=stack,stack_hex=m[stack:stack+64].hex())
  (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n')
  (root/(label+'.memory')).write_bytes(m)
 with (root/'return-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
class ReturnFetch(gdb.Breakpoint):
 def stop(self):
  global index
  q=state();r=expected[index]
  assert (q['segments'][1]['value'],q['cpu_regs.ip.dword[0]'])==(r['cs'],r['ip']),(index,q,r)
  observe(q);index+=1
  if index==len(expected):self.enabled=False
  return False
trace=ReturnFetch('fist_cpu_trace');trace.enabled=False
class CodeRead(gdb.Breakpoint):
 def __init__(self,address,entry):
  self.entry=entry;super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global index
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if q['segments'][1]['value']!=kernel_cs or ip!=self.entry:return False
  m=memory();code=q['segments'][1]['base'];image=(repo/'re_out/fist_image.bin').read_bytes()
  address=physical((code+ip)&0xffffffff,m,q);size=1 if ip==0x76fc else 5
  assert m[address:address+size]==image[ip:ip+size],('code',ip,m[address:address+size].hex())
  self.enabled=False
  if ip==0x6032:
   path=physical((q['segments'][3]['base']+q['registers'][6])&0xffffffff,m,q)
   actual=m[path:path+24].split(b'\0',1)[0]
   assert actual==image[0x76f3:].split(b'\0',1)[0],('embedded filename',actual)
   watches.append(CodeRead(int(gdb.parse_and_eval('MemBase'))+physical((code+0x76fc)&0xffffffff,m,q),0x76fc))
  else:observe(q);index=1;trace.enabled=True
  return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state()
  if q['segments'][1]['value']!=kernel_cs:return False
  m=memory();image=(repo/'re_out/fist_image.bin').read_bytes();code=physical((q['segments'][1]['base']+0x6032)&0xffffffff,m,q)
  if m[code:code+5]!=image[0x6032:0x6037]:return False
  self.enabled=False;watches.append(CodeRead(int(gdb.parse_and_eval('MemBase'))+code,0x6032));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=500:return False
  watches.append(CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True));self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
