"""Independent complete b0be retreat/support state with real audio-return input.

This is reference-only evidence. Heap-dependent support selection is preserved
for observation; no inferred gameplay repair is installed in the rewrite.
"""
import struct

from automatic_fire_contract import allocate_missile
from ground_maneuver_contract import random_draw
from roster_promotion_contract import store, word
from test_vehicle_motion import rotate

SMOKE_STOCK = (0xb5, 0xfa, 0xb4, None)
ARTILLERY_MESSAGE = b'ARTILLERY REQUEST RECEIVED\0'


def display(data, actor, handle):
    if word(data, 0x6d34) == actor:
        store(data, 0x969e, 120)
        store(data, 0x96a0, handle)


def notice(data, actor, code):
    if word(data, 0x6d34) == actor and data[0x6ce6] != 2:
        data[0x9fd6] = code
        store(data, 0x9fd7, word(data, 0x6cde) + 180)


def air(data, actor, evidence):
    # b0be only admits the side-eight actor. Its bf3c voice call consequently
    # rejects before issuing any additional audio request.
    clock = word(data, 0x6cde)
    if (clock - word(data, 0x9462)) & 65535 < 480:
        evidence['support'] = 'air_cooldown'
        return
    if word(data, 0x9452) and word(data, 0x9456) == 0:
        store(data, 0x9462, clock)
        queue = word(data, 0x945a)
        for index in range(16):
            entry = queue + index * 12
            if word(data, entry) == 0:
                store(data, entry, actor)
                store(data, entry + 2, word(data, 0x452))
                display(data, actor, 0x2f2a)
                notice(data, actor, 37)
                evidence.update(support='air_confirmed', queue_index=index)
                return
    notice(data, actor, 39)
    display(data, actor, 0x2edd)
    evidence['support'] = 'air_unavailable'


def artillery(data, actor, evidence):
    clock = word(data, 0x6cde)
    if (clock - word(data, 0x9f19)) & 65535 < 480:
        evidence['support'] = 'artillery_cooldown'
        return
    count = word(data, 0x9ccd)
    chosen = None
    for index in range(count):
        pointer = word(data, 0x9cd7 + index * 2)
        if word(data, pointer + 0x1f):
            chosen = pointer
            break
    if chosen is None:
        notice(data, actor, 42)
        display(data, actor, 0x2ef7)
        evidence['support'] = 'artillery_empty' if count else 'artillery_not_in_place'
        return
    store(data, chosen + 0x1f, word(data, chosen + 0x1f) - 1)
    store(data, 0x9f19, clock)
    display(data, actor, 0x2f40)
    if word(data, 0x9ce5):
        notice(data, actor, 42)
        display(data, actor, 0x2ef7)
        evidence['support'] = 'artillery_busy'
        return
    target = word(data, actor + 0x97)
    if not target:
        raise ValueError('Untargeted standalone artillery is outside the b0be admission contract')
    for index in range(16):
        entry = 0x9dc7 + index * 14
        if word(data, entry) == 0:
            store(data, entry, 2)
            store(data, entry + 2, clock)
            data[entry + 6:entry + 14] = data[target + 4:target + 12]
            data[0x7a52:0x7a52 + len(ARTILLERY_MESSAGE)] = ARTILLERY_MESSAGE
            data[0x87c2] = 3
            store(data, 0x7a50, 360)
            notice(data, actor, 41)
            evidence.update(support='artillery_confirmed', queue_index=index)
            return
    evidence['support'] = 'artillery_queue_full'


def support(before, actor, *, height, audio_return):
    data = bytearray(before)
    kind = word(data, actor)
    evidence = {'smoke': 'not_called', 'allocation': None, 'audio': False,
                'support': 'not_called', 'queue_index': None, 'height_position': None,
                'mailbox_ebx': None, 'consumed_al': None}
    if (not data[actor + 22] & 8 or not word(data, actor + 0x97) or
            word(data, actor + 0x99) < 20 or
            ((word(data, 0x6cde) - word(data, 0x9794)) & 65535) < 1800):
        return data, evidence
    value = random_draw(data)
    retreat = data[actor + 0x43] == 4
    if retreat and value >> 8 <= 64:
        stock = SMOKE_STOCK[kind]
        count = word(data, actor + stock) if kind == 2 else data[actor + stock] if stock else 1
        allocation = None
        if count:
            if kind == 2:
                store(data, actor + stock, count - 1)
                data[actor + 0xe4] = 3
            elif stock:
                data[actor + stock] -= 1
            allocation = allocate_missile(data)
        if allocation is None:
            value = 0x3031
            display(data, actor, value)
            evidence['smoke'] = 'capacity' if count else 'empty'
        else:
            marker, slot, registry = allocation
            store(data, marker, 20)
            heading = word(data, actor + 0x26)
            rotated = rotate(heading, 128, data[0x2040])
            velocity = struct.unpack_from('<2h', data, actor + 0x59)
            for offset, delta, speed in zip((4, 8), rotated, velocity):
                coordinate = struct.unpack_from('<I', data, actor + offset)[0]
                struct.pack_into('<I', data, marker + offset, (coordinate + ((delta + speed) << 5)) & 0xffffffff)
            data[marker + 13] = height
            store(data, 0xea10, 0x54)
            evidence.update(smoke='created', allocation=(marker, slot, registry),
                            height_position=bytes(data[marker + 4:marker + 12]),
                            mailbox_ebx=velocity[1] & 0xffffffff)
            if kind == 2:
                data[actor + 0xa8] = 8
            value = 39
            if word(data, 0x9fdf) == actor:
                store(data, 0x9fdd, 39)
                store(data, 0xea10, 0x64)
                evidence.update(audio=True, mailbox_ebx=(velocity[1] & 0xffff0000) | 39)
                value = audio_return & 65535
    evidence['consumed_al'] = value & 255
    if value & 255 > (76 if retreat else 12):
        return data, evidence
    store(data, 0x9794, word(data, 0x6cde))
    if value & 32:
        artillery(data, actor, evidence)
    else:
        air(data, actor, evidence)
    return data, evidence
