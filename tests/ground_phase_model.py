"""Independent parent composition of the previously proved child contracts.

Synthetic missions exercise production probes without Unicorn or installed game
files. Frozen tables come from the pinned reference under /tmp. Required original
comparisons additionally execute unchanged instructions; this model never does.
"""
import copy
import struct
import types

from automatic_fire_contract import automatic_fire
from ground_bearing_contract import bearing
from ground_command_phase_contract import prefix
from ground_maneuver_contract import idle_turret, maneuver
from ground_phase_probe_contract import (fixture, original_input, pointer,
                                        result_observation, world_observation)
from ground_throttle_contract import throttle
from mission_ready_contract import _IMAGE, prepare
from original_target_acquisition_oracle import TEXT_BASE
from original_unit_oracle import DGROUP
from remaining_ground_contract import remaining
from remaining_ground_corpus import PreparedWorld
from roster_promotion_contract import promote, store, word
from target_acquisition_contract import acquisition
from target_discovery_contract import TABLES, discovery
from test_ground import contact
from test_ground_command import select
from test_ground_goal import assign
from test_ground_route import progress
from test_mission_world import install, record
from test_projectile_flight import Pool
from test_target_discovery import physical
from test_units import scenario_data


def random_state(data):
    return (list(struct.unpack_from('<4H', data, 0x1f84)),
            ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4)


def store_random(data, state):
    seeds, cursor = state
    struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2, *seeds)


