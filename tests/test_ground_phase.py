#!/usr/bin/env python3
"""Whole-world ground command composition, admission and retained RNG on both targets.

Default groups use independent synthetic mission models and require complete
streams without skips. Original corpus/domain/constructor gates remain separate
required acceptance work; this suite alone does not accept the living game.
"""
import argparse
import collections
import copy
import hashlib
import itertools
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from ground_phase_model import observation, synthetic_world
from ground_phase_probe_contract import (CASE_BYTES, encode, fixture, original_input,
                                        world_observation)
from orders_contract import scenario_order_blocks
from roster_promotion_contract import store, word
from test_original_ground_maneuver import seed_for_draw

BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = RESULT = None
GROUPS = 7


def safe_fixture(world, slot, callback, automatic, **changes):
    f = fixture(world, slot, callback, automatic, **changes)
    raw = bytearray(f['raw'])
    raw[0x43], raw[0x45] = 0, 0
    f['raw'] = bytes(raw)
    return f


class GroundPhaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='fist-ground-phase-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = pathlib.Path(cls.temp.name)
        cls.world, cls.prepared = synthetic_world()
        cls.scenario = cls.directory / 'synthetic.fsg'
        cls.scenario.write_bytes(cls.world.scenario)
        cls.request = cls.directory / 'request.bin'
        cls.prefix = 'prepared\n' + world_observation(
            cls.prepared, scenario_order_blocks(cls.world.scenario))
        cls.commands = {}
        if TARGET in ('native', 'all'):
            cls.commands['native'] = [str(NATIVE_PROBE or BUILD / 'native/fist_ground_phase_probe')]
        if TARGET in ('wasm', 'all'):
            cls.commands['wasm'] = ['node', str(BUILD / 'wasm/fist_ground_phase_probe.js')]
        cls.counts = collections.Counter()
        cls.digest = hashlib.sha256()

    def compare(self, cases, *, rejected=False, group, observations=None):
        wanted = self.prefix
        for index, f in enumerate(cases):
            wanted += (observations[index] if observations is not None else
                       f'case {index} -1\natomic 1\n' if rejected else
                       observation(self.world, f, index)[0])
        self.request.write_bytes(struct.pack('<2I', self.world.side, len(cases)) +
                                 self.world.pixels + b''.join(map(encode, cases)))
        for target, command in self.commands.items():
            result = subprocess.run([*command, str(self.scenario), str(self.request)],
                                    capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, (target, result.stderr))
            self.assertEqual(result.stderr, '', target)
            if result.stdout != wanted:
                actual, expected = result.stdout.splitlines(), wanted.splitlines()
                first = next((index for index, (a, b) in enumerate(zip(actual, expected))
                              if a != b), min(len(actual), len(expected)))
                self.fail(f'{target}/{group}: complete output differs at line {first}: '
                          f'{actual[first:first+2]} != {expected[first:first+2]}; '
                          f'{len(actual)} vs {len(expected)} lines')
        self.counts[group] += len(cases)
        self.digest.update(wanted.encode())

    def batches(self, cases, **options):
        pending = []
        for f in cases:
            pending.append(f)
            if len(pending) == 64:
                self.compare(pending, **options)
                pending = []
        if pending:
            self.compare(pending, **options)

    def test_complete_counter_domain_all_classes_and_banks(self):
        self.batches((safe_fixture(self.world, slot, (counter + 1) % 16, automatic,
                                   cursor=counter % 4, diagnostic=slot if counter & 1 else 65535)
                      | {'raw': self.counter_raw(slot, counter, automatic)}
                      for slot, automatic, counter in itertools.product(
                          self.world.ground_actors(), (0, 1), range(256))), group='counter')
        self.assertEqual(self.counts['counter'], 2048)

    def counter_raw(self, slot, counter, automatic):
        raw = bytearray(safe_fixture(self.world, slot, 0, automatic)['raw'])
        raw[0x42] = counter
        return bytes(raw)

    def test_heading_signed_edges_all_lanes_and_full_byte_wrap(self):
        cases = []
        for slot, automatic, offset, value, counter in itertools.product(
                self.world.ground_actors(), (0, 1), (0x26, 0x28, 0x2a, 0x2c),
                (0, 1, 32767, 32768, 65534, 65535), (15, 255)):
            f = safe_fixture(self.world, slot, 0, automatic)
            raw = bytearray(f['raw'])
            for lane, fixed in ((0x26, 32768), (0x28, 65535), (0x2a, 32767), (0x2c, 1)):
                store(raw, lane, fixed)
            store(raw, offset, value)
            store(raw, 0x2e, value ^ 0xa55a)
            raw[0x42] = counter
            f['raw'] = bytes(raw)
            cases.append(f)
        self.batches(cases, group='signed_heading_edges')
        self.assertEqual(self.counts['signed_heading_edges'], 384)

    def test_inhibition_draws_before_unused_invalid_platoon_and_height(self):
        def inputs():
            for slot, caption, cursor in itertools.product(
                    self.world.ground_actors(), range(1, 256), range(4)):
                f = safe_fixture(self.world, slot, caption % 16, cursor & 1,
                                 inhibition=caption, cursor=cursor, diagnostic=65535,
                                 orders_loaded=0, no_height=1)
                raw = bytearray(f['raw'])
                raw[27] = (caption + cursor) % 256
                f['raw'] = bytes(raw)
                yield f
        self.batches(inputs(), group='inhibited')
        self.assertEqual(self.counts['inhibited'], 4080)

    def test_outer_atomicity_and_used_caption_platoon_domains(self):
        cases = []
        for slot in self.world.ground_actors():
            cases.extend(safe_fixture(self.world, slot, 5, 0, invalid=invalid)
                         for invalid in (1, 2, 3, 4))
            cases.extend(safe_fixture(self.world, slot, 0, 0, inhibition=caption)
                         for caption in range(2, 256))
            cases.append(safe_fixture(self.world, slot, 0, 0, orders_loaded=0))
            for platoon in range(8, 256):
                f = safe_fixture(self.world, slot, 0, 0)
                raw = bytearray(f['raw'])
                raw[27] = platoon
                f['raw'] = bytes(raw)
                cases.append(f)
        self.batches(cases, rejected=True, group='atomic_rejections')
        self.assertEqual(self.counts['atomic_rejections'], 4 * (4 + 254 + 1 + 248))

    def test_conditional_idle_random_draws_are_retained(self):
        cases = []
        for slot, draw in itertools.product(self.world.ground_actors(),
                                           (0, 1, 63, 64, 1024, 16384, 65535)):
            f = safe_fixture(self.world, slot, 11, 1)
            f['seeds'] = (seed_for_draw(draw),) * 4
            raw = bytearray(f['raw'])
            store(raw, 0x40, 1)
            f['raw'] = bytes(raw)
            cases.append(f)
        self.compare(cases, group='conditional_rng')
        self.assertEqual(self.counts['conditional_rng'], 28)

    def test_retained_complete_banks_keep_heading_rng_and_child_history(self):
        for slot, automatic in itertools.product(self.world.ground_actors(), (0, 1)):
            initial = safe_fixture(self.world, slot, 0, automatic)
            before = original_input(self.world, initial)
            saved = {other: initial['raw'] if other == slot else
                     self.world.data[address:address + 251]
                     for other, address in self.world.ground_actors().items()}
            cases, observations = [], []
            for index in range(512):
                f = dict(initial, retain=int(index != 0), tick=index * 31,
                         inhibition=int(index % 37 == 0))
                before = bytearray(before)
                store(before, 0x452, f['tick'])
                before[0x978a] = f['inhibition']
                # The retained C episode keeps its admitted-voice history,
                # queues, RNG, orders and captured references. Feed the same
                # current producer values to independent event prediction.
                effective = copy.deepcopy(f)
                effective['voice_prior'] = word(before, 0x9fca)
                effective['notice_ticks'] = word(before, 0x969e)
                effective['message_ticks'] = word(before, 0x7a50)
                effective['advisory_until'] = word(before, 0x9fd7)
                effective['advisory_code'] = before[0x9fd6]
                output, before, _ = observation(self.world, effective, index,
                                                 before=bytes(before), saved=saved)
                cases.append(f)
                observations.append(output)
            self.compare(cases, group='retained_returns', observations=observations)
        self.assertEqual(self.counts['retained_returns'], 4096)

    def test_malformed_counts_and_extent_fail_before_preparation(self):
        cases = []
        for count in (0, 1, 2**31, 2**31 + 1, 2**32 - 1):
            for tail in (b'', bytes(CASE_BYTES)):
                if count == 1 and tail:
                    continue
                cases.append(struct.pack('<2I', 512, count) + bytes(512**2) + tail)
        for side in (0, 1, 511, 513, 4095, 4097, 65535, 2**31):
            cases.append(struct.pack('<2I', side, 1))
        f = safe_fixture(self.world, 150, 0, 0)
        valid = struct.pack('<2I', 512, 1) + self.world.pixels + encode(f)
        cases.extend((b'', valid[:7], valid[:-1], valid + b'\0', valid[:8]))
        for data in cases:
            self.request.write_bytes(data)
            for target, command in self.commands.items():
                r = subprocess.run([*command, str(self.scenario), str(self.request)],
                                   capture_output=True, timeout=30)
                self.assertEqual((r.returncode, r.stdout, r.stderr), (1, b'', b''), target)
        self.counts['malformed_frames'] += len(cases)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--result-json', type=pathlib.Path)
    args = parser.parse_args()
    TARGET, BUILD, NATIVE_PROBE, RESULT = args.target, args.build_root, args.native_probe, args.result_json
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped and program.result.testsRun == GROUPS
    receipt = {'success': bool(success), 'groups': program.result.testsRun,
               'skips': len(program.result.skipped), 'counts_per_target': dict(GroundPhaseTests.counts),
               'output_sha256': GroundPhaseTests.digest.hexdigest(),
               'scope': 'Default synthetic complete-parent contract; original/class/scene acceptance remains required'}
    print(json.dumps(receipt, sort_keys=True), flush=True)
    if RESULT:
        RESULT.write_text(json.dumps(receipt, indent=2) + '\n')
    raise SystemExit(0 if success else 1)
