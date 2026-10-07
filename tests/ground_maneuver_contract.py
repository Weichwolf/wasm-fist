"""Independent whole-DGROUP model of original maneuver, lookahead and idle turret.

Reference evidence only. Distinct word/dword wrapping, ordered physical registry
and numeric scratch writes expose the actual complete original boundaries.
"""
import math
import struct

from roster_promotion_contract import store, word
from test_geometry import measure, signed, squared
from test_vehicle_motion import rotate
from test_vehicle_start import step

DISPATCH = (0xae7b, 0xaea8, 0xaf0b, 0xaea2)
OFFSETS = (910, -910, 1820, -1820, 2730, -2730, 3640, -3640,
           4550, -4550, 5460, -5460, 6370, -6370, 7280, -7280,
           8190, -8190, 9100, -9100, 10010, -10010, 10920, -10920,
           13650, -13650, 14560, -14560, 17290, -17290, 17290, -17290)


def position(data, pointer):
    return struct.unpack_from('<2i', data, pointer + 4)


def coordinates(data, offset, value):
    struct.pack_into('<2i', data, offset, *(signed(lane) for lane in value))


def distance(data, source, target):
    total = sum(squared((b - a) & 0xffffffff) for a, b in zip(source, target))
    struct.pack_into('<QH', data, 0x2034, total, 0)
    shift = 0 if total < 2**32 else 4 if total < 2**40 else 8 if total < 2**48 else 16
    return (math.isqrt(total >> (shift * 2)) << shift) & 65535


def predict(data, actor, candidate, margin, vector, evidence):
    """Full f69:b2a0: start at eight vectors, then sample one through 24 more."""
    ax, ay = position(data, actor)
    dx, dy = vector
    target = position(data, candidate)
    struct.pack_into('<2i', data, 0x9962, dx, dy)
    current = signed(ax + dx * 8), signed(ay + dy * 8)
    coordinates(data, 0x9684, current)
    coordinates(data, 0x9690, target)
    struct.pack_into('<i', data, 0xe440, current[0])
    struct.pack_into('<i', data, 0xe448, current[1])
    limit = (word(data, candidate + 0x14) + word(data, actor + 0x14) + margin) & 65535
    store(data, 0x979e, limit)
    evidence['prediction_calls'] += 1
    for _ in range(24):
        current = signed(current[0] + dx), signed(current[1] + dy)
        coordinates(data, 0x9684, current)
        struct.pack_into('<i', data, 0xe444, current[0])
        struct.pack_into('<i', data, 0xe44c, current[1])
        evidence['prediction_samples'] += 1
        if distance(data, target, current) <= limit:
            evidence['prediction_hits'] += 1
            return True
    return False


def obstacle_search(data, actor, vector, evidence):
    store(data, 0x97a2, 0xdfbc)
    store(data, 0x97a4, 182)
    for index in range(182):
        candidate = word(data, 0xdfbc + index * 4)
        store(data, 0x97a2, 0xdfbc + (index + 1) * 4)
        store(data, 0x97a4, 181 - index)
        evidence['registry_visits'] += 1
        if not candidate or candidate == actor or word(data, candidate) == 21 or not data[candidate + 22] & 64:
            continue
        angle, _ = measure((position(data, candidate), position(data, actor), data[0x2040]))
        # Complete b112 adds a half turn after the real 0731 bearing helper.
        difference = (angle + 32768 - word(data, actor + 0x10)) & 65535
        if 16384 <= difference < 49152:
            continue
        separation = sum(abs(signed(a - b)) for a, b in zip(position(data, actor), position(data, candidate))) & 0xffffffff
        if separation > 7680:
            continue
        store(data, 0x97a0, 1024)
        if predict(data, actor, candidate, 1024, vector, evidence):
            return True
    return False


