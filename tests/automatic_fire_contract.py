"""Independent complete original af97/afa2 and genuine parent fire model.

Reference evidence only: all actor, allocation, display and device-request writes
are predicted before executing unchanged original instructions.
"""
import struct

from ground_maneuver_contract import random_draw
from original_object_pool_oracle import COUNT, REGISTRY, SHORT_BASE, SHORT_SLOTS
from roster_promotion_contract import store, word

FIRE_THRESHOLDS = (80, 140, 40, 0)
MISSILE_MESSAGES = {'empty': 0x2fc9, 'target': 0x2fb5, 'capacity': 0x2fd7}
SHOT_PACKET = bytes((7, 0, 0))


def message(data, actor, reason):
    if word(data, 0x6d34) == actor:
        store(data, 0x969e, 120)
        store(data, 0x96a0, MISSILE_MESSAGES[reason])


def allocate_missile(data):
    count = word(data, 0xe294)
    if count == SHORT_SLOTS:
        return None
    if count > SHORT_SLOTS:
        raise ValueError('Malformed original short-pool count')
    used = data[0xe2f7:0xe2f7 + SHORT_SLOTS]
    if 0 not in used:
        raise ValueError('Original count/occupancy disagree')
    bindings = [(word(data, REGISTRY + 4 * index), word(data, REGISTRY + 4 * index + 2))
                for index in range(COUNT)]
    if (0, 0) not in bindings:
        raise ValueError('Original exhausted-registry scan escapes its bounds')
    index = bindings.index((0, 0))
    slot = used.index(0)
    pointer = SHORT_BASE + 55 * slot
    store(data, 0xe294, count + 1)
    store(data, 0xe298, max(word(data, 0xe298), count + 1))
    data[0xe2f7 + slot] = 1
    data[pointer:pointer + 55] = struct.pack('<HH', 15, slot) + bytes(51)
    store(data, REGISTRY + 4 * index, pointer)
    store(data, REGISTRY + 4 * index + 2, 1)
    return pointer, slot, index


def missile(data, actor, kind, effect):
    counts = data[actor + 0xb5:actor + 0xb7]
    states = data[actor + 0xb7:actor + 0xb9]
    first_ready = states[0] == 4 and (kind == 3 or counts[0] != 0)
    ready = 0 if first_ready else 1 if states[1] == 4 else None
    dirty = (0xe0, 0xe2) if kind == 1 else (0xc6, 0xc8)
    if ready is None:
        first_empty = states[0] == 0 and (kind == 1 or counts[0] != 0)
        loading = 0 if first_empty else 1 if states[1] == 0 else None
        if loading is None:
            effect['branch'] = 'racks_busy'
            return
        data[actor + 0xb7 + loading] = 2
        effect['branch'] = 'rack_loading'
        effect['rack'] = loading
    else:
        effect['rack'] = ready
        target = word(data, actor + 0x97)
        if counts[ready] == 0:
            effect['branch'] = 'missile_empty'
            message(data, actor, 'empty')
        elif target == 0 or not data[target + 22] & 16:
            effect['branch'] = 'missile_target_rejected'
            message(data, actor, 'target')
        else:
            # The reserve is consumed before the real allocation attempt.
            data[actor + 0xb5 + ready] -= 1
            allocation = allocate_missile(data)
            if allocation is None:
                effect['branch'] = 'missile_capacity'
                message(data, actor, 'capacity')
            else:
                pointer, slot, index = allocation
                data[pointer + 23] |= 1
                store(data, pointer + 0x2b, actor)
                store(data, pointer + 0x28, 364)
                store(data, pointer + 0x1c, 160)
                data[pointer + 4:pointer + 12] = data[actor + 4:actor + 12]
                struct.pack_into('<I', data, pointer + 12,
                                 (struct.unpack_from('<I', data, actor + 12)[0] + 3072) & 0xffffffff)
                store(data, pointer + 0x1a, target)
                store(data, pointer + 0x10, word(data, actor + 0x10))
                effect.update(branch='missile_launched', allocation=(pointer, slot, index))
                if word(data, 0x9fdf) == actor:
                    store(data, 0x9fdd, 24)
                    # Complete c047 -> e2c2 produces this actual op-64 request.
                    store(data, 0xea10, 100)
                    effect['request'] = {'ax': 7, 'dl': 0, 'ecx': 0, 'ebx': 24}
    for offset in dirty:
        data[actor + offset] = 3


def automatic_fire(before, actor, *, entry=0xaf97):
    data = bytearray(before)
    kind = word(data, actor)
    effect = {'branch': 'class_rejected', 'rack': None, 'allocation': None, 'request': None}
    if entry == 0xaf97 and kind not in (1, 3):
        return data, effect
    if entry not in (0xaf97, 0xafa2, 0x8711, 0x96c0):
        raise ValueError('Unknown complete fire boundary')
    if entry in (0x8711, 0x96c0):
        missile(data, actor, 1 if entry == 0x8711 else 3, effect)
        return data, effect
    descriptor = word(data, 0x9796)
    behavior = word(data, descriptor)
    effect['branch'] = 'behavior_rejected'
    if behavior == 3:
        return data, effect
    target = word(data, actor + 0x97)
    effect['branch'] = 'target_absent'
    if target == 0:
        return data, effect
    phase = word(data, 0x978c) & 255
    effect['branch'] = 'phase_rejected'
    if not data[actor + 22] & 8 and phase & 3:
        return data, effect
    effect['branch'] = 'probability_rejected'
    if word(data, actor + 0x99) > 200 and not word(data, actor + 0x40) & 128:
        if phase >= data[(0x9952 + behavior) & 65535]:
            return data, effect
    if data[target + 22] & 16 and kind in (1, 3):
        missile(data, actor, kind, effect)
        return data, effect
    difference = (word(data, actor + 0x8b) - word(data, actor + 0x89)) & 65535
    effect['branch'] = 'alignment_rejected'
    if 182 <= difference < 65354:
        return data, effect
    data[actor + 0x92] = 48
    store(data, actor + 0x40, word(data, actor + 0x40) & 0xff7f)
    effect['branch'] = 'fire_requested'
    return data, effect


def parent(before, actor):
    data = bytearray(before)
    if word(data, 0x7ae0) == actor:
        raise ValueError('Selected diagnostic tail needs its complete separate owner')
    store(data, 0x9a08, actor)
    store(data, 0x978c, random_draw(data))
    effect = {'branch': 'global_rejected', 'rack': None, 'allocation': None, 'request': None,
              'parent_index': None, 'parent_bank': None}
    if data[0x978a]:
        return data, effect
    data[actor + 0x42] = (data[actor + 0x42] + 1) & 255
    index = data[actor + 0x42] & 15
    if index not in (5, 10, 14):
        raise ValueError('This whole parent requires its genuine fire entry')
    platoon = data[actor + 27]
    store(data, 0x9796, word(data, 0x85a0 + platoon * 2))
    store(data, 0x9798, word(data, 0x7d2a + platoon * 2))
    automatic = bool(word(data, actor + 0x40) & 1)
    if automatic:
        data, effect = automatic_fire(data, actor, entry=0xafa2 if index == 10 else 0xaf97)
    else:
        effect['branch'] = 'genuine_ret'
    effect.update(parent_index=index, parent_bank='automatic' if automatic else 'controlled')
    return data, effect
