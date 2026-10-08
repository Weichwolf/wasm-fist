#!/usr/bin/env python3
"""Consuming shared station/support transitions and explicit original defect repairs.

Original models from closed0104/0106 predict complete actor and queue fields.
This probe does not install the still-incomplete command parent or dispatch queued strikes.
"""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from original_object_pool_oracle import COUNT, LONG_BASE, REGISTRY, SHORT_BASE
from remaining_ground_contract import preferences, station
from remaining_ground_probe_input import BRANCHES, DISPLAY, NONE, SMOKE, encode, line
from roster_promotion_contract import store, word
from support_audio_contract import SMOKE_STOCK, support
from test_original_ground_maneuver import seed_for_draw
from test_object_pool import EXTENDED
from test_vehicle_start import state_lines, step
from test_weapon_control import update, weapon_start

BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
RESULT = None


def fixture(kind=0, **changes):
    fields = dict(operation=1, selected=True, source=True, coarse=False, target_type=5,
                  variant=0, context=0, clock=1800, tick=12345, gate=65535,
                  voice_prior=0, prior=0, air_age=480, artillery_age=480,
                  air_stock=1, air_delay=0, artillery_delay=0, count=1,
                  air_used=0, artillery_used=0, full=False, guns=(2, 2, 2, 2),
                  draw=0, cursor=0, retained=0, invalid=0,
                  target_pose=(123456789, -123456789), display_ticks=41, display_kind=0,
                  advisory=19, advisory_until=43, message=47, selector=53,
                  configured=True, steps=1, reverse=False, height=17, gun_index=0,
                  mode=4, flags=8, distance=20, stock=2, heading=0,
                  velocity=(0, 0), pose=(0, 0), selected_station=0, loaded=0,
                  rounds=(15, 20, 2000, 5), ready_stock=2, countdown=71)
    fields.update(changes)
    raw = bytearray(weapon_start(kind, selected=fields['selected_station'],
                                loaded=fields['loaded'], countdown=fields['countdown'],
                                rounds=fields['rounds'], stock=fields['ready_stock']))
    raw[22], raw[0x43] = fields['flags'], fields['mode']
    store(raw, 0x97, 0x4321)  # Opaque saved word, deliberately not a live near pointer.
    store(raw, 0x99, fields['distance'])
    store(raw, 0x26, fields['heading'])
    struct.pack_into('<2i', raw, 4, *fields['pose'])
    struct.pack_into('<2h', raw, 0x59, *fields['velocity'])
    stock_offset = SMOKE_STOCK[kind]
    if kind == 2:
        store(raw, stock_offset, fields['stock'])
    elif stock_offset is not None:
        raw[stock_offset] = fields['stock']
    fields['kind'], fields['raw'] = kind, bytes(raw)
    seeds = [1, 2, 32768, 65535]
    seeds[fields['cursor'] % 4] = seed_for_draw(fields['draw'])
    fields['seeds'] = seeds
    return fields




