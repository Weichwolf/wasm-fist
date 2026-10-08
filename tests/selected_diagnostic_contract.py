"""Independent complete original selected-ground diagnostic observations.

The descriptor near word is an original diagnostic address, not a portable
runtime identity. Unsafe caption/platoon indices are excluded from this model.
"""
from ground_maneuver_contract import random_draw
from roster_promotion_contract import store, word

# b152 reaches8fde; its genuine ab03 caller adds one near return word.
STACK_BEGIN, STACK_END = 0x8fdc, 0x9004
GENUINE_CONTROLLED_RET = frozenset((5, 8, 9, 10, 12, 14))


def panel(before, actor, screen):
    data, output = bytearray(before), bytearray(screen)
    admission, platoon = data[0x978a], data[actor + 27]
    if admission not in (0, 1) or platoon >= 8 or len(output) != 4000:
        raise ValueError('Diagnostic requires authored caption/platoon and complete text surface')
    store(data, 0x9a06, actor)

    def write(row, column, text, attribute=None):
        for index, character in enumerate(text.encode('ascii')):
            position = row * 160 + (column + index) * 2
            output[position] = character
            if attribute is not None:
                output[position + 1] = attribute

    write(12, 10, ('OFF', 'ON ')[admission], 7)
    descriptor = word(data, 0x85a0 + platoon * 2)
    route = word(data, 0x7d2a + platoon * 2)
    fields = ((13, 10, data[actor + 0x43], 2),
              (14, 10, word(data, actor + 0x57), 4),
              (15, 10, descriptor, 4),
              (16, 10, word(data, descriptor + 6), 4),
              (17, 10, data[route], 4),
              (18, 10, word(data, 0x97ee), 4),
              (19, 10, word(data, actor + 0x53), 4),
              (20, 10, word(data, actor + 0x9d), 4),
              (21, 10, data[actor + 0x94], 4),
              (22, 10, data[actor + 0x45], 4),
              (23, 10, data[actor + 0x43], 4),
              (24, 9, platoon, 2),
              (24, 12, data[actor + 0x1c], 2))
    for row, column, value, width in fields:
        write(row, column, f'{value:0{width}X}')
    store(data, 0x3a, 0x5e58)
    store(data, 0x29c, 24 * 160 + 12 * 2 + 4)
    return bytes(data), bytes(output)


def parent(before, actor, screen):
    """Complete genuine ab03 for skipped banks or the controlled RET entries.

    This is not a model for omitted active automatic callbacks or heading wrap.
    Their already recovered owners remain prerequisites to full parent work.
    """
    data = bytearray(before)
    store(data, 0x9a08, actor)
    store(data, 0x978c, random_draw(data))
    if data[0x978a] == 0:
        counter = (data[actor + 0x42] + 1) & 255
        if word(data, actor + 0x40) & 1 or counter & 15 not in GENUINE_CONTROLLED_RET:
            raise ValueError('Active nested callback requires its complete separate contract')
        data[actor + 0x42] = counter
        platoon = data[actor + 27]
        if platoon >= 8:
            raise ValueError('Used invalid platoon')
        store(data, 0x9796, word(data, 0x85a0 + platoon * 2))
        store(data, 0x9798, word(data, 0x7d2a + platoon * 2))
    if word(data, 0x7ae0) == actor:
        return panel(data, actor, screen)
    return bytes(data), screen


def without_stack(data):
    return data[:STACK_BEGIN] + data[STACK_END:]
