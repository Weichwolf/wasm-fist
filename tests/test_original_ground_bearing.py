#!/usr/bin/env python3
"""Required original ab88 return/domain/caller and unsafe-target evidence.

Original-only research. Missing inputs, incomplete calls and skips fail; no C
callback, target-loss repair, parent bank or playable battle acceptance follows.
"""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from ground_bearing_contract import DISPATCH, RETREAT_DURATIONS, bearing
from original_ground_bearing_oracle import OriginalGroundBearingOracle
from original_unit_oracle import DGROUP
from test_vehicle_motion import start

ROOT = pathlib.Path(__file__).resolve().parents[1]
REVIEW = None


def actor(kind=0, *, flags=1, mode=0, counter=255, maneuver=255, heading=0x1234,
          source=(-100, 200), goal=(654321, -123456), target=0x7100, platoon=0):
    raw = bytearray(start(kind, x=source[0], y=source[1]))
    raw[0x1b], raw[0x43], raw[0x44], raw[0x45] = platoon, mode, counter, maneuver
    struct.pack_into('<H', raw, 0x40, flags)
    struct.pack_into('<H', raw, 0x47, heading)
    struct.pack_into('<ii', raw, 0x49, *goal)
    struct.pack_into('<H', raw, 0x97, target)
    return bytes(raw)


class BearingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalGroundBearingOracle()
        cls.machine = cls.owner.machine((1, 2, 32768, 65535), 3)
        cls.scans = cls.phases = 0
        cls.digest = hashlib.sha256()
        cls.evidence = {'scope': 'complete original bearing/caller evidence; no C acceptance',
                        'dispatch': list(DISPATCH), 'retreat_durations': list(RETREAT_DURATIONS),
                        'corpus': {}, 'reaching': {}, 'complete_wasm_streak': 0}

    def check(self, raw, behavior=0, target=(123456, -654321), coarse=0, *, machine=None,
              pointer=0x7000):
        machine = machine or self.machine
        platoon = raw[0x1b]
        descriptor = (behavior, 0, 0, 0, 65535, 32768, 17, 19, 23, 29, 31)
        machine.mem_write(DGROUP + pointer, raw)
        machine.mem_write(DGROUP + 0x85b6 + platoon * 22, struct.pack('<11H', *descriptor))
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        target_pointer, = struct.unpack_from('<H', raw, 0x97)
        if target is not None:
            machine.mem_write(DGROUP + target_pointer + 4, struct.pack('<ii', *target))
        original = bytes(machine.mem_read(DGROUP + pointer, 251))
        expected = bearing(original, descriptor, target, coarse)
        actual = self.owner.direction(machine, pointer, platoon)
        if actual != expected:
            offsets = [i for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
            raise AssertionError(f'Complete original bearing differs: mode={raw[0x43]} offsets={offsets}')
        type(self).scans += 1
        self.digest.update(actual)
        return actual

    def test_complete_bank_all_control_words_and_class_admission(self):
        for mode in range(0, 16, 2):
            for flags in range(65536):
                self.check(actor(flags % 4, flags=flags, mode=mode, counter=255,
                                 maneuver=4, platoon=flags % 8))
            print(f'Complete original bearing: mode {mode}, all control words', flush=True)
        for kind in range(4):
            for mode in range(0, 16, 2):
                for flags in (0, 1, 2, 3, 64, 65, 66, 67, 65535):
                    self.check(actor(kind, mode=mode, flags=flags))

    def test_every_packed_range_word_and_geometric_wrap_boundaries(self):
        for mode in (4, 6):
            for value in range(65536):
                raw = actor(value % 4, mode=mode, flags=1, source=(0, 0))
                actual = self.check(raw, target=(value * 256, 0))
                self.assertEqual(struct.unpack_from('<H', actual, 0x53)[0], max(0, value - 30))
            print(f'Complete original bearing: mode {mode}, all packed range words', flush=True)
        points = ((0, 0), (-1, 1), (65535, -65536), (16777215, 16777216),
                  (-2147483648, 2147483647), (2147483647, -2147483648))
        for kind in range(4):
            for mode in (0, 2, 4, 6):
                for source in points:
                    for target in points:
                        for coarse in (0, 1, 255):
                            self.check(actor(kind, mode=mode, flags=3, source=source, goal=target),
                                       target=target, coarse=coarse)

    def test_complete_counter_duration_maneuver_and_retained_heading_domains(self):
        for kind in range(4):
            for behavior in range(4):
                for counter in range(256):
                    for flags in (0, 1, 64, 65):
                        self.check(actor(kind, mode=4, flags=flags, counter=counter), behavior)
        for heading in range(65536):
            actual = self.check(actor(heading % 4, mode=8, maneuver=4, heading=heading),
                                behavior=65535, target=None)
            self.assertEqual(struct.unpack_from('<H', actual, 0x30)[0], heading)
        for kind in range(4):
            for maneuver in range(256):
                for flags in (0, 1, 65535):
                    self.check(actor(kind, mode=8, flags=flags, maneuver=maneuver),
                               behavior=65535, target=None)
        # The selector and target are genuinely unused outside an admitted
        # continuation/target branch. Do not invent blanket validation.
        for mode in range(0, 16, 2):
            for behavior in (4, 255, 256, 32768, 65535):
                self.check(actor(mode=mode, flags=0, target=0), behavior, target=None)
        self.evidence['reaching']['counter_wrap'] = {
            'byte_arithmetic': 'increment wraps before unsigned duration comparison',
            'zero_duration': 'initial call computes retreat; next admitted continuation clears bit 64',
            'unused_fields': 'manual branches and original returns ignore target/duration/maneuver heading'}

    def test_all_target_prefixes_self_orphan_and_actual_null_reuse_reads(self):
        from unicorn import UC_HOOK_MEM_READ
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI
        for kind in range(28):
            raw_target = struct.pack('<HHii', kind, 17, -7654321, 3456789)
            self.machine.mem_write(DGROUP + 0x7100, raw_target)
            for mode in (4, 6):
                self.check(actor(kind % 4, mode=mode), target=(-7654321, 3456789))
        self.check(actor(mode=6, target=0x7000), target=(-100, 200))
        null = []
        for mode in (4, 6):
            for position in ((4096, -8192), (-1234567, 7654321)):
                reads = []
                def observe(uc, access, address, size, value, context):
                    reads.append((address, size))
                hook = self.machine.hook_add(UC_HOOK_MEM_READ, observe,
                                             begin=DGROUP + 4, end=DGROUP + 11)
                actual = self.check(actor(mode=mode, target=0), target=position)
                self.machine.hook_del(hook)
                # Repeat the identical input without observation; no hook
                # replaces a read, instruction or callback return.
                self.assertEqual(self.check(actor(mode=mode, target=0), target=position), actual)
                self.assertTrue(reads)
                # Actual 0731/b71 load low/high WORDs separately, with
                # SUB/SBB carrying across the complete 32-bit coordinate.
                self.assertEqual(set(reads), {(DGROUP + offset, 2) for offset in (4, 6, 8, 10)})
                null.append({'mode': mode, 'unrelated_DS_position': position, 'reads': reads,
                             'heading': struct.unpack_from('<H', actual, 0x30)[0],
                             'range': struct.unpack_from('<H', actual, 0x53)[0]})
        self.assertNotEqual(null[0]['heading'], null[1]['heading'])
        self.evidence['reaching']['null_target'] = null
        machine = self.owner.machine()
        self.owner.far_call(machine, 0x1b176)
        pointers = []
        for index in range(2):
            machine.reg_write(UC_X86_REG_AX, 0)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, 1)
            self.owner.far_call(machine, 0x1b1a2)
            pointers.append(machine.reg_read(UC_X86_REG_DI))
        source, target_pointer = pointers
        raw = actor(mode=6, source=(0, 0), target=target_pointer)
        first = self.check(raw, target=(32768, 0), machine=machine, pointer=source)
        machine.reg_write(UC_X86_REG_AX, 0)
        machine.reg_write(UC_X86_REG_BX, 1)
        machine.reg_write(UC_X86_REG_CX, 1)
        self.owner.far_call(machine, 0x1b1a2)
        successor = machine.reg_read(UC_X86_REG_DI)
        self.assertNotEqual(successor, target_pointer)
        machine.mem_write(DGROUP + successor + 4, struct.pack('<ii', 0, -65536))
        orphan = self.owner.direction(machine, source, 0)
        self.assertEqual(orphan, first)
        type(self).scans += 1
        self.digest.update(orphan)
        self.evidence['reaching']['orphan'] = {
            'target': target_pointer, 'current_registry_successor': successor,
            'retained_complete_target_result': True}
        reuse = []
        for replacement in (0, 2):
            machine = self.owner.machine()
            self.owner.far_call(machine, 0x1b176)
            pointers = []
            for index in range(2):
                machine.reg_write(UC_X86_REG_AX, 0)
                machine.reg_write(UC_X86_REG_BX, index)
                machine.reg_write(UC_X86_REG_CX, 1)
                self.owner.far_call(machine, 0x1b1a2)
                pointers.append(machine.reg_read(UC_X86_REG_DI))
            source, target_pointer = pointers
            raw = actor(mode=6, source=(0, 0), target=target_pointer)
            first = self.check(raw, target=(32768, 0), machine=machine, pointer=source)
            machine.reg_write(UC_X86_REG_AX, 1)
            self.owner.far_call(machine, 0x1b2ef)
            # The actor's saved target survives actual release; the deleted
            # payload still supplies position without any liveness check.
            released = self.owner.direction(machine, source, 0)
            self.assertEqual(released, first)
            type(self).scans += 1
            self.digest.update(released)
            machine.reg_write(UC_X86_REG_AX, replacement)
            self.owner.far_call(machine, 0x1b1df)
            self.assertEqual(machine.reg_read(UC_X86_REG_DI), target_pointer)
            following = self.check(first, target=(0, -65536), machine=machine, pointer=source)
            self.assertNotEqual(following[0x30:0x32], first[0x30:0x32])
            reuse.append({'replacement': replacement, 'pointer': target_pointer,
                          'before': first[0x30:0x32].hex() + first[0x53:0x55].hex(),
                          'released': released[0x30:0x32].hex() + released[0x53:0x55].hex(),
                          'reused': following[0x30:0x32].hex() + following[0x53:0x55].hex()})
        self.evidence['reaching']['release_reuse'] = reuse

    def test_saved_counter_heading_initialization_and_complete_behavior_ui(self):
        from original_ground_command_oracle import OriginalGroundCommandOracle
        for kind in range(4):
            for counter in (0, 1, 127, 128, 254, 255):
                for heading in (0, 1, 32767, 32768, 65535):
                    raw = actor(kind, counter=counter, heading=heading)
                    restored, _ = self.owner.initialize([(0, 1, raw)], (1, 2, 32768, 65535), 3, 0)
                    self.assertEqual((restored[0][2][0x44], restored[0][2][0x47:0x49]),
                                     (counter, raw[0x47:0x49]))
        cycles = OriginalGroundCommandOracle().choice_cycles()
        self.assertEqual(len(cycles), 128)
        self.evidence['reaching']['restoration'] = {
            'complete_class_starts': 120, 'retained': ['byte +44', 'word +47'],
            'actual_UI_choice_cycles': len(cycles), 'behavior_domain': [0, 1, 2, 3]}

    def test_all_47_actual_preparation_and_complete_parent_callbacks_on_four_details(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        from original_object_pool_oracle import REGISTRY
        from orders_contract import scenario_order_blocks
        from test_ground_command import select
        from test_ground_goal import assign
        from test_units import records_from_scenario
        from test_vehicle_start import step
        from unicorn.x86_const import UC_X86_REG_DI
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrain = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(path.name for path in directory.glob('*.FSG')), sorted(manifest))
        self.assertEqual(len(manifest), 47)
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            height_name = None
            offset = 0
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size + 6
            self.assertIsNotNone(height_name)
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual(len(grouped), 8)
        for height_name, missions in grouped.items():
            data = (directory / height_name).read_bytes()
            info = terrain[height_name]
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            decoded = decoder.klc(data)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed, side)
                for name, data in missions:
                    machine, _ = self.owner.prepare_saved(records_from_scenario(data),
                        (1, 2, 32768, 65535), 3, 0, scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    # Explicit constructed counter entry into genuine bank
                    # slots 0 -> 1 -> 2. Actual ab03 executes every instruction;
                    # selected diagnostic presentation is outside this boundary.
                    machine.mem_write(DGROUP + 0x978a, b'\0')
                    machine.mem_write(DGROUP + 0x7ae0, bytes(2))
                    pointers = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))[::2]
                    actors = [pointer for pointer in pointers if pointer and int.from_bytes(
                              machine.mem_read(DGROUP + pointer, 2), 'little') < 4]
                    counts = {'actors': len(actors), 'parent_returns': 0, 'modes': {},
                              'targetless_numeric_branches': 0}
                    for pointer in actors:
                        machine.mem_write(DGROUP + pointer + 0x42, bytes([15]))
                        for phase in range(3):
                            before = bytes(machine.mem_read(DGROUP, 65536))
                            raw = bytearray(before[pointer:pointer + 251])
                            words, cursor = self.owner.random_state(machine)
                            phase_random, cursor = step(words, cursor)
                            raw[0x42] = (raw[0x42] + 1) % 256
                            if raw[0x42] & 15 == 0:
                                old = struct.unpack_from('<3H', raw, 0x28)
                                hull, = struct.unpack_from('<H', raw, 0x26)
                                signed = lambda value: value if value < 32768 else value - 65536
                                average = ((sum(map(signed, (*old, hull))) & 4294967295) >> 2) & 65535
                                struct.pack_into('<4H', raw, 0x28, hull, old[0], old[1], average)
                            platoon = raw[0x1b]
                            descriptor = struct.unpack_from('<11H', before, 0x85b6 + platoon * 22)
                            route = before[0x7d40 + platoon * 268:0x7d40 + (platoon + 1) * 268]
                            if phase == 0:
                                status, expected, (words, cursor) = select((bytes(raw), descriptor,
                                                                          phase_random, words, cursor))
                                self.assertEqual(status, 0)
                            elif phase == 1:
                                leader, = struct.unpack_from('<H', before, 0x6d3c + platoon * 8)
                                leader_raw = before[leader:leader + 251] if leader else bytes(251)
                                presence = (0 if not leader else 2 if int.from_bytes(leader_raw[:2], 'little') == 23
                                            else 3 if leader == pointer else 1)
                                # The newly incremented actor is the current
                                # leader input when formation targets itself.
                                status, expected = assign((bytes(raw), leader_raw, presence,
                                                            descriptor, route, words, cursor))
                                self.assertEqual(status, 0)
                            else:
                                target_pointer, = struct.unpack_from('<H', raw, 0x97)
                                target = struct.unpack_from('<ii', before, target_pointer + 4)
                                expected = bearing(bytes(raw), descriptor, target, before[0x2040])
                                counts['modes'][str(raw[0x43])] = counts['modes'].get(str(raw[0x43]), 0) + 1
                                if raw[0x43] in (4, 6) and raw[0x40] & 1 and not target_pointer and not (
                                        raw[0x43] == 4 and raw[0x40] & 64):
                                    counts['targetless_numeric_branches'] += 1
                            machine.reg_write(UC_X86_REG_DI, pointer)
                            self.owner.call(machine, 0xab03)
                            self.assertEqual(machine.reg_read(UC_X86_REG_DI), pointer)
                            after = bytes(machine.mem_read(DGROUP, 65536))
                            self.assertEqual(after[pointer:pointer + 251], expected)
                            self.assertEqual(self.owner.random_state(machine), (words, cursor))
                            # Complete parent permitted destinations, including
                            # real RNG/caller globals and existing local scratch.
                            ranges = sorted(((pointer, pointer + 251), (0x0342, 0x0344),
                                (0x1f82, 0x1f8c), (0x2034, 0x203e), (0x8fc0, 0x9002),
                                (0x978c, 0x978e), (0x9796, 0x979a), (0x9a08, 0x9a0a)))
                            offset = 0
                            for begin, end in (*ranges, (65536, 65536)):
                                self.assertEqual(before[offset:begin], after[offset:begin])
                                offset = end
                            type(self).phases += 1
                            counts['parent_returns'] += 1
                            self.digest.update(expected)
                    self.evidence['corpus'].setdefault(name, {})[str(side)] = counts
                    self.assertEqual(bytes(machine.mem_read(0, DGROUP)), self.owner.image[:DGROUP])
            print(f'Complete original parent bearing: {height_name}, four installed details', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or
                   REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Review evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped and program.result.testsRun == 6
    evidence = BearingTests.evidence
    evidence['gate'] = {'success': success, 'groups': program.result.testsRun,
                        'skips': len(program.result.skipped), 'complete_returns': BearingTests.scans,
                        'parent_returns': BearingTests.phases,
                        'output_sha256': BearingTests.digest.hexdigest()}
    print(json.dumps(evidence['gate'], sort_keys=True), flush=True)
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'ground-bearing.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