def maneuver(before, actor):
    data = bytearray(before)
    evidence = {'state': data[actor + 0x45] & 6, 'searches': 0, 'search_index': None,
                'registry_visits': 0, 'prediction_calls': 0, 'prediction_samples': 0, 'prediction_hits': 0}
    state = evidence['state']
    blocked = bool(word(data, actor + 0x40) & 8)
    if state == 0:
        if not blocked:
            data[actor + 0x51] = 0
            return data, evidence
        data[actor + 0x51] = (data[actor + 0x51] + 1) & 255
        if data[actor + 0x51] < 3:
            data[actor + 0x45], data[actor + 0x46] = 2, 3
            return data, evidence
    data[actor + 0x46] = (data[actor + 0x46] - 1) & 255
    if data[actor + 0x46] != 0:
        return data, evidence
    if state in (0, 6):
        data[actor + 0x45] = 0
    elif state == 4:
        data[actor + 0x45], data[actor + 0x46] = (4, 4) if blocked else (0, 0)
    else:
        heading = word(data, actor + 0x26)
        index = 0
        store(data, 0x97a8, 0)
        for offset in OFFSETS[:15]:
            vector = tuple(value * 8 for value in rotate((heading + offset) & 65535, 32, data[0x2040]))
            struct.pack_into('<2i', data, 0x9962, *vector)
            evidence['searches'] += 1
            if not obstacle_search(data, actor, vector, evidence):
                break
            index += 1
            store(data, 0x97a8, index * 2)
        store(data, 0x97a6, index * 2 + 16)
        store(data, actor + 0x47, heading + OFFSETS[index + 8])
        data[actor + 0x45], data[actor + 0x46] = 4, 4
        evidence['search_index'] = index
    return data, evidence


def random_draw(data):
    seeds = list(struct.unpack_from('<4H', data, 0x1f84))
    cursor = ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4
    value, cursor = step(seeds, cursor)
    struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *seeds)
    store(data, 0x342, value)
    return value


def idle_turret(before, actor):
    data = bytearray(before)
    draws = 0
    if not word(data, actor + 0x40) & 4 and not word(data, actor + 0x97):
        value = random_draw(data)
        draws = 1
        if not value & 0x3f:
            store(data, actor + 0x8b, 32768)
        elif not value & 0x3c0:
            store(data, actor + 0x8b, 0)
        elif not value & 0x1c00:
            value = random_draw(data)
            draws = 2
            store(data, actor + 0x8b, (value & 0x3fff) - 8192)
    return data, {'random_draws': draws}


def motion_obstacle(before, actor, candidate):
    data = bytearray(before)
    evidence = {'prediction_calls': 0, 'prediction_samples': 0, 'prediction_hits': 0}
    angle, _ = measure((position(data, candidate), position(data, actor), data[0x2040]))
    difference = (angle + 32768 - word(data, actor + 0x10)) & 65535
    if difference < 16384 or difference >= 49152:
        vector = tuple(value * 8 for value in struct.unpack_from('<2h', data, actor + 0x59))
        if data[actor + 0x45]:
            vector = tuple(value * 8 for value in rotate(word(data, actor + 0x26), 64, data[0x2040]))
        struct.pack_into('<2i', data, 0x9962, *vector)
        store(data, 0x97a0, 3584)
        if predict(data, actor, candidate, 3584, vector, evidence):
            store(data, actor + 0x40, word(data, actor + 0x40) | 8)
    return data, evidence


def parent(before, actor):
    """Genuine diagnostic-unselected entries seven, eleven and fifteen only."""
    data = bytearray(before)
    if word(data, 0x7ae0) == actor:
        raise ValueError('Selected diagnostic producer is a separate complete boundary')
    store(data, 0x9a08, actor)
    store(data, 0x978c, random_draw(data))
    evidence = {'parent_callback': None}
    if not data[0x978a]:
        data[actor + 0x42] = (data[actor + 0x42] + 1) & 255
        index = data[actor + 0x42] & 15
        if index not in (7, 11, 15):
            raise ValueError('This complete parent observation requires entry seven, eleven or fifteen')
        platoon = data[actor + 27]
        store(data, 0x9796, word(data, 0x85a0 + platoon * 2))
        store(data, 0x9798, word(data, 0x7d2a + platoon * 2))
        if word(data, actor + 0x40) & 1:
            callback = idle_turret if index == 11 else maneuver
            data, effect = callback(data, actor)
            evidence.update(effect)
            evidence['parent_callback'] = 'idle_turret' if index == 11 else 'maneuver'
        else:
            evidence['parent_callback'] = 'genuine_ret'
    return data, evidence
