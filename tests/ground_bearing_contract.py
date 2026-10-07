"""Independent persistent-state model of the complete original ab88 bank.

Original-only evidence: runtime target loss still needs a deliberate consuming
repair. Supplying DS:0000 coordinates documents the original null-read defect;
it does not make those bytes a valid rewrite target.
"""
import struct

from test_geometry import measure

DISPATCH = (0xab91, 0xabb7, 0xac09, 0xabde, 0xac5d, 0xac72, 0xac73, 0xac74)
RETREAT_DURATIONS = (8, 4, 12, 0)


def bearing(original, descriptor, target=None, coarse=0):
    raw = bytearray(original)
    if len(raw) != 251:
        raise ValueError('A complete ground snapshot is required')
    mode = raw[0x43]
    if mode not in range(0, 16, 2):
        raise ValueError('A complete declared ground command mode is required')
    flags, = struct.unpack_from('<H', raw, 0x40)
    automatic = bool(flags & 1)
    position = None
    retreat = mode == 4
    if mode == 0 and flags & 2 or mode == 2 and automatic and flags & 2:
        position = struct.unpack_from('<ii', raw, 0x49)
    elif mode in (4, 6) and automatic:
        if retreat and flags & 64:
            raw[0x44] = (raw[0x44] + 1) % 256
            if descriptor[0] >= len(RETREAT_DURATIONS):
                raise ValueError('The used behavior selector is outside the original UI')
            if raw[0x44] >= RETREAT_DURATIONS[descriptor[0]]:
                flags &= 65535 ^ 64
        else:
            if target is None:
                raise ValueError('Original target dereference has no declared coordinate input')
            position = target
            if retreat:
                flags |= 64
                raw[0x44] = 0
    elif mode == 8 and automatic and raw[0x45] == 4:
        raw[0x30:0x32] = raw[0x47:0x49]
    if position is not None:
        heading, distance = measure((struct.unpack_from('<ii', raw, 4), position, coarse))
        packed = (distance >> 8) & 65535
        if automatic:
            struct.pack_into('<H', raw, 0x30, (heading + (32768 if retreat else 0)) % 65536)
        struct.pack_into('<H', raw, 0x53, max(0, packed - 30) if mode in (4, 6) else packed)
    struct.pack_into('<H', raw, 0x40, flags)
    return bytes(raw)
