"""Independent raw-state acquisition/aim expectations for unchanged original returns."""
import struct

from test_geometry import measure, signed
from test_vehicle_start import step
from test_visibility import visible

ACQUISITION_THRESHOLDS = (120, 200, 40, 0)
TARGET_VOICES = (15, 16, 15, 16, *([18] * 23), 17)


def aim_positions(owner, actor, target):
    kind = int.from_bytes(actor[:2], 'little')
    target_kind = int.from_bytes(target[:2], 'little')
    source = struct.unpack_from('<3i', actor, 4)
    destination = struct.unpack_from('<3i', target, 4)
    offset = (owner.variant_heights[target[25] & 3] if target_kind == 26
              else owner.target_heights[target_kind])
    return ((*source[:2], signed(source[2] + owner.source_heights[kind])),
            (*destination[:2], signed(destination[2] + offset)))


def acquisition(owner, pointer, actor, objects, random, behavior, side, pixels, *,
                automatic, candidate, selected, gate, clock, last_voice, display):
    after = bytearray(actor)
    seeds, cursor = list(random[0]), random[1]
    attempt, draw = not automatic, None
    if automatic and actor[0x94] and struct.unpack_from('<H', actor, 0x97)[0] == 0:
        draw, cursor = step(seeds, cursor)
        attempt = (draw & 255) <= ACQUISITION_THRESHOLDS[behavior]
    transfer = None
    messages, requests = [], []
    if attempt and candidate:
        target = objects[candidate]
        if target[23] & 64 == 0 and target[22] & 1 == 0:
            source, destination = aim_positions(owner, actor, target)
            admitted = visible(side, pixels, (source, destination))
            transfer = {'source': source, 'target': destination, 'visible': admitted}
            struct.pack_into('<H', after, 0x97, candidate if admitted else 0)
            if admitted and selected == pointer:
                kind = int.from_bytes(target[:2], 'little')
                bank = owner.enemy_text if target[22] & 8 else owner.friendly_text
                message = bank[kind]
                if message == 65535:
                    # Actual acquisition uses the whole byte, unlike aim's &3.
                    # The observer separately proves this adjacent-table read.
                    message = owner.variant_text[target[25]]
                display = (120, message)
                messages.append(message)
                voice = TARGET_VOICES[kind] if target[22] & 8 else 20
                if gate == 65535 and actor[22] & 8 == 0 and (clock-last_voice) % 65536 >= 30:
                    requests.append((0x0280 | voice, clock & 0xff00, 0))
                    last_voice = clock
    if automatic and attempt:
        flags = struct.unpack_from('<H', after, 0x40)[0] | 128
        struct.pack_into('<H', after, 0x40, flags)
    return {'actor': bytes(after), 'random': (seeds, cursor), 'draw': draw,
            'transfer': transfer, 'display': display, 'messages': messages,
            'requests': requests, 'last_voice': last_voice, 'complete_dgroup_bytes': 65536}


def target_geometry(owner, actor, objects, coarse):
    target = struct.unpack_from('<H', actor, 0x97)[0]
    if target == 0:
        return bytes(actor), None
    source, destination = aim_positions(owner, actor, objects[target])
    heading, distance = measure((source[:2], destination[:2], coarse))
    # 0578 feeds signed wrapped Z difference and unsigned planar distance to
    # the existing 077e angle owner. It retains the complete planar CX:DX.
    elevation = measure(((0, 0), (signed(destination[2]-source[2]), signed(distance)), coarse))[0]
    after = bytearray(actor)
    struct.pack_into('<H', after, 0x9b, heading)
    struct.pack_into('<H', after, 0x38, elevation)
    struct.pack_into('<H', after, 0x99, (distance >> 8) & 65535)
    return bytes(after), {'source': source, 'target': destination, 'heading': heading,
                          'elevation': elevation, 'distance': distance}
