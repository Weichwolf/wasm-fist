"""Independent complete a202/a1e9/a26d/a265/a25b state transitions."""
import struct

PHASE, RAISE, LOWER, QUICK_LOWER, CENTER = range(5)


def predict(original, clock, controls, action):
    raw = bytearray(original)
    previous, step, held = controls
    direction = 1 if action == RAISE else -1
    if action == PHASE:
        flags = raw[0x19]
        if flags & 0x60 == 0:
            return bytes(raw), controls, None
        direction = 1 if flags & 0x20 else -1
        active_step = held
    else:
        active_step = step
    if action in (PHASE, RAISE, LOWER):
        elapsed = (clock - previous) & 65535
        active_step = min(active_step + 1, 364) if active_step < 364 and elapsed < 20 else active_step
        if elapsed >= 20:
            active_step = 18
        previous = clock
    elif action == QUICK_LOWER:
        active_step = 728
    target = struct.unpack_from('<H', raw, 0x97)[0]
    if target:
        struct.pack_into('<H', raw, 0x97, 0)
        struct.pack_into('<h', raw, 0x38, 0)
    if action == CENTER:
        struct.pack_into('<H', raw, 0x8b, 0)
        return bytes(raw), controls, None
    elevation = struct.unpack_from('<h', raw, 0x38)[0]
    bits = (elevation + direction * active_step) & 65535
    value = bits if bits < 32768 else bits - 65536
    value = min(value, 9100) if direction > 0 else max(value, -5460)
    struct.pack_into('<h', raw, 0x38, value)
    return bytes(raw), (previous, step if action == PHASE else active_step, held), active_step


def domain_cases():
    settings = ((0,0,0),(18,19,0),(363,19,65535),(364,20,1),(65535,65535,32768),(1,0,12345))
    for kind in range(4):
        for flags in range(256):
            for target in (0,1,32768,65535):
                for held, elapsed, previous in settings:
                    yield 'flags', kind, flags, target, -1234, ((previous+elapsed)&65535), (previous,123,held), PHASE
    for direction, flags in ((1,32),(-1,64)):
        for word in range(65536):
            yield 'elevation', word&3, flags, 0, word, 20, (0,65535,18), PHASE
    for flags in (32,64):
        for word in range(65536):
            elapsed = (0,19,20,65535)[word&3]
            yield 'held_step', word&3, flags, (0,1,0,65535)[word&3], (0,32767,32768,65535)[word&3], ((12345+elapsed)&65535), (12345,word^65535,word), PHASE
    for elapsed in range(65536):
        yield 'clock', elapsed&3, 32 if elapsed&1 else 64, elapsed&1, 9100 if elapsed&1 else -5460, ((65500+elapsed)&65535), (65500,42,363), PHASE
    for action in (RAISE,LOWER,QUICK_LOWER,CENTER):
        for kind in range(4):
            for elevation in (0,9099,9100,9101,-5459,-5460,-5461,32767,-32768,65535):
                for step in (0,17,18,363,364,365,728,32767,32768,65535):
                    for elapsed in (0,19,20,65535):
                        for target in (0,1):
                            yield 'manual', kind, 255, target, elevation, ((65530+elapsed)&65535), (65530,step,111), action


DOMAIN_COUNTS = {'flags':24576,'elevation':131072,'held_step':131072,'clock':65536,'manual':12800}


def snapshot(kind, flags, target, elevation):
    raw = bytearray((i*37+kind*11+flags) & 255 for i in range(251))
    struct.pack_into('<HH', raw, 0, kind, kind)
    raw[0x19] = flags
    struct.pack_into('<H', raw, 0x97, target)
    struct.pack_into('<H', raw, 0x38, elevation&65535)
    return bytes(raw)
