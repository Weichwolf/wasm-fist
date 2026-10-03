set pagination off
set confirm off
python
import ast,gdb,json,os,struct
from pathlib import Path
repo=Path(os.environ['FIST_DETAIL_REPO']);root=Path(os.environ['FIST_DETAIL_OPERANDS_DIR'])
shared=repo/'tools/oracle/file_error.gdb'
program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name in ('state','memory','physical') or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='fields' for t in n.targets)]
assert len(nodes)==4
exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),globals())
fields=fields+('cpu.stack.mask','cpu.stack.notmask')
import sys
sys.path.insert(0,str(repo/'tools/oracle'))
from resident_image import load,relocate
resident=load(repo)
relocated=lambda base:relocate(resident,base)
resident_cs=None;watches=[];stores=0
bounds={0x4242,0x4257,0x4267,0x4288,0x428f,0x4293,0x4296,0x429a,0x42a5}
def observe(q):
 global stores
 ip=q['cpu_regs.ip.dword[0]'];assert not q['paging.enabled'] and not q['cpu.pmode']
 address=(q['segments'][1]['base']+ip)&0xffffffff
 raw=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase'))+address,32))
 assert raw==relocated(q['segments'][1]['base'])[ip:ip+32],hex(ip)
 if ip==0x429b:stores+=1
 q.update(fetched_code_physical=address,fetched_code_hex=raw.hex(),stores=stores)
 with (root/'stub-fetches.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
 if ip in bounds or ip==0x429b and stores in (1,256):
  name='%04x-%03d'%(ip,stores);(root/(name+'.json')).write_text(json.dumps(q,indent=2)+'\n');(root/(name+'.memory')).write_bytes(memory())
class Fetch(gdb.Breakpoint):
 def stop(self):
  q=state();assert q['segments'][1]['value']==resident_cs
  observe(q)
  if q['cpu_regs.ip.dword[0]']==0x42a5:self.enabled=False
  return False
trace=Fetch('fist_cpu_trace');trace.enabled=False
class Entry(gdb.Breakpoint):
 def __init__(self,address):
  self.address=address;super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global resident_cs
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if ip!=0x4242:return False
  m=memory();code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  if int(gdb.parse_and_eval('MemBase'))+code!=self.address:return False
  assert m[code:code+32]==relocated(q['segments'][1]['base'])[ip:ip+32]
  self.enabled=False;resident_cs=q['segments'][1]['value'];observe(q);trace.enabled=True;return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('paging.enabled')):return False
  p=(int(gdb.parse_and_eval('Segs.phys[1]'))+0x4242)&0xffffffff
  raw=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase'))+p,32))
  if raw!=relocated(p-0x4242)[0x4242:0x4262]:return False
  self.enabled=False;watches.append(Entry(int(gdb.parse_and_eval('MemBase'))+p));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=1:return False
  watches.append(CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True));self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
