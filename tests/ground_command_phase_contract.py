"""Independent original ab03 prefix and the complete two callback banks.

Child behavior remains with its existing independently proved contract. This
module describes the parent only, including the signed four-word heading sum.
"""
from ground_maneuver_contract import random_draw
from roster_promotion_contract import store, word

AUTOMATIC = (0xab82, 0xac75, 0xab88, 0xad2f, 0xad08, 0xaf97, 0xb011, 0xae66,
             0xae32, 0xae5c, 0xafa2, 0xb017, 0xb0be, 0xb053, 0xaf97, 0xae66)
CONTROLLED = (0xab82, 0xac75, 0xab88, 0xad3b, 0xad08, 0xb111, 0xb011, 0xb111,
              0xb111, 0xb111, 0xb111, 0xb111, 0xb111, 0xb053, 0xb111, 0xb111)


def prefix(before, actor):
    data = bytearray(before)
    store(data, 0x9a08, actor)
    phase_random = random_draw(data)
    store(data, 0x978c, phase_random)
    effect = {'callback': None, 'entry': None, 'automatic': None,
              'heading_sampled': False, 'phase_random': phase_random}
    if data[0x978a]:
        return data, effect
    counter = (data[actor + 0x42] + 1) % 256
    data[actor + 0x42] = counter
    if counter % 16 == 0:
        hull, first, second, third = (word(data, actor + offset)
                                     for offset in (0x26, 0x28, 0x2a, 0x2c))
        total = sum(value if value < 32768 else value - 65536
                    for value in (hull, first, second, third))
        for offset, value in ((0x28, hull), (0x2a, first), (0x2c, second),
                              (0x2e, ((total % 2**32) // 4) % 65536)):
            store(data, actor + offset, value)
        effect['heading_sampled'] = True
    platoon = data[actor + 27]
    if platoon >= 8:
        raise ValueError('Used platoon is outside the authored eight-entry tables')
    store(data, 0x9796, word(data, 0x85a0 + platoon * 2))
    store(data, 0x9798, word(data, 0x7d2a + platoon * 2))
    automatic = bool(word(data, actor + 0x40) & 1)
    index = counter % 16
    effect.update(callback=index, automatic=automatic,
                  entry=(AUTOMATIC if automatic else CONTROLLED)[index])
    return data, effect
