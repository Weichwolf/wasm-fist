"""Independent complete mission-start preparation model and actual DOS observer."""
import copy
import struct

from original_unit_oracle import DGROUP, IMAGE_SHA256
from reference_images import load_image
from test_ground import contact
from test_original_mission_ready import complete_expected, ground_reset
from test_projectile_flight import Pool, line
from test_vehicle_start import step


_IMAGE = load_image('fist_dat_image.bin', IMAGE_SHA256)


def words(offset, count):
    return struct.unpack_from('<' + str(count) + 'H', _IMAGE, DGROUP + offset)


TREE_MASKS = words(0x9322, 4)
TREE_BASE = words(0x932a, 4)
TARGET_EXTENTS = words(0x9ecf, 8)
TARGET_SCALES = words(0x9ef7, 8)
TARGET_LIMITS = _IMAGE[DGROUP + 0x9edf:DGROUP + 0x9edf + 8]
ARTILLERY_EXTENTS = words(0x9cdf, 2)
HEIGHT_TYPES = (21, 23, 25, 26, 27)
RELEASE_TYPES = (4, 5, 6, 8, 11, 13, 17, 18, 19)


def preparation_lines(pool):
    state = getattr(pool, 'preparation', None)
    if state is None:
        return ''
    output = line('preparation', [state['trees'], state['counted'],
                                 len(state['artillery'][0]), len(state['artillery'][1])])
    for side, entries in enumerate(state['artillery']):
        values = [*entries, *([(0, 65535, 0, 0)] * (4 - len(entries)))]
        output += line('artillery', [side, *(value for entry in values for value in entry)])
    return output


def prepare(world, side, pixels, link=0):
    if side < 1 or side > 65536 or side & (side - 1) or len(pixels) != side * side:
        return -1, world
    prepared = copy.deepcopy(world)
    pool, objects, _, seeds, cursor = prepared
    pool.preparation = {'trees': 0, 'counted': 0, 'artillery': [[], []]}
    for index, (slot, generation) in enumerate(pool.registry):
        if slot == 65535:
            continue
        allocation, raw = objects[slot]
        kind = allocation[0]
        if allocation != (kind, slot, index, generation):
            raise AssertionError('Independent model encountered an invalid physical binding')
        if kind < 4:
            objects[slot] = allocation, bytearray(ground_reset(raw, link))
            continue
        if kind == 16:
            pool.preparation['counted'] += 1
            continue
        if kind in RELEASE_TYPES:
            pool.release(allocation)
            raw[22] |= 1
            continue
        if kind not in HEIGHT_TYPES:
            return 2, world
        x, y = struct.unpack_from('<ii', raw, 4)
        raw[13] = contact(side, pixels, (x, y, 0))[0]
        if kind == 21:
            variant = raw[25]
            if variant >= 4:
                return -1, world
            heading, cursor = step(seeds, cursor)
            extent, cursor = step(seeds, cursor)
            struct.pack_into('<3H', raw, 16, heading,
                             (extent & TREE_MASKS[variant]) + TREE_BASE[variant], 512)
            raw[22] |= 64
            raw[23] |= 4
            pool.preparation['trees'] += 1
        elif kind == 26:
            variant = raw[25]
            if variant >= 8:
                return -1, world
            struct.pack_into('<2H', raw, 18, TARGET_EXTENTS[variant], TARGET_SCALES[variant])
            raw[27] = TARGET_LIMITS[variant]
            raw[22] |= 78
            raw[23] |= 20
            if variant & 4:
                raw[22] = (raw[22] | 1) & 249
                raw[23] &= 247
        elif kind == 27:
            variant = raw[25]
            side_index = bool(raw[22] & 8)
            entries = pool.preparation['artillery'][side_index]
            if variant >= 2 or len(entries) == 4:
                return -1, world
            struct.pack_into('<H', raw, 18, ARTILLERY_EXTENTS[variant])
            struct.pack_into('<H', raw, 31, 5)
            entries.append(allocation)
            if side_index:
                if variant != 1:
                    raw[22] |= 70
                else:
                    raw[22] &= 249
                    raw[23] &= 231
    return 0, (pool, objects, prepared[2], seeds, cursor)


def inject(world, operation, flags):
    pool, objects, _, _, _ = world
    kind = {1: 4, 2: 8, 3: 19}[operation]
    slots = range(150, 182) if kind == 19 else range(150)
    slot = next((slot for slot in slots if pool.slots[slot][0] == 0), None)
    index = next((index for index, binding in enumerate(pool.registry)
                  if binding == (65535, 0)), None)
    if slot is None or index is None:
        return kind, 1
    pool.slots[slot] = 1, kind
    pool.registry[index] = slot, 1
    raw = bytearray(251 if kind == 19 else 55)
    struct.pack_into('<2H', raw, 0, kind, slot - 150 if kind == 19 else slot)
    raw[22] = flags
    objects[slot] = ((kind, slot, index, 1), raw)
    return kind, 0


def observed_world(owner, machine, objects, prepared=False):
    pool = Pool([])
    state = owner.state(machine).splitlines()
    pool.slots = [tuple(map(int, value.split(':'))) for value in state[1].split()[1:]]
    pool.registry = [tuple(map(int, value.split(':'))) for value in state[2].split()[1:]]
    payloads = {}
    for slot, (index, generation, pointer, size) in objects.items():
        raw = bytearray(machine.mem_read(DGROUP + pointer, size))
        payloads[slot] = ((int.from_bytes(raw[:2], 'little'), slot, index, generation), raw)
    pointers = struct.unpack('<32H', machine.mem_read(DGROUP + 0x6d3c, 64))
    roster = [owner.slot(pointer) if pointer else 65535 for pointer in pointers]
    seeds, cursor = owner.random_state(machine)
    if prepared:
        trees = int.from_bytes(machine.mem_read(DGROUP + 0x930a, 2), 'little')
        counted = int.from_bytes(machine.mem_read(DGROUP + 0x9c89, 2), 'little')
        entries = [[], []]
        for side, (count, base) in enumerate(((0x9ccb, 0x9ccf), (0x9ccd, 0x9cd7))):
            length = int.from_bytes(machine.mem_read(DGROUP + count, 2), 'little')
            if length > 4:
                raise AssertionError('Original artillery list exceeds its complete contract')
            for pointer in struct.unpack('<' + str(length) + 'H', machine.mem_read(DGROUP + base, length * 2)):
                entries[side].append(payloads[owner.slot(pointer)][0])
        pool.preparation = {'trees': trees, 'counted': counted, 'artillery': entries}
    return pool, payloads, roster, list(seeds), cursor


def original_prepare(owner, machine, objects, side, pixels):
    before, actual, transfers = owner.reset(machine, pixels, side=side)
    expected, _, _, _ = complete_expected(before, transfers)
    if len(actual) != 65536 or len(expected) != 65536:
        raise AssertionError('Incomplete original DGROUP observation')
    if any(first != second for offset, (first, second) in enumerate(zip(actual, expected))
           if not 0x8ff0 <= offset < 0x9002):
        raise AssertionError('Complete original DGROUP differs outside actual stack writes')
    return observed_world(owner, machine, objects, True)
