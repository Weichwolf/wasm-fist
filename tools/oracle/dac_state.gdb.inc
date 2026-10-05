# Complete original hardware/renderer palette; shared by reaching and startup probes.
def raw_field(name):
 value=gdb.parse_and_eval(name)
 return bytes(gdb.selected_inferior().read_memory(int(value.address),value.type.sizeof)).hex()
def dac_state():
 return dict(fields={k:int(gdb.parse_and_eval('vga.dac.'+k)) for k in ('bits','pel_mask','pel_index','state','write_index','read_index','first_changed')},
  combine_hex=raw_field('vga.dac.combine'),rgb_hex=raw_field('vga.dac.rgb'),xlat16_hex=raw_field('vga.dac.xlat16'),
  mode=int(gdb.parse_and_eval('vga.mode')),render_rgb_hex=raw_field('render.pal.rgb'),render_lut_hex=raw_field('render.pal.lut'),
  render_modified_hex=raw_field('render.pal.modified'),render_changed=int(gdb.parse_and_eval('render.pal.changed')),
  render_first=int(gdb.parse_and_eval('render.pal.first')),render_last=int(gdb.parse_and_eval('render.pal.last')))
