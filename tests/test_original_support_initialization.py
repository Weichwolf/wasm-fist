#!/usr/bin/env python3
"""Complete original support reset/catalog producer and ordered artillery list.

Catalog bytes are an explicit already-decoded input, not an invented campaign
parser. All original producer instructions, copies and near/far returns execute.
"""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import time
import unittest

from original_unit_oracle import DGROUP, IMAGE_SHA256, SERVICE_CS
from original_vehicle_motion_oracle import OriginalVehicleMotionOracle
from roster_promotion_contract import store, word

CATALOG = 0x6dda
MAILBOX = 0x40000
TEXT = 0x2d740


def reset(before, entry):
    data = bytearray(before)
    clock = word(data, 0x6cde)
    if entry == 0x9d3c:
        for offset in (0x9460, 0x9462):
            store(data, offset, clock - 480)
        store(data, 0x944c, 6 if word(data, 0x6db4) else 5)
        store(data, 0x944e, 5 if word(data, 0x6db4) else 6)
        for base in (0x9464, 0x9524):
            for i in range(16):
                store(data, base + i * 12, 0)
    elif entry == 0xb2a2:
        for offset in (0x9ccb, 0x9ccd, 0x9c89):
            store(data, offset, 0)
        for offset in (0x9f17, 0x9f19):
            store(data, offset, clock - 480)
        for base in (0x9ce7, 0x9dc7):
            for i in range(16):
                store(data, base + i * 14, 0)
    else:
        raise ValueError('Unknown complete support initializer')
    return data


def catalog(before, mailbox):
    data, inbox = bytearray(before), bytearray(mailbox)
    for source, destination in ((0x4c, 0x7a), (0x3c, 0x8a), (0x5c, 0x9a), (0x6c, 0xaa)):
        inbox[destination:destination + 16] = data[CATALOG + source:CATALOG + source + 16]
    data[0x79a9:0x79b9] = data[CATALOG + 0x21:CATALOG + 0x31]
    data[0x6dae] = data[CATALOG + 0xfc]
    store(data, 0x6dbc, word(data, CATALOG + 0x31))
    for source, destination in ((0x33, 0xe3ae), (0x34, 0xe3b0)):
        store(data, destination, data[CATALOG + source] + 1)
    for source, destination, factor in ((0x35, 0x9ce3, 0xfa8a), (0x36, 0x9ce5, 0xfa8a),
                                         (0x37, 0x9454, 0xfa8c), (0x38, 0x9456, 0xfa8c)):
        store(data, destination, data[CATALOG + source] * word(data, factor))
    for source, destination in ((0x39, 0x9450), (0x3a, 0x9452)):
        store(data, destination, data[CATALOG + source])
    return data, inbox


class InitializationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalVehicleMotionOracle()
        cls.machine = cls.owner.machine()
        cls.baseline = bytes(cls.machine.mem_read(0, 0x60000))
        cls.count = 0
        cls.counts = collections.Counter()
        cls.groups = {}
        cls.digest = hashlib.sha256()

    def fixture(self, clock=0, side=0):
        from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                      UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                      UC_X86_REG_EBP)
        if self.count == 256:
            self.machine = self.owner.machine()
            self.assertEqual(bytes(self.machine.mem_read(0, 0x60000)), self.baseline)
            self.count = 0
        self.count += 1
        machine = self.machine
        machine.mem_write(0, self.baseline)
        data = bytearray(machine.mem_read(DGROUP, 65536))
        for base, stride in ((0x9464, 12), (0x9524, 12), (0x9ce7, 14), (0x9dc7, 14)):
            for i in range(16):
                data[base + i * stride:base + (i + 1) * stride] = bytes((i * 17 + j + 1) & 255 for j in range(stride))
        for offset in (0x9ccb, 0x9ccd, 0x9c89):
            store(data, offset, 65535)
        store(data, 0x6cde, clock)
        store(data, 0x6db4, side)
        store(data, 0xea2c, 0)
        store(data, 0xea2e, MAILBOX // 16)
        store(data, 0x6db6, 0)
        machine.mem_write(DGROUP, bytes(data))
        for register, value in zip((UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                    UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                    UC_X86_REG_EBP),
                                   (0x11112345, 0x22223456, 0x33334567, 0x44445678,
                                    0x55556789, 0x6666789a, 0x777789ab)):
            machine.reg_write(register, value)
        return machine

    def check(self, entry):
        from unicorn.x86_const import (UC_X86_REG_CS, UC_X86_REG_DI, UC_X86_REG_DS,
                                      UC_X86_REG_SP, UC_X86_REG_SS, UC_X86_REG_ES,
                                      UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                      UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                      UC_X86_REG_EBP, UC_X86_REG_EFLAGS)
        machine = self.machine
        far = entry == 0x1bf45
        if far:
            machine.reg_write(UC_X86_REG_EFLAGS, machine.reg_read(UC_X86_REG_EFLAGS) | 1024)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT, 65536))
        registers = (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                     UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
        predicted = [machine.reg_read(r) for r in registers]
        expected_mailbox = mailbox
        if far:
            expected, expected_mailbox = catalog(before, mailbox)
            low = (before[CATALOG + 0x3a], CATALOG, 0,
                   (before[CATALOG + 0x38] * word(before, 0xfa8c)) >> 16,
                   CATALOG + 0x31, 0x79b9)
            expected_es = 0x1c00
        else:
            expected = reset(before, entry)
            if entry == 0x9d3c:
                low = (6 if word(before, 0x6db4) else 5, 192, 0)
            else:
                low = ((word(before, 0x6cde) - 480) & 65535, 0x9ea7, 0)
            expected_es = machine.reg_read(UC_X86_REG_ES)
        for i, value in enumerate(low):
            predicted[i] = (predicted[i] & 0xffff0000) | value
        self.owner.execute(machine, entry, 0xeff0, SERVICE_CS if far else 0)
        self.assertEqual(bytes(machine.mem_read(DGROUP, 65536)), expected)
        self.assertEqual(bytes(machine.mem_read(MAILBOX, 4096)), expected_mailbox)
        self.assertEqual(bytes(machine.mem_read(0, DGROUP)), code)
        self.assertEqual(bytes(machine.mem_read(TEXT, 65536)), text)
        self.assertEqual([machine.reg_read(r) for r in registers], predicted)
        self.assertEqual((machine.reg_read(UC_X86_REG_CS), machine.reg_read(UC_X86_REG_DS),
                          machine.reg_read(UC_X86_REG_SS), machine.reg_read(UC_X86_REG_SP),
                          machine.reg_read(UC_X86_REG_ES)),
                         (0, 0x1c00, 0x1c00, 0x9004 if far else 0x9002, expected_es))
        if far:
            self.assertEqual(machine.reg_read(UC_X86_REG_EFLAGS) & 1024, 0)
        self.digest.update(expected)
        self.digest.update(expected_mailbox)
        self.digest.update(struct.pack('<7I', *predicted))
        self.counts[hex(entry)] += 1

    def test_01_complete_air_artillery_clock_words_and_air_side_words(self):
        start = sum(self.counts.values())
        for clock in range(65536):
            for side, entry in itertools.product((0, 1), (0x9d3c, 0xb2a2)):
                self.fixture(clock, side)
                self.check(entry)
        for side in range(65536):
            self.fixture(side ^ 65535, side)
            self.check(0x9d3c)
        count = sum(self.counts.values()) - start
        self.assertEqual(count, 327680)
        self.groups['complete_reset_clock_and_side_words_preserved_queue_tails'] = count

    def test_02_complete_catalog_bytes_and_wrapped_delay_factor_words(self):
        start = sum(self.counts.values())
        for value in range(256):
            machine = self.fixture(value * 257, value * 257)
            machine.mem_write(DGROUP + CATALOG, bytes((value + i * 17) & 255 for i in range(253)))
            self.check(0x1bf45)
        for factor in (0xfa8a, 0xfa8c):
            for value in range(65536):
                machine = self.fixture(value, value ^ 65535)
                machine.mem_write(DGROUP + CATALOG, bytes((255 - i) & 255 for i in range(253)))
                machine.mem_write(DGROUP + factor, struct.pack('<H', value))
                self.check(0x1bf45)
        count = sum(self.counts.values()) - start
        self.assertEqual(count, 131328)
        self.groups['complete_catalog_byte_inputs_and_delay_factor_words'] = count


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp')) or args.review_dir.resolve() == pathlib.Path('/tmp'):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    output = args.review_dir / 'support-initialization.json'
    output.unlink(missing_ok=True)
    begin = time.monotonic()
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 2 and not program.result.skipped
    if success:
        result = {'success': True, 'groups': 2, 'skips': 0, 'seconds': time.monotonic() - begin,
                  'counts': dict(InitializationTests.counts), 'coverage': InitializationTests.groups,
                  'output_sha256': InitializationTests.digest.hexdigest(), 'original_image_sha256': IMAGE_SHA256,
                  'scope': 'Complete original 9d3c/b2a2 resets and 1bf45 already-decoded catalog producer; campaign parser/full mission initialization/shared C remain open',
                  'complete_wasm_streak': 0}
        output.write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps(result, sort_keys=True), flush=True)
    raise SystemExit(not success)
