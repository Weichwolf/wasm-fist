#!/usr/bin/env python3
"""Required full original diagnostic returns and genuine selected-parent tails.

Standalone original proof, not shared C or complete parent/class acceptance.
Missing originals/dependency/output, failed comparisons and skipped groups fail.
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
from roster_promotion_contract import store, word
from selected_diagnostic_contract import GENUINE_CONTROLLED_RET, STACK_BEGIN, STACK_END, without_stack

REVIEW = None
SCREEN = bytes((index * 37 + 19) & 255 for index in range(4000))


class DiagnosticTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.begin = time.monotonic()
        cls.owner = OriginalSelectedDiagnosticOracle()
        cls.corpus = Corpus()
        cls.world = next(cls.corpus.worlds())
        cls.actor = next(iter(cls.world.ground_actors().values()))
        cls.counts, cls.groups = collections.Counter(), {}
        cls.digest = hashlib.sha256()

    def setUp(self):
        self.start_count = self.counts['complete_returns']
        print('Required diagnostic group: ' + self._testMethodName, flush=True)

    def tearDown(self):
        self.groups[self._testMethodName] = self.counts['complete_returns'] - self.start_count
        print('Complete diagnostic/parent returns: ' + str(self.counts['complete_returns']), flush=True)

    def fixture(self, *, kind=0, platoon=0, caption=0, text_segment=0x5000):
        machine = self.owner.relocated(self.world.data, text_segment=text_segment)
        machine.mem_write(DGROUP + self.actor, struct.pack('<H', kind))
        machine.mem_write(DGROUP + self.actor + 27, bytes([platoon]))
        machine.mem_write(DGROUP + 0x978a, bytes([caption]))
        return machine

    def check(self, machine, actor=None, *, entry=0xb152):
        data, output = self.owner.observe(machine, self.actor if actor is None else actor,
                                          entry=entry, screen=SCREEN)
        self.digest.update(struct.pack('<H', entry) + without_stack(data) + output)
        self.counts['complete_returns'] += 1
        self.counts['diagnostic_returns' if entry == 0xb152 else 'parent_returns'] += 1
        if self.counts['complete_returns'] % 32768 == 0:
            print('Whole-memory checked returns: ' + str(self.counts['complete_returns']), flush=True)
        return data, output

    def test_genuine_relocation_and_complete_text_contexts(self):
        for segment in (0x4000, 0x5000):
            for kind in range(4):
                for caption in (0, 1):
                    machine = self.fixture(kind=kind, caption=caption, text_segment=segment)
                    random = self.owner.random_state(machine)
                    self.check(machine)
                    self.assertEqual(self.owner.random_state(machine), random)
                    self.counts['genuine_relocation_returns'] += 1
        self.assertEqual(len(self.owner.near_pairs), 23)

    def test_every_word_in_each_diagnostic_source(self):
        machine = self.fixture()
        descriptor = word(bytes(machine.mem_read(DGROUP, 65536)), 0x85a0)
        fields = (self.actor + 0x57, self.actor + 0x53, self.actor + 0x9d,
                  0x97ee, descriptor + 6)
        for offset in fields:
            print(f'Complete diagnostic word domain: {offset:#x}', flush=True)
            for value in range(65536):
                machine.mem_write(DGROUP + offset, struct.pack('<H', value))
                self.check(machine)
                self.counts['word_domain_returns'] += 1

    def test_every_byte_and_all_platoon_member_pairs(self):
        machine = self.fixture()
        route = word(bytes(machine.mem_read(DGROUP, 65536)), 0x7d2a)
        for offset in (self.actor + 0x43, self.actor + 0x94, self.actor + 0x45,
                       self.actor + 0x1c, route):
            for value in range(256):
                machine.mem_write(DGROUP + offset, bytes([value]))
                self.check(machine)
                self.counts['byte_domain_returns'] += 1
        for kind in range(4):
            for platoon in range(8):
                for caption in (0, 1):
                    machine = self.fixture(kind=kind, platoon=platoon, caption=caption)
                    for member in range(256):
                        machine.mem_write(DGROUP + self.actor + 0x1c, bytes([member]))
                        self.check(machine)
                        self.counts['platoon_member_returns'] += 1

    def test_all47_actual_prepared_worlds_and_authored_actors(self):
        missions, kinds = {}, set()
        for world in self.corpus.worlds():
            machine = self.owner.relocated(world.data)
            details = missions.setdefault(world.name, set())
            self.assertNotIn(world.side, details, 'Duplicate prepared mission/detail')
            details.add(world.side)
            for actor in world.ground_actors().values():
                kinds.add(word(world.data, actor))
                for caption in (0, 1):
                    machine.mem_write(DGROUP + 0x978a, bytes([caption]))
                    self.check(machine, actor)
                    self.counts['canonical_actor_returns'] += 1
            self.counts['prepared_worlds'] += 1
        self.assertEqual((len(missions), kinds), (47, {0, 1, 2, 3}))
        self.assertTrue(all(details == {512, 1024, 2048, 4096} for details in missions.values()))
        self.assertEqual(self.counts['canonical_actor_returns'], 7680)

    def test_genuine_selected_and_unselected_parent_tails(self):
        from unicorn.x86_const import UC_X86_REG_DI
        for kind in range(4):
            for platoon in range(8):
                machine = self.fixture(kind=kind, platoon=platoon)
                for counter in range(256):
                    if (counter + 1) & 15 not in GENUINE_CONTROLLED_RET:
                        continue
                    machine.mem_write(DGROUP + self.actor + 0x40, bytes(2))
                    machine.mem_write(DGROUP + self.actor + 0x42, bytes([counter]))
                    machine.mem_write(DGROUP + 0x7ae0, struct.pack('<H', self.actor))
                    self.check(machine, entry=0xab03)
                    self.counts['selected_controlled_ret_returns'] += 1
            for admission in range(256):
                for flags in ((0,) if admission == 0 else (0, 1, 2, 32768, 65535)):
                    machine = self.fixture(kind=kind, caption=admission)
                    machine.mem_write(DGROUP + self.actor + 0x40, struct.pack('<H', flags))
                    machine.mem_write(DGROUP + self.actor + 0x42, b'\x04')
                    machine.mem_write(DGROUP + 0x7ae0, bytes(2))
                    self.check(machine, entry=0xab03)
                    self.counts['unselected_admission_returns'] += 1
            machine = self.fixture(kind=kind, caption=1)
            machine.mem_write(DGROUP + 0x7ae0, struct.pack('<H', self.actor))
            for counter in range(256):
                for flags in (0, 1, 65535):
                    machine.mem_write(DGROUP + self.actor + 0x40, struct.pack('<H', flags))
                    machine.mem_write(DGROUP + self.actor + 0x42, bytes([counter]))
                    self.check(machine, entry=0xab03)
                    self.assertEqual(machine.reg_read(UC_X86_REG_DI), self.actor)
                    self.counts['selected_skipped_bank_returns'] += 1

    def test_used_invalid_table_reads_are_negative_prefix_evidence(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_SI, UC_X86_REG_SP
        for caption in range(2, 256):
            machine = self.fixture(caption=caption)
            machine.reg_write(UC_X86_REG_DI, self.actor)
            machine.reg_write(UC_X86_REG_SP, 0x9000)
            machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            before = bytes(machine.mem_read(0, 0x60000))
            self.owner.execute(machine, 0xb152, 0xb163)
            data = bytearray(before[DGROUP:DGROUP + 65536])
            store(data, 0x9a06, self.actor)  # b153 precedes the invalid table fetch.
            self.assertEqual(machine.reg_read(UC_X86_REG_SI), word(data, 0x99fa + caption * 2))
            expected = bytearray(before)
            expected[DGROUP + 0x9a06:DGROUP + 0x9a08] = struct.pack('<H', self.actor)
            after = bytes(machine.mem_read(0, 0x60000))
            expected[DGROUP + STACK_BEGIN:DGROUP + STACK_END] = after[DGROUP + STACK_BEGIN:DGROUP + STACK_END]
            self.assertEqual(after, bytes(expected))
            self.counts['invalid_caption_prefixes'] += 1
        # No fake return: these intentionally stop at the actual first bad
        # descriptor-table read. Their count is separate from complete returns.
        for platoon in range(8, 256):
            machine = self.fixture(platoon=platoon)
            machine.reg_write(UC_X86_REG_DI, self.actor)
            machine.reg_write(UC_X86_REG_SP, 0x9000)
            machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            before = bytes(machine.mem_read(0, 0x60000))
            self.owner.execute(machine, 0xb152, 0xb1a6)
            data = before[DGROUP:DGROUP + 65536]
            self.assertEqual(machine.reg_read(UC_X86_REG_AX), word(data, 0x85a0 + platoon * 2))
            expected = bytearray(before)
            for offset, value in ((0x9a06, self.actor), (0x3a, 0x5e50), (0x29c, 15 * 160 + 20)):
                expected[DGROUP + offset:DGROUP + offset + 2] = struct.pack('<H', value)
            address = self.owner.screen_address(machine)
            for row, column, text, attribute in ((12, 10, 'OFF', 7),
                    (13, 10, f'{data[self.actor + 0x43]:02X}', None),
                    (14, 10, f'{word(data, self.actor + 0x57):04X}', None)):
                for index, character in enumerate(text.encode('ascii')):
                    position = address + row * 160 + (column + index) * 2
                    expected[position] = character
                    if attribute is not None:
                        expected[position + 1] = attribute
            after = bytes(machine.mem_read(0, 0x60000))
            expected[DGROUP + STACK_BEGIN:DGROUP + STACK_END] = after[DGROUP + STACK_BEGIN:DGROUP + STACK_END]
            self.assertEqual(after, bytes(expected))
            self.counts['invalid_platoon_prefixes'] += 1
        self.assertEqual((self.counts['invalid_caption_prefixes'], self.counts['invalid_platoon_prefixes']), (254, 248))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    REVIEW = args.review_dir
    REVIEW.mkdir(parents=True, exist_ok=True)
    destination = REVIEW / 'selected-diagnostic.json'
    destination.unlink(missing_ok=True)
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(DiagnosticTests)
    result = unittest.TextTestRunner(verbosity=1).run(suite)
    counts = dict(DiagnosticTests.counts)
    required = {'word_domain_returns': 327680, 'byte_domain_returns': 1280,
                'platoon_member_returns': 16384, 'canonical_actor_returns': 7680,
                'prepared_worlds': 188, 'selected_controlled_ret_returns': 3072,
                'unselected_admission_returns': 5104, 'selected_skipped_bank_returns': 3072,
                'genuine_relocation_returns': 16, 'invalid_caption_prefixes': 254,
                'invalid_platoon_prefixes': 248}
    success = result.wasSuccessful() and not result.skipped and result.testsRun == 6
    success = success and all(counts.get(k) == v for k, v in required.items())
    if success:
        evidence = {'success': True, 'groups': result.testsRun, 'skips': 0, 'counts': counts,
                    'output_sha256': DiagnosticTests.digest.hexdigest(), 'original_image_sha256': IMAGE_SHA256,
                    'seconds': time.monotonic() - DiagnosticTests.begin,
                    'scope': 'Complete original selected diagnostic, genuine text relocation/services and declared selected/unselected parent tails; invalid prefix reads are negative evidence only. No shared C/full parent/class/battle/PCM acceptance.',
                    'group_complete_returns': DiagnosticTests.groups, 'complete_wasm_streak': 0}
        destination.write_text(json.dumps(evidence, indent=2) + '\n')
        print(json.dumps(evidence, sort_keys=True), flush=True)
    raise SystemExit(0 if success else 1)
