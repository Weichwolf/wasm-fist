#!/usr/bin/env python3
"""M1 pending/automatic primary dispatch, failed cooldown and canonical world births."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_object_pool import bind, release, trace
from test_projectile_launch import allocation, encode, fixture, initial, payloads
from test_units import records_from_scenario
from test_vehicle_start import state_lines

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def fire_fixture(*, secondary=0, behavior=6, behavior_flags=255, **kwargs):
    case = list(fixture(**kwargs))
    raw = bytearray(case[0])
    raw[0x17], raw[0x3e], raw[0x63] = secondary, behavior, behavior_flags
    case[0] = bytes(raw)
    return tuple(case)


def fire_initial(case):
    raw, index, value, origin = initial(case)
    for offset in (0x17, 0x3e, 0x63):
        raw[offset] = case[0][offset]
    return raw, index, value, origin


def expected(case, tick=0, failed=0, state=None, pool=None):
    _, bindings, _, steps, coarse, *_ = case
    raw, index, value, origin = fire_initial(case) if state is None else state
    commands = [bind(*entry) for entry in bindings] + [release(entry) for entry in case[-1]]
    output = ''
    observations = []
    for _ in range(steps):
        requested = bool(raw[0x92] or raw[0x17] & 128)
        if raw[0x92]: raw[0x92] -= 1
        shell = smoke = None
        outcome = 1
        voice = 255
        dispatched = requested and raw[0xa8] == 0
        if requested:
            raw[0xdb] = 3
        if dispatched:
            ammo, = struct.unpack_from('<H', raw, 0xad)
            if ammo:
                struct.pack_into('<H', raw, 0xad, ammo - 1)
                raw[0xe4] = 3
                outcome = 2
                shell = allocation(commands, 8) if pool is None else pool.allocate_short(8)
                if shell:
                    smoke = allocation(commands, 18, low=1) if pool is None else pool.allocate_short(18, low_priority=True)
                    raw[0xa8], raw[0x3c], raw[0x92] = 20, 16, 0
                    outcome = 0
                    voice = 12
            if shell is None and (tick - failed) % 65536 >= 240:
                failed = tick
                voice = 13
        parts, objects = payloads(raw, origin, coarse, shell, smoke)
        metadata = '\n'.join(trace(commands).splitlines()[-3:]) + '\n' if pool is None else pool.state()
        output += f'fire {int(requested)} {int(dispatched)} {int(requested)} {voice} {failed}\n'
        output += f'launch {outcome} {int(smoke is not None)} {12 if shell else 255}\n' + parts
        output += state_lines([(index, value, bytes(raw))]) + f'damage {raw[58]} {raw[149]}\n' + metadata
        observations.append((bytes(raw), metadata, sorted(objects), requested, dispatched, requested, voice, failed))
        tick = (tick + 1) % 65536
    return output, observations


class PrimaryFireTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-primary-fire-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.request = pathlib.Path(cls.temp.name)/'request.bin'
        cls.commands = []
        if TARGET in ('all', 'native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD/'native/fist_projectile_launch_probe'), 'fire'])
        if TARGET in ('all', 'wasm'):
            cls.commands.append(['node', str(BUILD/'wasm/fist_projectile_launch_probe.js'), 'fire'])
        cls.fixtures = cls.stages = 0

    @classmethod
    def tearDownClass(cls):
        print(f'Primary fire per target: {cls.fixtures} fixtures, {cls.stages} complete stage observations', flush=True)

    def run_probe(self, data, wanted, valid=True):
        self.request.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, str(self.request)], capture_output=True, text=True, timeout=120)
            self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
            self.assertEqual(result.stdout, wanted)

    def check(self, cases, *, tick=0, failed=0, original_safe=True):
        wanted = ''
        for case in cases:
            output, observations = expected(case, tick, failed)
            wanted += output
            if ORACLE is not None and original_safe:
                raw, _, _, _ = fire_initial(case)
                actual = ORACLE.launch(bytes(raw), case[1], case[2], case[3], case[4], case[-1],
                                       fire=True, tick=tick, failed=failed)
                self.assertEqual(actual, observations)
        raw = encode(cases)
        self.run_probe(raw[:4]+struct.pack('<HH', tick, failed)+raw[4:], wanted)
        type(self).fixtures += len(cases)
        type(self).stages += sum(case[3] for case in cases)
        return wanted

    def test_pending_automatic_and_reload_complete_byte_domains(self):
        for secondary in (0, 128):
            for reload in (0, 1, 255):
                self.check([fire_fixture(secondary=secondary, trigger=trigger, reload=reload)
                            for trigger in range(256)])
        self.check([fire_fixture(secondary=secondary, trigger=0) for secondary in range(256)])
        self.check([fire_fixture(reload=reload, trigger=1, steps=2) for reload in range(256)])

    def test_behavior_preservation_component_mark_and_idle_preservation(self):
        for trigger in (0, 1):
            self.check([fire_fixture(behavior_flags=value, behavior=value, trigger=trigger,
                                    reload=127, recoil=255) for value in range(256)])
        self.check([fire_fixture(secondary=255, trigger=48, reload=1, steps=50),
                    fire_fixture(secondary=0, trigger=48, reload=1, steps=50)])

    def test_failed_attempt_cooldown_unsigned_wrap_and_retry_lifetimes(self):
        for failed in (0, 1, 65500, 65535):
            for elapsed in (0, 1, 239, 240, 241, 32767, 32768, 65535):
                self.check([fire_fixture(ammo=0, trigger=1), fire_fixture(ammo=0, secondary=128,
                                                                       trigger=0, steps=3)],
                           tick=(failed+elapsed) % 65536, failed=failed)
        self.check([fire_fixture(ammo=0, secondary=128, trigger=0, steps=1000)], tick=65000, failed=64999)

    def test_empty_capacity_optional_muzzle_and_overwritten_origin(self):
        for occupied in (0, 118, 119, 120, 149, 150):
            bindings=[(8,index,1) for index in range(occupied)]+[(0,181,12)]
            self.check([fire_fixture(bindings=bindings, origin=occupied, ammo=ammo,
                                     secondary=128, trigger=48, steps=4) for ammo in (0,1,65535)], tick=300)
        self.check([fire_fixture(bindings=[(0,9,12),(0,9,2),(8,0,1)], origin=0)])
        bindings=[(8 if index < 150 else 0,index,2) for index in range(182)]
        self.check([fire_fixture(bindings=bindings, origin=150, secondary=128, steps=3)],
                   tick=300, original_safe=False)
        releases=tuple(index for index in range(182) if index != 150)
        self.check([fire_fixture(bindings=bindings, origin=150, releases=releases)],
                   tick=300, original_safe=False)

    def test_required_original_m1_corpus_full_occupancy_at_fire_boundary(self):
        if not ORIGINALS: self.skipTest('Pinned original corpus is an explicit required additional gate')
        manifest=json.loads((ROOT/'tools/rewrite/scenario_originals.json').read_text())
        self.assertEqual(len(manifest),47)
        cases=[]
        for name, info in manifest.items():
            data=(ROOT/'armoredfist/FISTDATA'/name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            records=records_from_scenario(data)
            bindings=[(int.from_bytes(raw[:2],'little'),index,value) for index,value,raw in records]
            for origin, (_,_,raw) in enumerate(records):
                if bindings[origin][0] == 0:
                    raw=bytearray(raw);struct.pack_into('<H',raw,0x97,0)
                    cases.append(fire_fixture(snapshot=bytes(raw),bindings=bindings,origin=origin,
                                              trigger=48,steps=2))
        self.assertEqual(len(cases),179)
        self.check(cases,tick=300)
        print('Fire corpus: all 179 M1 actors, all 47 pinned occupancy contexts, declared untargeted station-zero boundary; not complete class/mission ticks',flush=True)

    def test_required_real_train1_world_selection_reload_request_and_publication(self):
        if not ORIGINALS: self.skipTest('Real TRAIN1 installation is an explicit required additional gate')
        from test_mission_world import install
        from test_weapon_control import update
        manifest=json.loads((ROOT/'tools/rewrite/scenario_originals.json').read_text())
        data=(ROOT/'armoredfist/FISTDATA/TRAIN1.FSG').read_bytes()
        self.assertEqual(hashlib.sha256(data).hexdigest(),manifest['TRAIN1.FSG']['sha256'])
        records=records_from_scenario(data)
        status,world=install(records)
        self.assertEqual(status,0)
        _,objects,roster,_,_=world
        allocation,raw=objects[roster[0]]
        self.assertEqual(allocation[0],0)
        raw,_=update(raw,0,0)
        raw=bytearray(raw)
        flags,=struct.unpack_from('<H',raw,0x40)
        if flags & 1:
            struct.pack_into('<H',raw,0x40,flags & 65534)
            raw[0xc7]=3
        for _ in range(320):
            raw[0x3d]=(raw[0x3d]+2) % 256
            raw,_=update(raw,4,0)
            raw=bytearray(raw)
        self.assertEqual(raw[0xa8],0)
        raw[0x92]=48
        bindings=[(int.from_bytes(saved[:2],'little'),index,value) for index,value,saved in records]
        origin=next(n for n,(_,_,saved) in enumerate(records) if int.from_bytes(saved[:2],'little')==0)
        case=fire_fixture(snapshot=bytes(raw),bindings=bindings,origin=origin)
        prepared=bytes(raw)
        wanted,observations=expected(case,320,state=(bytearray(prepared),allocation[2],allocation[3],allocation[1]))
        if ORACLE is not None:
            self.assertEqual(ORACLE.launch(prepared,bindings,origin,1,False,fire=True,tick=320),observations)
        source=pathlib.Path(self.temp.name)/'train1.fsg'
        source.write_bytes(data)
        for command in self.commands:
            result=subprocess.run([*command[:-1],'mission-fire',str(source)],capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stderr);self.assertEqual(result.stdout,wanted)
        type(self).fixtures+=1;type(self).stages+=1
        print('TRAIN1: complete 85-object typed world, source release, actual selection/reload/request boundaries, canonical shell/muzzle birth and unrelated-world preservation; not complete class ticks',flush=True)

    def test_invalid_requests_have_no_output(self):
        raw=encode([fire_fixture()]);valid=raw[:4]+bytes(4)+raw[4:]
        for data in (b'',valid[:7],valid[:-1],valid+b'x',struct.pack('<I',2)+valid[4:]):
            self.run_probe(data,'',valid=False)
        malformed=bytearray(valid);malformed[8+15+0x91]=2
        self.run_probe(bytes(malformed),'',valid=False)
        self.run_probe(bytes(8),'')


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--target',choices=['all','native','wasm'],default=TARGET)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true');parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();BUILD,TARGET,NATIVE_PROBE,ORIGINALS=args.build_root,args.target,args.native_probe,args.originals
    if args.oracle:
        from original_projectile_launch_oracle import OriginalProjectileLaunchOracle
        ORACLE=OriginalProjectileLaunchOracle()
    program=unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped)!=(0 if ORIGINALS else 2))
