"""Complete original hardware/renderer palette transport for reaching fixtures."""
import struct
from cpu_vga_fixture import initial_packet as vga_initial, extra_parts as vga_parts
from cpu_core_exit_fixture import observation


def dac_packet(q):
    d = q['dac']
    f = d['fields']
    packet = struct.pack('<6BQ', *[f[k] for k in
        ('bits', 'pel_mask', 'pel_index', 'state', 'write_index', 'read_index')], f['first_changed'])
    packet += b''.join(bytes.fromhex(d[k]) for k in ('combine_hex', 'rgb_hex', 'xlat16_hex'))
    packet += struct.pack('<I', d['mode'])
    packet += b''.join(bytes.fromhex(d[k]) for k in
                      ('render_rgb_hex', 'render_lut_hex', 'render_modified_hex'))
    packet += struct.pack('<BQQ', d['render_changed'], d['render_first'], d['render_last'])
    assert len(packet) == 3635
    return packet


def initial_packet(q, folder):
    return vga_initial(q, folder) + dac_packet(q)


def extra_parts(q, folder):
    return vga_parts(q, folder) + [('dac-state', dac_packet(q))]


def io_observations(rows):
    return [observation(dict(q['cpu'], kind=q['phase'] + '-' + str(q['serial']))) for q in rows]


def io_palettes(rows):
    return b''.join(dac_packet(q) for q in rows)
