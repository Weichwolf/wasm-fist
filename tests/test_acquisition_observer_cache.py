#!/usr/bin/env python3
"""Compare complete optimized original observation against unconditional cache eviction."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import struct
import unittest

from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_target_acquisition_oracle import OriginalTargetAcquisitionOracle, TEXT_BASE, MAILBOX
from original_unit_oracle import DGROUP
from test_original_target_discovery import record

REVIEW = None


class UncachedOracle(OriginalTargetAcquisitionOracle):
    execute = staticmethod(OriginalProjectileFlightOracle.execute)


class CacheTests(unittest.TestCase):
    evidence = None

    def test_complete_cached_and_uncached_returns_same_boundary_and_boundary_changes(self):
        owners = (OriginalTargetAcquisitionOracle(), UncachedOracle())
        pixels = bytes(512**2)
        kernels = [owner.visibility.prepare(512,pixels) for owner in owners]
        calls = 0
        digest = hashlib.sha256()
        for kind, behavior in itertools.product(range(4),range(4)):
            prepared = [owner.prepare_saved([record(kind,0,flags=4),record(26,1,x=-4096)],
                (2,2,2,2),0,0,(bytes(2144),bytes(176))) for owner in owners]
            actor = prepared[0][1][150][2]
            target = prepared[0][1][0][2]
            initial = owners[0].raw(prepared[0][0],actor)
            for value,position,automatic,mode,selected,gate in itertools.product(
                    (0,29,30,40,120,200,255),(-4096,262144),(False,True),
                    (0,3,7,255),(False,True),(0,65535)):
                observed = []
                for index,(owner,(machine,_)) in enumerate(zip(owners,prepared)):
                    raw = bytearray(initial);raw[0x94]=1
                    struct.pack_into('<H',raw,0x9d,target)
                    machine.mem_write(DGROUP+actor,bytes(raw))
                    machine.mem_write(DGROUP+target+4,struct.pack('<i',position))
                    machine.mem_write(DGROUP+target+25,bytes([mode]))
                    machine.mem_write(DGROUP+0x85b6,struct.pack('<H',behavior))
                    machine.mem_write(DGROUP+0x1f82,struct.pack('<5H',0x1f8a,2*(value+1),2,2,2))
                    result,expected = owner.acquire(machine,actor,kernels[index],pixels=pixels,
                        automatic=automatic,candidate=target,selected=actor if selected else 0,
                        gate=gate,clock=30)
                    self.assertEqual(result,expected)
                    # Toggle actual near/far helper boundaries between repeated
                    # acquisition returns; these require cache eviction again.
                    if value % 2:
                        owner.aim(machine,actor,value % 3)
                    observed.append((result,bytes(machine.mem_read(DGROUP,65536)),
                        bytes(machine.mem_read(0,DGROUP)),bytes(machine.mem_read(TEXT_BASE,65536)),
                        bytes(machine.mem_read(MAILBOX,4096))))
                self.assertEqual(observed[0],observed[1])
                calls += 1
                digest.update(observed[0][1])
        self.assertEqual(calls,7168)
        type(self).evidence = {'success':True,'groups':1,'skips':0,'paired_complete_sequences':calls,
            'each_pair':'acquisition plus conditional complete far aim, full DGROUP/code/GS/mailbox',
            'baseline':'existing unconditional eviction in OriginalProjectileFlightOracle',
            'output_sha256':digest.hexdigest(),'complete_wasm_streak':0}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--review-dir',type=Path)
    args=parser.parse_args()
    REVIEW=args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(Path('/tmp')) or REVIEW.resolve()==Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program=unittest.main(argv=[__file__],exit=False)
    success=program.result.wasSuccessful() and program.result.testsRun==1 and not program.result.skipped
    if REVIEW and success:
        REVIEW.mkdir(parents=True,exist_ok=True)
        (REVIEW/'observer-cache.json').write_text(json.dumps(CacheTests.evidence,indent=2)+'\n')
    raise SystemExit(not success)