def original_fixture(f):
    """Declared prepared typed input; actual pool layout uses original arena classes."""
    data = bytearray(65536)
    short, extended = [], []
    def allocate(kind, registry, generation):
        arena = extended if kind in EXTENDED else short
        slot = next(i for i in range(32 if arena is extended else 150) if i not in arena)
        arena.append(slot)
        pointer = (LONG_BASE + slot * 251) if arena is extended else (SHORT_BASE + slot * 55)
        flat = 150 + slot if arena is extended else slot
        size = 251 if arena is extended else 55
        data[pointer:pointer + size] = struct.pack('<HH', kind, slot) + bytes(size - 4)
        store(data, REGISTRY + registry * 4, pointer)
        store(data, REGISTRY + registry * 4 + 2, generation)
        data[(0xe38d if arena is extended else 0xe2f7) + slot] = 1
        return pointer, flat
    actor, _ = allocate(f['kind'], 0, 1)
    data[actor:actor + 251] = f['raw']
    if f['invalid'] == 11 and f['kind'] == 2:
        store(data, actor + 0xb4, 65535)
    target, target_slot = allocate(f['target_type'], 1, 2) if f['target_type'] != NONE else (0, 0)
    if target:
        struct.pack_into('<2i', data, target + 4, *f['target_pose'])
        data[target + 25] = f['variant']
    guns = [allocate(27, i + 2, i + 3) for i in range(4)]
    for i, (gun, _) in enumerate(guns):
        store(data, gun + 31, f['guns'][i])
        store(data, 0x9cd7 + i * 2, gun)
    target_live = bool(target) and not f['retained'] & 1
    gun_live = [True] * 4
    def release(pointer, flat, registry, generation, reuse):
        long = flat >= 150
        arena, slot = (extended, flat - 150) if long else (short, flat)
        if not reuse:
            arena.remove(slot)
            data[(0xe38d if long else 0xe2f7) + slot] = 0
            store(data, REGISTRY + registry * 4, 0)
            store(data, REGISTRY + registry * 4 + 2, generation - 1)
    if target and f['retained'] & 1:
        release(target, target_slot, 1, 2, f['retained'] & 2)
    i = f['gun_index']
    if f['retained'] & 4:
        gun_live[i] = False
        release(*guns[i], i + 2, i + 3, f['retained'] & 8)
    if f['retained'] & 16:
        allocate(27, i + 2, i + 3)  # Retain the old physical gun as a real orphan.
    if f['full']:
        while len(short) < 150:
            registry = next(i for i in range(COUNT) if (word(data, REGISTRY + i * 4),
                            word(data, REGISTRY + i * 4 + 2)) == (0, 0))
            allocate(18, registry, 1)
    store(data, 0xe294, len(short))
    store(data, 0xe296, len(extended))
    store(data, actor + 0x97, target if target_live else 0)
    fields = ((0x6d34, actor if f['selected'] else 0), (0x9fdf, actor if f['source'] else 0),
              (0x6da2, f['gate']), (0x6cde, f['clock']), (0x9794, f['prior']),
              (0x452, f['tick']), (0x9fca, f['voice_prior']), (0x9452, f['air_stock']),
              (0x9456, f['air_delay']), (0x9462, f['clock'] - f['air_age']), (0x945a, 0x9524),
              (0x9ccd, f['count']), (0x9f19, f['clock'] - f['artillery_age']),
              (0x9ce5, f['artillery_delay']), (0x969e, f['display_ticks']), (0x96a0, 0),
              (0x9fd7, f['advisory_until']), (0x7a50, f['message']), (0x9fdd, f['selector']))
    for offset, value in fields:
        store(data, offset, value)
    data[0x6ce6], data[0x2040], data[0x9fd6] = f['context'], f['coarse'], f['advisory']
    struct.pack_into('<5H', data, 0x1f82, 0x1f84 + ((f['cursor'] + 3) % 4) * 2, *f['seeds'])
    for i in range(16):
        struct.pack_into('<6H', data, 0x9524 + i * 12,
                         actor if i < f['air_used'] else 0, 18 + i, 0, 0, 0, 0)
        struct.pack_into('<7H', data, 0x9dc7 + i * 14,
                         1 if i < f['artillery_used'] else 0, 32 + i, 0, 34 + i, 0, 36 + i, 0)
    return data, actor, target_slot, target_live, guns, gun_live




