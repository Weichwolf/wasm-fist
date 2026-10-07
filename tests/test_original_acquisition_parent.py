#!/usr/bin/env python3
"""Required genuine ab03 entry-eight acquisition/RET returns, without parent substitutions."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import struct
import unittest

from original_target_acquisition_oracle import OriginalTargetAcquisitionOracle, MAILBOX, TEXT_BASE
from original_unit_oracle import DGROUP
from target_acquisition_contract import acquisition
from test_original_target_discovery import record
from test_vehicle_start import step

REVIEW = None


class ParentTests(unittest.TestCase):
    evidence = None

    def test_genuine_parent_bank_entry_all_classes_modes_behaviors_random_bytes(self):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_EAX, UC_X86_REG_SP
        owner = OriginalTargetAcquisitionOracle()
        pixels = bytes(512**2)
        kernel = owner.visibility.prepare(512,pixels)
        digest = hashlib.sha256()
        calls = transfers = installs = 0
        for kind, automatic, behavior in itertools.product(range(4),(False,True),range(4)):
            machine, objects = owner.prepare_saved(
                [record(kind,0,flags=4),record(0,1,x=-4096)],(2,2,2,2),0,0,
                (bytes(2144),bytes(176)))
            actor,target = objects[150][2],objects[151][2]
            original = bytearray(owner.raw(machine,actor))
            original[0x42] = 7  # The next low-four-bit parent entry is eight.
            original[0x94] = 1
            struct.pack_into('<H',original,0x40,0x8000 | int(automatic))
            struct.pack_into('<H',original,0x9d,target)
            machine.mem_write(DGROUP+0x85b6,struct.pack('<H',behavior))
            for random_byte in range(256):
                for position in (-4096,262144):
                    machine.mem_write(DGROUP+actor,bytes(original))
                    machine.mem_write(DGROUP+target+4,struct.pack('<i',position))
                    seeds = [2,2*(random_byte+1),32768,65535]
                    machine.mem_write(DGROUP+0x1f82,struct.pack('<5H',0x1f8a,*seeds))
                    for offset,value in ((0x6d34,actor),(0x6da2,0),(0x7ae0,0),(0x452,30),
                                          (0x9fca,0),(0x969e,77),(0x96a0,0x1234)):
                        machine.mem_write(DGROUP+offset,struct.pack('<H',value))
                    machine.mem_write(DGROUP+0x978a,b'\0')
                    machine.mem_write(DGROUP+0xea2c,struct.pack('<HH',0,MAILBOX//16))
                    machine.reg_write(UC_X86_REG_DI,actor)
                    machine.reg_write(UC_X86_REG_SP,0x9000)
                    machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
                    before = bytes(machine.mem_read(DGROUP,65536))
                    code = bytes(machine.mem_read(0,DGROUP))
                    gs = bytes(machine.mem_read(TEXT_BASE,65536))
                    mailbox = bytes(machine.mem_read(MAILBOX,4096))
                    first,cursor = step(seeds,0)
                    sampled = bytearray(original)
                    sampled[0x42] = 8
                    if automatic:
                        expected = acquisition(owner,actor,bytes(sampled),{target:owner.raw(machine,target)},
                            (seeds,cursor),behavior,512,pixels,automatic=True,candidate=target,
                            selected=actor,gate=0,clock=30,last_voice=0,display=(77,0x1234))
                    else:
                        expected = {'actor':bytes(sampled),'random':(seeds,cursor),'transfer':None,
                                    'display':(77,0x1234)}
                    entry = 0xab03
                    if expected['transfer'] is not None:
                        owner.execute(machine,entry,0xe2a0)
                        source,destination = struct.unpack('<3i',machine.mem_read(MAILBOX+0xd2,12)),struct.unpack(
                            '<3i',machine.mem_read(MAILBOX+0xde,12))
                        self.assertEqual((source,destination),(expected['transfer']['source'],expected['transfer']['target']))
                        self.assertEqual(bytes(machine.mem_read(DGROUP+0xea10,2)),b'\x58\0')
                        visible = owner.visibility.visible(kernel,[(source,destination)])[0]
                        self.assertEqual(visible,expected['transfer']['visible'])
                        machine.reg_write(UC_X86_REG_EAX,kernel.reg_read(UC_X86_REG_EAX))
                        entry = 0xe2a3
                        transfers += 1
                    owner.execute(machine,entry,0xeff0)
                    di,ds,sp,ss = owner.machine_registers()
                    self.assertEqual(tuple(machine.reg_read(reg) for reg in (di,ds,sp,ss)),
                                     (actor,0x1c00,0x9002,0x1c00))
                    after = bytes(machine.mem_read(DGROUP,65536))
                    self.assertEqual(owner.raw(machine,actor),expected['actor'])
                    self.assertEqual(owner.random_state(machine),expected['random'])
                    self.assertEqual(struct.unpack_from('<HH',after,0x969e),expected['display'])
                    self.assertEqual(struct.unpack_from('<H',after,0x978c)[0],first)
                    self.assertEqual(struct.unpack_from('<HH',after,0x9796),(0x85b6,0x7d40))
                    self.assertEqual(struct.unpack_from('<H',after,0x9a08)[0],actor)
                    owner.unchanged(before,after,((actor+0x40,actor+0x43),(actor+0x97,actor+0x99),
                        (0x342,0x344),(0x1f82,0x1f8c),(0x8fc0,0x9000),(0x969e,0x96a2),
                        (0x978c,0x978e),(0x9796,0x979a),(0x9a08,0x9a0a),(0xea10,0xea12)),
                        'Genuine parent acquisition changed unrelated DGROUP')
                    owner.unchanged(mailbox,bytes(machine.mem_read(MAILBOX,4096)),
                                    ((0xd2,0xea),(0x3f2,0x3f6)),'Parent changed unrelated mailbox')
                    self.assertEqual(bytes(machine.mem_read(0,DGROUP)),code)
                    self.assertEqual(bytes(machine.mem_read(TEXT_BASE,65536)),gs)
                    calls += 1
                    installs += bool(struct.unpack_from('<H',expected['actor'],0x97)[0])
                    digest.update(expected['actor'])
                    digest.update(struct.pack('<5H',expected['random'][1],*expected['random'][0]))
        self.assertEqual(calls,16384)
        type(self).evidence = {'scope':'genuine complete unselected-diagnostic ab03 entry eight; other entries remain open',
            'success':True,'groups':1,'skips':0,'complete_parent_returns':calls,
            'actual_visibility_transfers':transfers,'installed_targets':installs,
            'ground_classes':4,'control_banks':2,'behavior_choices':4,'random_byte_domain':256,
            'actual_visibility_positions':[-4096,262144],'output_sha256':digest.hexdigest(),
            'complete_wasm_streak':0}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--review-dir',type=Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(Path('/tmp')) or REVIEW.resolve()==Path('/tmp')):
        parser.error('Required evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__],exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun==1 and not program.result.skipped
    if REVIEW and success:
        REVIEW.mkdir(parents=True,exist_ok=True)
        (REVIEW/'acquisition-parent.json').write_text(json.dumps(ParentTests.evidence,indent=2)+'\n')
    raise SystemExit(not success)
