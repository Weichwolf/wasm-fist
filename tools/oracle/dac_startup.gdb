set pagination off
set confirm off
python
import ast,gdb,json,os
from pathlib import Path
repo=Path(os.environ['FIST_DETAIL_REPO']);root=Path(os.environ['FIST_DETAIL_OPERANDS_DIR'])
shared=repo/'tools/oracle/file_error.gdb'
program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='state' or isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='fields' for t in n.targets)]
exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),globals())
exec((repo/'tools/oracle/dac_state.gdb.inc').read_text(),globals())
def snapshot(kind):
 q=state();q.update(kind=kind,machine=int(gdb.parse_and_eval('machine')),svga_card=int(gdb.parse_and_eval('svgaCard')))
 q['renderer_output']=dict(out_mode=int(gdb.parse_and_eval('render.scale.outMode')),host_type=int(gdb.parse_and_eval("'sdlmain.cpp'::sdl.desktop.type")),pixel_format={k:int(gdb.parse_and_eval("'sdlmain.cpp'::sdl.surface->format."+k)) for k in ('BitsPerPixel','BytesPerPixel','Rloss','Gloss','Bloss','Aloss','Rshift','Gshift','Bshift','Ashift','Rmask','Gmask','Bmask','Amask')})
 q['dac']=dac_state()
 q['attr']={k:int(gdb.parse_and_eval('vga.attr.'+k)) for k in ('index','mode_control','color_select','disabled')};q['attr']['palette_hex']=raw_field('vga.attr.palette')
 with (root/'startup.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
class Executed(gdb.FinishBreakpoint):
 def __init__(self,name):self.name=name;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):snapshot('after-DOS-Execute-'+self.name);return False
class Execute(gdb.Breakpoint):
 def stop(self):
  assert int(gdb.parse_and_eval('$pc'))==int(gdb.parse_and_eval('(void *)DOS_Execute'))
  name=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('$rdi')),260)).split(b'\0',1)[0]
  name=name.upper()
  filename='FIST-RUN' if b'FIST.RUN' in name else 'FIST-DAT' if b'FIST.DAT' in name else None
  if filename is None:return False
  snapshot('before-DOS-Execute-'+filename);Executed(filename);return False
Execute('*DOS_Execute')
end
run
