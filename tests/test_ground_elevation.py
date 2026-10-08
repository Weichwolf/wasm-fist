#!/usr/bin/env python3
"""Complete shared phase/manual elevation, wrap, target loss and transactions."""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from ground_elevation_contract import (PHASE, RAISE, LOWER, QUICK_LOWER, CENTER,
    DOMAIN_COUNTS, domain_cases, predict, snapshot)
from test_units import records_from_scenario
from test_vehicle_start import state_lines

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False


def fixture(raw, clock, controls, action, *, steps=1, advance=0, reference=(0,0)):
    return raw, clock, controls, action, steps, advance, reference


def encoded(case):
    raw, clock, controls, action, steps, advance, reference = case
    return raw + struct.pack('<4HBHHQH',clock,*controls,action,steps,advance,*reference)


def expected(case):
    raw, clock, controls, action, steps, advance, reference = case
    output = []
    for _ in range(steps):
        active = action != PHASE or raw[0x19]&0x60 != 0
        if active and reference[0] != 0 and struct.unpack_from('<H',raw,0x97)[0] == 0:
            raw = bytearray(raw)
            struct.pack_into('<H',raw,0x97,1)
        raw, controls, _ = predict(raw,clock,controls,action)
        if active:
            reference = (0,0)
        output.append(state_lines([(0,0,raw)]) +
            'elevation_controls '+' '.join(map(str,controls))+'\n'+
            'elevation_target '+' '.join(map(str,reference))+'\n')
        clock = (clock+advance)&65535
    return ''.join(output)


class ElevationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='wasm-fist-elevation-',dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ('all','native'):
            cls.commands.append([str(NATIVE_PROBE or BUILD/'native/fist_ground_elevation_probe')])
        if TARGET in ('all','wasm'):
            cls.commands.append(['node',str(BUILD/'wasm/fist_ground_elevation_probe.js')])
        cls.checked = collections.Counter()
        cls.digest = hashlib.sha256()

    @classmethod
    def tearDownClass(cls):
        print('Elevation per target: '+json.dumps(dict(cls.checked),sort_keys=True)+
              ' output_sha256='+cls.digest.hexdigest(),flush=True)

    def check(self,cases,group):
        path = Path(self.temp.name)/'input.bin'
        data = struct.pack('<I',len(cases))+b''.join(encoded(case) for case in cases)
        wanted = ''.join(expected(case) for case in cases)
        path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command,str(path)],capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(result.stdout,wanted)
        self.checked[group] += sum(case[4] for case in cases)
        self.digest.update(wanted.encode())

    def test_complete_original_phase_and_manual_domains(self):
        counts = collections.Counter()
        batch = []
        for group,kind,flags,target,elevation,clock,controls,action in domain_cases():
            batch.append(fixture(snapshot(kind,flags,target,elevation),clock,controls,action))
            counts[group] += 1
            if len(batch) == 1024:
                self.check(batch,'domains')
                batch.clear()
        if batch:
            self.check(batch,'domains')
        self.assertEqual(counts,DOMAIN_COUNTS)

    def test_retained_clock_acceleration_wrap_and_selected_target_release(self):
        cases = []
        for kind,action,target,advance in itertools.product(range(4),range(5),(0,65535),(0,1,19,20,65535)):
            cases.append(fixture(snapshot(kind,96,target,32767),65530,(65529,0,363),action,
                                 steps=400,advance=advance))
        for offset in range(0,len(cases),16):
            self.check(cases[offset:offset+16],'retained')

    def test_runtime_target_lifetimes_and_unused_malformed_references(self):
        cases = []
        for kind,action,flags,reference in itertools.product(range(4),range(5),(0,32,64,96),
                ((0,0),(1,150),(2**64-1,181),(42,0))):
            cases.append(fixture(snapshot(kind,flags,0,-32768),0,(65535,363,65535),action,
                                 steps=3,advance=1,reference=reference))
        self.check(cases,'runtime_targets')
        self.check([fixture(snapshot(kind,0,65535,123),1,(0,18,18),PHASE,reference=reference)
                    for kind,reference in itertools.product(range(4),((1,182),(0,1),(1,65535)))],
                   'unused_malformed')

    def test_invalid_input_fails_before_any_output(self):
        path = Path(self.temp.name)/'invalid.bin'
        good = fixture(snapshot(0,32,0,123),1,(0,18,18),PHASE)
        valid = struct.pack('<I',1)+encoded(good)
        bad_cases = [b'',valid[:-1],valid+b'x',struct.pack('<I',2)+encoded(good)]
        bad_cases += [struct.pack('<I',1)+encoded(fixture(*good[:4],steps=0))]
        bad_cases += [struct.pack('<I',1)+encoded(fixture(good[0],good[1],good[2],255))]
        for reference in ((1,182),(0,1),(1,65535)):
            bad = fixture(*good[:4],reference=reference)
            bad_cases.append(struct.pack('<I',2)+encoded(good)+encoded(bad))
        for data in bad_cases:
            path.write_bytes(data)
            for command in self.commands:
                result = subprocess.run([*command,str(path)],capture_output=True,text=True,timeout=60)
                self.assertEqual((result.returncode,result.stdout),(1,''),result.stderr)
        self.check([],'empty')
        for command in self.commands:
            result = subprocess.run([*command,str(Path(self.temp.name)/'missing')],capture_output=True,text=True,timeout=60)
            self.assertEqual((result.returncode,result.stdout),(1,''))

    def test_all_original_ground_snapshots(self):
        if not ORIGINALS:
            self.skipTest('Provisioned original corpus requested separately')
        manifest = json.loads((ROOT/'tests/scenario_originals.json').read_text())
        files = sorted((ROOT/'armoredfist/FISTDATA').glob('*.FSG'))
        self.assertEqual([p.name for p in files],sorted(manifest))
        count = 0
        for path in files:
            data = path.read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(),manifest[path.name]['sha256'])
            cases = []
            for _,_,raw in records_from_scenario(data):
                if struct.unpack_from('<H',raw)[0] >= 4:
                    continue
                count += 1
                for action,flags in itertools.product(range(5),(0,32,64,96)):
                    source = bytearray(raw)
                    source[0x19] = flags
                    cases.append(fixture(bytes(source),65530,(65529,18,363),action,
                                         steps=3,advance=1))
            for offset in range(0,len(cases),256):
                self.check(cases[offset:offset+256],'originals')
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(),manifest[path.name]['sha256'])
        self.assertEqual(count,960)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-root',type=Path,default=BUILD)
    parser.add_argument('--target',choices=('native','wasm','all'),default=TARGET)
    parser.add_argument('--native-probe',type=Path)
    parser.add_argument('--originals',action='store_true')
    args = parser.parse_args()
    BUILD,TARGET,NATIVE_PROBE,ORIGINALS = args.build_root,args.target,args.native_probe,args.originals
    program = unittest.main(argv=[__file__],exit=False)
    raise SystemExit(not program.result.wasSuccessful() or (ORIGINALS and bool(program.result.skipped)))
