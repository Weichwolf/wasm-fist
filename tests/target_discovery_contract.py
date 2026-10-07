"""Semantic discovery expectation, checked against complete original returns."""
import struct
import types

from test_geometry import measure
from test_proximity import proximity
from test_visibility import visible


TABLES = types.SimpleNamespace(
    preferences=((0, 0, 0, 0, 99, 1, 1, *([99] * 19), 2, 2),
                 (1, 1, 1, 1, 99, 0, 0, *([99] * 19), 2, 2)),
    target_heights=(1792, 2048, 1792, 1536, 0, 256, 256, *([0] * 20), 2048),
    source_heights=(2048, 2560, 2048, 1920, *([0] * 22), 1536, 0),
    variant_heights=(3840, 4352, 2560, 3072),
    ranges=((1000, 1000, 1000, 1000), (150, 150, 150, 150), (625, 625, 450, 450)))


def discovery(owner, actor_pointer, actor, registry, raw_objects, link, side, pixels,
              *, coarse=0, gate=0, selected=None, clock=0, last_voice=0):
    kind = int.from_bytes(actor[:2], 'little')
    preference = owner.preferences[kind & 1]
    range_bank = 0 if link < 2 else 2 if actor[26] & 16 else 1
    limit = owner.ranges[range_bank][kind]
    primary = secondary = 0
    primary_range = secondary_operand = 65535
    priority = 255
    count = 0
    trace = []
    actor_pose = struct.unpack_from('<3i', actor, 4)
    source = (*actor_pose[:2], (actor_pose[2] + owner.source_heights[kind] + 2**31) % 2**32 - 2**31)
    for index, pointer in enumerate(registry):
        if not pointer or pointer == actor_pointer:
            continue
        candidate = raw_objects[pointer]
        if not candidate[22] & 4 or not (candidate[22] ^ actor[22]) & 8:
            continue
        candidate_kind = int.from_bytes(candidate[:2], 'little')
        pose = struct.unpack_from('<3i', candidate, 4)
        aim = (owner.variant_heights[candidate[25] & 3] if candidate_kind == 26
               else owner.target_heights[candidate_kind])
        target = (*pose[:2], (pose[2] + aim + 2**31) % 2**32 - 2**31)
        admitted = visible(side, pixels, (source, target))
        operand = 0
        if admitted:
            rank = preference[candidate_kind]
            operand = 65280 | rank
            if rank <= priority:
                if rank < priority:
                    primary_range = secondary_operand = 65535
                    priority = rank
                value = proximity((pose[:2], actor_pose[:2], 0))
                operand = value & 65535
                if value < 2**24:
                    operand = value >> 8
                    if operand <= limit and operand < primary_range:
                        primary, primary_range = pointer, operand
                        count = (count + 1) % 256
        trace.append({'registry': index, 'pointer': pointer, 'source': source, 'target': target,
                      'visible': admitted, 'secondary_operand': operand,
                      'primary': primary, 'priority': priority})
        if candidate[23] & 8 and operand < secondary_operand:
            secondary, secondary_operand = pointer, operand
    after = bytearray(actor)
    after[25] &= 127
    if secondary:
        after[25] |= 128
        candidate_pose = struct.unpack_from('<2i', raw_objects[secondary], 4)
        heading = (measure((candidate_pose, actor_pose[:2], coarse))[0] + 32768) % 65536
        struct.pack_into('<H', after, 0x8e, heading)
    after[0x94] = count
    struct.pack_into('<H', after, 0x9d, primary)
    old = struct.unpack_from('<H', actor, 0x97)[0]
    if actor[0x40] & 1 and old and old != primary:
        old_kind = int.from_bytes(raw_objects[old][:2], 'little')
        if preference[old_kind] > priority:
            struct.pack_into('<H', after, 0x97, 0)
    notification = (actor[0x94] == 0 and count != 0 and gate == 65535 and
                    (selected is None or selected == actor_pointer) and actor[22] & 8 == 0 and
                    (clock - last_voice) % 65536 >= 30)
    return {'actor': bytes(after), 'transfers': trace,
            'requests': [(0x028e, clock & 0xff00, 0)] if notification else [],
            'primary': primary, 'secondary': secondary, 'primary_range': primary_range,
            'secondary_operand': secondary_operand, 'priority': priority, 'count': count,
            'limit': limit, 'last_voice': clock if notification else last_voice,
            'complete_dgroup_bytes': 65536}
