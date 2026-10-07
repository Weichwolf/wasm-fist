#!/usr/bin/env python3
"""Required full original mission-ready evidence, separate from C acceptance."""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from original_mission_ready_oracle import RESET_BANK, OriginalMissionReadyOracle
from original_object_pool_oracle import REGISTRY
from original_unit_oracle import DGROUP
from test_original_command_boundary import chunks
from test_units import records_from_scenario, snapshot
from test_vehicle_start import (CAMERA_HEIGHT, COMPONENT_OFFSET, COMPONENTS,
                                initialized, step)

ROOT = pathlib.Path(__file__).resolve().parents[2]
REVIEW = None


def word(raw, offset):
    return struct.unpack_from('<H', raw, offset)[0]


def store(raw, offset, value):
    struct.pack_into('<H', raw, offset, value % 65536)


def ground_reset(raw, link):
    result = bytearray(raw)
    kind = word(raw, 0)
    result[0x1a] = (raw[0x1a] & 0xef) | 1 | (16 if link == 2 else 0)
    store(result, 0x87, CAMERA_HEIGHT[kind])
    store(result, 0x97, 0)
    store(result, 0x9d, 0)
    store(result, 0x40, word(raw, 0x40) | 1)
    result[0x36] = 0
    offset = COMPONENT_OFFSET[kind]
    result[offset:offset + len(COMPONENTS[kind])] = COMPONENTS[kind]
    return bytes(result)


