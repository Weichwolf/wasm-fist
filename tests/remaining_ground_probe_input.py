"""Shared remaining-ground binary input and typed observation vocabulary."""
import struct

from roster_promotion_contract import store

NONE = 65535
BRANCHES = ('not_called', 'air_cooldown', 'air_unavailable', 'air_confirmed',
            'artillery_cooldown', 'artillery_not_in_place', 'artillery_empty',
            'artillery_busy', 'artillery_queue_full', 'artillery_confirmed')
SMOKE = ('not_called', 'empty', 'capacity', 'created')
DISPLAY = {0x3031: 4, 0x2f2a: 5, 0x2edd: 6, 0x2f40: 7, 0x2ef7: 8}


def encode(f):
    h = bytearray(80)
    h[:4] = bytes((f['operation'], f['selected'], f['source'], f['coarse']))
    store(h, 4, f['target_type'])
    h[6:8] = bytes((f['variant'], f['context']))
    for offset, value in ((8, f['clock']), (10, f['tick']), (12, f['gate']),
                          (14, f['voice_prior']), (16, f['prior']),
                          (18, f['clock'] - f['air_age']), (20, f['clock'] - f['artillery_age']),
                          (22, f['air_stock']), (24, f['air_delay']),
                          (26, f['artillery_delay'])):
        store(h, offset, value)
    h[28:32] = bytes((f['count'], f['air_used'], f['artillery_used'], f['full']))
    struct.pack_into('<4H4H', h, 32, *f['guns'], *f['seeds'])
    h[48], h[49], h[51] = f['cursor'], f['retained'], f['invalid']
    h[50] = f.get('post_requester', 0)
    struct.pack_into('<2i', h, 52, *f['target_pose'])
    store(h, 60, f['display_ticks'])
    h[62], h[63] = f['display_kind'], f['advisory']
    for offset, key in ((64, 'advisory_until'), (66, 'message'), (68, 'selector')):
        store(h, offset, f[key])
    h[70:75] = bytes((f['configured'], f['steps'], f['reverse'], f['height'], f['gun_index']))
    return bytes(h) + f['raw']


def line(name, values):
    return name + ' ' + ' '.join(map(str, values)) + '\n'
