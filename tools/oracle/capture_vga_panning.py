#!/usr/bin/env python3
"""Original panning programs reuse the common complete drawing transport."""
from pathlib import Path
import argparse,ast,hashlib,itertools,json,struct,subprocess,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tests'))
from capture_vga_draw_programs import original_program
from test_port_io import tool
def capture(repo,work):
 assert work.is_relative_to(Path('/tmp'));work.mkdir(parents=True,exist_ok=False)
 original,fields,configs,extras=original_program(repo)
 path=repo/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/vga_draw.cpp';tree=path.parents[2];s=path.read_text();start=s.index('static void VGA_PanningLatch(');end=s.index('static void VGA_VerticalTimer(',start)
 original=original.replace('int main() {',s[start:end]+'\nint main() {',1).replace('default:abort();','case 5:VGA_PanningLatch(q[39]);break;default:abort();',1)
 # Reuse the complete drawing-state/RAM/request transport, not another model.
 s=(repo/'tools/oracle/capture_vga_draw_programs.py').read_text();module=ast.parse(s)
 function=next(n for n in module.body if isinstance(n,ast.FunctionDef) and n.name=='capture')
 assignment=next(n for n in function.body if isinstance(n,ast.Assign) and any(isinstance(x,ast.Name) and x.id=='portable' for x in n.targets));portable=ast.literal_eval(assignment.value)
 portable=portable.replace('default:abort();','case 5:fist_vga_panning_latch(&v);break;default:abort();',1)
 (work/'original.cpp').write_text(original);(work/'portable.c').write_text(portable)
 header=(repo/'re_out/fist_vga_draw.h').read_text();(work/'fist_vga_draw.h').write_text(header)
 def build(name,args):
  p=subprocess.run(args,capture_output=True,text=True,timeout=120);(work/(name+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
 build('original',['g++','-O2','-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),str(work/'original.cpp'),'-o',str(work/'original')])
 def target_build(directory,text):
  directory.mkdir(exist_ok=True);(directory/'fist_vga_draw.h').write_text(text);(directory/'portable.c').write_text(portable);runs=[]
  for name,compiler,options,output,runner in [('native',['gcc','-m32'],[],directory/'native',[]),('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],directory/'wasm.js',[tool('node','Git/emsdk/node/*/bin/node')])]:
   build(name,[*compiler,'-O2','-DNDEBUG','-I'+str(directory),str(directory/'portable.c'),*options,'-o',str(output)]);runs.append((name,[*runner,str(output)]))
  return runs
 commands=target_build(work/'targets',header)
 base=[0,320,200,80,48000,5,2,65535,320,320,1,0,200,0,150,2,512,4,50,1,4,8,0,1,1,0,0,0]
 base+=list(struct.unpack('<Q',struct.pack('<d',3.177755)))+[0,0,2,5,5,3,65,142,262144];assert len(base)==38
 ram=bytes((i*37+(i>>8)+(i>>16)*73+11)&255 for i in range(0x40000));cases=[]
 for draw,config,value in itertools.product((0,1,255,256,0xffffffff,0x100000000,0x12345678abcdef00,0xffffffffffffffff),range(256),(0,0x80000000,0xffffffffffffffff)):
  q=base.copy();q[5]=draw;q[32]=config;cases.append(struct.pack('<40Q',*q,5,value))
 assert len(cases)==6144
 record_size=38*8+len(ram)+4;hashes={name:hashlib.sha256() for name in ('original','native','wasm')}
 def run(command,data):
  p=subprocess.run(command,input=data,capture_output=True,timeout=45);assert p.returncode==0,p.stderr[-1000:];assert len(p.stdout)==len(data)//(40*8+len(ram))*record_size;return p.stdout
 for start in range(0,len(cases),32):
  data=b''.join(q+ram for q in cases[start:start+32]);expected=None
  for target,command in [('original',[str(work/'original')]),*commands]:
   got=run(command,data)
   if expected is None:expected=got
   else:assert got==expected,(target,start)
   hashes[target].update(got)
  if start%1024==0:print('PASS complete original panning',min(start+32,len(cases)),'/',len(cases),flush=True)
 q=base.copy();q[5]=0x12345678abcdef00;q[32]=0x80;data=struct.pack('<40Q',*q,5,0xffffffffffffffff)+ram;expected=run([str(work/'original')],data)
 branch='v->panning=v->pel_panning;';assert header.count(branch)==1
 mutants=[('omitted-latch','v->panning=v->panning;', [5]),('retained-high-draw','v->panning=(v->panning&0xffffffff00000000ULL)|v->pel_panning;', [5]),('reversed-ownership','v->pel_panning=(uint8_t)v->panning;', [5,32])];negative=[]
 for name,replacement,wanted in mutants:
  for target,command in commands:assert run(command,data)==expected
  for target,command in target_build(work/name,header.replace(branch,replacement)):
   got=run(command,data);a=struct.unpack_from('<38Q',expected);b=struct.unpack_from('<38Q',got);differences=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
   assert differences==wanted and got[38*8:]==expected[38*8:],(name,target,differences)
   negative.append(dict(fault=name,target=target,unmodified_complete_positive_equal=True,differing_full_drawing_words=differences,full256KiB_RAM_requests_equal=True))
  print('PASS original panning distinguishes',name,'on both targets',flush=True)
 def digest(p):
  with Path(p).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
 paths=[Path(__file__),repo/'tools/oracle/capture_vga_draw_programs.py',path,repo/'re_out/fist_vga_draw.h',*tree.rglob('*.h'),work/'original.cpp',work/'portable.c']
 proof=dict(scope='Verbatim original VGA_PanningLatch and common full38-word drawing/256KiB RAM/request transport. All256 valid BYTE pel_panning values,dirty64-bit draw values and ignored callback arguments match both32-bit targets. Three causes rejected after exact positives on both. Actual callers/runtime/full output acceptance remain open.',cases=len(cases),record_size=record_size,outputs_sha256={n:h.hexdigest() for n,h in hashes.items()},causal_negatives=negative,inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
 (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS all6144 original panning programs/6 causal results',flush=True)
 return proof

if __name__=='__main__':
 parser=argparse.ArgumentParser(description='Compare verbatim original VGA_PanningLatch and complete drawing/RAM/request outputs on both targets.')
 parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
 args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
