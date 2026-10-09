"""Independent complete selected a57a composition using proved child models."""
import struct

from analog_drive_contract import predict as axes
from ground_elevation_contract import RAISE, LOWER, predict as elevation
from manual_turret_contract import REFRESH, predict as turret

DRIVE_BANK = (0xa5ea, 0xa5eb, 0xa5f1, 0xa62a, 0xa624, 0xa630)
WEAPON_BANK = (0xa59b, 0xa59c, 0xa5a6, 0xa5b0, 0xa5cd)
VIEW_METHODS = (0x17671, 0x18029, 0x187bc, 0x190af)
VIEW_COMPONENTS = ((0xed, 0xef, 0xf1), (0xf0, 0xf2, 0xf4),
                   (0xe7, 0xe9, 0xeb), (0xed, 0xef, 0xf1))
VIEW_DISPLAY = (0x8e49, 0x8e4b, 0x8e4d)


def predict(original, selected, mode, clock, controls, selector):
    raw = bytearray(original)
    if len(raw) != 251 or int.from_bytes(raw[:2], 'little') >= 4:
        raise ValueError('Complete ground actor required')
    evidence = {'drive': False, 'view': False, 'profile': None,
                'drive_mode': None, 'weapon': None, 'demand': None,
                'turret_index': None, 'turret_step': None, 'elevation_step': None}
    if not selected:
        return bytes(raw), controls, selector, evidence
    if not int.from_bytes(raw[0x40:0x42], 'little') & 1:
        effective = ((mode << 1) & 65535) // 2
        if effective >= len(DRIVE_BANK):
            raise ValueError('Unsafe used drive index')
        evidence['drive_mode'] = effective
        if effective in (1, 2, 3, 4):
            raw, profile, demand = axes(raw)
            raw = bytearray(raw)
            evidence.update(drive=True, profile=profile, demand=demand)
            if effective == 2:
                value = raw[0xa4]
                raw[0x86] = 1 if value < 80 else 2 if value < 160 else 6
                kind = int.from_bytes(raw[:2], 'little')
                for offset in VIEW_COMPONENTS[kind]:
                    raw[offset] = 3
                evidence['view'] = True
    action = raw[0xa0]
    if action not in (0, 2, 4, 6, 8):
        raise ValueError('Unsafe used weapon index')
    evidence['weapon'] = action
    if action in (2, 4):
        selector = 88 if action == 2 else 232
        raw, index, step = turret(raw, selector, action == 4)
        evidence.update(turret_index=index, turret_step=step)
    elif action in (6, 8):
        kind = int.from_bytes(raw[:2], 'little')
        flags = int.from_bytes(raw[0x40:0x42], 'little')
        if flags & 1:
            struct.pack_into('<H', raw, 0x40, flags & 65534)
            raw[REFRESH[kind]] = 3
        raw, controls, step = elevation(raw, clock, controls,
                                         RAISE if action == 6 else LOWER)
        evidence['elevation_step'] = step
    return bytes(raw), controls, selector, evidence