def complete_expected(before, transfers):
    """Independent complete reset writes, including releases and shared RNG.

    Retain every byte not written by an actual recovered method. The two
    artillery arrays are four words each; fixtures exceeding that contract fail
    rather than accepting a neighboring-array overwrite as valid preparation.
    """
    result = bytearray(before)
    heights = {index: (pointer, height) for index, pointer, height in transfers}
    seeds = list(struct.unpack_from('<4H', before, 0x1f84))
    cursor = ((word(before, 0x1f82) - 0x1f84) // 2 + 1) % 4
    consumed = 0
    releases = []
    artillery = [[], []]
    for index in range(182):
        pointer = word(before, REGISTRY + index * 4)
        if not pointer:
            continue
        kind = word(before, pointer)
        if kind < 4:
            result[pointer:pointer + 251] = ground_reset(before[pointer:pointer + 251], before[0x6dae])
        elif kind == 16:
            store(result, 0x9c89, word(result, 0x9c89) + 1)
        elif RESET_BANK[kind] == 0xc30f:
            result[pointer + 22] |= 1
            store(result, REGISTRY + index * 4, 0)
            store(result, REGISTRY + index * 4 + 2, word(before, REGISTRY + index * 4 + 2) - 1)
            extended = bool(before[0xe614 + kind] & 1)
            result[(0xe38d if extended else 0xe2f7) + word(before, pointer + 2)] = 0
            count = 0xe296 if extended else 0xe294
            store(result, count, word(result, count) - 1)
            releases.append(index)
        else:
            actual_pointer, height = heights.pop(index)
            if actual_pointer != pointer:
                raise AssertionError('Height result belongs to another registry object')
            result[pointer + 13] = height
            if kind == 21:
                variant = before[pointer + 25]
                if variant >= 4:
                    raise ValueError('Tree variant escapes its original authored tables')
                store(result, 0x930a, word(result, 0x930a) + 1)
                heading, cursor = step(seeds, cursor)
                store(result, pointer + 16, heading)
                store(result, pointer + 20, 512)
                result[pointer + 23] |= 4
                result[pointer + 22] |= 64
                extent, cursor = step(seeds, cursor)
                store(result, pointer + 18,
                      (extent & word(before, 0x9322 + variant * 2)) + word(before, 0x932a + variant * 2))
                store(result, 0x0342, extent)
                store(result, 0x1f82, 0x1f84 + ((cursor + 3) % 4) * 2)
                struct.pack_into('<4H', result, 0x1f84, *seeds)
                consumed += 2
            elif kind == 26:
                variant = before[pointer + 25]
                masked = variant % 16
                store(result, pointer + 18, word(before, 0x9ecf + masked * 2))
                store(result, pointer + 20, word(before, 0x9ef7 + masked * 2))
                result[pointer + 27] = before[0x9edf + variant]
                result[pointer + 23] |= 20
                result[pointer + 22] |= 78
                if variant & 4:
                    result[pointer + 22] = (result[pointer + 22] | 1) & 0xf9
                    result[pointer + 23] &= 0xf7
            elif kind == 27:
                variant = before[pointer + 25]
                store(result, pointer + 18, word(before, 0x9cdf + variant * 2))
                store(result, pointer + 31, 5)
                side = int(bool(before[pointer + 22] & 8))
                artillery[side].append(pointer)
                if len(artillery[side]) > 4:
                    raise ValueError('Artillery census exceeds its original four-word side array')
                count, base = ((0x9ccb, 0x9ccf), (0x9ccd, 0x9cd7))[side]
                store(result, base + (len(artillery[side]) - 1) * 2, pointer)
                store(result, count, len(artillery[side]))
                if side:
                    if variant != 1:
                        result[pointer + 22] |= 70
                    else:
                        result[pointer + 22] &= 0xf9
                        result[pointer + 23] &= 0xe7
            elif kind not in (23, 25):
                raise AssertionError('Unhandled non-ground reset is not an empty return')
    if heights:
        raise AssertionError('Unconsumed height result')
    store(result, 0xe29e, REGISTRY + 182 * 4)
    store(result, 0xe2a0, 0)
    if transfers:
        store(result, 0xea10, 0x54)
    return bytes(result), consumed, releases, artillery


class MissionReadyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalMissionReadyOracle()
        cls.manifest = json.loads((ROOT / 'tools/rewrite/scenario_originals.json').read_text())
        cls.evidence = {'scope': 'original complete d755 with declared op54 transfer; no C readiness acceptance',
                        'corpus': {}, 'ground_methods': 0, 'registry_passes': 0,
                        'physical_payloads': 0, 'height_transfers': 0, 'tree_random_calls': 0,
                        'released_bindings': 0, 'complete_wasm_streak': 0}

    @classmethod
    def tearDownClass(cls):
        print('Original mission-ready: ' + ', '.join(f'{name}={cls.evidence[name]}' for name in (
            'ground_methods', 'registry_passes', 'physical_payloads', 'height_transfers',
            'tree_random_calls', 'released_bindings')), flush=True)
        if REVIEW:
            REVIEW.mkdir(parents=True, exist_ok=True)
            (REVIEW / 'mission-ready.json').write_text(json.dumps(cls.evidence, indent=2) + '\n')

    def compare_dgroup(self, actual, expected, stack=(0x8ff0, 0x9002)):
        self.assertEqual((len(actual), len(expected)), (65536, 65536))
        differences = [(hex(offset), first, second) for offset, (first, second)
                       in enumerate(zip(actual, expected))
                       if first != second and not stack[0] <= offset < stack[1]]
        self.assertEqual(differences, [], 'Complete DGROUP differs outside actual call-stack writes')

    def test_actual_complete_banks_methods_and_caller_instructions(self):
        image = self.owner.image
        self.assertEqual(image[0xe80a:0xe813], bytes.fromhex('e8f9f7e845efe813b5'))
        self.assertEqual(image[0xe8d6:0xe8df], bytes.fromhex('e82df7e879eee847b4'))
        self.assertEqual(image[0xd73c:0xd740], bytes.fromhex('e81600c3'))
        self.assertEqual(image[0xd755:0xd76a], bytes.fromhex(
            'e8ffe9e809ea720c8bfe8b35d1e6ff9468e6ebefc3'))
        self.assertEqual(image[0xc30f:0xc31a], bytes.fromhex('9ac4bc690f9a5fbc690fc3'))
        # Deletion and class initialization are different actual banks.
        init = struct.unpack_from('<4H', image, DGROUP + 0xe4e0)
        delete = struct.unpack_from('<4H', image, DGROUP + 0xe6a0)
        self.assertEqual(init, (0x7b91, 0x8744, 0x8f9f, 0x973a))
        self.assertEqual(delete, (0xc30e,) * 4)
        self.assertEqual(image[0xc30e], 0xc3)
        self.assertNotEqual(image[0xc30f], 0xc3)
        self.evidence['reset_bank'] = list(RESET_BANK)
        self.evidence['callers'] = [0xe80d, 0xe8d9, 0xd73c]

    def test_complete_ground_methods_preserve_ammunition_motion_rng_and_other_bytes(self):
        from unicorn.x86_const import UC_X86_REG_DI
        for kind in range(4):
            for link in (0, 2, 255):
                machine = self.owner.machine((1, 2, 32768, 65535), kind, link)
                for flags in range(256):
                    raw = bytearray(snapshot(kind, flags=flags))
                    raw[0x1a] = flags
                    raw[0x36] = 255 - flags
                    store(raw, 0x97, flags * 257)
                    store(raw, 0x9d, 65535 - flags * 257)
                    store(raw, 0x40, flags * 257)
                    machine.mem_write(DGROUP + 0x7000, bytes(raw))
                    machine.reg_write(UC_X86_REG_DI, 0x7000)
                    before = bytes(machine.mem_read(DGROUP, 65536))
                    self.owner.call(machine, RESET_BANK[kind])
                    expected = bytearray(before)
                    expected[0x7000:0x70fb] = ground_reset(raw, link)
                    self.compare_dgroup(bytes(machine.mem_read(DGROUP, 65536)), bytes(expected))
                    self.assertEqual(machine.reg_read(UC_X86_REG_DI), 0x7000)
                    self.evidence['ground_methods'] += 1

    def test_all_47_complete_saved_worlds_orders_releases_and_target_reset(self):
        self.assertEqual(sorted(path.name for path in (ROOT / 'armoredfist/FISTDATA').glob('*.FSG')),
                         sorted(self.manifest))
        targets = []
        total_ground = 0
        for ordinal, name in enumerate(sorted(self.manifest)):
            data = (ROOT / 'armoredfist/FISTDATA' / name).read_bytes()
            pin = self.manifest[name]
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (pin['size'], pin['sha256']))
            records = records_from_scenario(data)
            blocks = chunks(data)
            restored = []
            counts = [0, 0]
            for registry, generation, raw in records:
                extended = int(bool(self.owner.type_flags[word(raw, 0)] & 1))
                saved = bytearray(raw)
                store(saved, 2, counts[extended])
                counts[extended] += 1
                restored.append((registry, generation, bytes(saved)))
                if word(raw, 0) < 4:
                    total_ground += 1
                    if word(raw, 0x97):
                        targets.append((name, registry, word(raw, 0), word(raw, 0x97)))
            cases = []
            for link, pixels in ((0, bytes((31, 67, 123, 241))), (2, bytes((241, 123, 67, 31)))):
                with self.subTest(mission=name, link=link):
                    machine, objects = self.owner.prepare_saved(records, (1, 2, 32768, 65535),
                                                               ordinal % 4, link,
                                                               (blocks[b'PATH'], blocks[b'PINF']))
                    ground = [(reg, generation, bytes(machine.mem_read(DGROUP + ptr, size)))
                              for reg, generation, ptr, size in objects.values()
                              if word(machine.mem_read(DGROUP + ptr, 2), 0) < 4]
                    expected_ground, expected_random = initialized(restored, (1, 2, 32768, 65535), ordinal % 4, link)
                    self.assertEqual(ground, expected_ground)
                    self.assertEqual(self.owner.random_state(machine), expected_random)
                    before, actual, transfers = self.owner.reset(machine, pixels)
                    expected, consumed, released, artillery = complete_expected(before, transfers)
                    self.compare_dgroup(actual, expected)
                    self.assertEqual(self.owner.orders.blocks(machine), (blocks[b'PATH'], blocks[b'PINF']))
                    for _, _, ptr, size in objects.values():
                        self.assertEqual(actual[ptr:ptr + size], expected[ptr:ptr + size])
                        if word(before, ptr) < 4:
                            self.assertEqual((word(before, ptr + 0x97), word(actual, ptr + 0x97)),
                                             (word(before, ptr + 0x97), 0))
                    cases.append({'link': link, 'height_plane': pixels.hex(), 'objects': len(objects),
                                  'height_calls': len(transfers), 'random_calls': consumed,
                                  'releases': released, 'artillery_by_side': list(map(len, artillery)),
                                  'before_dgroup_sha256': hashlib.sha256(before).hexdigest(),
                                  'after_dgroup_sha256': hashlib.sha256(actual).hexdigest()})
                    self.evidence['registry_passes'] += 1
                    self.evidence['physical_payloads'] += len(objects)
                    self.evidence['height_transfers'] += len(transfers)
                    self.evidence['tree_random_calls'] += consumed
                    self.evidence['released_bindings'] += len(released)
            self.evidence['corpus'][name] = {'sha256': pin['sha256'], 'cases': cases}
            self.assertEqual((ROOT / 'armoredfist/FISTDATA' / name).read_bytes(), data)
        self.assertEqual((len(self.manifest), total_ground, len(targets)), (47, 960, 14))
        self.evidence['saved_nonzero_targets'] = targets
        self.evidence['saved_ground_records'] = total_ground

    def test_empty_and_all_28_class_entries_with_both_release_arena_widths(self):
        for records in ([], [(27 - kind, (kind * 257) % 65536, self.safe_snapshot(kind))
                             for kind in range(28)]):
            machine, objects = self.owner.prepare_saved(records, (1, 2, 32768, 65535), 0, 0,
                                                       (bytes(2144), bytes(176)))
            before, actual, transfers = self.owner.reset(machine, bytes((1, 2, 3, 4)))
            expected, consumed, releases, artillery = complete_expected(before, transfers)
            self.compare_dgroup(actual, expected)
            self.assertEqual(len(transfers), 5 if records else 0)
            self.assertEqual(consumed, 2 if records else 0)
            self.assertEqual(len(releases), 18 if records else 0)
            self.assertEqual(list(map(len, artillery)), [1 if records else 0, 0])
            self.evidence['registry_passes'] += 1
            self.evidence['physical_payloads'] += len(objects)
            self.evidence['height_transfers'] += len(transfers)
            self.evidence['tree_random_calls'] += consumed
            self.evidence['released_bindings'] += len(releases)

    @staticmethod
    def safe_snapshot(kind):
        raw = bytearray(snapshot(kind, flags=0))
        raw[25] = 0  # Authored variant table domain for tree/static/artillery.
        if kind == 23:
            raw[35:37] = bytes(2)  # The original excludes the slot-zero wreck.
        return bytes(raw)

    def test_complete_registry_pass_visits_deleted_nonparticipants_and_skips_orphans(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI
        machine = self.owner.machine((1, 2, 32768, 65535), 3, 255)
        self.owner.far_call(machine, 0x1b176)
        pointers = []
        # Physical input order differs from ordered registry traversal. The
        # duplicate binding leaves an allocated orphan, which must not reset.
        for kind, index, value in ((0, 181, 0), (1, 0, 65535), (2, 181, 1), (3, 91, 2)):
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, value)
            self.owner.far_call(machine, 0x1b1a2)
            pointer = machine.reg_read(UC_X86_REG_DI)
            raw = bytearray(snapshot(kind, flags=1))  # Deleted and not participating.
            raw[2:4] = machine.mem_read(DGROUP + pointer + 2, 2)
            store(raw, 0x97, 65535)
            machine.mem_write(DGROUP + pointer, bytes(raw))
            pointers.append(pointer)
        before, actual, transfers = self.owner.reset(machine, bytes((1, 2, 3, 4)))
        expected, consumed, releases, _ = complete_expected(before, transfers)
        self.compare_dgroup(actual, expected)
        self.assertEqual((transfers, consumed, releases), ([], 0, []))
        self.assertEqual(actual[pointers[0]:pointers[0] + 251], before[pointers[0]:pointers[0] + 251])
        self.assertEqual(word(actual, pointers[0] + 0x97), 65535)
        self.assertEqual([word(actual, ptr + 0x97) for ptr in pointers[1:]], [0, 0, 0])
        self.evidence['registry_passes'] += 1
        self.evidence['physical_payloads'] += 4

    def test_real_train1_reset_rng_reaches_complete_first_controlled_command(self):
        from original_driver_oracle import OriginalDriverOracle
        from unicorn.x86_const import UC_X86_REG_DI
        data = (ROOT / 'armoredfist/FISTDATA/TRAIN1.FSG').read_bytes()
        pin = self.manifest['TRAIN1.FSG']
        self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (pin['size'], pin['sha256']))
        blocks = chunks(data)
        pixels = bytes((31, 67, 123, 241))
        driver = OriginalDriverOracle(2, pixels)
        observations = []
        for ready in (False, True):
            machine, objects = self.owner.prepare_saved(records_from_scenario(data),
                                                       (1, 2, 32768, 65535), 0, 0,
                                                       (blocks[b'PATH'], blocks[b'PINF']))
            if ready:
                prior, reset, transfers = self.owner.reset(machine, pixels)
                expected, calls, _, _ = complete_expected(prior, transfers)
                self.compare_dgroup(reset, expected)
                self.assertEqual((len(transfers), calls), (82, 98))
                self.evidence['registry_passes'] += 1
                self.evidence['physical_payloads'] += len(objects)
                self.evidence['height_transfers'] += len(transfers)
                self.evidence['tree_random_calls'] += calls
            pointer = word(machine.mem_read(DGROUP + 0x6d3c, 2), 0)
            self.assertEqual(self.owner.slot(pointer), 151)
            machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', pointer))
            machine.reg_write(UC_X86_REG_DI, pointer)
            driver.take_control(machine)
            before = bytes(machine.mem_read(DGROUP, 65536))
            self.assertEqual((before[0x978a], word(before, 0x7ae0), before[pointer + 0x42]),
                             (0, 0, 254))
            self.assertEqual(word(before, pointer + 0x40) & 1, 0)
            self.assertEqual(word(before, 0x98fc + 30), 0xb111)
            seeds, cursor = self.owner.random_state(machine)
            words = list(seeds)
            value, next_cursor = step(words, cursor)
            self.owner.call(machine, 0xab03)
            actual = bytes(machine.mem_read(DGROUP, 65536))
            expected = bytearray(before)
            store(expected, 0x9a08, pointer)
            store(expected, 0x0342, value)
            store(expected, 0x978c, value)
            store(expected, 0x1f82, 0x1f84 + ((next_cursor + 3) % 4) * 2)
            struct.pack_into('<4H', expected, 0x1f84, *words)
            expected[pointer + 0x42] = 255
            platoon = before[pointer + 27]
            store(expected, 0x9796, word(before, 0x85a0 + platoon * 2))
            store(expected, 0x9798, word(before, 0x7d2a + platoon * 2))
            self.compare_dgroup(actual, bytes(expected))
            self.assertEqual(self.owner.random_state(machine), (words, next_cursor))
            observations.append({'ready_reset': ready, 'random_before': [seeds, cursor],
                                 'random_after': [words, next_cursor], 'phase_value': value,
                                 'counter_before_after': [254, 255],
                                 'complete_callback': 'ab03 -> controlled bank entry 15 -> genuine b111 RET'})
        self.assertNotEqual(observations[0]['phase_value'], observations[1]['phase_value'])
        self.evidence['train1_first_controlled_command'] = observations


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Temporary evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped and program.result.testsRun == 6
    MissionReadyTests.evidence['gate'] = {'success': success, 'groups': program.result.testsRun,
                                        'skips': len(program.result.skipped),
                                        'failures': len(program.result.failures),
                                        'errors': len(program.result.errors)}
    if REVIEW:
        (REVIEW / 'mission-ready.json').write_text(json.dumps(MissionReadyTests.evidence, indent=2) + '\n')
    raise SystemExit(not success)
