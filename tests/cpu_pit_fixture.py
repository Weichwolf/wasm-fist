"""Complete PIT0 reaching boundaries, sharing the existing CPU/RAM/PIC fixture."""
import struct
from cpu_core_exit_fixture import core_packet

FIELDS=('read_latch','write_latch','mode','latch_mode','read_state','write_state',
        'bcd','go_read_latch','new_mode','counterstatus_set','counting','update_count')
STATUS=('framestart','vrstart','vrend','hblkstart','hblkend','htotal','vdend','vtotal')

def pit_packet(q):
    p=q['pit'];data=b''
    for c in p['counters']:
        data+=struct.pack('<I',c['cntr'])+bytes.fromhex(c['delay_bits'])+bytes.fromhex(c['start_bits'])
        data+=struct.pack('<12I',*[c[key] for key in FIELDS])
    return data+struct.pack('<3I',*[p[key] for key in ('gate2','status','status_locked')])

def status_packet(q):
    p=q['vga_status_device']
    return b''.join(bytes.fromhex(p[key]) for key in STATUS)+struct.pack('<2I',p['attrindex'],p['pcjr_flipflop'])

def identity(e):
    for i,name in enumerate(('VGA_PanningLatch','VGA_VerticalTimer','PIT0_Event','VGA_DrawPart','VGA_VertInterrupt','VGA_DisplayStartLatch')):
        if name in e['handler']:return i
    raise AssertionError(e)

def calendar_packet(q):
    return struct.pack('<I',len(q['calendar']))+b''.join(struct.pack('<3I',e['index_bits'],e['value'],identity(e)) for e in q['calendar'])

def initial_packet(q,folder):
    clock=struct.pack('<3I',q['PIC_Ticks'],q['CPU_Cycles'],q['CPU_CycleLeft'])+calendar_packet(q)
    return core_packet(q,folder,clock)+pit_packet(q)+status_packet(q)+struct.pack('<Q',q['CPU_IODelayRemoved'])

def extra_parts(q,folder):
    return [('pit',pit_packet(q)),('status',status_packet(q)),('io',struct.pack('<Q',q['CPU_IODelayRemoved'])),('calendar',calendar_packet(q))]
