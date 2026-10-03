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
fields=fields+('cpu.stack.mask','cpu.stack.notmask','cpu.idt.table_base','cpu.idt.table_limit','cpu.gdt.table_base','cpu.gdt.table_limit')
image=(repo/'re_out/fist_image.bin').read_bytes();ext_cs=None;resident_cs=None;watches=[]
boundaries={0x138d,0x138e,0x1394,0x1397,0x139c,0x139f,0x13a0,0x13a4,0x13a6,0x13a7,0x13ad,0x13ae,0x13b0,0x13b2,0x13b7,0x13bb,0x13bd,0x13be}
resident_boundaries={0x1fba,0x1fbe,0x1fc2,0x1fd6,0x1fdd,0x225b,0x225c,0x2237,0x223b,0x223e,0x2241,0x2246}
phase=0
def observe(q):
 global phase,resident_cs
 if q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]']==0x13a4:phase=1
 if q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]']==0x13bb:phase=2
 if resident_cs is None and phase==1 and q['segments'][1]['value']!=ext_cs and q['cpu_regs.ip.dword[0]']==0x197d:
  fixture=json.loads((repo/'tools/oracle/sb_irq_frame_case.json').read_text())['resident_entry_region'];bias=fixture['asset_file_offset']-fixture['ip'];blob=(repo/'armoredfist/FIST.RUN').read_bytes();m=memory();address=physical((q['segments'][1]['base']+0x197d)&0xffffffff,m,q)
  assert m[address:address+4]==blob[bias+0x197d:bias+0x1981];resident_cs=q['segments'][1]['value']
 q['phase']=phase
 if q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]'] in boundaries or phase==2 and q['segments'][1]['value']==resident_cs and q['cpu_regs.ip.dword[0]'] in resident_boundaries:
  ip=q['cpu_regs.ip.dword[0]'];m=memory();code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  ds=q['segments'][3]['base'];flat=lambda a:physical((ds+a)&0xffffffff,m,q)
  q.update(fetched_code_physical=code,fetched_code_hex=m[code:code+32].hex(),saved_vector=struct.unpack_from('<I',m,flat(0x12c0))[0],irq=struct.unpack_from('<I',m,flat(0x12c4))[0])
  name=('%04x'%ip if q['segments'][1]['value']==ext_cs else 'resident-%04x'%ip);(root/(name+'.json')).write_text(json.dumps(q,indent=2)+'\n');(root/(name+'.memory')).write_bytes(m)
 with (root/'vector-fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
class Fetch(gdb.Breakpoint):
 def stop(self):
  q=state();observe(q)
  if q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]']==0x13be:self.enabled=False
  return False
trace=Fetch('fist_cpu_trace');trace.enabled=False
class Entry(gdb.Breakpoint):
 def __init__(self,address):
  self.address=address;super().__init__('*(unsigned char *)(%d)'%address,type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_READ,internal=True)
 def stop(self):
  global ext_cs
  q=state();ip=q['cpu_regs.ip.dword[0]']
  if ip!=0x138d:return False
  m=memory();code=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
  if int(gdb.parse_and_eval('MemBase'))+code!=self.address:return False
  assert m[code:code+1]==image[ip:ip+1]
  self.enabled=False;ext_cs=q['segments'][1]['value'];observe(q);trace.enabled=True;return False
class CSWrite(gdb.Breakpoint):
 def stop(self):
  q=state();m=memory();base=q['segments'][1]['base']
  p=physical((base+0x84c0)&0xffffffff,m,q)
  if m[p:p+32]!=image[0x84c0:0x84e0]:return False
  p=physical((base+0x138d)&0xffffffff,m,q)
  if m[p:p+1]!=image[0x138d:0x138e]:return False
  self.enabled=False;watches.append(Entry(int(gdb.parse_and_eval('MemBase'))+p));return False
class Arm(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('PIC_Ticks'))!=10:return False
  watches.append(CSWrite('Segs.val[1]',type=gdb.BP_WATCHPOINT,wp_class=gdb.WP_WRITE,internal=True));self.enabled=False;return False
Arm('TIMER_AddTick')
end
run