def predict(f):
    data, actor, target_slot, target_live, guns, gun_live = original_fixture(f)
    output = ''
    air_clock0 = art_clock0 = (f['clock'] - 480) & 65535
    for iteration in range(f['steps']):
        clock, tick = (f['clock'] + iteration * 1800) & 65535, (f['tick'] + iteration * 30) & 65535
        store(data, 0x6cde, clock)
        store(data, 0x452, tick)
        before = bytes(data)
        events, logical_voice = [0] * 5, [0] * 5
        result = [0] * 13
        common_failure = f['invalid'] in (1, 2, 3)
        variant_failure = f['operation'] == 0 and target_live and f['target_type'] == 26 and f['variant'] >= 4
        admitted = (f['flags'] & 8 and target_live and f['distance'] >= 20
                    and ((clock - word(data, 0x9794)) & 65535) >= 1800)
        seeds = list(struct.unpack_from('<4H', data, 0x1f84))
        cursor = ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4
        draw, _ = step(seeds, cursor)
        smoke_called = admitted and f['mode'] == 4 and draw >> 8 <= 64
        stock = 65535 if f['invalid'] == 11 else (word(data, actor + SMOKE_STOCK[f['kind']])
                if f['kind'] == 2 else data[actor + SMOKE_STOCK[f['kind']]] if f['kind'] != 3 else 1)
        smoke_created = smoke_called and stock != 0 and not f['full']
        request_called = admitted and (smoke_called or (draw & 255) <= (76 if f['mode'] == 4 else 12))
        artillery_scan = request_called and (smoke_called or draw & 32) and (
            ((clock - word(data, 0x9f19)) & 65535) >= 480)
        failing_support = f['operation'] == 1 and (
            (admitted and f['invalid'] == 6) or
            (smoke_called and f['invalid'] == 11 and f['kind'] in (0, 1)) or
            (smoke_created and f['invalid'] == 4) or
            (request_called and not f['configured']) or
            (artillery_scan and (f['invalid'] == 8 or
             (f['count'] > 0 and (f['invalid'] == 9 or
              (gun_live[0] and f['invalid'] in (10, 12)))))))
        failed = common_failure if f['operation'] != 2 else False
        failed |= variant_failure or failing_support
        effect = dict(support='not_called', smoke='not_called', allocation=None, audio=False,
                      queue_index=None)
        if not failed and f['operation'] == 0:
            data, effect = station(data, actor, b'')
            chosen = preferences(f['kind'], f['target_type'] if target_live else 0, f['variant'])
            selected = next((s for s in chosen if word(before, actor + (0xac if f['kind'] == 2 else 0xad) + s)), chosen[-1])
            _, events = update(before[actor:actor + 251], 0, selected)
            logical_voice = [*effect['request'], 1] if effect['audio'] else [0, 0, 0, 0]
            logical_voice += [int(events[3] != 255 and f['selected'] and f['context'] != 2)]
        elif not failed and f['operation'] == 1:
            # The deliberate repair always uses the original unmatched source
            # smoke return39, independently of the separately emitted packet.
            actual_ammo = [word(data, gun + 31) for gun, _ in guns]
            for i, (gun, _) in enumerate(guns):
                if not gun_live[i]:
                    store(data, gun + 31, 0)
            data, effect = support(data, actor, height=f['height'], audio_return=39)
            for i, (gun, _) in enumerate(guns):
                if not gun_live[i] and (effect['allocation'] is None or effect['allocation'][1] != guns[i][1]):
                    store(data, gun + 31, actual_ammo[i])
            chosen = next((i for i in range(f['count']) if gun_live[i] and actual_ammo[i]), None)
            art = effect['support'].startswith('artillery_') and effect['support'] not in (
                'artillery_cooldown', 'artillery_not_in_place', 'artillery_empty')
            if art:
                # Repair index2/3 only; leave every other predicted field intact.
                data[0x9f19:0x9f1c] = before[0x9f19:0x9f1c]
                store(data, 0x9f19, clock)
            allocation = effect['allocation']
            marker = [allocation[1], allocation[2], 1] if allocation else [NONE, 0, 0]
            result = [BRANCHES.index(effect['support']), SMOKE.index(effect['smoke']),
                      effect['queue_index'] if effect['queue_index'] is not None else 255,
                      guns[chosen][1] if art else NONE, *marker,
                      11 if effect['audio'] else 0, 0, 0, int(effect['audio']),
                      int(f['selected'] and f['context'] != 2 and effect['support'] in (
                          'air_confirmed', 'air_unavailable', 'artillery_not_in_place',
                          'artillery_empty', 'artillery_busy', 'artillery_confirmed')),
                      int(effect['support'] == 'artillery_confirmed')]
            logical_voice = [0] * 5
        elif not failed and f['operation'] == 2:
            for i in range(16):
                store(data, 0x9524 + i * 12, 0)
                store(data, 0x9dc7 + i * 14, 0)
            air_clock0 = art_clock0 = (clock - 480) & 65535
            store(data, 0x9462, air_clock0)
            store(data, 0x9f19, art_clock0)
        if failed:
            events, logical_voice = [0] * 5, [0] * 5
        raw = bytearray(data[actor:actor + 251])
        store(raw, 0x97, word(f['raw'], 0x97))
        actor_lines = state_lines([(0, 1, raw)])
        if f['invalid'] == 2:
            lines = actor_lines.splitlines()
            index = next(i for i, s in enumerate(lines) if s.startswith('control '))
            lines[index] = lines[index].rsplit(' ', 1)[0] + ' 0'
            lines[5] = 'components'
            actor_lines = '\n'.join(lines) + '\n'
        if f['invalid'] == 11 and f['kind'] != 2:
            lines = actor_lines.splitlines()
            values = lines[3].split()
            # weapons follows state/ground/control.
            values[5] = '65535'
            lines[3] = ' '.join(values)
            actor_lines = '\n'.join(lines) + '\n'
        output += line('status', [-1 if failed else 0]) + actor_lines
        runtime_slot = target_slot if target_live or failed else 0
        output += line('target', [1 if f['invalid'] == 3 else runtime_slot,
                                 int(target_live and f['invalid'] != 3), f['distance'], word(raw, 0x9b)])
        output += line('station', [*events, *logical_voice])
        output += line('result', result)
        display_kind = DISPLAY.get(word(data, 0x96a0), f['display_kind'])
        output += line('history', [word(data, 0x9fca), word(data, 0x969e), display_kind,
                                  data[0x9fd6], word(data, 0x9fd7), 1, word(data, 0x7a50),
                                  word(data, 0x9fdd), word(data, 0x9794)])
        words = struct.unpack_from('<4H', data, 0x1f84)
        cursor = ((word(data, 0x1f82) - 0x1f84) // 2 + 1) % 4
        output += line('random', [*words, 4 if f['invalid'] == 6 else cursor])
        output += line('clocks', [air_clock0, word(data, 0x9462), art_clock0, word(data, 0x9f19)])
        output += line('configuration', [23, f['air_stock'], 17, f['air_delay'], 19,
                                        f['artillery_delay'], int(f['reverse']),
                                        6 if f['reverse'] else 5, 5 if f['reverse'] else 6,
                                        int(f['configured'] or f['operation'] == 2)])
        for side in range(2):
            cleared = f['operation'] == 2
            air_entries, art_entries = [], []
            for i in range(16):
                if side == 0:
                    air_entries.append(f'{0 if cleared else 1}:{0 if cleared else 150}:{17+i}')
                    art_entries.append(f'{0 if cleared else 1}:{31+i}:{33+i}:{35+i}')
                else:
                    pointer, pit = struct.unpack_from('<2H', data, 0x9524 + i * 12)
                    air_entries.append(f'{int(bool(pointer))}:{150 if pointer else 0}:{pit}')
                    phase, time = struct.unpack_from('<2H', data, 0x9dc7 + i * 14)
                    x, y = struct.unpack_from('<2i', data, 0x9dc7 + i * 14 + 6)
                    art_entries.append(f'{phase}:{time}:{x}:{y}')
            output += line(f'air{side}', air_entries) + line(f'artillery{side}', art_entries)
        output += line('guns', [f'{slot}:{word(data, gun+31)}:{int(gun_live[i] and not (f["invalid"] == 9 and i == 0))}'
                               for i, (gun, slot) in enumerate(guns)])
        output += line('pool', [word(data, 0xe294), word(data, 0xe296)])
        if not failed and effect.get('allocation'):
            pointer, slot, registry = effect['allocation']
            x, y, altitude = struct.unpack_from('<3i', data, pointer + 4)
            output += line('marker', [20, slot, registry, 1, x, y, altitude, 0, 0, 0, 0, 0, 0, 0])
        output += 'end\n'
    return output


class RemainingGroundTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-remaining-ground-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_remaining_ground_probe')])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD / 'wasm/fist_remaining_ground_probe.js')])
        cls.counts = collections.Counter()
        cls.digest = hashlib.sha256()

    def compare(self, cases):
        cases = list(cases)
        self.assertTrue(cases)
        payload = struct.pack('<I', len(cases)) + b''.join(map(encode, cases))
        expected = ''.join(map(predict, cases))
        path = pathlib.Path(self.temp.name) / 'cases.bin'
        path.write_bytes(payload)
        for command in self.commands:
            result = subprocess.run([*command, str(path)], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0, result.stderr)
            if result.stdout != expected:
                actual_lines, expected_lines = result.stdout.splitlines(), expected.splitlines()
                mismatch = next((i for i, (a, b) in enumerate(zip(actual_lines, expected_lines)) if a != b),
                                min(len(actual_lines), len(expected_lines)))
                self.fail(f'{command[0]} mismatch line {mismatch}: '
                          f'actual={actual_lines[mismatch:mismatch+2]!r}, '
                          f'expected={expected_lines[mismatch:mismatch+2]!r}')
            self.counts[command[0]] += sum(f['steps'] for f in cases)
        self.digest.update(payload)
        self.digest.update(expected.encode())

    def batches(self, cases):
        iterator = iter(cases)
        while batch := list(itertools.islice(iterator, 512)):
            self.compare(batch)

    def test_authored_preferences_complete_ammunition_masks(self):
        def cases():
            for kind, target, mask, selected, loaded in itertools.product(
                    range(4), (NONE, *range(28)), range(16), range(0, 8, 2), range(0, 8, 2)):
                for variant in (range(4) if target == 26 else (0,)):
                    yield fixture(kind, operation=0, target_type=target, variant=variant,
                                  rounds=tuple(65535 if mask & (1 << i) else 0 for i in range(4)),
                                  selected_station=selected, loaded=loaded, flags=0)
        self.batches(cases())

    def test_complete_selected_loaded_bytes_and_voice_gate_words(self):
        self.batches(fixture((selected + loaded) % 4, operation=0, flags=0,
                             selected_station=selected, loaded=loaded,
                             target_type=(1, 5, 26, 27)[selected % 4], variant=loaded % 4)
                     for selected, loaded in itertools.product(range(256), repeat=2))
        self.batches(fixture(value % 4, operation=0, flags=(value >> 8) & 255,
                             gate=value, tick=value, voice_prior=(value - (29 + value % 3)) & 65535,
                             context=value & 255, selected=value % 2 == 1,
                             selected_station=255, loaded=255)
                     for value in range(65536))

    def test_complete_rng_words_and_stream_cursors(self):
        self.batches(fixture(value % 4, draw=value, cursor=(value >> 2) % 4,
                             mode=4 if value & 1 else 0, stock=(value >> 8) & 255,
                             source=value % 3 == 0, clock=value,
                             prior=(value - 1800) & 65535)
                     for value in range(65536))

    def test_used_variant_rejection_and_real_target_reuse(self):
        self.batches(fixture(kind, operation=0, target_type=26, variant=variant, flags=0)
                     for kind, variant in itertools.product(range(4), range(4, 256)))
        self.compare([fixture(kind, operation=op, retained=retained, target_type=26, variant=255,
                              flags=0 if op == 0 else 8)
                      for kind, op, retained in itertools.product(range(4), range(2), (1, 3))])

    def test_support_stock_capacity_and_audio_source_independence(self):
        self.compare([fixture(kind, stock=stock, full=full, source=source, selected=selected,
                              coarse=coarse, heading=65535, velocity=(-32768, 32767),
                              pose=(2147483647, -2147483648))
                      for kind, stock, full, source, selected, coarse in itertools.product(
                          range(4), (0, 1, 255), (False, True), (False, True),
                          (False, True), (False, True))])

    def test_ordered_guns_queue_exits_and_typed_clock_repair(self):
        self.batches(fixture(kind, guns=tuple(65535 if mask & (1 << i) else 0 for i in range(4)),
                             count=count, artillery_used=used, artillery_delay=busy,
                             selected=selected, context=context)
                     for kind, mask, count, used, busy, selected, context in itertools.product(
                         range(4), range(16), range(5), (0, 1, 15, 16), (0, 1),
                         (False, True), (0, 2)))

    def test_admission_cooldown_wrap_and_conditional_random(self):
        self.batches(fixture(kind, mode=mode, flags=flags, distance=distance,
                             clock=clock, prior=(clock - age) & 65535, draw=draw,
                             air_age=side_age, artillery_age=side_age)
                     for kind, mode, flags, distance, clock, age, draw, side_age in itertools.product(
                         range(4), (0, 4), (0, 8), (19, 20), (0, 65535),
                         (1799, 1800, 65535), (0, 12, 13, 32, 64, 76, 77, 0x4100, 65535),
                         (479, 480)))

    def test_air_queue_capacity_and_configuration_inputs(self):
        self.batches(fixture(kind, mode=0, draw=0, air_stock=stock, air_delay=delay,
                             air_used=used, selected=selected, context=context)
                     for kind, stock, delay, used, selected, context in itertools.product(
                         range(4), (0, 1, 65535), (0, 1, 65535), range(17),
                         (False, True), (0, 2)))
        self.compare([fixture(kind, operation=2, reverse=reverse, air_used=16, artillery_used=16,
                              count=4, configured=False, clock=clock)
                      for kind, reverse, clock in itertools.product(range(4), (False, True),
                                                                  (0, 479, 480, 65535))])

    def test_lifetime_capture_orphans_and_retained_sequences(self):
        self.compare([fixture(kind, retained=retained, gun_index=index, count=4,
                              guns=tuple(2 if i == index else 0 for i in range(4)), steps=3)
                      for kind, retained, index in itertools.product(range(4), (4, 12, 16), range(4))])
        self.compare([fixture(kind, mode=mode, stock=1, count=4, steps=17)
                      for kind, mode in itertools.product(range(4), (0, 4))])

    def test_atomic_invalid_inputs(self):
        self.compare([fixture(kind, invalid=invalid) for kind, invalid in itertools.product(
            range(4), (1, 2, 3, 4, 6, 8, 9, 10, 11, 12))])
        self.compare([fixture(kind, operation=0, flags=0, invalid=invalid)
                      for kind, invalid in itertools.product(range(4), (1, 2, 3))])
        self.compare([fixture(kind, configured=False) for kind in range(4)])

    def test_unused_invalid_inputs_do_not_change_admission_order(self):
        cases = []
        for kind in range(4):
            cases.extend((fixture(kind, invalid=4, full=True),
                          fixture(kind, configured=False, mode=0, draw=13),
                          fixture(kind, configured=False, flags=0),
                          fixture(kind, configured=False, distance=19),
                          fixture(kind, configured=False, prior=1),
                          fixture(kind, invalid=6, distance=19)))
            if kind != 3:
                cases.append(fixture(kind, invalid=4, stock=0))
            cases.extend(fixture(kind, mode=0, invalid=invalid) for invalid in (8, 9, 10, 12))
        self.compare(cases)

    def test_missing_malformed_and_unknown_operation_output_fails(self):
        valid = struct.pack('<I', 1) + encode(fixture())
        unknown = bytearray(valid)
        unknown[4] = 255
        path = pathlib.Path(self.temp.name) / 'invalid.bin'
        for payload in (b'', bytes(4), valid[:-1], valid + b'\0', bytes(unknown)):
            path.write_bytes(payload)
            for command in self.commands:
                result = subprocess.run([*command, str(path)], capture_output=True, timeout=30)
                self.assertNotEqual(result.returncode, 0)

    @classmethod
    def tearDownClass(cls):
        receipt = {'scope': 'Shared consuming station/support fixtures; full WI0107 acceptance remains open',
                   'counts': dict(cls.counts), 'sha256': cls.digest.hexdigest()}
        cls.receipt = receipt
        print(json.dumps(receipt, sort_keys=True), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--result-json', type=pathlib.Path)
    args, remaining = parser.parse_known_args()
    TARGET, BUILD, NATIVE_PROBE, RESULT = args.target, args.build_root, args.native_probe, args.result_json
    program = unittest.main(argv=[__file__, *remaining], exit=False)
    receipt = getattr(RemainingGroundTests, 'receipt', {})
    receipt.update(passed=program.result.wasSuccessful(), groups=program.result.testsRun,
                   skipped=len(program.result.skipped), failures=len(program.result.failures),
                   errors=len(program.result.errors))
    if RESULT:
        RESULT.write_text(json.dumps(receipt, indent=2) + '\n')
    raise SystemExit(0 if program.result.wasSuccessful() and not program.result.skipped else 1)
