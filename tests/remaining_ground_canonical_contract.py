"""Canonical child inputs and independently predicted typed observations for WI0107.

Prepared actor/resource payloads come from complete original preparation. Explicit
caller stimuli exercise children without inventing a command parent or campaign parser.
Audio-dependent original responses are observed separately from the deliberate typed repair.
"""
import struct

from remaining_ground_contract import station
from remaining_ground_probe_input import BRANCHES, DISPLAY, NONE, SMOKE, encode, line
from roster_promotion_contract import store, word
from support_audio_contract import SMOKE_STOCK, support
from test_ground import contact
from test_original_ground_maneuver import seed_for_draw
from test_vehicle_start import state_lines
from test_weapon_control import update

CASES = ('station_saved', 'support_saved', 'station_target', 'air', 'artillery',
         'smoke_unmatched', 'smoke_matched')


def cases(world):
    physical = world.physical()
    for slot, actor in world.ground_actors().items():
        candidate = next((other for other, pointer in physical.items() if other != slot
                          and (word(world.data, pointer) != 26 or world.data[pointer + 25] < 4)), None)
        if candidate is None:
            raise AssertionError('Canonical world has no valid live target for reaching children')
        for name in CASES:
            yield seeded_case(world, slot, candidate, name)


def seeded_case(world, slot, target_slot, name):
    if name not in CASES:
        raise ValueError('Unknown canonical child stimulus')
    actor = world.ground_actors()[slot]
    raw = bytearray(world.data[actor:actor + 251])
    kind = word(raw, 0)
    targeted = name not in ('station_saved', 'support_saved')
    target = world.physical()[target_slot] if targeted else 0
    selected = name not in ('station_saved', 'support_saved')
    mode, flags, distance = raw[0x43], raw[22], word(raw, 0x99)
    seeds = list(struct.unpack_from('<4H', world.data, 0x1f84))
    cursor = ((word(world.data, 0x1f82) - 0x1f84) // 2 + 1) % 4
    if name in ('air', 'artillery', 'smoke_unmatched', 'smoke_matched'):
        flags |= 8
        mode = 0 if name == 'air' else 4
        distance = 20
        cursor = 0
        seeds[0] = seed_for_draw(0x4120 if name == 'artillery' else 0)
        raw[22], raw[0x43] = flags, mode
        store(raw, 0x99, distance)
        if name.startswith('smoke'):
            stock = SMOKE_STOCK[kind]
            if kind == 2:
                store(raw, stock, 1)
            elif stock is not None:
                raw[stock] = 1
    resources = world.artillery(1)
    # Caller globals are explicit child stimuli. Actor state, identities,
    # ordered resources and ammunition come only from actual preparation.
    f = dict(kind=kind, operation=0 if name.startswith('station') else 1,
             target_type=word(world.data, target) if target else NONE,
             variant=world.data[target + 25] if target else 0,
             target_pose=struct.unpack_from('<2i', world.data, target + 4) if target else (0, 0),
             distance=distance, selected=selected, source=name == 'smoke_matched',
             count=len(resources), air_stock=3,
             guns=tuple([5] * len(resources) + [0] * (4 - len(resources))),
             coarse=False, context=0, clock=1800, tick=12345, gate=65535,
             voice_prior=0, prior=0, air_age=480, artillery_age=480,
             air_delay=0, artillery_delay=0, air_used=0, artillery_used=0,
             full=False, retained=0, invalid=0, display_ticks=41, display_kind=0,
             advisory=19, advisory_until=43, message=47, selector=53,
             configured=True, steps=1, reverse=False, height=17, gun_index=0)
    f.update(raw=bytes(raw), seeds=seeds, cursor=cursor, name=name, actor_slot=slot,
             target_slot=target_slot if target else NONE, actor_pointer=actor,
             target_pointer=target, resources=resources, identity=world.objects[slot][:2])
    before = bytearray(world.data)
    before[actor:actor + 251] = raw
    store(before, actor + 0x97, target)
    for offset, value in ((0x6d34, actor if selected else 0), (0x9fdf, actor if f['source'] else 0),
                          (0x6da2, f['gate']), (0x6cde, f['clock']), (0x9794, f['prior']),
                          (0x452, f['tick']), (0x9fca, f['voice_prior']), (0x9452, f['air_stock']),
                          (0x9456, f['air_delay']), (0x9462, f['clock'] - f['air_age']),
                          (0x945a, 0x9524), (0x9f19, f['clock'] - f['artillery_age']),
                          (0x9ce5, f['artillery_delay']), (0x969e, f['display_ticks']), (0x96a0, 0),
                          (0x9fd7, f['advisory_until']), (0x7a50, f['message']), (0x9fdd, f['selector'])):
        store(before, offset, value)
    before[0x6ce6], before[0x2040], before[0x9fd6] = f['context'], f['coarse'], f['advisory']
    struct.pack_into('<5H', before, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *seeds)
    for i in range(16):
        struct.pack_into('<6H', before, 0x9524 + i * 12, 0, 18 + i, 0, 0, 0, 0)
        struct.pack_into('<7H', before, 0x9dc7 + i * 14, 0, 32 + i, 0, 34 + i, 0, 36 + i, 0)
    f['before'] = bytes(before)
    return f


def encoded_case(f):
    data = bytearray(encode(f))
    struct.pack_into('<2H', data, 76, f['actor_slot'], f['target_slot'])
    return bytes(data)


def predict(world, f):
    """Typed repair prediction from original source-unmatched smoke behavior."""
    actor, before = f['actor_pointer'], bytearray(f['before'])
    if f['operation'] == 0:
        after, effect = station(before, actor, b'')
        _, events = update(before[actor:actor + 251], 0, effect['station'])
        voice = [*effect['request'], 1] if effect['audio'] else [0] * 4
        notice = events[3] != 255 and f['selected'] and f['context'] != 2
        sound = False
        allocation = None
        result = [0] * 13
    else:
        # c047's register response is absent from the simulation API. The
        # stable ART choice is a documented repair, not inferred original intent.
        store(before, 0x9fdf, 0)
        after, effect = support(before, actor, height=0, audio_return=39)
        if effect['height_position'] is not None:
            x, y = struct.unpack('<2i', effect['height_position'])
            height = contact(world.side, world.pixels, (x, y, 0))[0]
            after, effect = support(before, actor, height=height, audio_return=39)
        sound = effect['allocation'] is not None and f['source']
        if sound:
            store(after, 0x9fdd, 39)
        events, voice = [0] * 5, [0] * 4
        allocation = effect['allocation']
        notice = f['selected'] and f['context'] != 2 and effect['support'] in (
            'air_confirmed', 'air_unavailable', 'artillery_not_in_place',
            'artillery_empty', 'artillery_busy', 'artillery_confirmed')
        spent = next((slot for slot, _, _ in f['resources']
                      if word(before, world.physical()[slot] + 31) != word(after, world.physical()[slot] + 31)), NONE)
        if spent != NONE:
            # Canonical first available gun is index0 here; the complete
            # ordered-index clock repair is checked by the domain fixture gate.
            store(after, 0x9f19, f['clock'])
        marker = [allocation[1], allocation[2], 1] if allocation else [NONE, 0, 0]
        result = [BRANCHES.index(effect['support']), SMOKE.index(effect['smoke']),
                  effect['queue_index'] if effect['queue_index'] is not None else 255,
                  spent, *marker, 11 if sound else 0, 0, 0, int(sound), int(notice),
                  int(effect['support'] == 'artillery_confirmed')]
    raw = bytearray(after[actor:actor + 251])
    store(raw, 0x97, word(f['raw'], 0x97))
    output = line('status', [0]) + state_lines([(*f['identity'], raw)])
    output += line('target', [f['target_slot'] if f['target_slot'] != NONE else 0,
                              int(f['target_slot'] != NONE), f['distance'], word(raw, 0x9b)])
    output += line('station', [*events, *voice, int(notice) if f['operation'] == 0 else 0])
    output += line('result', result)
    output += line('history', [word(after, 0x9fca), word(after, 0x969e),
                              DISPLAY.get(word(after, 0x96a0), f['display_kind']), after[0x9fd6],
                              word(after, 0x9fd7), 1, word(after, 0x7a50), word(after, 0x9fdd),
                              word(after, 0x9794)])
    cursor = ((word(after, 0x1f82) - 0x1f84) // 2 + 1) % 4
    output += line('random', [*struct.unpack_from('<4H', after, 0x1f84), cursor])
    output += line('clocks', [(f['clock'] - 480) & 65535, word(after, 0x9462),
                             (f['clock'] - 480) & 65535, word(after, 0x9f19)])
    output += line('configuration', [23, f['air_stock'], 17, f['air_delay'], 19,
                                    f['artillery_delay'], int(f['reverse']), 5, 6, 1])
    for side in range(2):
        air_entries, art_entries = [], []
        for i in range(16):
            if side == 0:
                air_entries.append(f'1:{f["actor_slot"]}:{17+i}')
                art_entries.append(f'1:{31+i}:{33+i}:{35+i}')
            else:
                pointer, pit = struct.unpack_from('<2H', after, 0x9524 + i * 12)
                air_entries.append(f'{int(bool(pointer))}:{f["actor_slot"] if pointer else 0}:{pit}')
                phase, clock = struct.unpack_from('<2H', after, 0x9dc7 + i * 14)
                x, y = struct.unpack_from('<2i', after, 0x9dc7 + i * 14 + 6)
                art_entries.append(f'{phase}:{clock}:{x}:{y}')
        output += line(f'air{side}', air_entries) + line(f'artillery{side}', art_entries)
    guns = [f'{slot}:{word(after, world.physical()[slot]+31)}:1' for slot, _, _ in f['resources']]
    output += line('guns', guns + [f'{NONE}:0:0'] * (4 - len(guns)))
    output += line('pool', [word(after, 0xe294), word(after, 0xe296)])
    if allocation:
        pointer, slot, registry = allocation
        output += line('marker', [20, slot, registry, 1, *struct.unpack_from('<3i', after, pointer + 4),
                                 0, 0, 0, 0, 0, 0, 0])
    output += 'end\n'
    return output, after, effect
