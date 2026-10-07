#!/usr/bin/env python3
"""Required complete original target acquisition, aim and throttle consumer evidence."""
import argparse
import hashlib
import itertools
import json
import pathlib
import random
import struct
import unittest

from ground_throttle_contract import throttle as throttle_model
from original_ground_throttle_oracle import OriginalGroundThrottleOracle
from original_object_pool_oracle import REGISTRY
from original_target_acquisition_oracle import OriginalTargetAcquisitionOracle
from original_unit_oracle import DGROUP
from orders_contract import scenario_order_blocks
from target_acquisition_contract import ACQUISITION_THRESHOLDS, target_geometry
from target_discovery_contract import discovery as discovery_model
from test_original_target_discovery import record
from test_units import records_from_scenario

ROOT = pathlib.Path(__file__).resolve().parents[1]
REVIEW = None
SEEDS = (1, 2, 32768, 2)


class TargetAcquisitionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalTargetAcquisitionOracle()
        cls.throttle = OriginalGroundThrottleOracle()
        cls.flat = bytes(512**2)
        cls.kernel = cls.owner.visibility.prepare(512, cls.flat)
        cls.acquisitions = cls.aims = cls.transfers = cls.requests = cls.throttles = 0
        cls.digest = hashlib.sha256()
        cls.evidence = {'scope': 'original acquisition/aim/consuming throttle; shared C and PCM remain open',
                        'reaching': {}, 'corpus': {}, 'complete_wasm_streak': 0}

    def prepare(self, kind=0, target_kind=0, **target_values):
        machine, objects = self.owner.prepare_saved(
            [record(kind, 0, flags=4), record(target_kind, 1, x=-4096, **target_values)],
            SEEDS, 3, 0, (bytes(2144), bytes(176)))
        actor = objects[150][2]
        target = next(value[2] for value in objects.values() if value[0] == 1)
        machine.mem_write(DGROUP + actor + 0x94, b'\1')
        machine.mem_write(DGROUP + actor + 0x9d, struct.pack('<H', target))
        machine.mem_write(DGROUP + 0x969e, struct.pack('<HH', 77, 0x1234))
        return machine, actor, target

    def check(self, machine, actor, *, kernel=None, pixels=None, side=512, **inputs):
        actual, expected = self.owner.acquire(machine, actor, kernel or self.kernel,
                                             side=side, pixels=self.flat if pixels is None else pixels,
                                             **inputs)
        self.assertEqual(actual, expected)
        type(self).acquisitions += 1
        type(self).transfers += actual['transfer'] is not None
        type(self).requests += len(actual['requests'])
        compact = {**actual, 'actor': actual['actor'].hex()}
        self.digest.update(json.dumps(compact, sort_keys=True).encode())
        return actual

    def aim(self, machine, actor, coarse=0):
        raw = self.owner.raw(machine, actor)
        pointer = struct.unpack_from('<H', raw, 0x97)[0]
        expected, details = target_geometry(self.owner, raw,
                                           {pointer: self.owner.raw(machine, pointer)} if pointer else {}, coarse)
        actual = self.owner.aim(machine, actor, coarse)
        self.assertEqual(actual, expected)
        type(self).aims += 1
        self.digest.update(actual)
        return details

    def consume(self, machine, actor):
        raw = bytearray(self.owner.raw(machine, actor))
        raw[0x43] = 6
        machine.mem_write(DGROUP + actor, bytes(raw))
        descriptor = struct.unpack('<11H', machine.mem_read(DGROUP + 0x85b6 + raw[27]*22, 22))
        expected = throttle_model(bytes(raw), descriptor)
        for offset in (0x8e58, 0x8e5a, 0x8e5c, 0x8e5e):
            machine.mem_write(DGROUP+offset, b'\0')
        actual, dirty = self.throttle.command(machine, actor, raw[27])
        self.assertEqual(actual, expected[0])
        self.assertEqual(bool(dirty[0] == 3), expected[1])
        type(self).throttles += 1
        self.digest.update(actual)

    def begin_corpus_mission(self, name, side):
        """Attach a consuming C loader to the unchanged original world boundary."""

    def test_every_rng_word_four_behavior_choices_all_cursors(self):
        from unicorn.x86_const import UC_X86_REG_DI
        machine, actor, _ = self.prepare()
        raw = bytearray(self.owner.raw(machine, actor))
        struct.pack_into('<H', raw, 0x9d, 0)  # Real null-candidate a6e3 branch.
        original = bytes(raw)
        admitted_counts = []
        for behavior, threshold in enumerate(ACQUISITION_THRESHOLDS):
            machine.mem_write(DGROUP + 0x85b6, struct.pack('<H', behavior))
            admitted = 0
            for value in range(65536):
                cursor = value % 4
                seeds = list(SEEDS)
                seeds[cursor] = value
                machine.mem_write(DGROUP + actor, original)
                machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H', 0x1f84 + ((cursor+3)%4)*2, *seeds))
                result = self.check(machine, actor)
                flags = struct.unpack_from('<H', result['actor'], 0x40)[0]
                admitted += bool(flags & 128)
                self.assertEqual(machine.reg_read(UC_X86_REG_DI), actor)
            self.assertEqual(admitted, (threshold+1)*256)
            admitted_counts.append(admitted)
            print('Complete acquisition RNG/behavior', behavior, admitted, flush=True)
        self.evidence['reaching']['full_rng_admission_counts'] = admitted_counts

    def test_counter_old_target_lazy_behavior_and_control_flag_domains(self):
        for kind in range(4):
            machine, actor, target = self.prepare(kind)
            original = self.owner.raw(machine, actor)
            for count, old in itertools.product(range(256), (0, target)):
                raw = bytearray(original)
                raw[0x94] = count
                struct.pack_into('<H', raw, 0x97, old)
                # Invalid behavior is unreachable when an earlier gate rejects.
                machine.mem_write(DGROUP + 0x85b6, struct.pack('<H', 65535 if not count or old else 0))
                machine.mem_write(DGROUP + actor, bytes(raw))
                self.check(machine, actor)
            machine.mem_write(DGROUP + 0x85b6, bytes(2))
            for flags in (0, 1, 2, 4, 8, 127, 128, 129, 32768, 65535):
                for automatic in (False, True):
                    raw = bytearray(original)
                    struct.pack_into('<H', raw, 0x40, flags)
                    machine.mem_write(DGROUP + actor, bytes(raw))
                    self.check(machine, actor, automatic=automatic, candidate=target)

    def test_all_classes_types_and_both_target_flag_bytes(self):
        for kind, target_kind, side in itertools.product(range(4), range(28), (0, 8)):
            machine, actor, target = self.prepare(kind, target_kind, flags=4 | side)
            self.check(machine, actor, automatic=False, candidate=target, selected=actor,
                       gate=65535, clock=30)
            self.aim(machine, actor)
        for kind in range(4):
            machine, actor, target = self.prepare(kind)
            original = self.owner.raw(machine, actor)
            for offset in (22, 23):
                for flags in range(256):
                    machine.mem_write(DGROUP + actor, original)
                    machine.mem_write(DGROUP + target + offset, bytes([flags]))
                    self.check(machine, actor, automatic=False, candidate=target)
                machine.mem_write(DGROUP + target + offset, bytes((12 if offset == 22 else 8,)))
            for candidate, old in itertools.product((0, target, actor), (0, target)):
                raw = bytearray(original)
                struct.pack_into('<H', raw, 0x97, old)
                machine.mem_write(DGROUP + actor, bytes(raw))
                self.check(machine, actor, automatic=False, candidate=candidate)

    def test_actual_visibility_boundaries_and_unconditional_attempt_flag(self):
        blocked = bytes([255]) * 512**2
        blocked_kernel = self.owner.visibility.prepare(512, blocked)
        observations = []
        for kind, position, pixels in itertools.product(range(4),
                ((-4096, 0, 4096), (262143, 0, 4096), (262144, 0, 4096),
                 (-262144, 0, 4096), (-262145, 0, 4096), (0, 0, -2147483648)),
                (self.flat, blocked)):
            machine, actor, target = self.prepare(kind)
            machine.mem_write(DGROUP + target + 4, struct.pack('<3i', *position))
            result = self.check(machine, actor, kernel=self.kernel if pixels is self.flat else blocked_kernel,
                                pixels=pixels, selected=actor)
            self.assertEqual(struct.unpack_from('<H', result['actor'], 0x40)[0] & 128, 128)
            if result['transfer'] is not None:
                self.assertEqual(struct.unpack_from('<H', result['actor'], 0x97)[0] != 0,
                                 result['transfer']['visible'])
                observations.append(result['transfer']['visible'])
            self.aim(machine, actor)
            self.consume(machine, actor)
        self.assertIn(False, observations)
        self.assertIn(True, observations)

    def test_selected_voice_gates_clock_wrap_and_variant_display_domain(self):
        for kind, target_side, actor_side, gate, selected, clock in itertools.product(
                range(4), (0, 8), (0, 8), (0, 1, 65535), (False, True), (0, 29, 30, 31, 65535)):
            machine, actor, target = self.prepare(kind, flags=4 | target_side)
            machine.mem_write(DGROUP + actor + 22, bytes([4 | actor_side]))
            self.check(machine, actor, automatic=False, candidate=target, selected=actor if selected else 0,
                       gate=gate, clock=clock, last_voice=65520 if clock < 29 else 0)
        for kind, side in itertools.product(range(4), (0, 8)):
            machine, actor, target = self.prepare(kind, 26, flags=4 | side)
            original = self.owner.raw(machine, actor)
            for mode in range(256):
                machine.mem_write(DGROUP + actor, original)
                machine.mem_write(DGROUP + target + 25, bytes([mode]))
                result = self.check(machine, actor, automatic=False, candidate=target, selected=actor)
                self.assertEqual(result['display'][1], self.owner.variant_text[mode])
                self.aim(machine, actor)
        self.evidence['reaching']['variant_message_alias'] = {
            'aim_uses_low_two_bits': True, 'message_uses_unmasked_byte': True,
            'valid_variant_count': 4, 'observed_mode_4_handle': self.owner.variant_text[4],
            'actual_adjacent_table_read': 'GS:2e1f + 2*4 lies past the four-entry variant message table',
            'repair': 'shared C must declare safe typed display output; do not copy adjacent raw text addresses'}

    def test_complete_spatial_aim_wrap_domains_and_throttle_ranges(self):
        machine, actor, target = self.prepare()
        for kind, candidate, coarse, distance in itertools.product(range(4), (0, 5, 26, 27),
                (0, 1, 255), (0, 1, 255, 256, 11488, 11520, 15359, 15360, 23039, 23040,
                             65535, 65536, 16776960, 16777216, 2147483647, -2147483648)):
            raw = bytearray(self.owner.raw(machine, actor))
            struct.pack_into('<H', raw, 0, kind)
            struct.pack_into('<3i', raw, 4, 2147483647, -2147483648, 4096)
            struct.pack_into('<H', raw, 0x97, target)
            machine.mem_write(DGROUP + actor, bytes(raw))
            other = bytearray(251)
            struct.pack_into('<H', other, 0, candidate)
            from test_geometry import signed
            struct.pack_into('<3i', other, 4, signed(2147483647+distance), -2147483648, 4096)
            machine.mem_write(DGROUP + target, bytes(other))
            self.aim(machine, actor, coarse)
            self.consume(machine, actor)
        rng = random.Random(0x0096)
        for index in range(2048):
            raw = bytearray(self.owner.raw(machine, actor))
            raw[:2] = struct.pack('<H', index % 4)
            struct.pack_into('<3i', raw, 4, *(rng.randrange(-2**31, 2**31) for _ in range(3)))
            struct.pack_into('<H', raw, 0x97, target)
            other = bytearray(251)
            struct.pack_into('<H', other, 0, index % 28)
            struct.pack_into('<3i', other, 4, *(rng.randrange(-2**31, 2**31) for _ in range(3)))
            other[25] = index % 256
            machine.mem_write(DGROUP + actor, bytes(raw)); machine.mem_write(DGROUP + target, bytes(other))
            self.aim(machine, actor, index % 3)
        machine.mem_write(DGROUP + actor + 0x97, bytes(2))
        self.aim(machine, actor)

    def test_actual_release_reallocation_and_orphan_candidates(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        observations = []
        for kind in range(4):
            for replacement in (0, 2):
                machine, actor, target = self.prepare(kind)
                before = bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4))
                machine.reg_write(UC_X86_REG_AX, 1)
                self.owner.far_call(machine, 0x1b2ef)
                rejected = self.check(machine, actor, automatic=False, candidate=target)
                self.assertEqual(struct.unpack_from('<H', rejected['actor'], 0x97)[0], 0)
                machine.reg_write(UC_X86_REG_AX, replacement)
                self.owner.far_call(machine, 0x1b1df)
                self.assertEqual(machine.reg_read(UC_X86_REG_EFLAGS) & 1, 0)
                self.assertEqual(machine.reg_read(UC_X86_REG_DI), target)
                self.assertEqual(bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4)), before)
                machine.mem_write(DGROUP + target + 4, struct.pack('<3i', -8192, 0, 4096))
                machine.mem_write(DGROUP + target + 22, bytes((12, 8)))
                installed = self.check(machine, actor, automatic=False, candidate=target)
                self.assertEqual(struct.unpack_from('<H', installed['actor'], 0x97)[0], target)
                self.aim(machine, actor); self.consume(machine, actor)
                # A live physical orphan is eligible for direct acquisition.
                machine.mem_write(DGROUP + REGISTRY + 4, bytes(4))
                orphan = self.check(machine, actor, automatic=False, candidate=target)
                self.assertEqual(struct.unpack_from('<H', orphan['actor'], 0x97)[0], target)
                observations.append({'actor_class': kind, 'replacement_class': replacement,
                                     'same_slot_registry_value': True, 'released_candidate_rejected': True,
                                     'successor_silently_installed': True, 'live_orphan_installed': True})
        self.evidence['reaching']['lifetime'] = observations

    def test_all_pinned_missions_prepared_discovery_acquisition_aim_throttle(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrain = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        self.assertEqual(len(manifest), 47)
        grouped = {}
        for name, info in manifest.items():
            data = (directory/name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            offset, height_name = 0, None
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset+6:offset+22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size+6
            self.assertIsNotNone(height_name)
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual(len(grouped), 8)
        worlds = actors_total = 0
        for height_name, missions in grouped.items():
            data = (directory/height_name).read_bytes(); info = terrain[height_name]
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
                    machine, objects = self.owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                                scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    bindings = struct.unpack('<364H', machine.mem_read(DGROUP+REGISTRY,728))[::2]
                    actors = [p for p in bindings if p and int.from_bytes(self.owner.raw(machine,p)[:2],'little')<4]
                    counts = {'actors': len(actors), 'automatic_installs': 0, 'direct_installs': 0}
                    for actor in actors:
                        actor_raw = self.owner.raw(machine,actor)
                        raw_objects = {p:self.owner.raw(machine,p) for p in set(bindings)-{0}}
                        old = struct.unpack_from('<H',actor_raw,0x97)[0]
                        if old:
                            raw_objects[old] = self.owner.raw(machine,old)
                        expected_scan = discovery_model(self.owner,actor,actor_raw,bindings,raw_objects,
                                                        0,side,pixels,gate=0,selected=actor)
                        discovery = self.owner.scan(machine, actor, kernel, gate=0, selected=actor)
                        self.assertEqual(discovery,expected_scan)
                        result = self.check(machine, actor, kernel=kernel, pixels=pixels, side=side, selected=actor)
                        counts['automatic_installs'] += bool(struct.unpack_from('<H',result['actor'],0x97)[0])
                        self.aim(machine, actor); self.consume(machine, actor)
                        # Declared direct/manual child boundary also consumes the actual scan winner.
                        result = self.check(machine, actor, kernel=kernel, pixels=pixels, side=side,
                                            selected=actor, automatic=False, candidate=discovery['primary'])
                        counts['direct_installs'] += bool(struct.unpack_from('<H',result['actor'],0x97)[0])
                        self.aim(machine,actor,1); self.consume(machine,actor)
                    worlds += 1; actors_total += len(actors)
                    self.evidence['corpus'].setdefault(name,{})[str(side)] = counts
            print('Complete acquisition/aim/throttle corpus:', height_name, 'four details',flush=True)
        self.assertEqual((worlds,actors_total),(188,3840))
        self.evidence['canonical_prepared_worlds'] = worlds
        self.evidence['canonical_ground_actors'] = actors_total


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    parser.add_argument('--test', action='append', help='Supporting pilot only; full acceptance requires all eight groups')
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve()==pathlib.Path('/tmp')):
        parser.error('Temporary evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__, *(args.test or [])], exit=False)
    success = program.result.wasSuccessful() and not program.result.skipped and program.result.testsRun == 8 and not args.test
    evidence = TargetAcquisitionTests.evidence
    evidence['gate'] = {'success':success, 'groups':program.result.testsRun, 'skips':len(program.result.skipped),
        'failures':len(program.result.failures),'errors':len(program.result.errors),
        'complete_acquisitions':TargetAcquisitionTests.acquisitions,'complete_aims':TargetAcquisitionTests.aims,
        'actual_visibility_transfers':TargetAcquisitionTests.transfers,'admitted_requests':TargetAcquisitionTests.requests,
        'complete_consuming_throttles':TargetAcquisitionTests.throttles,
        'complete_output_sha256':TargetAcquisitionTests.digest.hexdigest()}
    print(json.dumps(evidence['gate'],sort_keys=True),flush=True)
    if REVIEW:
        REVIEW.mkdir(parents=True,exist_ok=True)
        (REVIEW/'target-acquisition.json').write_text(json.dumps(evidence,indent=2)+'\n')
    # A filtered pilot may pass, but can never produce an accepted receipt.
    raise SystemExit(not (success if not args.test else
                         program.result.wasSuccessful() and not program.result.skipped))
