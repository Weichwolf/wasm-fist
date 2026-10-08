"""Independent original station/support and parent-nine/twelve predictions.

These are reference contracts. Unsafe adjacent-table reads are classified,
not admitted as additional authored variants or installed as rewrite rules.
"""
from ground_maneuver_contract import random_draw
from roster_promotion_contract import store, word
from support_audio_contract import support
from test_weapon_control import CONTINUOUS, COUNTS, DIRTY, LOAD_CUES, TIMES

NORMAL_TABLES = (0x22bc, 0x24b4, 0x26a0, 0x296c)
VARIANT_TABLES = (0x2304, 0x24fc, 0x26e8, 0x29b4)
RELOAD_TABLES = (0x8f54, 0x90f6, 0x9186, 0x9282)
CUE_TABLES = (0x8f0e, 0x90ee, 0x9196, 0x9242)
DEFAULT = ((2, 0, 6), (0, 4, 2), (0, 2, 4), (4, 0, 2))
ARMORED = ((0, 6, 2), (2, 4, 0), (2, 4, 0), (2, 0, 4))
AIRBORNE = ((4, 4, 4), (6, 6, 6), (6, 6, 6), (6, 6, 6))
ARTILLERY = ((6, 0, 2), (2, 4, 0), (4, 2, 0), (2, 0, 4))
VARIANTS = (((4, 0, 2), (6, 0, 2)), ((6, 4, 2), (2, 4, 6)),
            ((6, 2, 0), (4, 2, 0)), ((6, 0, 6), (2, 0, 6)))


def preferences(kind, target_type, variant=0):
    if target_type == 26:
        if variant >= 4:
            raise ValueError('Unvalidated original variant table read')
        return VARIANTS[kind][variant & 1]
    if target_type in (1, 3):
        return ARMORED[kind]
    if target_type in (5, 6):
        return AIRBORNE[kind]
    if target_type == 27:
        return ARTILLERY[kind]
    if kind == 2 and target_type in (0, 2):
        return (0, 2, 4)
    return DEFAULT[kind]


def station(before, actor, text):
    data = bytearray(before)
    kind = word(data, actor)
    target = word(data, actor + 0x97)
    target_type = word(data, target) if target else 0
    variant = data[target + 25] if target_type == 26 else None
    valid = target_type < 28 and (variant is None or variant < 4)
    if valid:
        choices = preferences(kind, target_type, variant or 0)
    else:
        # Complete byte indexing is observable negative evidence. Read precisely
        # the original adjacent table and wrapped SI, never mask the variant.
        table = VARIANT_TABLES[kind] if variant is not None else NORMAL_TABLES[kind]
        index = variant if variant is not None else target_type
        pointer = word(text, (table + ((index << 1) & 65535)) & 65535)
        choices = tuple(text[(pointer + i) & 65535] for i in range(3))
    base = actor + (0xac if kind == 2 else 0xad)
    for index, selected in enumerate(choices):
        if word(data, (base + selected) & 65535):
            break
    effect = {'operation': 'station', 'choices': choices, 'choice_index': index,
              'station': selected, 'valid_variant': valid, 'request': None,
              'unsafe_station': selected & 1 != 0 or selected // 2 >= COUNTS[kind],
              'allocation': None, 'audio': False, 'consumed_al': None,
              'height_position': None, 'mailbox_ebx': None}
    if selected == data[actor + 0x91]:
        return data, effect
    data[actor + 0x91] = selected
    if selected != CONTINUOUS[kind] and selected != data[actor + 0xa5]:
        data[actor + 0xa5] = selected
        slot = selected // 2
        countdown = TIMES[kind][slot] if slot < COUNTS[kind] else data[RELOAD_TABLES[kind] + slot]
        cue = LOAD_CUES[kind][slot] if slot < COUNTS[kind] else data[CUE_TABLES[kind] + slot]
        data[actor + 0xa8] = countdown
        voice_bx = (slot * 2) if kind == 0 else slot
        if kind == 0 and word(data, (base + slot * 2) & 65535) == 0:
            data[actor + 0xa8], cue = 255, 13
        elif (kind, slot) in ((1, 0), (3, 2)) and word(data, base + slot * 2) == 0:
            if data[actor + 0xbb] == 0:
                cue = 13
            elif word(data, 0x6d34) == actor and data[0x6ce6] != 2:
                data[0x9fd6] = 25
                store(data, 0x9fd7, word(data, 0x6cde) + 90)
        if (word(data, 0x6da2) == 65535 and word(data, 0x6d34) == actor
                and not data[actor + 22] & 8 and cue != 255
                and ((word(data, 0x452) - word(data, 0x9fca)) & 65535) >= 30):
            store(data, 0x9fca, word(data, 0x452))
            store(data, 0xea10, 0x64)
            effect.update(audio=True, mailbox_ebx=voice_bx,
                          request=(0x0280 | cue, word(data, 0x452) & 0xff00, 0))
    for offset in DIRTY[kind]:
        data[actor + offset] = 3
    for offset in (0x8e60, 0x8e64, 0x8e68, 0x8e6c):
        data[offset] = 3
    return data, effect


def remaining(before, actor, text, *, entry, height, audio_return):
    data = bytearray(before)
    parent = entry == 0xab03
    index = bank = None
    if parent:
        if word(data, 0x7ae0) == actor:
            raise ValueError('Selected diagnostic requires its complete separate owner')
        store(data, 0x9a08, actor)
        store(data, 0x978c, random_draw(data))
        if data[0x978a]:
            entry = 0xb111
        else:
            data[actor + 0x42] = (data[actor + 0x42] + 1) & 255
            index = data[actor + 0x42] & 15
            if index not in (9, 12):
                raise ValueError('Only complete parent entries nine/twelve are covered')
            platoon = data[actor + 27]
            store(data, 0x9796, word(data, 0x85a0 + platoon * 2))
            store(data, 0x9798, word(data, 0x7d2a + platoon * 2))
            bank = 'automatic' if word(data, actor + 0x40) & 1 else 'controlled'
            entry = (0xae5c if index == 9 else 0xb0be) if bank == 'automatic' else 0xb111
    if entry == 0xae5c:
        data, effect = station(data, actor, text)
    elif entry == 0xb0be:
        data, effect = support(data, actor, height=height, audio_return=audio_return)
        effect.update(operation='support', request=(11, 0, 0) if effect['audio'] else None)
    elif entry == 0xb111:
        effect = {'operation': 'genuine_ret', 'audio': False, 'request': None,
                  'allocation': None, 'consumed_al': None, 'height_position': None,
                  'mailbox_ebx': None}
    else:
        raise ValueError('Unknown complete remaining-ground entry')
    effect.update(parent_index=index, parent_bank=bank)
    return data, effect
