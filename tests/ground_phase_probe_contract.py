"""Whole prepared-world input/output contract for the complete command parent.

Original instructions supply complete world observations. Event predictions use
previously proved independent child contracts. Typed reference and deliberate
repair expectations are explicit; unavailable coverage is never called success.
"""
import copy
import struct

from automatic_fire_contract import automatic_fire, MISSILE_MESSAGES
from ground_command_phase_contract import prefix
from ground_throttle_contract import throttle
from mission_ready_contract import observed_world, preparation_lines
from orders_contract import orders_lines, scenario_order_blocks
from original_unit_oracle import DGROUP
from remaining_ground_contract import station
from remaining_ground_probe_input import DISPLAY, BRANCHES, SMOKE
from roster_promotion_contract import store, word
from support_audio_contract import support
from target_acquisition_contract import acquisition, ACQUISITION_THRESHOLDS
from target_discovery_contract import discovery
from test_ground import contact
from test_mission_world import payload_lines
from test_projectile_flight import line
from test_target_discovery import physical, pointer
from test_vehicle_start import random_line
from test_weapon_control import update

NONE = 65535
HEADER = 80
CASE_BYTES = HEADER + 502 + 2320
FIELDS = dict(actor=0, target=2, candidate=4, diagnostic=6, selected=8, source=10,
              tick=12, clock=14, gate=16, saved=18, voice_prior=36, request_prior=38,
              notice_ticks=64, message_ticks=66, advisory_until=70, selector=72)
BYTES = dict(inhibition=20, context=21, link=22, coarse=23, cursor=32, retain=33,
             orders_loaded=34, invalid=35, configured=60, reverse=61,
             target_override=62, notice_kind=63, advisory_code=68, advisory_active=69,
             diagnostic_lifetime=74, target_lifetime=75, candidate_lifetime=76,
             no_height=77)
PAIRS = dict(air_stock=40, air_delay=44, artillery_delay=48, air_clock=52,
             artillery_clock=56)


def fixture(world, slot, callback, automatic, **changes):
    actor = world.ground_actors()[slot]
    raw = bytearray(world.data[actor:actor + 251])
    raw[0x42] = (callback - 1) % 256
    store(raw, 0x40, (word(raw, 0x40) & 65534) | automatic)
    f = dict(actor=slot, target=NONE, candidate=NONE, diagnostic=slot, selected=slot,
             source=NONE, tick=12345, clock=1800, gate=65535, saved=0xa55a,
             inhibition=0, context=0, link=0, coarse=0, cursor=0, retain=0,
             orders_loaded=1, invalid=0, configured=1, reverse=0, target_override=0,
             notice_kind=0, notice_ticks=41, message_ticks=47, advisory_code=19,
             advisory_active=1, advisory_until=43, selector=53, voice_prior=0,
             request_prior=0, seeds=(1, 2, 32768, 65535), air_stock=(3, 3),
             air_delay=(0, 0), artillery_delay=(0, 0), air_clock=(1320, 1320),
             artillery_clock=(1320, 1320), diagnostic_lifetime=0, target_lifetime=0,
             candidate_lifetime=0, no_height=0, raw=bytes(raw), target_raw=bytes(251),
             orders=scenario_order_blocks(world.scenario))
    f.update(changes)
    return f


def encode(f):
    header = bytearray(HEADER)
    for name, offset in FIELDS.items(): store(header, offset, f[name])
    for name, offset in BYTES.items(): header[offset] = f[name]
    for name, offset in PAIRS.items(): struct.pack_into('<2H', header, offset, *f[name])
    struct.pack_into('<4H', header, 24, *f['seeds'])
    if len(f['raw']) != 251 or len(f['target_raw']) != 251:
        raise ValueError('Incomplete snapshot fixture')
    result = bytes(header) + f['raw'] + f['target_raw'] + b''.join(f['orders'])
    if len(result) != CASE_BYTES: raise ValueError('Incomplete parent case')
    return result


