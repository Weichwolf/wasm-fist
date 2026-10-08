"""Independent full f69:aae4 axis/profile consumer, not its device producer."""
import struct
from ground_throttle_contract import throttle


def predict(original):
    raw = bytearray(original)
    if len(raw) != 251 or int.from_bytes(raw[:2], 'little') >= 4:
        raise ValueError('A complete ground actor is required')
    steer, pedal = struct.unpack_from('<bb', raw, 0xa1)
    if steer >= 24 or steer <= -24:
        heading, = struct.unpack_from('<H', raw, 0x26)
        struct.pack_into('<H', raw, 0x30, (heading + steer) & 65535)
    applied = pedal if pedal >= 24 or pedal < -24 else 0
    demand = max(-240, min(284, -2 * applied))
    struct.pack_into('<h', raw, 0x57, demand)
    speed, = struct.unpack_from('<h', raw, 0x55)
    profile = 3 if speed < 0 else 0 if raw[0x90] > 1 else None
    if profile is not None:
        raw, refresh = throttle(raw, (0, 0, 0, 0), profile + 2)
        assert refresh
    return bytes(raw), profile, demand


def domains():
    for kind in range(4):
        for steer in range(-128, 128):
            for pedal in range(-128, 128):
                yield 'axes', kind, steer, pedal, 0xffff, 0x1234, (-1, 0)[pedal & 1], (0, 1, 2, 255)[steer & 3]
    for kind in range(4):
        for heading in range(65536):
            steer = (-128, -25, -24, -23, 0, 23, 24, 127)[heading & 7]
            yield 'heading_words', kind, steer, -24, heading, heading ^ 0xffff, 0, 1
    for kind in range(4):
        for mode in range(256):
            for speed in (-32768, -1, 0, 32767):
                yield 'profile_modes', kind, 24, 127, 65535, 0, speed, mode

COUNTS = {'axes': 262144, 'heading_words': 262144, 'profile_modes': 4096}
