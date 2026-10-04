"""Complete source-backed memory packets and cross-target cache/VGA comparisons."""
import struct
import sys
from test_port_io import ROOT

sys.path.insert(0,str(ROOT/'tools/oracle'))
from memory_context import HANDLERS, packet
from capture_pit_irq_frames import SYSTEM_FIELDS


def system_words(q):
    return [q[k]&0xffffffff for k in SYSTEM_FIELDS]


def memory_packet(q,folder):
    c = q['memory_context']
    return packet(c)+b''.join((folder/c['vga'][key]['file']).read_bytes() for key in ('linear','fastmem'))


def expected_memory_context(q,folder):
    state = q['memory_context']; p,c,v = state['provider'],state['cache'],state['vga']
    data = [state['architecture'],p['pages'],int(p['a20_enabled']),p['a20_controlport'],
            v['vmemsize'],v['vmemwrap'],v['svga.bank_read_full'],v['svga.bank_write_full'],
            v['pages']['base'],v['pages']['mask'],*p['firstmb']]
    for slot in p['slots']:
        h = p['handlers'][str(slot)];data += [HANDLERS.index(h['type']),h['flags']]
    data += [len(c['linked_pages']),*c['linked_pages']]
    for linear in c['linked_pages']:
        e = c['entries'][str(linear)]
        data += [linear,e['phys_page'],HANDLERS.index(e['handler_types']['readhandler']),
                 int(bool(e['read'])),int(bool(e['write']))]
    return struct.pack('<%dI'%len(data),*data)+b''.join((folder/v[key]['file']).read_bytes() for key in ('linear','fastmem'))