def original_input(world, f):
    if f['retain'] or f['invalid'] or any(f[name] for name in (
            'diagnostic_lifetime', 'target_lifetime', 'candidate_lifetime')):
        raise ValueError('Lifecycle/retained fixtures require their explicit episode owner')
    data = bytearray(world.data)
    actor = pointer(f['actor'])
    data[actor:actor + 251] = f['raw']
    if f['target_override']:
        target = pointer(f['target'])
        size = 251 if word(f['target_raw'], 0) < 4 else 55
        data[target:target + size] = f['target_raw'][:size]
    store(data, actor + 0x97, pointer(f['target']) if f['target'] != NONE else 0)
    store(data, actor + 0x9d, pointer(f['candidate']) if f['candidate'] != NONE else 0)
    data[0x978a], data[0x6ce6], data[0x6dae], data[0x2040] = (
        f['inhibition'], f['context'], f['link'], f['coarse'])
    for offset, value in ((0x7ae0, pointer(f['diagnostic']) if f['diagnostic'] != NONE else 0),
                          (0x6d34, pointer(f['selected']) if f['selected'] != NONE else 0),
                          (0x9fdf, pointer(f['source']) if f['source'] != NONE else 0),
                          (0x452, f['tick']), (0x6cde, f['clock']), (0x6da2, f['gate']),
                          (0x97ee, f['saved']), (0x9fca, f['voice_prior']),
                          (0x9794, f['request_prior']), (0x969e, f['notice_ticks']),
                          (0x96a0, 0), (0x7a50, f['message_ticks']),
                          (0x9fd7, f['advisory_until']), (0x9fdd, f['selector'])):
        store(data, offset, value)
    data[0x9fd6] = f['advisory_code']
    for side in range(2):
        for offset, name in ((0x9450, 'air_stock'), (0x9454, 'air_delay'),
                             (0x9ce3, 'artillery_delay'), (0x9460, 'air_clock'),
                             (0x9f17, 'artillery_clock')):
            store(data, offset + side * 2, f[name][side])
        store(data, 0x944c + side * 2, 6 if bool(side) != bool(f['reverse']) else 5)
        store(data, 0x9458 + side * 2, (0x9464, 0x9524)[side])
    for base, stride in ((0x9464, 12), (0x9524, 12), (0x9ce7, 14), (0x9dc7, 14)):
        data[base:base + 16 * stride] = bytes(16 * stride)
    struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((f['cursor'] + 3) % 4) * 2, *f['seeds'])
    for platoon in range(8):
        path = word(data, 0x7d2a + platoon * 2)
        descriptor = word(data, 0x85a0 + platoon * 2)
        data[path:path + 268] = f['orders'][0][platoon * 268:(platoon + 1) * 268]
        data[descriptor:descriptor + 22] = f['orders'][1][platoon * 22:(platoon + 1) * 22]
    return bytes(data)


def reference(near, live=True):
    return (physical(near), 1, int(live)) if near else (0, 0, 0)


def dynamic_payload(raw, allocation):
    kind = allocation[0]
    if kind == 15:
        values = (*allocation, *struct.unpack_from('<3iH', raw, 4),
                  *reference(word(raw, 0x2b)), *reference(word(raw, 0x1a)),
                  word(raw, 0x12), word(raw, 0x14), word(raw, 0x26), word(raw, 0x28),
                  raw[22], raw[23], *struct.unpack_from('<5h', raw, 0x1c),
                  raw[24], raw[25], raw[0x2a])
        return line('surface_air', values)
    if kind == 20:
        return line('support_marker', [*allocation, *struct.unpack_from('<3i3H', raw, 4),
                                       *raw[22:26]])
    return payload_lines(raw, allocation)


