import hashlib
def renderer_state():
 fields=('updating','src.width','src.height','src.bpp','src.start','scale.cachePitch','scale.inLine','scale.outLine')
 q={k:int(gdb.parse_and_eval('render.'+k)) for k in fields}
 q['handler']=str(gdb.parse_and_eval('RENDER_DrawLine'))
 base=int(gdb.parse_and_eval('&scalerSourceCache'))
 q['cache_cursor']=int(gdb.parse_and_eval('render.scale.cacheRead'))-base
 for key in ('frameskip.index','scale.outPitch','active','fullFrame'):
  q[key]=int(gdb.parse_and_eval('render.'+key))
 q['scale.hasOutWrite']=int(int(gdb.parse_and_eval('render.scale.outWrite'))!=0)
 q['CaptureState']=int(gdb.parse_and_eval('CaptureState'))
 q['Scaler_ChangedLineIndex']=int(gdb.parse_and_eval('Scaler_ChangedLineIndex'))
 return q
def state():
 q=startup_pit_state();q['vga_draw']=draw_state();q['renderer_device']=renderer_state()
 lag=gdb.parse_and_eval("'pic.cpp'::srv_lag")
 q['PIC_event_service']=dict(active=int(gdb.parse_and_eval("'pic.cpp'::InEventService")),lag_bits=bytes(gdb.selected_inferior().read_memory(int(lag.address),4)).hex())
 q['paging.cr2']=int(gdb.parse_and_eval('paging.cr2'))
 return q
def renderer_snapshot(label,q):
 r=q['renderer_device'];size=r['scale.cachePitch']*r['src.height']
 assert 0<size<=int(gdb.parse_and_eval('sizeof(scalerSourceCache)'))
 data=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('&scalerSourceCache')),size))
 name=label+'.renderer-cache';(root/name).write_bytes(data)
 palette=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('&render.pal.rgb')),1024))
 pname=label+'.renderer-palette';(root/pname).write_bytes(palette)
 parts=dict(renderer_cache=dict(file=name,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()),renderer_palette=dict(file=pname,bytes=len(palette),sha256=hashlib.sha256(palette).hexdigest()))
 for key,expr,size in (('renderer_aspect','&Scaler_Aspect',1024),('renderer_changed','&Scaler_ChangedLines',2048),('renderer_skip','&render.frameskip.hadSkip',16)):
  data=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval(expr)),size))
  name=label+'.'+key;(root/name).write_bytes(data)
  parts[key]=dict(file=name,bytes=size,sha256=hashlib.sha256(data).hexdigest())
 return parts
startup_save=save
def save(kind,extra=None):
 extra=dict(extra or {})
 if kind=='engine-e339':extra.update(renderer_snapshot(kind,state()))
 return startup_save(kind,extra)
vga_seen=False;vga_active=False;vga_finished=False;vga_round=0
def vga_record(kind,arguments=None):
 q=state();q.update(kind=kind,pic=pic_state(),calendar=calendar())
 if arguments is not None:q['arguments']=arguments
 q.update(renderer_snapshot(kind,q))
 used=int(gdb.parse_and_eval('pf_queue.used'))
 q['PF_QUEUE']=dict(used=used,entries=[{k:int(gdb.parse_and_eval('pf_queue.entries[%d].%s'%(i,k))) for k in ('cs','eip','page_addr','mpl')} for i in range(used)])
 q['decoder_symbol']=str(gdb.parse_and_eval('cpudecoder'))
 name=kind+'.memory';q['memory_file']=name;q['memory_context']=memory_context(name);(root/name).write_bytes(memory())
 with (root/'vga-events.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
 return q
class StartupLineReturned(gdb.FinishBreakpoint):
 def __init__(self,address,line):
  self.address=address;self.line=line;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  width=int(gdb.parse_and_eval('vga.draw.line_length'))
  data=bytes(gdb.selected_inferior().read_memory(int(self.return_value),width))
  q=dict(round=vga_round,address=self.address,line=self.line,data_hex=data.hex())
  with (root/'vga-lines.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  return False
class StartupLine(gdb.Breakpoint):
 def stop(self):
  if vga_active:StartupLineReturned(int(gdb.parse_and_eval('$rdi')),int(gdb.parse_and_eval('$rsi')))
  return False
class StartupDrawReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  global vga_active
  vga_record('after-startup-draw-'+str(vga_round));vga_active=False;return False
class StartupDraw(gdb.Breakpoint):
 def stop(self):
  global vga_seen,vga_active,vga_round
  if not started or vga_finished:return False
  assert int(gdb.parse_and_eval('$pc'))==int(gdb.parse_and_eval('(void *)VGA_DrawPart'))
  lines=int(gdb.parse_and_eval('$rdi'));vga_seen=True;vga_active=True;vga_round+=1
  vga_record('before-startup-draw-'+str(vga_round),dict(lines=lines))
  if vga_round==1:
   pointer=int(gdb.parse_and_eval("'vga_draw.cpp'::VGA_DrawLine"));StartupLine('*%d'%pointer,internal=True)
  StartupDrawReturned();return False
StartupDraw('*VGA_DrawPart',internal=True)
class StartupRenderEnd(gdb.Breakpoint):
 def stop(self):
  global vga_finished
  if vga_active:
   vga_record('before-startup-present',dict(abort=int(gdb.parse_and_eval('$rdi'))));vga_finished=True
  return False
StartupRenderEnd('*RENDER_EndUpdate',internal=True)
