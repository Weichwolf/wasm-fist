#!/usr/bin/env python3
"""Prove independent +99 retention through complete original class starts/readiness."""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from original_ground_throttle_oracle import OriginalGroundThrottleOracle
from original_mission_ready_oracle import RESET_BANK
from original_unit_oracle import DGROUP, IMAGE_SHA256
from test_original_ground_bearing import actor
from test_original_mission_ready import ground_reset
from test_vehicle_start import initialized

REVIEW = None
VALUES = (0, 1, 255, 256, 32767, 32768, 65534, 65535)


class RetentionTests(unittest.TestCase):
    evidence = None

    def test_complete_class_starts_and_ready_methods_all_classes_links_word_boundaries(self):
        from unicorn.x86_const import UC_X86_REG_DI
        owner = OriginalGroundThrottleOracle()
        digest = hashlib.sha256()
        contexts = 0
        for kind in range(4):
            for link in range(3):
                for value in VALUES:
                    seeds, cursor = (1, 2, 32768, 65535), 3
                    machine = owner.machine(seeds, cursor, link)
                    raw = bytearray(actor(kind, flags=65535, target=65535))
                    struct.pack_into('<H', raw, 0x99, value)
                    records, random = initialized([(0, 0, bytes(raw))], seeds, cursor, link)
                    expected = records[0][2]
                    machine.mem_write(DGROUP + 0x7000, bytes(raw))
                    machine.reg_write(UC_X86_REG_DI, 0x7000)
                    owner.call(machine, 0xc296)
                    actual = bytes(machine.mem_read(DGROUP + 0x7000, 251))
                    self.assertEqual(actual, expected)
                    self.assertEqual(struct.unpack_from('<H', actual, 0x99)[0], value)
                    self.assertEqual(owner.random_state(machine), random)
                    digest.update(actual)
                    before = bytes(machine.mem_read(DGROUP, 65536))
                    owner.call(machine, RESET_BANK[kind])
                    after = bytes(machine.mem_read(DGROUP, 65536))
                    actual = after[0x7000:0x70fb]
                    self.assertEqual(actual, ground_reset(expected, link))
                    self.assertEqual(struct.unpack_from('<H', actual, 0x99)[0], value)
                    self.assertEqual(owner.random_state(machine), random)
                    offset = 0
                    for begin, end in ((0x7000, 0x70fb), (0x8fc0, 0x9002), (65536, 65536)):
                        self.assertEqual(before[offset:begin], after[offset:begin])
                        offset = end
                    self.assertEqual(machine.reg_read(UC_X86_REG_DI), 0x7000)
                    digest.update(actual)
                    contexts += 1
        self.assertEqual(contexts, 96)
        type(self).evidence = {
            'scope': 'complete original +99 class-start/readiness retention; C coverage is separate',
            'success': True, 'groups': 1, 'skips': 0, 'class_contexts': contexts,
            'complete_class_start_returns': contexts, 'complete_ready_method_returns': contexts,
            'word_boundaries': list(VALUES), 'link_modes': [0, 1, 2], 'ground_classes': [0, 1, 2, 3],
            'original_image_sha256': IMAGE_SHA256, 'output_sha256': digest.hexdigest(),
            'complete_wasm_streak': 0,
        }


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Review evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 1 and not program.result.skipped
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'range-retention.json').write_text(json.dumps(RetentionTests.evidence, indent=2) + '\n')
    raise SystemExit(0 if success else 1)
