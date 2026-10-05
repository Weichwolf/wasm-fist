# Read-only source state, reached VGA callbacks and complete linear-line outputs.
pit_state=state
def state():
 q=pit_state()
 q['vga_panning']=dict(draw=int(gdb.parse_and_eval('vga.draw.panning')),config=int(gdb.parse_and_eval('vga.config.pel_panning')))
 lag=gdb.parse_and_eval("'pic.cpp'::srv_lag")
 q['PIC_event_service']=dict(active=int(gdb.parse_and_eval("'pic.cpp'::InEventService")),lag_bits=bytes(gdb.selected_inferior().read_memory(int(lag.address),4)).hex())
 return q
class PanningReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):save('after-panning');return False
class Panning(gdb.Breakpoint):
 def stop(self):
  if write_count<3 or after_write:return False
  save('before-panning');PanningReturned();self.enabled=False;return False
Panning('VGA_PanningLatch')

DRAW_FIELDS=('resizing','width','height','blocks','address','panning','bytes_skip','linear_mask','address_add','line_length','address_line_total','address_line','lines_total','vblank_skip','lines_done','lines_scaled','split_line','parts_total','parts_lines','parts_left','byte_panning_shift','bpp','double_scan','doublewidth','doubleheight','blinking','mode','vret_triggered')
def draw_state():
 q={k:int(gdb.parse_and_eval('vga.draw.'+k)) for k in DRAW_FIELDS}
 q['parts_delay_bits']=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('vga.draw.delay.parts').address),8)).hex()
 q['handler']=str(gdb.parse_and_eval("'vga_draw.cpp'::VGA_DrawLine"))
 base=int(gdb.parse_and_eval('vga.draw.linear_base'))
 q['linear_base_offset']=base-int(gdb.parse_and_eval('vga.mem.linear'))
 q['fastmem_base_offset']=base-int(gdb.parse_and_eval('vga.fastmem'))
 q['config']={k:int(gdb.parse_and_eval('vga.config.'+k)) for k in ('real_start','display_start','bytes_skip','pel_panning')}
 q.update(machine=int(gdb.parse_and_eval('machine')),vga_mode=int(gdb.parse_and_eval('vga.mode')),attr_mode_control=int(gdb.parse_and_eval('vga.attr.mode_control')),vertical_retrace_end=int(gdb.parse_and_eval('vga.crtc.vertical_retrace_end')),vmemwrap=int(gdb.parse_and_eval('vga.vmemwrap')))
 q['renderer']={k:int(gdb.parse_and_eval('render.'+k)) for k in ('updating','src.width','src.height','src.bpp','scale.cachePitch','scale.inLine','scale.outLine')}
 return q
original_save=save
def save(kind,extra=None):
 return original_save(kind,dict(extra or {},vga_draw=draw_state()))
callback_active=False
class DrawLineReturned(gdb.FinishBreakpoint):
 def __init__(self,address,line):
  self.address=address;self.line=line
  super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  width=int(gdb.parse_and_eval('vga.draw.line_length'))
  data=bytes(gdb.selected_inferior().read_memory(int(self.return_value),width))
  q=dict(address=self.address,line=self.line,data_hex=data.hex(),time=state()['PIC_Ticks'])
  with (root/'draw-lines.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  return False
class DrawLine(gdb.Breakpoint):
 def stop(self):
  if callback_active:DrawLineReturned(int(gdb.parse_and_eval('vidstart')),int(gdb.parse_and_eval('vga.draw.address_line')))
  return False
DrawLine('VGA_Draw_Linear_Line')
class VgaReturned(gdb.FinishBreakpoint):
 def __init__(self,name):
  self.name=name;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  global callback_active
  save('after-'+self.name)
  if self.name=='draw-part':callback_active=False
  return False
class VgaCallback(gdb.Breakpoint):
 def __init__(self,function,name):
  self.name=name
  # VGA_DrawPart starts with push RBX; its original64-bit line argument is
  # RDI at the actual SysV function entry. DWARF lines denotes the optimized
  # decremented RBX loop counter and is not a reliable entry argument.
  super().__init__(function)
 def stop(self):
  global callback_active
  if not started or write_count:return False
  args={}
  if self.name=='draw-part':
   assert int(gdb.parse_and_eval('$pc'))==int(gdb.parse_and_eval('(void *)VGA_DrawPart'))
   assert gdb.lookup_type('Bitu').sizeof==8
   args['lines']=int(gdb.parse_and_eval('$rdi'));callback_active=True
  save('before-'+self.name,args);VgaReturned(self.name);return False
for function,name in (('VGA_DrawPart','draw-part'),('VGA_VertInterrupt','vert-interrupt'),('VGA_DisplayStartLatch','display-start')):
 VgaCallback(function,name)
