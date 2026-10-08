#!/usr/bin/env python3
"""Required original artillery ammunition ownership and invalid retained lifetimes."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import time
import unittest

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_support_audio_oracle import OriginalSupportAudioOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_ground import contact
from test_original_ground_maneuver import SEEDS, seed_for_draw
from test_original_mission_ready import complete_expected
from test_units import snapshot
from test_vehicle_motion import start
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_EBX, UC_X86_REG_CX,
                              UC_X86_REG_DI, UC_X86_REG_EFLAGS)


class OwnershipTests(unittest.TestCase):
    results = {}
    digest = hashlib.sha256()

    def retain_state(self, data):
        self.digest.update(data[:0x8fc0])
        self.digest.update(data[0x9000:])

    def test_01_prepared_ammunition_owner_all_classes_variants_saved_words(self):
        audio = OriginalAudioRequestOracle()
        owner = OriginalSupportAudioOracle(audio)
        for offset, raw in ((0xb445, 'c7451f0500'), (0xb3e9, 'c7451f0000'),
                            (0x1aeda, '837c1f00'), (0x1af03, 'ff4c1f')):
            expected = bytes.fromhex(raw)
            self.assertEqual(owner.image[offset:offset + len(expected)], expected)
        results = []
        for kind, variant, saved_rounds in itertools.product(range(4), (0, 1), (0, 65535)):
            raw = bytearray(start(kind))
            raw[22] = 0
            gun = bytearray(snapshot(27, flags=8 | variant))
            gun[25] = variant
            store(gun, 0x1f, saved_rounds)
            machine, objects = owner.prepare_saved([(0, 1, bytes(raw)), (1, 2, bytes(gun))],
                SEEDS, 0, 0, (bytes(2144), bytes(176)))
            actor, resource = objects[150][2], objects[0][2]
            machine.mem_write(DGROUP + 0x6cde, struct.pack('<H', 1800))
            owner.call(machine, 0xb2a2)
            machine.reg_write(UC_X86_REG_EBX, 0x44445678)
            guards = {address: bytes(machine.mem_read(address, size)) for address, size in
                      ((0, DGROUP), (0x2d740, 65536), (0x40000, 4096))}
            # The actual e1d1 height wrapper copies EBX to mailbox +3f2.
            # c164 supplies this gun's registry-entry address, preserving the
            # declared high half; predict the complete packet before execution.
            mailbox = bytearray(guards[0x40000])
            struct.pack_into('<I', mailbox, 0x3f2, 0x44440000 | (0xdfbc + 4))
            guards[0x40000] = mailbox
            before, after, transfers = owner.reset(machine, bytes([17]) * 4)
            x, y = struct.unpack_from('<2i', before, resource + 4)
            self.assertEqual(transfers, [(1, resource, contact(2, bytes([17]) * 4, (x, y, 0))[0])])
            expected, _, _, lists = complete_expected(before, transfers)
            self.assertEqual(after[:0x8fc0], expected[:0x8fc0])
            self.assertEqual(after[0x9000:], expected[0x9000:])
            for address, guard in guards.items():
                self.assertEqual(bytes(machine.mem_read(address, len(guard))), guard)
            self.retain_state(after)
            self.assertEqual(word(after, resource + 0x1f), 5)
            self.assertEqual(lists[1], [resource])
            data = bytearray(after)
            data[actor + 22] |= 8
            data[actor + 0x43] = 4
            for offset, value in ((actor + 0x97, resource), (actor + 0x99, 20),
                                  (0x9794, 0), (0x6d34, actor), (0x7ae0, 0)):
                store(data, offset, value)
            struct.pack_into('<5H', data, 0x1f82, 0x1f8a, seed_for_draw(0x4120), *SEEDS[1:])
            machine.mem_write(DGROUP, bytes(data))
            after, effect = owner.observe(machine, actor, audio.fixture())
            self.assertEqual(effect['support'], 'artillery_confirmed')
            self.assertEqual(word(after, resource + 0x1f), 4)
            self.retain_state(after)
            results.append({'class': kind, 'saved_variant': variant, 'saved_rounds': saved_rounds,
                'prepared_rounds': 5, 'after_support_rounds': 4,
                'deleted_flag': bool(after[resource + 22] & 1)})
        audio.verify_assets()
        self.assertEqual(len(results), 16)
        self.results['prepared_ammunition'] = results

    def test_02_actual_released_and_same_type_reused_resource(self):
        audio = OriginalAudioRequestOracle()
        owner = OriginalSupportAudioOracle(audio)
        cases = []
        for kind in range(4):
            for successor in (False, True):
                machine, actor = owner.fixture(kind, draw=0x4120)
                old = bytes(machine.mem_read(DGROUP, 65536))
                resource = word(old, 0x9cd7)
                raw = bytearray(old[resource:resource + 55])
                old_binding = bytes(machine.mem_read(DGROUP + 0xdfbc + 2 * 4, 4))
                machine.reg_write(UC_X86_REG_AX, 2)
                owner.far_call(machine, 0x1b2ef)
                self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0xdfbc + 2 * 4), 0)
                self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0x9cd7), resource)
                if successor:
                    machine.reg_write(UC_X86_REG_AX, 27)
                    machine.reg_write(UC_X86_REG_BX, 2)
                    machine.reg_write(UC_X86_REG_CX, 3)
                    owner.far_call(machine, 0x1b1a2)
                    self.assertEqual(machine.reg_read(UC_X86_REG_EFLAGS) & 1, 0)
                    self.assertEqual(machine.reg_read(UC_X86_REG_DI), resource)
                    self.assertEqual(bytes(machine.mem_read(DGROUP + 0xdfbc + 2 * 4, 4)), old_binding)
                    # The saved-byte adapter supplies a new normal-side type27 record.
                    # The unchanged complete saved continuation executes, without rebuilding
                    # the old side-eight support list. This explicitly declares invalid
                    # retained-resource state; it does not invent natural game reachability.
                    raw[22] = 0
                    machine.mem_write(DGROUP + resource, bytes(raw))
                    owner.call(machine, 0xd84a)
                after, effect = owner.observe(machine, actor, audio.fixture())
                self.assertEqual(effect['support'], 'artillery_confirmed')
                self.assertEqual(word(after, resource + 0x1f), 1)
                self.retain_state(after)
                cases.append({'class': kind, 'resource_slot': owner.slot(resource), 'released': True,
                    'reused_same_type_binding': successor, 'ammo_spent': True,
                    'invalid_retained_resource': True})
        audio.verify_assets()
        self.assertEqual(len(cases), 8)
        self.results['invalid_retained_resources'] = cases


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'support-ownership.json'
    output.unlink(missing_ok=True)
    begin = time.monotonic()
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 2 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 2, 'skips': 0, 'seconds': time.monotonic() - begin,
            'counts': {'complete_original_preparation_returns': 16,
                       'complete_original_support_returns': 24,
                       'actual_original_release_returns': 8,
                       'actual_original_allocation_saved_pairs': 4},
            'cases': OwnershipTests.results, 'original_image_sha256': IMAGE_SHA256,
            'output_sha256': hashlib.sha256(json.dumps(OwnershipTests.results, sort_keys=True).encode()).hexdigest(),
            'whole_state_sha256': OwnershipTests.digest.hexdigest(),
            'scope': 'Original support ammunition ownership and explicitly invalid retained-resource lifetimes; no ordinary battle reachability/shared C/full0104 acceptance',
            'complete_wasm_streak': 0}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps({k: v for k, v in result.items() if k != 'cases'}, sort_keys=True), flush=True)
    raise SystemExit(not success)
