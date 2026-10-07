#!/usr/bin/env python3
"""Required original discovery, lifetime and logical notification evidence.

No native/WASM implementation or PCM acceptance is claimed. Missing originals,
unfinished returns and any skipped group fail this research gate.
"""
import argparse
import hashlib
import json
import pathlib
import struct
import unittest

from original_object_pool_oracle import REGISTRY
from original_target_discovery_oracle import OriginalTargetDiscoveryOracle
from original_unit_oracle import DGROUP
from orders_contract import scenario_order_blocks
from target_discovery_contract import discovery
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[1]
REVIEW = None
SEEDS = (1, 2, 32768, 65535)


def record(kind, index, *, x=0, y=0, altitude=4096, flags=12, secondary=8,
           mode=0, automatic=0, old_count=0):
    raw = bytearray(251 if kind in (0, 1, 2, 3, 19) else 55)
    struct.pack_into('<H', raw, 0, kind)
    struct.pack_into('<3i', raw, 4, x, y, altitude)
    raw[22:24] = bytes((flags, secondary))
    raw[25] = mode
    if kind < 4:
        struct.pack_into('<H', raw, 0x40, automatic)
        struct.pack_into('<H', raw, 0x8e, 0x1234)
        raw[0x94] = old_count
    return index, 1, bytes(raw)


class TargetDiscoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalTargetDiscoveryOracle()
        cls.flat = bytes(512**2)
        cls.kernel = cls.owner.visibility.prepare(512, cls.flat)
        cls.scans = cls.transfers = cls.requests = 0
        cls.digest = hashlib.sha256()
        cls.evidence = {'scope': 'complete original discovery and logical requests; C and PCM remain open',
                        'corpus': {}, 'reaching': {}, 'complete_wasm_streak': 0}

    def prepare(self, records, link=0):
        machine, objects = self.owner.prepare_saved(records, SEEDS, 3, link,
                                                    (bytes(2144), bytes(176)))
        actor = objects[150][2]
        return machine, actor, objects

    def begin_corpus_mission(self, name, side):
        """Override to attach a consuming C loader to the same original boundary."""

    def check(self, machine, actor, *, kernel=None, side=512, pixels=None, **inputs):
        actor_raw = self.owner.raw(machine, actor)
        registry = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))[::2]
        pointers = set(registry) - {0}
        old = struct.unpack_from('<H', actor_raw, 0x97)[0]
        if old:
            pointers.add(old)
        raw_objects = {pointer: self.owner.raw(machine, pointer) for pointer in pointers}
        link = machine.mem_read(DGROUP + 0x6dae, 1)[0]
        expected = discovery(self.owner, actor, actor_raw, registry, raw_objects, link,
                             side, self.flat if pixels is None else pixels, **inputs)
        actual = self.owner.scan(machine, actor, kernel or self.kernel, **inputs)
        self.assertEqual(actual, expected)
        type(self).scans += 1
        type(self).transfers += len(actual['transfers'])
        type(self).requests += len(actual['requests'])
        compact = {key: value.hex() if isinstance(value, bytes) else value for key, value in actual.items()}
        self.digest.update(json.dumps(compact, sort_keys=True).encode())
        return actual

    def run_records(self, records, link=0, **inputs):
        machine, actor, objects = self.prepare(records, link)
        return self.check(machine, actor, **inputs), machine, objects

    def test_all_classes_and_complete_target_variant_byte(self):
        for kind in range(4):
            for candidate in range(28):
                result, _, _ = self.run_records([
                    record(kind, 0, flags=4), record(candidate, 1, x=-4096)])
                self.assertEqual((result['count'], len(result['transfers'])), (1, 1))
            for mode in range(256):
                result, _, _ = self.run_records([
                    record(kind, 0, flags=4), record(26, 1, x=-4096, mode=mode)])
                self.assertEqual(result['transfers'][0]['target'][2],
                                 4096 + self.owner.variant_heights[mode & 3])
        for flags in range(256):
            # Participation bit has a separate constructor boundary. Observe
            # every scan flag combination after actual allocation/loading.
            machine, actor, objects = self.prepare([record(0, 0, flags=4), record(0, 1)])
            machine.mem_write(DGROUP + objects[151][2] + 22, bytes([flags]))
            result = self.check(machine, actor)
            self.assertEqual(len(result['transfers']), int(bool(flags & 4 and flags & 8)))

    def test_link_operating_ranges_ties_and_directional_boundaries(self):
        for kind in range(4):
            for link in range(256):
                for operating in (0, 16):
                    bank = 0 if link < 2 else 2 if operating else 1
                    limit = self.owner.ranges[bank][kind]
                    machine, actor, _ = self.prepare([
                        record(kind, 0, flags=4),
                        record(0, 1, x=-(limit * 256 + 255)),
                        record(0, 2, x=-(limit * 256 + 256))], link)
                    machine.mem_write(DGROUP + actor + 26, bytes([operating]))
                    result = self.check(machine, actor)
                    self.assertEqual((result['limit'], result['primary_range'], result['count']),
                                     (limit, limit, 1))
            for operating in range(256):
                machine, actor, _ = self.prepare([
                    record(kind, 0, flags=4), record(0, 1, x=-50000)], 2)
                machine.mem_write(DGROUP + actor + 26, bytes([operating]))
                self.check(machine, actor)
        for x in (-262145, -262144, -262143, -1, 0, 1, 262143, 262144, 262145,
                  -2147483648, 2147483647):
            for y in (0, x):
                for coarse in (0, 1, 255):
                    self.run_records([record(0, 0, flags=4), record(0, 1, x=x, y=y)], coarse=coarse)
        for x in (-1, 0, 1, -50000, 50000):
            result, _, objects = self.run_records([
                record(0, 0, flags=4), record(0, 180, x=x), record(0, 1, x=x)])
            self.assertEqual(result['primary'], objects[152][2], 'Registry order breaks distance ties')

    def test_priority_reset_and_all_secondary_operand_branches(self):
        for kind in range(4):
            preferred, other = (0, 5) if kind & 1 == 0 else (5, 0)
            cases = (
                # Invisible contender wins secondary operand zero.
                [record(preferred, 1, x=-4096), record(other, 2, x=262144)],
                # Worse priority preserves AH=ff, producing ff01.
                [record(preferred, 1, x=-4096, secondary=0), record(other, 2, x=-256)],
                # Better but out-of-range priority resets distances, not pointers.
                [record(other, 1, x=-256), record(preferred, 2, x=-260000, secondary=0)],
                # A worse-priority pointer can remain after a better rank fails range.
                [record(other, 1, x=-256), record(preferred, 2, x=-260000)],
                # Range rejection still permits secondary selection.
                [record(preferred, 1, x=-260000)],
                # Equal invisible operands retain the earlier registry entry.
                [record(other, 1, x=262144), record(preferred, 2, x=-262145)],
                # A later preferred candidate resets secondary distance.
                [record(other, 1, x=-256), record(preferred, 2, x=-10000)],
                # No secondary leaves the existing secondary bearing word intact.
                [record(preferred, 1, x=-256, secondary=0)])
            observations = []
            for candidates in cases:
                result, _, _ = self.run_records([record(kind, 0, flags=4, mode=255), *candidates])
                observations.append({key: result[key] for key in (
                    'primary', 'secondary', 'primary_range', 'secondary_operand', 'priority', 'count')})
            self.assertEqual(observations[0]['secondary_operand'], 0)
            self.assertEqual(observations[1]['secondary_operand'], 0xff01)
            self.assertEqual((observations[2]['primary_range'], observations[2]['count']), (65535, 1))
            self.assertNotEqual(observations[2]['primary'], 0)
            self.assertEqual(observations[4]['count'], 0)
            self.evidence['reaching'][f'priority-class-{kind}'] = observations
        for altitude in (-2147483648, -65536, -1, 0, 65535, 65536, 2147483647):
            for candidate in (0, 1, 2, 3, 5, 6, 26, 27):
                self.run_records([record(0, 0, flags=4, altitude=altitude),
                                  record(candidate, 1, x=-4096, altitude=altitude)])

    def test_old_target_priorities_manual_automatic_and_orphans(self):
        for kind in range(4):
            for old_kind in range(28):
                for new_kind in (0, 5, 26, 16):
                    for automatic in (0, 1):
                        machine, actor, objects = self.prepare([
                            record(kind, 0, flags=4, automatic=automatic),
                            record(old_kind, 10, flags=0, secondary=0),
                            record(new_kind, 20, x=-1000)])
                        old = next(value[2] for value in objects.values() if value[0] == 10)
                        machine.mem_write(DGROUP + actor + 0x97, struct.pack('<H', old))
                        self.check(machine, actor)
        # The old target can be physical, live and orphaned by an overwritten binding.
        machine, actor, objects = self.prepare([
            record(0, 0, flags=4, automatic=1), record(5, 10, flags=0),
            record(0, 10, x=-1024)])
        orphan = objects[0][2]
        machine.mem_write(DGROUP + actor + 0x97, struct.pack('<H', orphan))
        result = self.check(machine, actor)
        self.assertEqual(struct.unpack_from('<H', result['actor'], 0x97)[0], 0)
        self.assertEqual([item['registry'] for item in result['transfers']], [10])

    def test_notification_admission_threshold_wrap_selection_side_and_old_count(self):
        for kind in range(4):
            for gate in (0, 1, 65535):
                for clock, last_voice in ((0, 0), (29, 0), (30, 0), (31, 0),
                                          (65535, 65506), (65535, 65505),
                                          (0, 65506), (0, 65507), (0, 1)):
                    for side_flag in (0, 8):
                        for selected in (None, 0):
                            for old_count in (0, 1):
                                result, _, _ = self.run_records([
                                    record(kind, 0, flags=4 | side_flag, old_count=old_count),
                                    record(0, 1, x=-1024, flags=12 ^ side_flag)],
                                    gate=gate, selected=selected, clock=clock, last_voice=last_voice)
                                self.assertEqual(result['count'], 1)
        self.evidence['reaching']['notification'] = {
            'operation': 0x64, 'ax': 0x028e, 'dx': 'clock high byte, DL=0', 'ecx': 0,
            'threshold': 30, 'clock_arithmetic': 'wrapped unsigned word',
            'scope': 'actual bf3c admission and device request, no mixer/PCM consumption'}

    def test_actual_release_reallocation_and_retained_physical_target(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        observations = []
        for replacement_kind in (0, 2):
            machine, actor, objects = self.prepare([
                record(0, 0, flags=4, automatic=1), record(0, 1, x=-4096)])
            old = objects[151][2]
            machine.mem_write(DGROUP + actor + 0x97, struct.pack('<H', old))
            first = self.check(machine, actor)
            before_binding = bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4))
            machine.reg_write(UC_X86_REG_AX, 1)
            self.owner.far_call(machine, 0x1b2ef)
            self.assertEqual(bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4)), bytes(4))
            released = self.check(machine, actor)
            self.assertEqual((released['primary'], released['count']), (0, 0))
            self.assertEqual(struct.unpack_from('<H', released['actor'], 0x97)[0], old)
            self.assertTrue(self.owner.raw(machine, old)[22] & 1)
            machine.reg_write(UC_X86_REG_AX, replacement_kind)
            self.owner.far_call(machine, 0x1b1df)
            self.assertEqual(machine.reg_read(UC_X86_REG_EFLAGS) & 1, 0)
            self.assertEqual((machine.reg_read(UC_X86_REG_DI), machine.reg_read(UC_X86_REG_AX)), (old, 1))
            self.assertEqual(bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4)), before_binding)
            self.assertEqual(int.from_bytes(self.owner.raw(machine, old)[:2], 'little'), replacement_kind)
            # Declare the successor's runtime pose/flags after its real zeroing constructor.
            machine.mem_write(DGROUP + old + 4, struct.pack('<3i', -8192, 0, 4096))
            machine.mem_write(DGROUP + old + 22, bytes((12, 8)))
            reused = self.check(machine, actor)
            self.assertEqual((reused['primary'], struct.unpack_from('<H', reused['actor'], 0x97)[0]),
                             (old, old))
            observations.append({
                'slot': self.owner.slot(old), 'registry': 1, 'value_before_and_after': 1,
                'type_before_after': [0, replacement_kind], 'released_old_target_survives': True,
                'reallocation_silently_retargets_old_pointer': True,
                'candidate_counts': [first['count'], released['count'], reused['count']]})
        self.evidence['reaching']['lifetime'] = {
            'observations': observations,
            'conclusion': 'even type/slot/registry/value together cannot distinguish allocation lifetimes'}

    def test_all_pinned_missions_after_actual_preparation_on_four_details(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
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
            offset, height_name = 0, None
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
                kernel = self.owner.visibility.prepare(side, pixels)
                for name, data in missions:
                    self.begin_corpus_mission(name, side)
                    records = records_from_scenario(data)
                    machine, objects = self.owner.prepare_saved(
                        records, SEEDS, 3, 0, scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    bindings = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))[::2]
                    actors = [(index, pointer) for index, pointer in enumerate(bindings)
                              if pointer and int.from_bytes(self.owner.raw(machine, pointer)[:2], 'little') < 4]
                    counts = {'actors': len(actors), 'transfers': 0, 'primary': 0, 'secondary': 0}
                    for index, pointer in actors:
                        result = self.check(machine, pointer, kernel=kernel, side=side, pixels=pixels)
                        counts['transfers'] += len(result['transfers'])
                        counts['primary'] += result['primary'] != 0
                        counts['secondary'] += result['secondary'] != 0
                    self.evidence['corpus'].setdefault(name, {})[str(side)] = counts
                    self.assertEqual(hashlib.sha256((directory / name).read_bytes()).hexdigest(),
                                     manifest[name]['sha256'])
            print(f'Complete original discovery: {height_name}, four installed details', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or
                   REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Temporary evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped and program.result.testsRun == 7
    evidence = TargetDiscoveryTests.evidence
    evidence['gate'] = {'success': success, 'groups': program.result.testsRun,
                        'skips': len(program.result.skipped), 'failures': len(program.result.failures),
                        'errors': len(program.result.errors), 'complete_scans': TargetDiscoveryTests.scans,
                        'actual_visibility_transfers': TargetDiscoveryTests.transfers,
                        'admitted_device_requests': TargetDiscoveryTests.requests,
                        'complete_output_sha256': TargetDiscoveryTests.digest.hexdigest()}
    print(json.dumps(evidence['gate'], sort_keys=True), flush=True)
    if REVIEW:
        REVIEW.mkdir(parents=True, exist_ok=True)
        (REVIEW / 'target-discovery.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
