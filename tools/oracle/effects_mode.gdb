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
image=(repo/'re_out/fist_image.bin').read_bytes()
ext_cs=None;calls=0;active=False;at_return=False;watches=[]
boundaries={0x138d,0x76fd,0x7702,0x773e,0x7745,0x7747,0x774e,0x7750,0x7752,0x7757,0x775c,0x7761,0x23ec}
def observe(q,whole=False):
 m=memory();ip=q['cpu_regs.ip.dword[0]'];code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
 flat=lambda a:physical((q['segments'][3]['base']+a)&0xffffffff,m,q)
 q.update(call=calls,fetched_code_physical=code,fetched_code_hex=m[code:code+32].hex(),ready=m[flat(0x77e0)],mode=m[flat(0x77e1)],mixer_active=m[flat(0x2293)],mixer_pointer=struct.unpack_from('<I',m,flat(0x2716))[0])
 with (root/'effects-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 if whole:
  label='%02d-%04x'%(calls,ip)
  (root/(label+'.json')).write_text(json.dumps(q,indent=2)+'\n')
  (root/(label+'.memory')).write_bytes(m)
class Fetch(gdb.Breakpoint):
 def stop(self):
  global active,at_return
  if not active:return False
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if at_return:
   observe(q,True);active=False;at_return=False;self.enabled=False;return False
  observe(q,q['segments'][1]['value']==ext_cs and ip in boundaries)
  if q['segments'][1]['value']==ext_cs and ip==0x23ec:
   self.enabled=False
   m=memory();code=q['segments'][1]['base']
   watches.append(Resume(int(gdb.parse_and_eval('MemBase'))+physical((code+0x7757)&0xffffffff,m,q),0x7757,True))
   watches.append(Resume(int(gdb.parse_and_eval('MemBase'))+physical((code+0x138d)&0xffffffff,m,q),0x138d,False))
  if q['segments'][1]['value']==ext_cs and ip==0x7761:at_return=True
  return False
trace=Fetch('fist_cpu_trace');trace.enabled=False
class Resume(gdb.Breakpoint):
 def __init__(self,address,entry,resume):
  self.entry=entry;self.resume=resume
  super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  q=state()
  if q['segments'][1]['value']!=ext_cs or q['cpu_regs.ip.dword[0]']!=self.entry:return False
  self.enabled=False;observe(q,True)
  if self.resume:trace.enabled=True
  return False
class Entry(gdb.Breakpoint):
 def __init__(self,address):
  self.address=address;super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global ext_cs,calls,active
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if ip!=0x76fd:return False
  m=memory();code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  if int(gdb.parse_and_eval('MemBase'))+code!=self.address:return False
  assert m[code:code+5]==image[ip:ip+5]
  if ext_cs is None:ext_cs=q['segments'][1]['value']
  assert q['segments'][1]['value']==ext_cs and not active
  calls+=1;observe(q,True);active=True;trace.enabled=True;return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state();m=memory();base=q['segments'][1]['base']
  p=physical((base+0x84c0)&0xffffffff,m,q)
  if m[p:p+32]!=image[0x84c0:0x84e0]:return False
  p=physical((base+0x76fd)&0xffffffff,m,q)
  if m[p:p+5]!=image[0x76fd:0x7702]:return False
  self.enabled=False;watches.append(Entry(int(gdb.parse_and_eval('MemBase'))+p));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  watches.append(CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True));self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
