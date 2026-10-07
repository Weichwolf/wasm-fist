#!/usr/bin/env python3
"""World prefix and current-registry visits with actual effect birth/retirement."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_aircraft_death import effect_lines, new_effect
from test_collision import delta
from test_destruction import advance_parent, advance_smoke, fixture as parent_fixture, parent_lines, smoke_lines
from test_object_pool import EXTENDED
from test_projectile_flight import Pool, advance_explosion, line
from test_units import records_from_scenario
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
NONE = 65535


def command(op, *, low=0, kind=0, index=0, value=0, raw=None):
    return op, low, kind, index, value, raw


def bind(kind, index, value=1):
    return command(2, kind=kind, index=index, value=value)


def fixture(commands, *, tick=0, auxiliary=(1, 65535), voice_at=1, last=65535,
            time=(1, 0, 0), pending=255, mode=2, enabled=1, wind=(17, -23),
            seeds=(2, 76, 78, 1), cursor=0):
    return dict(commands=commands, clock=[tick, *auxiliary, voice_at, last, *time, pending, mode],
                enabled=enabled, wind=wind, seeds=seeds, cursor=cursor)


def encode(cases):
    out = struct.pack('<I', len(cases))
    for case in cases:
        out += struct.pack('<5H6B2i4HBI', *case['clock'], case['enabled'], *case['wind'],
                           *case['seeds'], case['cursor'], len(case['commands']))
        for op, low, kind, index, value, raw in case['commands']:
            out += struct.pack('<BB3H', op, low, kind, index, value)
            if op == 7: out += raw
    return out


def muzzle_lines(raw, allocation):
    return line('muzzle', [*allocation[1:], *struct.unpack_from('<3iH', raw, 4),
                           struct.unpack_from('<H', raw, 20)[0],
                           struct.unpack_from('<H', raw, 26)[0], raw[25], raw[22]])


class WorldGolden:
    def __init__(self, case):
        self.case = case
        self.clock = list(case['clock'])
        self.pool = Pool([])
        self.next_entry = 0
        self.visit = (28, NONE, 182, NONE)
        self.objects = {}
        self.words, self.cursor = list(case['seeds']), case['cursor']

    def shared(self):
        return line('clock', self.clock) + line('random', [self.cursor, *self.words]) + self.pool.state()

    def allocation(self, index):
        if index >= 182 or self.pool.registry[index][0] == NONE: return None
        slot, value = self.pool.registry[index]
        return self.pool.slots[slot][1], slot, index, value

    def create(self, kind, index, value):
        slots = range(150, 182) if kind in EXTENDED else range(150)
        slot = next((slot for slot in slots if not self.pool.slots[slot][0]), None)
        if slot is None: return 1
        self.pool.slots[slot] = 1, kind
        self.pool.registry[index] = slot, value
        self.objects.pop(slot, None)
        return 0

    def update(self):
        kind, slot, index, value = self.visit
        if self.allocation(index) != self.visit or slot not in self.objects: return -1, ''
        raw = self.objects[slot]
        output, born = '', []
        def spawn(strength):
            if self.case['enabled'] != 1: return None
            allocation = self.pool.allocate_short(17, low_priority=True)
            if allocation is None: return None
            random, self.cursor = step(self.words, self.cursor)
            smoke = bytearray(55); struct.pack_into('<HH', smoke, 0, 17, allocation[1])
            x, y, z = struct.unpack_from('<3i', raw, 4)
            extent = (strength + (random & 63)) % 65536
            struct.pack_into('<3iH2H', smoke, 4, x, y, delta(z + 768, 0), 0, extent, extent * 4 % 65536)
            self.objects[allocation[1]] = smoke
            born.append(smoke_lines(smoke, allocation))
            return smoke
        released = False
        if kind in (23, 26, 27):
            advance_parent(raw, spawn)
            output = parent_lines(raw, self.visit)
        elif kind == 17:
            released = advance_smoke(raw, self.case['wind'], self.case['enabled'])
            output = smoke_lines(raw, self.visit)
        elif kind == 18:
            counter = (struct.unpack_from('<H', raw, 26)[0] + 1) % 65536
            if counter >= 8:
                counter = 0; raw[25] = (raw[25] + 1) % 256; released = raw[25] >= 7
            struct.pack_into('<H', raw, 26, counter)
            if released: raw[22] |= 1
            output = muzzle_lines(raw, self.visit)
        elif kind == 4:
            frame, countdown, height, released = advance_explosion(raw[25], raw[30], raw[31], raw[32],
                struct.unpack_from('<H', raw, 26)[0], struct.unpack_from('<H', raw, 28)[0])
            raw[25], raw[32] = frame, countdown; struct.pack_into('<H', raw, 28, height)
            if released: raw[22] |= 1
            output = effect_lines((self.visit, raw))
        else: return -1, ''
        if released: self.pool.release(self.visit)
        return 0, output + ''.join(born)

    def apply(self, request):
        op, low, kind, index, value, raw = request
        status, output, due, voice = 0, '', 0, NONE
        if op == 0: self.pool = Pool([]); self.objects = {}
        elif op == 1:
            if kind >= 28 or low > 1: status = -1
            elif low and sum(used for used, _ in self.pool.slots[:150]) >= 120: status = 1
            else:
                vacancy = next((n for n, binding in enumerate(self.pool.registry) if binding == (NONE, 0)), None)
                status = 1 if vacancy is None else self.create(kind, vacancy, 1)
        elif op == 2: status = -1 if kind >= 28 or index >= 182 else self.create(kind, index, value)
        elif op == 3:
            allocation = self.allocation(index)
            if index >= 182: status = -1
            elif allocation is None: status = 1
            else: self.pool.release(allocation)
        elif op == 4: self.next_entry = 0
        elif op == 5:
            current = next((n for n in range(self.next_entry, 182) if self.pool.registry[n][0] != NONE), None)
            if current is None: status = 1; self.next_entry = 182
            else: self.visit = self.allocation(current); self.next_entry = current + 1
        elif op == 6:
            tick, first, second, at, last, minutes, seconds, subticks, pending, mode = self.clock
            if minutes != 255 and (minutes, seconds, subticks) != (0, 0, 0):
                subticks = (subticks - 1) % 256
                if subticks == 255:
                    subticks = 59; seconds = (seconds - 1) % 256
                    if seconds == 255: seconds = 59; minutes = (minutes - 1) % 256
            tick = (tick + 1) % 65536
            if pending != 255 and tick == at:
                due = 1; voice = NONE if mode == 2 else 0x280 | pending
                pending, last = 255, index
            self.clock = [tick, max(first - 1, 0), max(second - 1, 0), at, last, minutes, seconds, subticks, pending, mode]
        elif op == 7:
            allocation = self.allocation(index)
            if allocation is None or allocation[0] not in (4, 17, 18, 23, 26, 27) or struct.unpack_from('<H', raw)[0] != allocation[0] or (allocation[0] == 26 and raw[25] < 4):
                status = -1
            else:
                owned = bytearray(raw); struct.pack_into('<H', owned, 2, allocation[1])
                self.objects[allocation[1]] = owned
        elif op == 8: status, output = self.update()
        elif op == 9:
            allocation = self.allocation(index)
            if index >= 182: status = -1
            elif allocation is None: status = 1
            elif kind >= 28 or (kind in EXTENDED) != (allocation[0] in EXTENDED): status = -1
            else:
                self.pool.slots[allocation[1]] = 1, kind; self.objects.pop(allocation[1], None)
        else: status = -1
        return status, output + line('result', [op, status, *self.visit, self.next_entry, due, voice]) + self.shared()


def expected(case):
    world = WorldGolden(case)
    return world.shared() + ''.join(world.apply(request)[1] for request in case['commands'])


def lifetime(kind, index, *, ticks=365, count=1, enabled=1, immediate_release=False):
    raw = bytearray(parent_fixture(kind)['raw']) if kind in (23, 26, 27) else bytearray(parent_fixture(17)['raw'])
    struct.pack_into('<H', raw, 0, kind)
    if kind == 17 and immediate_release: raw[25] = 29; struct.pack_into('<H', raw, 26, 11)
    if kind == 18: raw[25] = 6 if immediate_release else 0; struct.pack_into('<H', raw, 26, 7 if immediate_release else 0)
    if kind == 4: raw = new_effect((4, 0, index, 1), (1234, -6789, 65535), (20, 768, 4, 10, 9))[1]
    commands = [bind(kind, index), command(7, index=index, raw=bytes(raw))]
    used = {index}
    for entry in range(182):
        if len(used) >= count: break
        if entry in used: continue
        used.add(entry)
        filler = parent_fixture(17, counter=11, mode=29 if immediate_release else 0)['raw']
        commands += [bind(17, entry), command(7, index=entry, raw=filler)]
    case = fixture(commands, enabled=enabled)
    world = WorldGolden(case)
    for request in commands: world.apply(request)
    for tick in range(ticks):
        for request in (command(6, index=tick % 65536), command(4)):
            commands.append(request); world.apply(request)
        while True:
            request = command(5); commands.append(request)
            status, _ = world.apply(request)
            if status: break
            request = command(8); commands.append(request)
            assert world.apply(request)[0] == 0
    return case


class WorldStepTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-world-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'input.bin'
        cls.commands = []
        if TARGET in ('all', 'native'): cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_world_step_probe')])
        if TARGET in ('all', 'wasm'): cls.commands.append(['node', str(BUILD / 'wasm/fist_world_step_probe.js')])
        cls.fixtures = cls.visits = cls.updates = cls.ticks = 0

    @classmethod
    def tearDownClass(cls):
        print(f'World per target: {cls.fixtures} fixtures, {cls.visits} visits, {cls.updates} class updates, {cls.ticks} tick prefixes', flush=True)

    def check(self, cases, *, original_safe=True):
        wanted = ''.join(expected(case) for case in cases)
        if ORACLE and original_safe: self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command_line in self.commands:
            result = subprocess.run([*command_line, str(self.path)], capture_output=True, text=True, timeout=90)
            self.assertEqual(result.returncode, 0, result.stderr); self.assertEqual(result.stdout, wanted)
        type(self).fixtures += len(cases)
        for row in wanted.splitlines():
            if row.startswith('result '):
                fields = list(map(int, row.split()[1:]))
                type(self).visits += fields[0] == 5 and fields[1] == 0
                type(self).updates += fields[0] == 8 and fields[1] == 0
                type(self).ticks += fields[0] == 6 and fields[1] == 0

    def batches(self, cases, size=128):
        for begin in range(0, len(cases), size): self.check(cases[begin:begin + size])

    def test_countdown_byte_domains_borrow_disable_and_word_boundaries(self):
        cases = []
        for lane in range(3):
            others = [index for index in range(3) if index != lane]
            for byte in range(256):
                for a in (0, 1, 59, 255):
                    for b in (0, 1, 59, 255):
                        time = [0, 0, 0]; time[lane] = byte; time[others[0]] = a; time[others[1]] = b
                        cases.append(fixture([command(6)], time=time))
        self.batches(cases)
        self.batches([fixture([command(6, index=timer)], tick=tick, auxiliary=(first, second))
                      for tick in (0, 1, 65534, 65535) for first in (0, 1, 2, 32767, 65535)
                      for second in (0, 1, 2, 32768, 65535) for timer in (0, 65535)])
        self.check([fixture([command(6, index=tick % 65536) for tick in range(7205)], time=(0, 2, 0), tick=65500)])

    def test_voice_bytes_modes_due_miss_wrap_and_no_deferral(self):
        self.batches([fixture([command(6, index=65535), command(6, index=0)], pending=voice, mode=mode,
                              tick=tick, voice_at=(tick + offset) % 65536, last=1)
                      for voice in range(256) for mode in (0, 1, 2, 255) for tick in (0, 65535)
                      for offset in (0, 1, 2)])
        self.batches([fixture([command(6)], pending=127, mode=mode) for mode in range(256)])

    def test_live_registry_order_holes_overwrite_retype_release_and_end(self):
        self.batches([fixture([bind(21, entry), command(4), command(5), command(1, kind=17),
                               command(5), command(5), command(4), command(5), command(5), command(5)])
                      for entry in range(182)])
        self.batches([fixture([bind(kind, 181), bind(21, 0), bind(18, 91), command(4),
                               command(5), command(2, kind=17, index=91, value=65535), command(5),
                               command(9, kind=19 if kind in EXTENDED else 18, index=181),
                               command(5), command(3, index=181), command(1, kind=17), command(5),
                               command(4), command(5), command(5), command(5)]) for kind in range(28)])
        self.check([fixture([bind(17, 0), bind(17, 1), bind(18, 181), command(4), command(5),
                            command(3, index=1), command(1, kind=17), command(5), command(5), command(5)]),
                    fixture([bind(0, 0), bind(21, 0), command(4), command(5), command(5)])])

    def test_actual_mutable_pass_birth_capacity_reuse_and_complete_lifetimes(self):
        cases = [lifetime(kind, entry, ticks=500 if kind in (23,26,27) else 365)
                 for kind in (4,17,18,23,26,27) for entry in (0,91,181)]
        cases += [lifetime(kind, entry, ticks=2, count=count, immediate_release=release, enabled=enabled)
                  for kind in (23,26,27) for entry in (0,181) for count in (1,119,120,149,150)
                  for release in (False,True) for enabled in (0,1)]
        self.batches(cases,8)
        earlier = expected(lifetime(23,181,ticks=1)); later = expected(lifetime(23,0,ticks=1))
        self.assertEqual(earlier.count('smoke '),1); self.assertEqual(later.count('smoke '),2)
        self.assertIn(' 0 0\n', earlier.split('smoke ',1)[1].split('result ',1)[0])

    def test_invalid_metadata_requests_and_malformed_inputs(self):
        cases = [fixture([command(1, kind=28), command(2,kind=65535), command(3,index=182),
                          command(9,index=182), command(255), command(8), command(5)])]
        cases += [fixture([bind(0,index) for index in range(32)]+[command(1,kind=0),command(5)])]
        self.check(cases, original_safe=False)
        valid=encode([fixture([bind(23,0),command(7,index=0,raw=parent_fixture()['raw'])])])
        for contents in [valid[:n] for n in (0,3,4,40,41,48,len(valid)-1)]+[valid+b'\0']:
            self.path.write_bytes(contents)
            for command_line in self.commands:
                result=subprocess.run([*command_line,str(self.path)],capture_output=True,timeout=30)
                self.assertEqual((result.returncode,result.stdout),(1,b''))

    def test_required_all_mission_occupancy_and_orphans_at_visit_boundary(self):
        if not ORIGINALS: self.skipTest('The complete pinned corpus is an explicit additional gate')
        manifest=json.loads((ROOT/'tests/scenario_originals.json').read_text());self.assertEqual(len(manifest),47)
        cases=[];imports=0
        for name,info in manifest.items():
            data=(ROOT/'armoredfist/FISTDATA'/name).read_bytes();self.assertEqual(hashlib.sha256(data).hexdigest(),info['sha256'])
            records=records_from_scenario(data);imports+=len(records)
            commands=[bind(int.from_bytes(raw[:2],'little'),index,value) for index,value,raw in records]
            commands += [command(4)]+[command(5)]*183
            cases.append(fixture(commands))
        self.assertEqual(imports,4213)
        self.batches(cases,4)
        print('World corpus: all 4213 snapshot imports, current bindings/orphans and all 47 complete occupancy contexts; class methods outside this traversal-only corpus',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--target',choices=['all','native','wasm'],default=TARGET)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true');parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();BUILD,TARGET,NATIVE_PROBE,ORIGINALS=args.build_root,args.target,args.native_probe,args.originals
    if args.oracle:
        from original_world_step_oracle import OriginalWorldStepOracle
        ORACLE=OriginalWorldStepOracle()
    program=unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped)!=(0 if ORIGINALS else 1))
