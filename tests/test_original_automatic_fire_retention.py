#!/usr/bin/env python3
"""Required complete original missile-rack initialization/readiness retention."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from original_automatic_fire_oracle import OriginalAutomaticFireOracle
from original_mission_ready_oracle import RESET_BANK
from original_target_acquisition_oracle import MAILBOX, TEXT_BASE
from original_unit_oracle import DGROUP, IMAGE_SHA256
from test_original_mission_ready import ground_reset
from test_vehicle_motion import start
from test_vehicle_start import initialized, step

ACTOR = 0x7000
SEEDS = (1, 2, 32768, 65535)


class RetentionTests(unittest.TestCase):
    evidence = None

    def test_complete_both_class_rack_bytes_trigger_station_and_link_retention(self):
        from unicorn.x86_const import UC_X86_REG_CS, UC_X86_REG_DI, UC_X86_REG_SP
        owner = OriginalAutomaticFireOracle()
        digest = hashlib.sha256()
        contexts = 0
        for kind, link, value in itertools.product((1, 3), range(3), range(256)):
            cursor = value % 4
            machine = owner.machine(SEEDS, cursor, link)
            raw = bytearray(start(kind))
            raw[0xb5:0xb9] = bytes((value, value ^ 255, value, value ^ 255))
            raw[0x91], raw[0xa5], raw[0x92] = value, value ^ 255, value
            machine.mem_write(DGROUP + ACTOR, bytes(raw))
            records, random = initialized([(0, 0, bytes(raw))], SEEDS, cursor, link)
            expected_actor = records[0][2]
            words = list(SEEDS)
            _, following = step(words, cursor)
            last, following = step(words, following)
            self.assertEqual((words, following), random)
            for entry, actor_bytes in ((0xc296, expected_actor),
                                       (RESET_BANK[kind], ground_reset(expected_actor, link))):
                machine.reg_write(UC_X86_REG_DI, ACTOR)
                machine.reg_write(UC_X86_REG_SP, 0x9000)
                machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
                before = bytes(machine.mem_read(DGROUP, 65536))
                expected = bytearray(before)
                expected[ACTOR:ACTOR + 251] = actor_bytes
                if entry == 0xc296:
                    struct.pack_into('<5H', expected, 0x1f82,
                                     0x1f84 + ((following + 3) % 4) * 2, *words)
                    struct.pack_into('<H', expected, 0x342, last)
                code = bytes(machine.mem_read(0, DGROUP))
                text = bytes(machine.mem_read(TEXT_BASE, 65536))
                mailbox = bytes(machine.mem_read(MAILBOX, 4096))
                owner.execute(machine, entry, 0xeff0)
                after = bytes(machine.mem_read(DGROUP, 65536))
                expected[0x8fc0:0x9000] = after[0x8fc0:0x9000]
                if after != bytes(expected):
                    changes = [hex(i) for i, (a, b) in enumerate(zip(after, expected)) if a != b]
                    self.fail('Complete original rack retention DGROUP differs: ' + str(changes[:20]))
                self.assertEqual(after[ACTOR + 0xb5:ACTOR + 0xb9], bytes((1, 1, value, value ^ 255)))
                self.assertEqual((after[ACTOR + 0x91], after[ACTOR + 0xa5], after[ACTOR + 0x92]),
                                 (value, value ^ 255, value))
                self.assertEqual(owner.random_state(machine), random)
                self.assertEqual(tuple(machine.reg_read(reg) for reg in owner.machine_registers()),
                                 (ACTOR, 0x1c00, 0x9002, 0x1c00))
                self.assertEqual(machine.reg_read(UC_X86_REG_CS), 0)
                self.assertEqual(bytes(machine.mem_read(0, DGROUP)), code)
                self.assertEqual(bytes(machine.mem_read(TEXT_BASE, 65536)), text)
                self.assertEqual(bytes(machine.mem_read(MAILBOX, 4096)), mailbox)
                digest.update(after[:0x8fc0])
                digest.update(after[0x9000:])
            contexts += 1
        self.assertEqual(contexts, 1536)
        type(self).evidence = {'success': True, 'groups': 1, 'skips': 0, 'contexts': contexts,
            'complete_start_returns': contexts, 'complete_ready_returns': contexts,
            'classes': [1, 3], 'link_modes': [0, 1, 2], 'retained_byte_domains': [0xb7, 0xb8, 0x91, 0xa5, 0x92],
            'reserve_bytes_reset_by_start_retained_by_ready': [0xb5, 0xb6],
            'whole_dgroup_code_text_mailbox_and_return_checked': True,
            'original_image_sha256': IMAGE_SHA256, 'output_sha256': digest.hexdigest(),
            'scope': 'Complete original missile-rack input ownership; shared C consumption remains open',
            'complete_wasm_streak': 0}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    if args.review_dir and (not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp'))
                            or args.review_dir.resolve() == pathlib.Path('/tmp')):
        parser.error('Required evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 1 and not program.result.skipped
    if success and args.review_dir:
        args.review_dir.mkdir(parents=True, exist_ok=True)
        (args.review_dir / 'automatic-fire-retention.json').write_text(
            json.dumps(RetentionTests.evidence, indent=2) + '\n')
    raise SystemExit(not success)