def complete_world(owner, machine, world):
    metadata = dict(world.objects)
    data = bytes(machine.mem_read(DGROUP, 65536))
    used = data[0xe2f7:0xe2f7 + 150] + data[0xe38d:0xe38d + 32]
    for slot, alive in enumerate(used):
        if not alive: continue
        address = pointer(slot)
        if slot in metadata and word(data, address) == word(world.data, address): continue
        bindings = [(index, word(data, 0xdfbc + index * 4 + 2)) for index in range(182)
                    if word(data, 0xdfbc + index * 4) == address]
        if len(bindings) != 1: raise AssertionError('Dynamic payload lacks its complete binding')
        index, generation = bindings[0]
        metadata[slot] = (index, generation, address, 251 if slot >= 150 else 55)
    return observed_world(owner, machine, metadata, True)


def world_observation(state, orders, *, data=None, f=None, original_raw=None):
    pool, objects, roster, seeds, cursor = state
    output = pool.state() + random_line(seeds, cursor) + line('roster', roster)
    output += orders_lines(orders) + preparation_lines(pool)
    extras = ''
    for slot, (allocation, raw) in sorted(objects.items()):
        if not pool.slots[slot][0]: continue
        output += line('object', [slot, allocation[0]]) + dynamic_payload(raw, allocation)
        if allocation[0] < 4:
            runtime = original_raw.get(slot, raw) if original_raw is not None else raw
            states = raw[0xb7:0xb9] if allocation[0] in (1, 3) else (0, 0)
            extras += line('ground_extra', [slot, raw[27], raw[28], raw[0x46], raw[0x51],
                           raw[0x44], word(raw, 0x47), word(raw, 0x8e), raw[0x94],
                           word(raw, 0x99), word(raw, 0x9b), *states,
                           *reference(word(runtime, 0x97)), *reference(word(runtime, 0x9d))])
    output += extras
    if data is None:
        output += line('notifications', [0] * 11) + line('support_state', [0] * 3)
    else:
        handle = word(data, 0x96a0)
        notice_kind = DISPLAY.get(handle, 0)
        notice_type = variant = enemy = 0
        if handle in MISSILE_MESSAGES.values():
            notice_kind = {value: index + 1 for index, value in enumerate(MISSILE_MESSAGES.values())}[handle]
            notice_type = 28
        if f.get('installed_message'):
            target = word(data, pointer(f['actor']) + 0x97)
            notice_type = word(data, target)
            variant = data[target + 25] if notice_type == 26 else 0
            enemy = int(bool(data[target + 22] & 8))
        output += line('notifications', [word(data, 0x9fca), word(data, 0x969e),
                       notice_type, variant, enemy, notice_kind, word(data, 0x9fdd),
                       word(data, 0x9fd7), data[0x9fd6], f['advisory_active'], word(data, 0x7a50)])
        output += line('support_state', [f['configured'], word(data, 0x9794), f['reverse']])
    for side in range(2):
        values = [0] * 6 if data is None else [word(data, offset + side * 2) for offset in
                  (0x9450, 0x9454, 0x9ce3, 0x9460, 0x9f17, 0x944c)]
        output += line('support_side', [side, *values])
        for entry in range(16):
            air = (0x9464, 0x9524)[side] + entry * 12
            art = (0x9ce7, 0x9dc7)[side] + entry * 14
            output += line('air', [side, entry, *(reference(word(data, air)) if data else (0, 0, 0)),
                                  word(data, air + 2) if data else 0])
            values = [word(data, art), word(data, art + 2),
                      *struct.unpack_from('<2i', data, art + 6)] if data else [0] * 4
            output += line('art', [side, entry, *values])
    return output


