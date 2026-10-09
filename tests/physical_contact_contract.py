"""Independent original physical ground contacts and complete continuations.

Geometry/prediction/RNG reuse their existing owners. Admitted sound requires
explicit coupled kernel verification; its returned EAX is overwritten by later
contact arithmetic. Predictive numerical scratch has a separately declared
comparison scope. This reference model does not implement runtime C.
"""
import struct

from original_unit_oracle import DGROUP
from roster_promotion_contract import store, word
from test_geometry import signed
from test_proximity import proximity
from ground_maneuver_contract import random_draw


def predict(before, actor, entry, registers, near_obstacle=None, sound=False):
    if entry not in (0xa631, 0x1a0a4, 0x1a0ab):
        raise ValueError('Unknown original physical contact entry')
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
              'prediction_samples': 0, 'prediction_hits': 0,
              'tree_calls': 0, 'tree_released': 0, 'tree_wrapped_retained': 0,
              'audio_requests': 0, 'audio_packet': None, 'audio_attenuation': None}
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
            return tree_contact(data, state, effect, actor, candidate, common_sp, sound=sound)
        if word(data, DGROUP + 0x9fdf) == actor and not sound:
            raise ValueError('Admitted sound requires the coupled real kernel comparison')
        # Genuine c047 either returns on a source mismatch or produces a real
        # request for the separate kernel transport comparison.
        struct.pack_into('<HH', data, DGROUP + common_sp - 4, 0xaaa2, 0xf69)
        if word(data, DGROUP + 0x9fdf) == actor:
            sound_metadata(data, state, effect, actor, 45, common_sp)
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


def tree_contact(data, state, effect, actor, candidate, common_sp, *, sound=False):
    """Full unmodified type21 contact, damage, release and display predictions."""
    if word(data, DGROUP + 0x9fdf) == actor and not sound:
        raise ValueError('Admitted tree sound requires the coupled real kernel comparison')
    a, t = DGROUP + actor, DGROUP + candidate
    speed, throttle = struct.unpack_from('<2h', data, a + 0x55)
    struct.pack_into('<2h', data, a + 0x55, speed // 2, throttle // 2)
    # c047 preserves the saved candidate; admitted transport is checked separately.
    struct.pack_into('<3H', data, DGROUP + common_sp - 6, 0xaad9, 0xf69, candidate)
    if word(data, DGROUP + 0x9fdf) == actor:
        sound_metadata(data, state, effect, actor, 48, common_sp - 2)
    store(data, DGROUP + 0x9bd9, 0)
    store(data, DGROUP + 0x9bdb, 0)
    source = word(data, DGROUP + 0xe3b2)
    factor = word(data, DGROUP + 0xe3ae + 2 * bool(data[DGROUP + source + 0x16] & 8))
    group = bytearray(data[DGROUP:DGROUP + 65536])
    draw = random_draw(group)
    data[DGROUP:DGROUP + 65536] = group
    base, spread = data[DGROUP + 0x9b5d:DGROUP + 0x9b5f]
    value = ((base + 1 + ((draw & 255) * spread >> 8)) * factor & 65535) >> 8
    total = data[t + 0x1a] + value
    data[t + 0x1a] = total & 255
    released = data[t + 0x1a] >= 20
    effect['branch'] = 'first_tree_hit'
    effect['tree_calls'], effect['sound_calls'] = 1, 1
    effect['tree_released'] = int(released)
    effect['tree_wrapped_retained'] = int(total >= 256 and not released)
    # c31e/9c76/b274/0291, followed by c31e's real near60f4.
    struct.pack_into('<7H', data, DGROUP + common_sp - 14,
                     0x9b5d, 0xb277, 0x9c7e, 0xc330, 0xc334, 0xaae3, 0xf69)
    state[0], state[1], state[3], state[4] = value, 0x9b5d, factor, source
    if released:
        # bcc4 resolves the first physical registry binding and preserves SI.
        index = next(i for i in range(182) if word(data, DGROUP + 0xdfbc + i * 4) == candidate)
        cell = DGROUP + 0xdfbc + index * 4
        generation = word(data, cell + 2)
        physical = word(data, t + 2)
        data[t + 0x16] |= 1
        store(data, cell, 0)
        store(data, cell + 2, generation - 1)
        data[DGROUP + 0xe2f7 + physical] = 0
        store(data, DGROUP + 0xe294, word(data, DGROUP + 0xe294) - 1)
        store(data, DGROUP + 0x930a, word(data, DGROUP + 0x930a) - 1)
        # Final farbc5f/its savedDI overwrite earlier lookup/RNG stack words.
        struct.pack_into('<3H', data, DGROUP + common_sp - 14, candidate, 0x9c92, 0)
        state[0], state[1], state[2], state[3] = index, physical, 182 - index, generation
    data[DGROUP + 0x7b1e] = 0
    data[DGROUP + 0x87d5] = 3
    data[DGROUP + 0x87bd] = 3
    return data, state, effect


def sound_metadata(data, state, effect, actor, selector, call_sp):
    """DOS c047/e2c2 effects before the real op64 kernel transport boundary.

    The returned EAX is overwritten by subsequent collision arithmetic; no
    sample address is supplied or assumed by the contact prediction. The
    separate kernel oracle predicts every request byte/full general register.
    """
    store(data, DGROUP + 0x9fdd, selector)
    packet = word(data, DGROUP + 0x9fe1 + selector)
    attenuation = data[DGROUP + 0x9fe3 + selector] >> 1
    struct.pack_into('<H', data, DGROUP + call_sp - 6, actor)
    state[0], state[1], state[3] = packet, selector, attenuation
    if packet & 255 == 255 or packet & 255 >= 16:
        return
    state[1], state[2] = 100, 0
    store(data, DGROUP + 0xea10, 100)
    mailbox = word(data, DGROUP + 0xea2e) * 16 + word(data, DGROUP + 0xea2c)
    struct.pack_into('<I', data, mailbox + 0x3f2, selector)
    struct.pack_into('<H', data, DGROUP + call_sp - 8, 0xc06b)
    effect['audio_requests'] += 1
    effect['audio_packet'], effect['audio_attenuation'] = packet, attenuation
