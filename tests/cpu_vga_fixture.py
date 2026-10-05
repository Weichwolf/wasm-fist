"""Complete drawing/service transport extends the shared PIT/core fixture."""
import struct
from cpu_pit_fixture import initial_packet as pit_initial_packet,extra_parts as pit_extra_parts
DRAW_FIELDS=('resizing','width','height','blocks','address','panning','bytes_skip','linear_mask','address_add','line_length','address_line_total','address_line','lines_total','vblank_skip','lines_done','lines_scaled','split_line','parts_total','parts_lines','parts_left','byte_panning_shift','bpp','double_scan','doublewidth','doubleheight','blinking','mode','vret_triggered')
def drawing_packet(q):
 d=q['vga_draw'];c=d['config']
 assert 'VGA_Draw_Linear_Line' in d['handler']
 if 0<=d['fastmem_base_offset']<q['memory_context']['vga']['fastmem']['bytes']:base=(1,d['fastmem_base_offset'])
 else:
  assert 0<=d['linear_base_offset']<q['memory_context']['vga']['linear']['bytes'];base=(0,d['linear_base_offset'])
 return struct.pack('<28Q',*[d[k] for k in DRAW_FIELDS])+bytes.fromhex(d['parts_delay_bits'])+struct.pack('<9Q',*[c[k] for k in ('real_start','display_start','bytes_skip','pel_panning')],*[d[k] for k in ('machine','vga_mode','attr_mode_control','vertical_retrace_end','vmemwrap')])+struct.pack('<2I',*base)
def service_packet(q):
 return struct.pack('<I',q['PIC_event_service']['active'])+bytes.fromhex(q['PIC_event_service']['lag_bits'])
def initial_packet(q,folder):
 return pit_initial_packet(q,folder)+drawing_packet(q)+service_packet(q)
def extra_parts(q,folder):
 return pit_extra_parts(q,folder)+[('drawing',drawing_packet(q)),('service',service_packet(q))]

def draw_requests(case):
 from device_cpu_fixture import clock
 rows=case['events'][:21]
 return b''.join(struct.pack('<4Q',1,q['address'],q['line'],len(bytes.fromhex(q['data_hex'])))+bytes.fromhex(q['data_hex']) for q in case['draw_lines'])+struct.pack('<2IQ',2,0,clock(next(q for q in rows if q['kind']=='after-draw-part')))
