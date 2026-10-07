#!/usr/bin/env python3
"""Complete delivered mission payload installation, original loader order and tree updates."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from orders_contract import orders_lines
from test_destruction import parent_lines, smoke_lines
from test_other_damage import actor_lines
from test_projectile_flight import Pool, line
from test_units import expected as unit_expected, records_from_scenario, scenario_data, snapshot
from test_vehicle_start import initialized, random_line, state_lines

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None
DELIVERED = {0, 1, 2, 3, 5, 6, 11, 13, 16, 17, 18, 21, 23, 25, 26, 27}
EXTENDED = {0, 1, 2, 3, 19}
SEEDS = (1, 2, 32768, 65535)


def record(kind, index=0, value=1, *, flags=None, member=0, platoon=0, mode=0):
    flags = (32 if kind < 4 else 0) if flags is None else flags
    raw = bytearray(snapshot(kind, flags=flags, member=member, platoon=platoon))
    if kind == 26: raw[25] = mode
    return index, value, bytes(raw)


def tree_lines(raw, allocation):
    return line('tree', [*allocation[1:], *struct.unpack_from('<3i3H', raw, 4), *raw[22:26]])


def payload_lines(raw, allocation):
    kind, slot, index, value = allocation
    if kind < 4:
        return state_lines([(index, value, raw)]) + line('damage', [raw[58], raw[149]])
    if kind in (5, 6, 26, 27): return actor_lines(raw, allocation)
    if kind == 17: return smoke_lines(raw, allocation)
    if kind == 21: return tree_lines(raw, allocation)
    if kind == 23: return parent_lines(raw, allocation)
    if kind in (11, 13, 16, 25):
        return line('saved_base', [*allocation, *struct.unpack_from('<3i3H', raw, 4), *raw[22:26]])
    if kind == 18:
        return line('muzzle', [*allocation, *struct.unpack_from('<3iH', raw, 4),
                              struct.unpack_from('<H', raw, 20)[0],
                              struct.unpack_from('<H', raw, 26)[0], raw[25], raw[22]])
    raise AssertionError('No invented payload fallback')


def install(records, seeds=SEEDS, cursor=0, link=0):
    pool = Pool([])
    objects = {}
    participants = [entry for entry in records if int.from_bytes(entry[2][:2], 'little') < 4 and entry[2][22] & 32]
    starts, (words, end) = initialized(participants, seeds, cursor, link)
    starts = iter(starts)
    physical = []
    if len(records) > 182: return 1, None
    for index, value, saved in records:
        kind = int.from_bytes(saved[:2], 'little')
        slots = range(150, 182) if kind in EXTENDED else range(150)
        slot = next((n for n in slots if pool.slots[n][0] == 0), None)
        if slot is None: return 1, None
        pool.slots[slot] = 1, kind; pool.registry[index] = slot, value; physical.append(slot)
        if saved[22] & 32 and kind >= 4 and kind != 23: return -1, None
        if kind not in DELIVERED: return 2, None
        if kind == 26 and saved[25] >= 8: return -1, None
        raw = bytearray(next(starts)[2] if kind < 4 and saved[22] & 32 else saved)
        struct.pack_into('<H', raw, 2, slot - 150 if kind in EXTENDED else slot)
        if kind == 23: raw[22] |= 64; raw[23] |= 36
        objects[slot] = ((kind, slot, index, value), raw)
    _, _, _, roster = unit_expected(records)
    roster = [65535 if n == 65535 else physical[n] for n in roster]
    return 0, (pool, objects, roster, words, end)


def world_lines(world, orders=None):
    pool, objects, roster, words, cursor = world
    from mission_ready_contract import preparation_lines
    output = (pool.state() + random_line(words, cursor) + line('roster', roster) +
              orders_lines(orders) + preparation_lines(pool))
    for slot, (allocation, raw) in sorted(objects.items()):
        if pool.slots[slot][0] != 0:
            output += line('object', [slot, allocation[0]]) + payload_lines(raw, allocation)
    return output


def expected(records, seeds=SEEDS, cursor=0, link=0, commands=(), reload=False):
    status, world = install(records, seeds, cursor, link)
    output = line('status', [status])
    if status: return output, status
    output += world_lines(world)
    if reload:
        status, world = install(records, world[3], world[4], link)
        assert status == 0
        output += line('reload', [0]) + world_lines(world)
    pool, objects, _, _, _ = world
    for slot, changed, variant in commands:
        status = -1
        if slot in objects and objects[slot][0][0] == 21:
            allocation, raw = objects[slot]
            if pool.registry[allocation[2]] == (slot, allocation[3]):
                status = 0
                if changed == 1: raw[25] = variant
            output += line('update', [slot, status]) + tree_lines(raw, allocation)
        else: output += line('update', [slot, status])
    return output + world_lines(world), 0


class MissionWorldTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-mission-world-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.path = pathlib.Path(cls.temp.name)
        cls.commands = []
        if TARGET in ('all', 'native'): cls.commands.append([str(NATIVE_PROBE or BUILD/'native/fist_mission_world_probe')])
        if TARGET in ('all', 'wasm'): cls.commands.append(['node', str(BUILD/'wasm/fist_mission_world_probe.js')])
        cls.fixtures = cls.installed = cls.rejected = cls.tree_updates = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Mission world per target: {cls.fixtures} fixtures, {cls.installed} installed objects, {cls.rejected} explicit rejections, {cls.tree_updates} tree requests', flush=True)

    def run_data(self, data, *, seeds=SEEDS, cursor=0, link=0, commands=(), reload=False):
        records = records_from_scenario(data)
        wanted, status = expected(records, seeds, cursor, link, commands, reload)
        if ORACLE is not None and status == 0:
            self.assertEqual(ORACLE.install(records, seeds, cursor, link, commands, reload), wanted)
        scenario = self.path/'input.fsg'; scenario.write_bytes(data)
        request = self.path/'request.bin'
        request.write_bytes(struct.pack('<4HBBBI', *seeds, cursor, link, reload, len(commands)) +
                            b''.join(struct.pack('<HBB', *command) for command in commands))
        for command in self.commands:
            result = subprocess.run([*command, str(request), str(scenario)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stderr); self.assertEqual(result.stdout, wanted)
        type(self).fixtures += 1
        type(self).installed += len(records) * (2 if reload else 1) if status == 0 else 0
        type(self).rejected += status != 0
        type(self).tree_updates += len(commands) if status == 0 else 0
        return wanted

    def run_records(self, records, **kwargs):
        return self.run_data(scenario_data(records), **kwargs)

    def test_ground_flag_domains_nonparticipant_state_and_initialization_order(self):
        for flags in range(256):
            self.run_records([record(kind, kind, member=kind, flags=flags) for kind in range(4)], cursor=flags % 4)
        records = [record(21, 181), record(2, 12, member=2), record(23, 91, member=1),
                   record(0, 7), record(17, 30), record(3, 12, member=3), record(1, 2, member=1)]
        for link in range(256): self.run_records(records, link=link)
        for seeds in ((0,0,0,0), (65535,32768,32767,1), SEEDS):
            for cursor in range(4): self.run_records(records[::-1], seeds=seeds, cursor=cursor, link=2, reload=True)

    def test_duplicate_current_bindings_orphans_roster_and_saved_words(self):
        self.run_records([])
        for index in (0,91,181):
            for value in (0,1,65535):
                records = [record(0,index,value),record(21,index,value),record(2,182-index-1,value,member=2),
                           record(23,91,value,member=0),record(23,92,value,member=2),record(1,93,value,member=2)]
                self.run_records(records, commands=[(0,1,255),(150,1,255),(65535,1,255)])
        self.run_records([record(21,0),record(21,0)],commands=[(0,1,99),(1,1,255),(1,2,0)])

    def test_capacity_whole_transaction_unsupported_and_invalid_subtypes(self):
        for count in range(33): self.run_records([record(n % 4,n,member=n % 4,platoon=n // 4) for n in range(count)])
        self.run_records([record(0,n,member=n % 4,platoon=(n // 4) % 8) for n in range(33)])
        for count in (1,119,120,149,150,151): self.run_records([record(21,n) for n in range(count)])
        self.run_records([record(21,n) for n in range(150)]+[record(0,n+150,flags=0) for n in range(32)])
        self.run_records([record(21,n % 182) for n in range(183)])
        for kind in range(28):
            if kind not in DELIVERED: self.run_records([record(21,0),record(kind,1,flags=0)])
        for mode in range(256): self.run_records([record(0,0),record(26,1,mode=mode)])
        self.run_records([record(21,0,flags=32)])

    def test_new_saved_classes_full_flags_coordinates_and_orphan_restoration(self):
        for value in range(256):
            records = []
            for ordinal, kind in enumerate((11, 13, 16, 18, 25)):
                raw = bytearray(snapshot(kind, flags=value & ~32))
                struct.pack_into('<3i3H', raw, 4,
                                 -2147483648 + value * 65537,
                                 2147483647 - value * 65537,
                                 value * 16843009 - 2147483648,
                                 value * 257, 65535 - value * 257, value * 257)
                raw[23:27] = bytes((value, 255 - value, value, 255 - value))
                records.append((ordinal * 7, value * 257, bytes(raw)))
            # The new base and muzzle owners retain duplicate-binding orphans.
            records.append((0, 65535 - value * 257, records[0][2]))
            self.run_records(records, cursor=value % 4, link=value)
        for kind in (11, 13, 16, 18, 25):
            self.run_records([record(0), record(kind, 1, flags=32)])

    def test_tree_complete_byte_domains_and_live_identity(self):
        commands = [(0,changed,variant) for changed in range(256) for variant in range(256)]
        self.run_records([record(21,0,flags=255 & ~32)], commands=commands)
        self.run_records([record(21,0),record(21,0)], commands=[(0,1,99),(1,1,255),(1,0,1)])

    def test_required_pinned_complete_missions_or_explicit_rejection(self):
        if not ORIGINALS: self.skipTest('Pinned full mission corpus is an explicit additional gate')
        manifest=json.loads((ROOT/'tests/scenario_originals.json').read_text())
        self.assertEqual(len(manifest),47)
        complete=[];installed=0;rejected=0
        for name,info in manifest.items():
            data=(ROOT/'armoredfist/FISTDATA'/name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(),info['sha256'])
            records=records_from_scenario(data); status,_=install(records)
            if status == 0: complete.append(name);installed+=len(records)
            else: self.assertEqual(status,2);rejected+=1
            self.run_data(data, cursor=len(complete) % 4)
        self.assertEqual((len(complete),installed,rejected),(47,4213,0))
        self.assertIn('TRAIN1.FSG',complete)
        print('Mission corpus: all 47 complete typed installations/4213 objects; no skipped records or class-dispatch claim',flush=True)

    def test_malformed_request_and_scenario_have_no_output(self):
        scenario=self.path/'input.fsg';scenario.write_bytes(scenario_data([record(0)]))
        request=self.path/'request.bin';valid=struct.pack('<4HBBBI',*SEEDS,0,0,0,0)
        for contents in [valid[:n] for n in (0,1,8,14)]+[valid+b'\0',struct.pack('<4HBBBI',*SEEDS,4,0,0,0),struct.pack('<4HBBBI',*SEEDS,0,0,2,0)]:
            request.write_bytes(contents)
            for command in self.commands:
                result=subprocess.run([*command,str(request),str(scenario)],capture_output=True,timeout=30)
                self.assertEqual((result.returncode,result.stdout),(1,b''))
        request.write_bytes(valid)
        valid_scenario=scenario.read_bytes()
        for contents in (b'',valid_scenario[:-1],scenario_data([record(28)])):
            scenario.write_bytes(contents)
            for command in self.commands:
                result=subprocess.run([*command,str(request),str(scenario)],capture_output=True,timeout=30)
                self.assertEqual((result.returncode,result.stdout),(1,b''))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--target',choices=['all','native','wasm'],default=TARGET)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true');parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();BUILD,TARGET,NATIVE_PROBE,ORIGINALS=args.build_root,args.target,args.native_probe,args.originals
    if args.oracle:
        from original_mission_world_oracle import OriginalMissionWorldOracle
        ORACLE=OriginalMissionWorldOracle()
    program=unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped)!=(0 if ORIGINALS else 1))
