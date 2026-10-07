"""Independent whole-DGROUP model of original b053 and genuine parent entry 13.

This is reference evidence, not a runtime implementation. Byte-lane addressing
deliberately exposes malformed saved-member writes outside the authored roster.
"""
import struct

from test_vehicle_start import step

ROSTER = 0x6d3c


def word(raw, offset):
    return struct.unpack_from('<H', raw, offset)[0]


def store(raw, offset, value):
    struct.pack_into('<H', raw, offset, value & 65535)


def promote(before, actor):
    result = bytearray(before)
    member = result[actor + 28]
    if member == 0:
        return result
    # ADD BL has no carry into BH. The authored domain is eight platoons,
    # four members; this exact observation also covers malformed saved bytes.
    base = result[actor + 27] * 4
    index = (base & 0xff00) | ((base + member) & 255)
    previous = ROSTER + 2 * index - 2
    predecessor = word(result, previous)
    if predecessor:
        if word(result, predecessor) == 23:
            result[predecessor + 36] = (result[predecessor + 36] + 1) & 255
        else:
            if result[actor + 25] & 0x16 or not result[predecessor + 25] & 0x16:
                return result
            result[predecessor + 28] = (result[predecessor + 28] + 1) & 255
    store(result, previous, actor)
    store(result, previous + 2, predecessor)
    result[actor + 28] = (result[actor + 28] - 1) & 255
    store(result, actor + 0x40, word(result, actor + 0x40) & 0xfffd)
    return result


def parent(before, actor):
    result = bytearray(before)
    seeds = list(struct.unpack_from('<4H', result, 0x1f84))
    cursor = ((word(result, 0x1f82) - 0x1f84) // 2 + 1) % 4
    draw, cursor = step(seeds, cursor)
    struct.pack_into('<5H', result, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *seeds)
    store(result, 0x342, draw)
    store(result, 0x978c, draw)
    store(result, 0x9a08, actor)
    if result[0x978a] == 0:
        counter = (result[actor + 0x42] + 1) & 255
        if counter & 15 != 13:
            raise ValueError('This complete parent observation requires entry thirteen')
        result[actor + 0x42] = counter
        platoon = result[actor + 27]
        store(result, 0x9796, word(result, 0x85a0 + 2 * platoon))
        store(result, 0x9798, word(result, 0x7d2a + 2 * platoon))
        result = promote(result, actor)
    if word(result, 0x7ae0) == actor:
        raise ValueError('Selected diagnostic producer remains a separate complete boundary')
    return result
