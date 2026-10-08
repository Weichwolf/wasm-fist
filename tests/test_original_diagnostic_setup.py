#!/usr/bin/env python3
"""Complete genuine aaa8 diagnostic setup and prepared selected-parent coupling.

This executes the unchanged original frame/title/string methods. Caption1 in
the coupled test is an explicit caller input; the aad0 timed/device thunk is
not configured, replaced or claimed by this proof.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import struct
import time
import unittest

from original_selected_diagnostic_oracle import OriginalSelectedDiagnosticOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from remaining_ground_corpus import Corpus
from roster_promotion_contract import store
from selected_diagnostic_contract import STACK_BEGIN, STACK_END, without_stack

LABELS = (' INHIBIT:OFF', '  ACTION:', ' ACT-CTR:', '   PINFO:', ' P.SPEED:',
          '  P.PNTS:', ' DAISAVE:', ' DISTANC:', 'NR ENEMY:', ' ENEMIES:', 'AVOIDMOD:', 'SEEKMODE:')


def setup(before, screen):
    data, output = bytearray(before), bytearray(screen)
    data[0x978a] = 0
    attribute = output[11 * 160 + 1]

    def put(row, column, text, attr=None):
        for index, character in enumerate(text):
            position = row * 160 + (column + index) * 2
            output[position] = character
            if attr is not None:
                output[position + 1] = attr

    put(11, 0, bytes([0xda]) + bytes([0xc4]) * 14 + bytes([0xbf]), attribute)
    for row in range(12, 24):
        put(row, 0, bytes([0xb3]) + b' ' * 14 + bytes([0xb3]), attribute)
    put(24, 0, bytes([0xc0]) + bytes([0xc4]) * 14 + bytes([0xd9]), attribute)
    put(11, 5, b'DAIMGR')
    for row, label in enumerate(LABELS, start=12):
        put(row, 1, label.encode('ascii'), 7)
    store(data, 0x29c, 12 * 160 + 2)
    return bytes(data), bytes(output)


class SetupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.begin = time.monotonic()
        cls.owner = OriginalSelectedDiagnosticOracle()
        cls.world = next(Corpus().worlds())
        cls.actor = next(iter(cls.world.ground_actors().values()))
        cls.counts, cls.digest = collections.Counter(), hashlib.sha256()

    def check_setup(self, machine, screen):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SI
        address = self.owner.screen_address(machine)
        machine.mem_write(address, screen)
        before = bytes(machine.mem_read(0, 0x60000))
        expected, output = setup(before[DGROUP:DGROUP + 65536], screen)
        self.owner.call(machine, 0xaaa8)
        after = bytes(machine.mem_read(0, 0x60000))
        whole = bytearray(before)
        whole[DGROUP:DGROUP + 65536] = expected
        whole[address:address + 4000] = output
        whole[DGROUP + STACK_BEGIN:DGROUP + STACK_END] = after[DGROUP + STACK_BEGIN:DGROUP + STACK_END]
        self.assertEqual(after, bytes(whole))
        self.assertEqual((machine.reg_read(UC_X86_REG_DI), machine.reg_read(UC_X86_REG_SI)), (1782, 0x997d))
        self.digest.update(without_stack(after[DGROUP:DGROUP + 65536]) + output)
        self.counts['complete_setup_returns'] += 1

    def test_all_admission_bytes_and_frame_attributes_in_two_contexts(self):
        for segment in (0x4000, 0x5000):
            for value in range(256):
                machine = self.owner.relocated(self.world.data, text_segment=segment)
                machine.mem_write(DGROUP + 0x978a, bytes([value]))
                screen = bytearray((index * 31 + value) & 255 for index in range(4000))
                screen[11 * 160 + 1] = value
                self.check_setup(machine, bytes(screen))
                self.counts['admission_attribute_contexts'] += 1

    def test_complete_setup_then_genuine_selected_parent_tail(self):
        for kind in range(4):
            for segment in (0x4000, 0x5000):
                for caption in (0, 1):
                    for child in (5, 8, 9, 10, 12, 14):
                        machine = self.owner.relocated(self.world.data, text_segment=segment)
                        machine.mem_write(DGROUP + self.actor, struct.pack('<H', kind))
                        machine.mem_write(DGROUP + self.actor + 0x40, struct.pack('<H', caption))
                        machine.mem_write(DGROUP + self.actor + 0x42, bytes([child - 1]))
                        machine.mem_write(DGROUP + 0x7ae0, struct.pack('<H', self.actor))
                        self.check_setup(machine, b' \x07' * 2000)
                        machine.mem_write(DGROUP + 0x978a, bytes([caption]))
                        data, output = self.owner.observe(machine, self.actor, entry=0xab03)
                        self.digest.update(without_stack(data) + output)
                        self.counts['complete_coupled_parent_returns'] += 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    destination = args.review_dir / 'diagnostic-setup.json'
    destination.unlink(missing_ok=True)
    result = unittest.TextTestRunner(verbosity=1).run(unittest.defaultTestLoader.loadTestsFromTestCase(SetupTests))
    counts = dict(SetupTests.counts)
    success = result.wasSuccessful() and not result.skipped and result.testsRun == 2
    success = success and counts == {'complete_setup_returns': 608, 'admission_attribute_contexts': 512,
                                    'complete_coupled_parent_returns': 96}
    if success:
        evidence = {'success': True, 'groups': 2, 'skips': 0, 'counts': counts,
                    'output_sha256': SetupTests.digest.hexdigest(), 'original_image_sha256': IMAGE_SHA256,
                    'seconds': time.monotonic() - SetupTests.begin,
                    'scope': 'Complete original aaa8 frame/title/label setup, admission reset and genuine selected-parent coupling; caption1 is explicit caller input, aad0/device/full parent/shared C remain open.',
                    'complete_wasm_streak': 0}
        destination.write_text(json.dumps(evidence, indent=2) + '\n')
        print(json.dumps(evidence, sort_keys=True), flush=True)
    raise SystemExit(0 if success else 1)
