#!/usr/bin/env python3
"""Complete original maneuver-field retention and unused target/RNG admission."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_mission_ready_oracle import RESET_BANK
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_original_mission_ready import ground_reset
from test_vehicle_motion import start
from test_vehicle_start import initialized


class RetentionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.digest = hashlib.sha256()
        cls.evidence = {}

    def test_complete_class_start_and_ready_keep_all_maneuver_byte_domains(self):
        from unicorn.x86_const import UC_X86_REG_DI
        owner = OriginalGroundManeuverOracle()
        contexts = 0
        for kind, link, value in itertools.product(range(4), range(3), range(256)):
            cursor = value % 4
            machine = owner.machine(SEEDS, cursor, link)
            raw = bytearray(start(kind))
            fields = {0x45: value, 0x46: value ^ 255, 0x51: (value * 17) & 255}
            for offset, retained in fields.items():
                raw[offset] = retained
            store(raw, 0x47, value * 257)
            records, random = initialized([(0, 0, bytes(raw))], SEEDS, cursor, link)
            expected = records[0][2]
            machine.mem_write(DGROUP + 0x7000, bytes(raw))
            machine.reg_write(UC_X86_REG_DI, 0x7000)
            owner.call(machine, 0xc296)
            actual = bytes(machine.mem_read(DGROUP + 0x7000, 251))
            self.assertEqual(actual, expected)
            for offset, retained in fields.items():
                self.assertEqual(actual[offset], retained)
            self.assertEqual(word(actual, 0x47), value * 257)
            self.assertEqual(owner.random_state(machine), random)
            self.digest.update(actual)
            before = bytes(machine.mem_read(DGROUP, 65536))
            owner.call(machine, RESET_BANK[kind])
            after = bytes(machine.mem_read(DGROUP, 65536))
            actual = after[0x7000:0x70fb]
            self.assertEqual(actual, ground_reset(expected, link))
            for offset, retained in fields.items():
                self.assertEqual(actual[offset], retained)
            self.assertEqual(word(actual, 0x47), value * 257)
            self.assertEqual(owner.random_state(machine), random)
            begin = 0
            for first, end in ((0x7000, 0x70fb), (0x8fc0, 0x9002), (65536, 65536)):
                self.assertEqual(before[begin:first], after[begin:first])
                begin = end
            self.assertEqual(machine.reg_read(UC_X86_REG_DI), 0x7000)
            self.digest.update(actual)
            contexts += 1
        self.assertEqual(contexts, 3072)
        self.evidence['retention'] = {'contexts': contexts, 'complete_start_returns': contexts,
            'complete_ready_returns': contexts, 'byte_domains': [0x45, 0x46, 0x51],
            'word_retained': 0x47, 'links': 3, 'classes': 4, 'rng_preserved_after_ready': True}

    def test_actual_released_reused_target_presence_and_ignored_invalid_rng(self):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
                                      UC_X86_REG_DI, UC_X86_REG_EFLAGS)
        owner = OriginalGroundManeuverOracle()
        contexts = returns = 0
        for kind, target_kind, flags, cursor in itertools.product(range(4), range(4), (0, 4), range(4)):
            actor = bytearray(start(kind)); actor[22] = 64; store(actor, 0x40, flags)
            target = bytearray(start(target_kind)); target[22] = 64
            machine, objects = owner.prepare_saved([(0, 1, bytes(actor)), (1, 1, bytes(target))],
                SEEDS, cursor, 0, (bytes(2144), bytes(176)))
            receiver, retained = objects[150][2], objects[151][2]
            machine.mem_write(DGROUP + receiver + 0x97, struct.pack('<H', retained))
            # No RNG word/cursor is used after this early admission gate.
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H', 65535, 0xdead, 0xbeef, 1, 2))
            for stage in ('live', 'released', 'reused'):
                if stage == 'released':
                    machine.reg_write(UC_X86_REG_AX, 1)
                    owner.far_call(machine, 0x1b2ef)
                    self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0xdfc0), 0)
                elif stage == 'reused':
                    machine.reg_write(UC_X86_REG_AX, target_kind)
                    machine.reg_write(UC_X86_REG_BX, 1)
                    machine.reg_write(UC_X86_REG_CX, 1)
                    owner.far_call(machine, 0x1b1a2)
                    self.assertFalse(machine.reg_read(UC_X86_REG_EFLAGS) & 1)
                    self.assertEqual(machine.reg_read(UC_X86_REG_DI), retained)
                    self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0xdfc0), retained)
                before = bytes(machine.mem_read(DGROUP, 65536))
                after, effect = owner.observe(machine, receiver, operation='idle_turret')
                self.assertEqual(effect['random_draws'], 0)
                self.assertEqual(word(after, receiver + 0x97), retained)
                self.assertEqual(after[0x1f82:0x1f8c], before[0x1f82:0x1f8c])
                self.digest.update(after[:0x8fc0]); self.digest.update(after[0x9000:])
                returns += 1
            # Explicit target clearing, not release/reuse, reopens random idle work.
            machine.mem_write(DGROUP + receiver + 0x97, bytes(2))
            words = list(SEEDS); words[cursor] = seed_for_draw(65); words[(cursor + 1) % 4] = seed_for_draw(0)
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                0x1f84 + ((cursor + 3) % 4) * 2, *words))
            after, effect = owner.observe(machine, receiver, operation='idle_turret')
            self.assertEqual(effect['random_draws'], 0 if flags & 4 else 2)
            self.digest.update(after[:0x8fc0]); self.digest.update(after[0x9000:])
            returns += 1
            contexts += 1
        self.assertEqual((contexts, returns), (128, 512))
        self.evidence['target_presence'] = {'actual_allocation_contexts': contexts,
            'complete_idle_returns': returns, 'live_released_reused_returns': 384,
            'explicit_clear_returns': 128, 'unused_invalid_rng_cursor_ignored': True,
            'semantics': 'A retained nonzero target is a presence gate without dereference; release/reuse alone neither clears it nor consumes RNG'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    if args.review_dir and (not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp'))
                            or args.review_dir.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 2 and not program.result.skipped
    if success and args.review_dir:
        args.review_dir.mkdir(parents=True, exist_ok=True)
        result = {**RetentionTests.evidence, 'success': True, 'groups': 2, 'skips': 0,
                  'original_image_sha256': IMAGE_SHA256, 'output_sha256': RetentionTests.digest.hexdigest(),
                  'scope': 'Complete original maneuver retention and target-presence gate; shared C remains open',
                  'complete_wasm_streak': 0}
        (args.review_dir / 'ground-maneuver-retention.json').write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result, sort_keys=True), flush=True)
    raise SystemExit(not success)