def synthetic_world():
    from orders_contract import constructed_blocks
    orders = constructed_blocks(3)
    descriptors = bytearray(orders[1])
    for platoon in range(8):
        struct.pack_into('<4H', descriptors, platoon * 22, 0, 1, 0, 2)
    orders = orders[0], bytes(descriptors)
    records = [record(kind, kind, member=0, platoon=kind) for kind in range(4)]
    scenario = scenario_data(records, orders=orders)
    status, installed = install(records, (1, 2, 32768, 65535), 3, 0)
    assert status == 0
    side, pixels = 512, bytes([17]) * 512**2
    status, state = prepare(installed, side, pixels)
    assert status == 0
    pool, objects, roster, seeds, cursor = state
    data = bytearray(_IMAGE[DGROUP:DGROUP + 65536])
    # Original saved-world observers explicitly install this runtime TEXT
    # segment before executing the per-class station preference lookup.
    store(data, 0x70, TEXT_BASE // 16)
    data[0xe2f7:0xe2f7 + 150] = bytes(150)
    data[0xe38d:0xe38d + 32] = bytes(32)
    data[0xdfbc:0xdfbc + 182 * 4] = bytes(182 * 4)
    metadata = {}
    for slot, (allocation, raw) in objects.items():
        address = pointer(slot)
        data[address:address + len(raw)] = raw
        data[0xe38d + slot - 150] = 1
        index, generation = allocation[2:]
        struct.pack_into('<2H', data, 0xdfbc + index * 4, address, generation)
        metadata[slot] = (index, generation, address, len(raw))
    for index, slot in enumerate(roster):
        store(data, 0x6d3c + index * 2, pointer(slot) if slot != 65535 else 0)
    for offset in (0x930a, 0x9c89, 0x9ccb, 0x9ccd):
        store(data, offset, 0)
    data[0x9ccf:0x9ccf + 16] = bytes(16)
    store(data, 0xe294, len(objects))
    store(data, 0xe296, 0)
    store_random(data, (seeds, cursor))
    for platoon in range(8):
        path = word(data, 0x7d2a + platoon * 2)
        descriptor = word(data, 0x85a0 + platoon * 2)
        data[path:path + 268] = orders[0][platoon * 268:(platoon + 1) * 268]
        data[descriptor:descriptor + 22] = orders[1][platoon * 22:(platoon + 1) * 22]
    return PreparedWorld('synthetic.fsg', 'constant', side, pixels, scenario,
                         bytes(data), metadata, '', ''), state


def model_owner(world):
    owner = types.SimpleNamespace(**vars(TABLES), plane_side=world.side, plane=world.pixels)
    for name, offset, count in (('enemy_text', 0x2daf, 28),
                                ('friendly_text', 0x2de7, 28),
                                ('variant_text', 0x2e1f, 256)):
        setattr(owner, name, struct.unpack_from(f'<{count}H', _IMAGE, TEXT_BASE + offset))
    return owner


def state_from_data(data, world):
    pool = Pool([])
    objects = {}
    used = data[0xe2f7:0xe2f7 + 150] + data[0xe38d:0xe38d + 32]
    registry = [(word(data, 0xdfbc + index * 4), word(data, 0xdfbc + index * 4 + 2))
                for index in range(182)]
    for index, (near, generation) in enumerate(registry):
        pool.registry[index] = (physical(near), generation) if near else (65535, generation)
    for slot, alive in enumerate(used):
        if not alive:
            continue
        address = pointer(slot)
        kind = word(data, address)
        pool.slots[slot] = (1, kind)
        bindings = [(index, generation) for index, (near, generation) in enumerate(registry)
                    if near == address]
        if not bindings:
            index, generation = world.objects[slot][:2]
        else:
            assert len(bindings) == 1
            index, generation = bindings[0]
        objects[slot] = ((kind, slot, index, generation), bytearray(
            data[address:address + (251 if slot >= 150 else 55)]))
    pool.preparation = {'trees': word(data, 0x930a), 'counted': word(data, 0x9c89),
                        'artillery': [[], []]}
    for side in range(2):
        base = (0x9ccf, 0x9cd7)[side]
        for index in range(word(data, 0x9ccb + side * 2)):
            near = word(data, base + index * 2)
            slot = next(slot for slot in objects if pointer(slot) == near)
            pool.preparation['artillery'][side].append(objects[slot][0])
    roster = [next((slot for slot in objects if pointer(slot) == word(data, 0x6d3c + index * 2)),
                   65535) for index in range(32)]
    seeds, cursor = random_state(data)
    return pool, objects, roster, seeds, cursor


def child(data, actor, effect, owner):
    entry = effect['entry']
    if entry is None or entry == 0xb111:
        return data
    raw = bytes(data[actor:actor + 251])
    descriptor = struct.unpack_from('<11H', data, word(data, 0x9796))
    path = word(data, 0x9798)
    route = bytes(data[path:path + 268])
    seeds, cursor = random_state(data)
    leader = word(data, 0x6d3c + raw[27] * 8)
    leader_raw = bytes(data[leader:leader + 251]) if leader else bytes(251)
    case = (raw, leader_raw, 3 if leader == actor else 1 if leader else 0,
            descriptor, route, seeds, cursor)
    if entry == 0xab82:
        status, raw, random = select((raw, descriptor, effect['phase_random'], seeds, cursor))
        assert status == 0
        store_random(data, random)
    elif entry == 0xac75:
        status, raw = assign(case)
        assert status == 0
    elif entry == 0xab88:
        target = word(raw, 0x97)
        raw = bearing(raw, descriptor, struct.unpack_from('<2i', data, target + 4) if target else None,
                      data[0x2040])
    elif entry in (0xad2f, 0xad3b):
        raw, _ = throttle(raw, descriptor, int(entry == 0xad3b))
    elif entry == 0xad08:
        status, raw, route = progress(case)
        assert status == 0
        data[path:path + 268] = route
    elif entry in (0xaf97, 0xafa2):
        return automatic_fire(data, actor, entry=entry)[0]
    elif entry in (0xae66, 0xb017):
        return (maneuver if entry == 0xae66 else idle_turret)(data, actor)[0]
    elif entry == 0xb053:
        return promote(data, actor)
    elif entry in (0xae5c, 0xb0be):
        after, support_effect = remaining(data, actor, _IMAGE[TEXT_BASE:TEXT_BASE + 65536],
                                          entry=entry, height=0, audio_return=39)
        if support_effect['height_position'] is not None:
            xy = struct.unpack('<2i', support_effect['height_position'])
            height = contact(owner.plane_side, owner.plane, (*xy, 0))[0]
            after, _ = remaining(data, actor, _IMAGE[TEXT_BASE:TEXT_BASE + 65536],
                                  entry=entry, height=height, audio_return=39)
        return after
    elif entry in (0xb011, 0xae32):
        registry = struct.unpack_from('<364H', data, 0xdfbc)[::2]
        objects = {near: bytes(data[near:near + (251 if word(data, near) < 4 else 55)])
                   for near in registry if near}
        if entry == 0xb011:
            change = discovery(owner, actor, raw, registry, objects, data[0x6dae],
                               owner.plane_side, owner.plane, coarse=data[0x2040],
                               gate=word(data, 0x6da2), selected=word(data, 0x6d34),
                               clock=word(data, 0x452), last_voice=word(data, 0x9fca))
        else:
            change = acquisition(owner, actor, raw, objects, (seeds, cursor), descriptor[0],
                                 owner.plane_side, owner.plane, automatic=True,
                                 candidate=word(raw, 0x9d), selected=word(data, 0x6d34),
                                 gate=word(data, 0x6da2), clock=word(data, 0x452),
                                 last_voice=word(data, 0x9fca), display=(word(data, 0x969e),
                                                                       word(data, 0x96a0)))
            store_random(data, change['random'])
            struct.pack_into('<2H', data, 0x969e, *change['display'])
        raw = change['actor']
        store(data, 0x9fca, change['last_voice'])
    else:
        raise AssertionError(f'Unmodeled complete callback {entry:#x}')
    data[actor:actor + 251] = raw
    return data


def observation(world, f, index=0, before=None, saved=None):
    before = original_input(world, f) if before is None else before
    actor = pointer(f['actor'])
    data, effect = prefix(before, actor)
    owner = model_owner(world)
    after = child(data, actor, effect, owner)
    effect['diagnostic'] = word(before, 0x7ae0) == actor
    fields = copy.deepcopy(f)
    output = f'case {index} 0\n' + result_observation(owner, before, after, actor, effect, fields)
    state = state_from_data(after, world)
    original_raw = {slot: bytes(raw) for slot, (_, raw) in state[1].items()}
    saved = saved or {slot: (f['raw'] if slot == f['actor'] else
                            world.data[pointer(slot):pointer(slot) + 251])
                      for slot, (_, raw) in state[1].items() if word(raw, 0) < 4}
    for slot, (_, raw) in state[1].items():
        if word(raw, 0) < 4:
            store(raw, 0x97, word(saved[slot], 0x97))
            store(raw, 0x9d, word(saved[slot], 0x9d))
    orders = tuple(b''.join(after[word(after, table + platoon * 2):
                                  word(after, table + platoon * 2) + size]
                           for platoon in range(8))
                   for table, size in ((0x7d2a, 268), (0x85a0, 22)))
    if not f['orders_loaded']:
        orders = None
    return output + world_observation(state, orders, data=after, f=fields,
                                      original_raw=original_raw), after, effect
