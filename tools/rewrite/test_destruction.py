#!/usr/bin/env python3
"""Persistent destruction updates, exact smoke admission and complete natural retirement."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_collision import delta
from test_other_damage import fixture as damage_fixture, expected as damage_expected, actor_lines
from test_projectile_flight import Pool, line
from test_units import records_from_scenario
from test_vehicle_start import step

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def fixture(kind=23, *, operation=2, enabled=1, counter=63, parameter=768, mode=None,
            ticks=1, parent_ticks=None, wind=(17, -23), seeds=(2, 76, 78, 1), cursor=0,
            strength=768, raw=None, bindings=None, target=None):
    if raw is None:
        raw = bytearray(55)
        struct.pack_into('<H', raw, 0, kind)
        struct.pack_into('<3iH2H', raw, 4, 2147483646, -2147483647, 0x1234fffc, 54321, 1234, 1024)
        raw[22:25] = bytes((64, 3, 197))
        if kind == 23:
            struct.pack_into('<4H', raw, 27, 10, 0, counter, parameter)
            raw[35:37] = bytes((2, 3))
        elif kind == 26:
            raw[25], raw[26], raw[27], raw[30] = 4 if mode is None else mode, 99, 100, counter % 256
            struct.pack_into('<H', raw, 28, parameter)
        elif kind == 27:
            raw[25], raw[26] = 1 if mode is None else mode, 99
            struct.pack_into('<3H', raw, 27, counter, parameter, 0xabcd)
        elif kind == 17:
            raw[25] = 0 if mode is None else mode
            struct.pack_into('<H', raw, 26, counter)
    else:
        raw = bytearray(raw)
    target = (1 if operation == 3 else 0) if target is None else target
    bindings = bindings or ([(8, 181, 1), (kind, 5, 7)] if operation == 3 else [(kind, 5, 7)])
    return dict(raw=bytes(raw), operation=operation, enabled=enabled, seeds=seeds, cursor=cursor,
                bindings=bindings, ticks=ticks, parent_ticks=ticks if parent_ticks is None else parent_ticks,
                strength=strength, wind=wind, target=target)


def encode(cases):
    out = struct.pack('<I', len(cases))
    for case in cases:
        out += struct.pack('<2B4H2B4H2i2H', case['operation'], case['enabled'], *case['seeds'],
                           case['cursor'], 0, len(case['bindings']), case['ticks'],
                           case['parent_ticks'], case['strength'], *case['wind'], case['target'], 0)
        out += case['raw'] + b''.join(struct.pack('<3H', *entry) for entry in case['bindings'])
    return out


def smoke_lines(raw, allocation):
    return line('smoke', [*allocation[1:], *struct.unpack_from('<3i3H', raw, 4),
                          *raw[22:26], struct.unpack_from('<H', raw, 26)[0]])


def parent_lines(raw, allocation):
    kind = allocation[0]
    if kind == 23:
        return line('wreck', [*allocation[1:], *struct.unpack_from('<3iH', raw, 4),
                              *struct.unpack_from('<2H', raw, 27), struct.unpack_from('<H', raw, 20)[0],
                              struct.unpack_from('<H', raw, 33)[0], raw[35], raw[36], raw[22], raw[23],
                              struct.unpack_from('<H', raw, 31)[0]])
    return actor_lines(raw, allocation)


def advance_parent(raw, spawn):
    """Shared independent parent state arithmetic; the caller owns admission."""
    kind = struct.unpack_from('<H', raw)[0]
    parameter_offset, counter_offset = {23: (33, 31), 26: (28, 30), 27: (29, 27)}[kind]
    created = None
    if kind == 26 and raw[25] in (5, 7):
        raw[22] |= 8
    else:
        counter = (raw[counter_offset] + 1) % 256 if kind == 26 else (struct.unpack_from('<H', raw, counter_offset)[0] + 1) % 65536
        if kind == 26: raw[counter_offset] = counter
        else: struct.pack_into('<H', raw, counter_offset, counter)
        parameter, = struct.unpack_from('<H', raw, parameter_offset)
        emit = counter & 63 == 0 and parameter > 128
        created = spawn(parameter) if emit else None
        if emit and kind != 23:
            parameter -= 1
            if kind == 26 and parameter > 768: parameter -= 4
            struct.pack_into('<H', raw, parameter_offset, parameter)
    if kind == 23: raw[22] |= 64; raw[23] |= 68
    if kind == 27 and raw[25] == 1: raw[22] = raw[22] & 249 | 1; raw[23] &= 231
    return created


def advance_smoke(raw, wind, enabled):
    """Shared independent smoke state arithmetic; the caller owns release."""
    released = enabled != 1
    if not released:
        x, y, z = struct.unpack_from('<3i', raw, 4)
        bits = z % 2**32; bits = (bits & 0xffff0000) | ((bits + 8) & 65535)
        struct.pack_into('<3i', raw, 4, delta(x + wind[0], 0), delta(y + wind[1], 0), delta(bits, 0))
        counter = (struct.unpack_from('<H', raw, 26)[0] + 1) % 65536
        if counter >= 12:
            counter = 0; raw[25] = (raw[25] + 1) % 256; released = raw[25] >= 30
        struct.pack_into('<H', raw, 26, counter)
    if released: raw[22] |= 1
    return released


def expected(case):
    pool = Pool(case['bindings'])
    primary = pool.allocations[case['target']]
    raw = bytearray(case['raw'])
    words, cursor = list(case['seeds']), case['cursor']
    smokes, effects, output = {}, [], ''
    def shared():
        return line('random', [cursor, *words]) + pool.state()
    def spawn(strength):
        nonlocal cursor
        allocation = pool.allocate_short(17, low_priority=True) if case['enabled'] == 1 else None
        if allocation is None:
            return None
        roll, cursor = step(words, cursor)
        extent = (strength + (roll & 63)) % 65536
        created = bytearray(55)
        struct.pack_into('<H', created, 0, 17)
        x, y, z = struct.unpack_from('<3i', raw, 4)
        struct.pack_into('<3iH2H', created, 4, x, y, delta(z + 768, 0), 0, extent, extent * 4 % 65536)
        smokes[allocation[1]] = (allocation, created)
        return smoke_lines(created, allocation)
    if case['operation'] == 3:
        request = damage_fixture(primary[0], raw=raw, bindings=case['bindings'], target=case['target'],
                                 selected=65535, roster=(65535,) * 32, seeds=case['seeds'],
                                 cursor=case['cursor'], scales=(256, 256))
        request['raw'] = bytes(raw)
        transcript = damage_expected(request).splitlines()
        result = next(text.split()[1:] for text in transcript if text.startswith('result '))
        actor = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('actor '))
        struct.pack_into('<3iH2H', raw, 4, *actor[4:10]); raw[22:26] = bytes(actor[10:14])
        if primary[0] == 26:
            values = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('type26 '))
            raw[26], raw[27] = values[:2]; struct.pack_into('<H', raw, 28, values[2])
        else:
            values = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('type27 '))
            raw[26] = values[0]; struct.pack_into('<2H', raw, 29, *values[1:3])
        for text in transcript:
            if text.startswith('effect '):
                fields = list(map(int, text.split()[1:])); effects.append([(4, *fields[:3]), fields[-3] * (fields[-4] + 1)])
        registry = transcript[-1].split()[1:]; slots = transcript[-2].split()[1:]
        pool.registry = [tuple(map(int, entry.split(':'))) for entry in registry]
        pool.slots = [tuple(map(int, entry.split(':'))) for entry in slots]
        random = next(list(map(int, text.split()[1:])) for text in transcript if text.startswith('random '))
        cursor, words = random[0], random[1:]
        impact = next(text.split()[1] for text in transcript if text.startswith('impact '))
        output += line('damage', [result[0], result[1], result[4], result[3]]) + f'impact {impact}\n'
        output += parent_lines(raw, primary) + shared()
    if case['operation'] == 0:
        created = spawn(case['strength'])
        output += f'creation {int(created is None)}\n' + (created or '')
    elif case['operation'] == 1:
        smokes[primary[1]] = primary, raw
        output += smoke_lines(raw, primary)
    else:
        output += parent_lines(raw, primary)
    output += shared()
    for tick in range(case['ticks']):
        if case['operation'] >= 2 and tick < case['parent_ticks']:
            created = advance_parent(raw, spawn)
            output += f'emission {int(created is not None)}\n' + parent_lines(raw, primary) + (created or '')
        for slot, (allocation, smoke) in sorted(list(smokes.items())):
            released = advance_smoke(smoke, case['wind'], case['enabled'])
            if released:
                pool.release(allocation); del smokes[slot]
            output += smoke_lines(smoke, allocation)
        for effect in effects:
            if effect[1]:
                effect[1] -= 1
                if effect[1] == 0: pool.release(effect[0])
        output += shared()
    return output


class DestructionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-destruction-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name) / 'input.bin'
        cls.commands = []
        if TARGET in ('all', 'native'): cls.commands.append([str(NATIVE_PROBE or BUILD / 'native/fist_destruction_probe')])
        if TARGET in ('all', 'wasm'): cls.commands.append(['node', str(BUILD / 'wasm/fist_destruction_probe.js')])
        cls.fixtures = cls.emissions = cls.smoke = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Destruction per target: {cls.fixtures} fixtures, {cls.emissions} parent updates, {cls.smoke} complete smoke states', flush=True)

    def check(self, cases):
        wanted = ''.join(expected(case) for case in cases)
        if ORACLE: self.assertEqual(''.join(ORACLE.trace(case) for case in cases), wanted)
        self.path.write_bytes(encode(cases))
        for command in self.commands:
            result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr); self.assertEqual(result.stdout, wanted)
        type(self).fixtures += len(cases); type(self).emissions += wanted.count('emission '); type(self).smoke += wanted.count('smoke ')

    def batches(self, cases, size=128):
        for offset in range(0, len(cases), size): self.check(cases[offset:offset + size])

    def test_creation_quality_bytes_random_extent_wrap_and_capacity(self):
        self.batches([fixture(operation=0, enabled=setting, ticks=0) for setting in range(256)])
        self.batches([fixture(operation=0, strength=base, seeds=(seed, 2, 0, 0), ticks=0)
                      for base in (0, 128, 768, 16383, 16384, 65500, 65535) for seed in range(128)])
        self.batches([fixture(operation=0, bindings=[(23, 181, 1)] + [(21, index, 1) for index in range(count - 1)], ticks=0)
                      for count in range(1, 151)])

    def test_smoke_counter_frame_flags_wind_and_low_word_carry(self):
        cases = [fixture(17, operation=1, counter=counter, mode=frame, ticks=1)
                 for counter in (0, 1, 10, 11, 12, 255, 256, 32767, 65534, 65535) for frame in range(256)]
        for flags in range(256):
            case = fixture(17, operation=1, counter=11, mode=29); raw = bytearray(case['raw']); raw[22] = flags; raw[23] = 255 - flags; case['raw'] = bytes(raw); cases.append(case)
        for z in (0, 65527, 65528, 65535, 65536, -1, -65536, -2147483648, 2147483647):
            for wind in ((0, 0), (2147483647, -2147483648), (-2147483648, 2147483647)):
                case = fixture(17, operation=1, wind=wind); raw = bytearray(case['raw']); struct.pack_into('<i', raw, 12, z); case['raw'] = bytes(raw); cases.append(case)
        cases += [fixture(17, operation=1, enabled=setting) for setting in range(256)]
        self.batches(cases)

    def test_parent_counter_parameter_quality_and_mode_domains(self):
        variants = ((23, 0), (26, 4), (26, 5), (26, 6), (26, 7), (27, 0), (27, 1), (27, 255))
        self.batches([fixture(kind, counter=counter, parameter=parameter, mode=mode, enabled=setting)
                      for kind, mode in variants for counter in (0, 62, 63, 64, 127, 255, 256, 65535)
                      for parameter in (0, 127, 128, 129, 768, 769, 770, 65535) for setting in (0, 1, 2)])
        self.batches([fixture(26, mode=mode, counter=counter) for mode in range(4, 8) for counter in range(256)])
        self.batches([fixture(27, mode=mode) for mode in range(256)])
        cases=[]
        for kind in (23,26,27):
            for flag in range(256):
                case=fixture(kind);raw=bytearray(case['raw']);raw[22]=flag;raw[23]=255-flag;case['raw']=bytes(raw);cases.append(case)
        self.batches(cases)

    def test_complete_lifetimes_class_sequences_and_reaching_damage_counter_separation(self):
        self.check([fixture(operation=0, ticks=365), fixture(17, operation=1, counter=65535, mode=255, ticks=380)] +
                   [fixture(kind, mode=mode, ticks=500, parent_ticks=128) for kind, mode in ((23,0),(26,4),(26,5),(26,6),(26,7),(27,1))])
        self.check([fixture(kind, operation=3, mode=mode, ticks=500, parent_ticks=64)
                    for kind,mode in ((26,0),(26,1),(26,2),(26,3),(27,0))])
        self.batches([fixture(kind, enabled=setting, ticks=130, parent_ticks=130,
                             bindings=[(kind,181,1)]+[(21,index,1) for index in range(count-1)])
                      for kind in (23,26,27) for setting in (0,1) for count in (119,120,149,150)], size=4)

    def test_malformed_requests_fail(self):
        valid=encode([fixture()])
        invalid=[valid[:length] for length in (0,3,4,35,80,len(valid)-1)]+[valid+b'\0']
        invalid += [encode([fixture(26,mode=mode)]) for mode in (0,1,2,3,8,255)]
        for contents in invalid:
            self.path.write_bytes(contents)
            for command in self.commands:
                result=subprocess.run([*command,str(self.path)],capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,1)

    def test_original_snapshots_complete_occupancy_and_constructed_critical_successors(self):
        if not ORIGINALS: self.skipTest('The pinned complete corpus is an explicit additional gate')
        manifest=json.loads((ROOT/'tools/rewrite/scenario_originals.json').read_text());self.assertEqual(len(manifest),47)
        cases=[];counts={};critical=0
        for name,info in manifest.items():
            data=(ROOT/'armoredfist/FISTDATA'/name).read_bytes();self.assertEqual(hashlib.sha256(data).hexdigest(),info['sha256'])
            records=records_from_scenario(data);current={index:ordinal for ordinal,(index,_,_) in enumerate(records)}
            bindings=[(int.from_bytes(raw[:2],'little'),index,value) for index,value,raw in records]
            vacancy=next(index for index in range(182) if index not in current)
            for ordinal,(index,_,raw) in enumerate(records):
                kind=int.from_bytes(raw[:2],'little')
                if kind not in (17,23,26,27) or current[index]!=ordinal: continue
                if kind==26 and raw[25]<4:
                    modified=bytearray(raw);modified[26]=99
                    cases.append(fixture(kind,operation=3,raw=modified,bindings=[(8,vacancy,1)]+bindings,
                                         target=ordinal+1,ticks=1));critical+=1
                else:
                    cases.append(fixture(kind,operation=1 if kind==17 else 2,raw=raw,bindings=bindings,
                                         target=ordinal,ticks=2));counts[kind]=counts.get(kind,0)+1
        self.assertEqual(counts, {17:28,23:47,26:6,27:236})
        self.assertEqual(critical,796)
        self.assertEqual(len(cases),1113)
        self.batches(cases,16)
        print(f'Destruction corpus: {counts} unchanged saved targets and {critical} constructed critical successors, all 47 complete occupancy contexts',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--target',choices=['all','native','wasm'],default=TARGET)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true');parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();BUILD,TARGET,NATIVE_PROBE,ORIGINALS=args.build_root,args.target,args.native_probe,args.originals
    if args.oracle:
        from original_destruction_oracle import OriginalDestructionOracle
        ORACLE=OriginalDestructionOracle()
    program=unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped)!=(0 if ORIGINALS else 1))
