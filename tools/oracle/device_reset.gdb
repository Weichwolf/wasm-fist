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
image=(repo/'re_out/fist_image.bin').read_bytes();ext_cs=None;cached=None;return_ip=None;watches=[]
boundaries={0x23c4,0x133a,0x133b,0x1357,0x137f,0x1380,0x138c}
def observe(q):
 ip=q['cpu_regs.ip.dword[0]'];base=int(gdb.parse_and_eval('MemBase'))
 address=physical((q['segments'][1]['base']+ip)&0xffffffff,cached,q)
 code=bytes(gdb.selected_inferior().read_memory(base+address,32))
 assert code==image[ip:ip+32],hex(ip)
 q.update(fetched_code_physical=address,fetched_code_hex=code.hex(),entry_return_ip=return_ip)
 with (root/'reset-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 if ip in boundaries or ip==return_ip:
  (root/('%04x.json'%ip)).write_text(json.dumps(q,indent=2)+'\n')
  (root/('%04x.memory'%ip)).write_bytes(memory())
class Fetch(gdb.Breakpoint):
 def stop(self):
  q=state();assert q['segments'][1]['value']==ext_cs
  observe(q)
  if q['cpu_regs.ip.dword[0]']==return_ip:self.enabled=False
  return False
trace=Fetch('fist_cpu_trace');trace.enabled=False
class Entry(gdb.Breakpoint):
 def __init__(self,address):
  self.address=address;super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global ext_cs,cached,return_ip
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if ip!=0x23c4:return False
  m=memory();address=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  if int(gdb.parse_and_eval('MemBase'))+address!=self.address:return False
  assert m[address:address+32]==image[ip:ip+32]
  cached=m;ext_cs=q['segments'][1]['value']
  stack=physical((q['segments'][2]['base']+q['registers'][4])&0xffffffff,m,q)
  return_ip=struct.unpack_from('<I',m,stack)[0]
  self.enabled=False;observe(q);trace.enabled=True;return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state();m=memory();base=q['segments'][1]['base']
  p=physical((base+0x84c0)&0xffffffff,m,q)
  if m[p:p+32]!=image[0x84c0:0x84e0]:return False
  p=physical((base+0x23c4)&0xffffffff,m,q)
  if m[p:p+32]!=image[0x23c4:0x23e4]:return False
  self.enabled=False;watches.append(Entry(int(gdb.parse_and_eval('MemBase'))+p));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  watches.append(CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True));self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