def result_observation(owner, before, after, actor, effect, f):
    child_before, _ = prefix(before, actor)
    entry = effect['entry']
    raw = child_before[actor:actor + 251]
    final = after[actor:actor + 251]
    selected = word(before, 0x6d34) == actor
    target_objects = {pointer(slot): after[pointer(slot):pointer(slot) + (251 if slot >= 150 else 55)]
                      for slot in range(182) if (after[0xe2f7 + slot] if slot < 150 else after[0xe38d + slot - 150])}
    refresh = False
    if entry in (0xad2f, 0xad3b):
        descriptor = struct.unpack_from('<11H', child_before, word(child_before, 0x9796))
        _, refresh = throttle(raw, descriptor, int(entry == 0xad3b))
    output = line('phase', [effect['phase_random'], effect['callback'] if entry else 255,
                           int(bool(effect['automatic'])), int(effect['heading_sampled']),
                           int(effect['diagnostic']), int(refresh)])
    if effect['diagnostic']:
        platoon = final[27]
        descriptor = word(after, 0x85a0 + platoon * 2)
        path = word(after, 0x7d2a + platoon * 2)
        values = [*reference(actor), *reference(word(final, 0x9d)), word(final, 0x57),
                  word(after, descriptor + 6), after[path], f['saved'], word(final, 0x53),
                  final[0x43], final[0x94], final[0x45], platoon, final[28],
                  int(f['inhibition'] != 0), int(word(final, 0x9d) != 0)]
    else: values = [0] * 18
    output += line('diagnostic', values)
    discovery_values, discovery_voice = [0] * 10, [0] * 4
    acquisition_values, acquisition_voice = [0] * 3, [0] * 4
    fire_values, fire_sound = [0, NONE, 0, 0, 0, 0, 0, 0, 0], [0] * 4
    station_values, station_voice = [0, 0, 255, 255, 0, 0], [0] * 4
    support_values, support_sound = [0, NONE, 0, 0, NONE, 0, 0, 255, 0, 0], [0] * 4
    if entry == 0xb011:
        d = discovery(owner, actor, raw, struct.unpack_from('<364H', child_before, 0xdfbc)[::2],
                      target_objects, f['link'], owner.plane_side, owner.plane,
                      coarse=f['coarse'], gate=f['gate'], selected=word(child_before, 0x6d34),
                      clock=f['tick'], last_voice=f['voice_prior'])
        discovery_values = [*reference(d['primary']), *reference(d['secondary']),
                            d['primary_range'], d['secondary_operand'], d['priority'], d['count']]
        if d['requests']: discovery_voice = [*d['requests'][0], 1]
    elif entry == 0xae32:
        seeds = struct.unpack_from('<4H', child_before, 0x1f84)
        cursor = ((word(child_before, 0x1f82) - 0x1f84) // 2 + 1) % 4
        d = acquisition(owner, actor, raw, target_objects, (seeds, cursor),
                        word(child_before, word(child_before, 0x9796)), owner.plane_side, owner.plane,
                        automatic=True, candidate=word(raw, 0x9d),
                        selected=word(child_before, 0x6d34), gate=f['gate'], clock=f['tick'],
                        last_voice=f['voice_prior'], display=(f['notice_ticks'], 0))
        installed = bool(d['transfer'] and d['transfer']['visible'])
        acquisition_values = [int(d['draw'] is not None and (d['draw'] & 255) <= ACQUISITION_THRESHOLDS[word(child_before, word(child_before, 0x9796))]),
                              int(installed), int(bool(d['messages']))]
        if d['requests']: acquisition_voice = [*d['requests'][0], 1]
        f['installed_message'] = bool(d['messages'])
    elif entry in (0xaf97, 0xafa2):
        _, d = automatic_fire(child_before, actor, entry=entry)
        allocation = [15, d['allocation'][1], d['allocation'][2], 1] if d['allocation'] else [28, NONE, 182, 0]
        fire_values = [*allocation, d['rack'] if d['rack'] is not None else 255,
                       int(d['branch'] == 'fire_requested'), int(d['branch'] == 'rack_loading'),
                       int(d['branch'] == 'missile_launched'),
                       int(selected and d['branch'] in ('missile_empty', 'missile_target_rejected', 'missile_capacity'))]
        if d['request']: fire_sound = [7, 0, 0, 1]
    elif entry == 0xae5c:
        _, d = station(child_before, actor, b'')
        _, events = update(raw, 0, d['station'])
        notice = events[3] != 255 and selected and f['context'] != 2
        station_values = [*events, int(notice)]
        if d['request']: station_voice = [*d['request'], 1]
    elif entry == 0xb0be:
        _, d = support(child_before, actor, height=0, audio_return=39)
        allocation = [20, d['allocation'][1], d['allocation'][2], 1] if d['allocation'] else [0, NONE, 0, 0]
        resources = [pointer(slot) for slot, _, _ in world_resources(before)]
        spent = next((physical(p) for p in resources if word(before, p + 31) != word(after, p + 31)), NONE)
        notice = selected and f['context'] != 2 and d['support'] in (
            'air_confirmed', 'air_unavailable', 'artillery_not_in_place',
            'artillery_empty', 'artillery_busy', 'artillery_confirmed')
        support_values = [*allocation, spent, BRANCHES.index(d['support']), SMOKE.index(d['smoke']),
                          d['queue_index'] if d['queue_index'] is not None else 255,
                          int(notice), int(d['support'] == 'artillery_confirmed')]
        if d['allocation'] and f['source'] == f['actor']: support_sound = [11, 0, 0, 1]
    output += line('discovery', discovery_values) + line('discovery_voice', discovery_voice)
    output += line('acquisition', acquisition_values) + line('acquisition_voice', acquisition_voice)
    output += line('fire', fire_values) + line('fire_sound', fire_sound)
    output += line('station', station_values) + line('station_voice', station_voice)
    output += line('support', support_values) + line('support_sound', support_sound)
    return output


def world_resources(data):
    for side in range(2):
        length = word(data, 0x9ccb + side * 2)
        base = 0x9ccf if side == 0 else 0x9cd7
        for index in range(length):
            near = word(data, base + index * 2)
            yield physical(near), index, side


def observation(corpus, owner, world, f, index=0):
    before = original_input(world, f)
    actor = pointer(f['actor'])
    machine = owner.prepared_machine(before)
    after, effect = owner.observe(machine, actor)
    # The accepted support repair uses the source-unmatched ART decision after
    # every smoke attempt. Preserve actual matched-device evidence, then obtain
    # the complete genuine stable-branch return separately; don't hide the
    # difference by dropping queues/resources from observation.
    if effect['entry'] == 0xb0be and f['source'] == f['actor']:
        child, _ = prefix(before, actor)
        stable = bytearray(child)
        store(stable, 0x9fdf, 0)
        _, support_effect = support(stable, actor, height=0, audio_return=39)
        if support_effect['smoke'] != 'not_called':
            unmatched = bytearray(before)
            store(unmatched, 0x9fdf, 0)
            repaired_machine = owner.prepared_machine(bytes(unmatched))
            repaired, repaired_effect = owner.observe(repaired_machine, actor)
            repaired = bytearray(repaired)
            if support_effect['allocation'] is not None: store(repaired, 0x9fdd, 39)
            repaired_machine.mem_write(DGROUP, bytes(repaired))
            effect['typed_support_repair'] = True
            effect['repair_original_parent_returns'] = 1
            effect['matched_original_audio_returns'] = effect['audio_returns']
            machine, after = repaired_machine, bytes(repaired)
    output = line('case', [index, 0]) + result_observation(owner, before, after, actor, effect, f)
    state = complete_world(corpus.owner, machine, world)
    original_raw = {slot: bytes(raw) for slot, (_, raw) in state[1].items()}
    # Saved words remain opaque in C, while captured identities are checked in
    # the separate complete runtime-reference lines above.
    for slot, (_, raw) in state[1].items():
        if word(raw, 0) < 4:
            source = f['raw'] if slot == f['actor'] else world.data[pointer(slot):pointer(slot) + 251]
            store(raw, 0x97, word(source, 0x97))
            store(raw, 0x9d, word(source, 0x9d))
    paths = b''.join(after[word(after, 0x7d2a + platoon * 2):word(after, 0x7d2a + platoon * 2) + 268]
                     for platoon in range(8))
    descriptors = b''.join(after[word(after, 0x85a0 + platoon * 2):word(after, 0x85a0 + platoon * 2) + 22]
                           for platoon in range(8))
    return output + world_observation(state, (paths, descriptors), data=after, f=f,
                                      original_raw=original_raw), effect
