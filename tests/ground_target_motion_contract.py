"""Target-aware turret and ordered ground motion predictions.

Reuse the proved untargeted motion owner with a settled turret, then apply the
actual class-specific target stage. A retained target heading is intentionally
different from a freshly measured heading for M1/M3.
"""
import struct

from roster_promotion_contract import store, word
from target_acquisition_contract import target_geometry
from test_geometry import signed as signed_position
from test_vehicle_motion import DIRTY, rotate, signed, update


def turret(owner, original, objects, coarse=0):
    raw = bytearray(original)
    kind = word(raw, 0)
    target = word(raw, 0x97)
    if target:
        if kind >= 2:
            raw = bytearray(target_geometry(owner, raw, objects, coarse)[0])
        store(raw, 0x8b, word(raw, 0x9b) - word(raw, 0x26))
    delta = signed(word(raw, 0x8b) - word(raw, 0x89))
    step = max(-364, min(364, delta))
    store(raw, 0x89, word(raw, 0x89) + step)
    store(raw, 0x10, word(raw, 0x26) + word(raw, 0x89))
    raw[0xa9:0xac] = b'\x80\0\0'
    if step:
        raw[DIRTY[kind][1]] = 3
        if kind == 3:
            raw[0xd1] = 3
    return bytes(raw), bool(step)


def motion(owner, original, objects, coarse=0, actor=None):
    """Motion followed by the complete class turret wrapper in either angle mode."""
    settled = bytearray(original)
    store(settled, 0x8b, word(settled, 0x89))
    moved, events = update(settled)
    assert events[2] == 0
    moved = bytearray(moved)
    # Hull recenter changes both turret-relative words by the same amount.
    adjustment = word(moved, 0x89) - word(original, 0x89)
    store(moved, 0x8b, word(original, 0x8b) + adjustment)
    if coarse:
        velocity = rotate(word(moved, 0x26), signed(word(moved, 0x55)) // 2, True)
        for position, field, value in zip((4, 8), (0x59, 0x5b), velocity):
            old_velocity = signed(word(moved, field))
            old_position = struct.unpack_from('<i', moved, position)[0]
            struct.pack_into('<i', moved, position, signed_position(old_position - old_velocity + value))
            store(moved, field, value)
    if actor is not None and word(moved, 0x97) == actor:
        objects = {**objects, actor: bytes(moved)}
    result, changed = turret(owner, bytes(moved), objects, coarse)
    return result, (*events[:2], int(changed))
