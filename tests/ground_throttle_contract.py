"""Independent persistent actor/display model of full ad2f/ad3b/a19e returns."""
import struct

DISPATCH = (0xad62, 0xad8c, 0xae06, 0xaddb, 0xae07, 0xae25, 0xae26, 0xae2c)
SETTERS = (0x7da5, 0x8679, 0x8f3a, 0x96cb)
PROFILE_COMPONENTS = (0xd6, 0xc8, 0xca, 0xd2)
DISPLAY_OFFSETS = (0x8e58, 0x8e5a, 0x8e5c, 0x8e5e)
LEADER_THROTTLES = (96, 160, 208, 224)


def throttle(original, descriptor, operation=0):
    """0=ad2f, 1=ad3b, 2..257=a19e AL=operation-2; 258 starts at ad2f."""
    raw = bytearray(original)
    if len(raw) != 251 or int.from_bytes(raw[:2], 'little') >= 4:
        raise ValueError('A complete ground-class snapshot is required')
    if operation not in range(259):
        raise ValueError('Unknown complete callback boundary')
    if operation == 258:
        operation = 0
    refresh = False
    if operation == 0:
        mode = raw[0x43]
        if mode not in range(0, 16, 2):
            raise ValueError('Command mode is outside the complete eight-entry bank')
        flags, distance = struct.unpack_from('<H', raw, 0x40)[0], struct.unpack_from('<H', raw, 0x53)[0]
        value = None
        if mode == 0:
            value = 0 if not flags & 2 else 80 if distance <= 8 else None
            if value is None:
                if descriptor[3] >= 4:
                    raise ValueError('Used throttle selector is outside its original UI domain')
                value = LEADER_THROTTLES[descriptor[3]]
        elif mode == 2:
            value = 0 if not flags & 2 or distance <= 3 or distance == 65535 else next(
                speed for limit, speed in ((8, 16), (32, 32), (48, 128), (80, 240), (65535, 272))
                if distance <= limit)
        elif mode == 6:
            target_range, = struct.unpack_from('<H', raw, 0x99)
            value = next(speed for limit, speed in ((45, -48), (60, 0), (90, 80), (65536, 240))
                         if target_range < limit)
        elif mode == 8:
            value = {2: 0, 6: -48}.get(raw[0x45], 64)
        elif mode in (12, 14):
            value = 0
        if value is not None:
            struct.pack_into('<h', raw, 0x57, value)
    if operation >= 2:
        profile = operation - 2
    elif raw[0x90] <= 1:
        pitch, = struct.unpack_from('<h', raw, 0x34)
        profile = int(pitch >= 3584)
        if profile == raw[0x90]:
            profile = None
    else:
        profile = None
    if profile is not None:
        raw[0x90] = profile
        raw[PROFILE_COMPONENTS[int.from_bytes(raw[:2], 'little')]] = 3
        refresh = True
    return bytes(raw), refresh
