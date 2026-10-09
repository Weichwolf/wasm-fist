"""Independent direct contacts and logical near-obstacle composition.

Tree damage and admitted sound are separate, unproved continuations and fail
explicitly. Predictive calls retain the previously proved maneuver owner; their
bounded numerical stack/register scratch has a separate comparison scope.
"""
import struct

from original_unit_oracle import DGROUP
from roster_promotion_contract import store, word
from test_geometry import signed
from test_proximity import proximity


def predict(before, actor, entry, registers, near_obstacle=None):
    data = bytearray(before)
    state = list(registers)
    far = entry != 0xa631
    common_sp = 0x9000 if far else 0x8ffc
    state[9] = 0x9004 if far else 0x9002
    selected = word(data, DGROUP + 0x6d34) == actor
    admitted = not selected if entry == 0xa631 else selected if entry == 0x1a0a4 else True
    effect = {'branch': 'ignored', 'geometry_calls': 0, 'sound_calls': 0,
              'registry_visits': 0, 'hit_index': None, 'high_word_skips': 0,
              'near_obstacle_calls': 0, 'prediction_calls': 0,
              'prediction_samples': 0, 'prediction_hits': 0}
    if not admitted:
        return data, state, effect
    raw_actor = DGROUP + actor
    store(data, raw_actor + 0x40, word(data, raw_actor + 0x40) & 65527)
    if not far:
        struct.pack_into('<HH', data, DGROUP + 0x8ffc, 0xa63c, 0)
    if data[raw_actor + 0x93]:
        data[raw_actor + 0x93] -= 1
        effect['branch'] = 'cooldown'
        return data, state, effect
    # AA36's initial pair is popped before the first registry visit. Later
    # pairs are pushed only for non-null pointers, including filtered entries.
    struct.pack_into('<HH', data, DGROUP + common_sp - 4, 182, 0xdfbc)
    for index in range(182):
        candidate = word(data, DGROUP + 0xdfbc + index * 4)
        next_cell, remaining = 0xdfbc + (index + 1) * 4, 181 - index
        state[1], state[2], state[4] = next_cell, remaining, candidate
        effect['registry_visits'] += 1
        if not candidate:
            continue
        struct.pack_into('<HH', data, DGROUP + common_sp - 4, remaining, next_cell)
        raw_candidate = DGROUP + candidate
        flags = data[raw_candidate + 0x16]
        if not flags & 64 or flags & 16 or candidate == actor:
            continue
        source = struct.unpack_from('<2i', data, raw_candidate + 4)
        target = struct.unpack_from('<2i', data, raw_actor + 4)
        separation = proximity((source, target, 0))
        struct.pack_into('<I', data, DGROUP + 0x458, separation)
        # Independent complete call-chain stack: near 08e8, far a17e,
        # saved actor/candidate and the pending physical scan pair.
        struct.pack_into('<7H', data, DGROUP + common_sp - 14,
                         0xa181, 0xaa67, 0xf69, actor, candidate, remaining, next_cell)
        state[0], state[3] = separation & 65535, separation >> 16
        effect['geometry_calls'] += 1
        if state[3]:
            effect['high_word_skips'] += 1
            continue
        margin = ((word(data, raw_actor + 0x14) + word(data, raw_candidate + 0x14) + 256) & 65535) >> 1
        state[0] = (separation - margin) & 65535
        if separation >= margin:
            if state[0] <= 7680:
                if near_obstacle is None:
                    raise ValueError('Predictive b059 continuation remains unproved in this subset')
                # Reuse the independently proved logical b059 owner. This
                # composition predicts every semantic byte; scratch registers
                # and its bounded nested stack are a separately declared scope.
                group, evidence = near_obstacle(bytes(data[DGROUP:DGROUP + 65536]), actor, candidate)
                data[DGROUP:DGROUP + 65536] = group
                effect['near_obstacle_calls'] += 1
                for key in ('prediction_calls', 'prediction_samples', 'prediction_hits'):
                    effect[key] += evidence[key]
            continue
        effect['hit_index'] = index
        if data[raw_actor + 0x62] & 2:
            data[raw_actor + 0x93] = 4
            effect['branch'] = 'repeated_hit'
            return data, state, effect
        data[raw_actor + 0x62] |= 2
        if word(data, raw_candidate) == 21:
            raise ValueError('Complete tree damage continuation remains unproved in this subset')
        if word(data, DGROUP + 0x9fdf) == actor:
            raise ValueError('Admitted c047 sound-source continuation remains unproved in this subset')
        # Genuine c047 compares DI with the sound owner and returns immediately
        # in this explicit unadmitted-source context. No PM call is substituted.
        struct.pack_into('<HH', data, DGROUP + common_sp - 4, 0xaaa2, 0xf69)
        vx, vy = struct.unpack_from('<2h', data, raw_actor + 0x59)
        x, y = struct.unpack_from('<2i', data, raw_actor + 4)
        struct.pack_into('<2i', data, raw_actor + 4, signed(x - vx * 32), signed(y - vy * 32))
        store(data, raw_actor + 0x55, -word(data, raw_actor + 0x55))
        store(data, raw_actor + 0x57, 0)
        data[raw_actor + 0x93] = 4
        state[0] = (vy * 32) & 65535
        effect['branch'], effect['sound_calls'] = 'first_non_tree_hit', 1
        return data, state, effect
    data[raw_actor + 0x62] &= 253
    effect['branch'] = 'no_hit'
    return data, state, effect
