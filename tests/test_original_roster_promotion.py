#!/usr/bin/env python3
"""Required complete original roster promotion, malformed-domain and parent evidence."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from original_target_acquisition_oracle import OriginalTargetAcquisitionOracle, TEXT_BASE, MAILBOX
from original_unit_oracle import DGROUP, IMAGE_SHA256
from orders_contract import scenario_order_blocks
from roster_promotion_contract import ROSTER, parent, promote, store, word
from test_units import records_from_scenario, snapshot

ROOT = pathlib.Path(__file__).resolve().parents[1]
REVIEW = None
ACTOR, PREDECESSOR = 0x7000, 0x7100
SEEDS = (1, 2, 32768, 65535)


class PromotionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalTargetAcquisitionOracle()
        cls.digest = hashlib.sha256()
        cls.calls = cls.parents = cls.changes = 0
        cls.evidence = {'scope': 'Complete original b053 and genuine diagnostic-unselected ab03 entry 13; shared C and complete parent remain open',
                        'corpus': {}, 'malformed': {}, 'complete_wasm_streak': 0}

    def check(self, machine, actor, *, through_parent=False):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SP
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        expected = parent(before, actor) if through_parent else promote(before, actor)
        code = bytes(machine.mem_read(0, DGROUP))
        text = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        self.owner.execute(machine, 0xab03 if through_parent else 0xb053, 0xeff0)
        di, ds, sp, ss = self.owner.machine_registers()
        self.assertEqual(tuple(machine.reg_read(reg) for reg in (di, ds, sp, ss)),
                         (actor, 0x1c00, 0x9002, 0x1c00))
        actual = bytes(machine.mem_read(DGROUP, 65536))
        expected[0x8fc0:0x9000] = actual[0x8fc0:0x9000]  # Only bounded call-stack scratch.
        self.assertEqual(actual, bytes(expected), 'Complete roster/actor/predecessor/unrelated DGROUP differs')
        self.assertEqual(bytes(machine.mem_read(0, DGROUP)), code)
        self.assertEqual(bytes(machine.mem_read(TEXT_BASE, 65536)), text)
        self.assertEqual(bytes(machine.mem_read(MAILBOX, 4096)), mailbox)
        changed = before[ROSTER:ROSTER + 64] != actual[ROSTER:ROSTER + 64]
        type(self).calls += 1
        type(self).parents += through_parent
        type(self).changes += changed
        self.digest.update(actual[:0x8fc0])
        self.digest.update(actual[0x9000:])
        return actual, changed

    def fixture(self, machine, kind=0, platoon=0, member=1, *, actor_flags=0,
                predecessor_kind=0, predecessor_flags=16, presence=1,
                predecessor_member=0, control=65535):
        machine.mem_write(DGROUP + ROSTER, bytes(64))
        actor = bytearray(snapshot(kind, platoon=platoon, member=member))
        actor[25] = actor_flags
        store(actor, 0x40, control)
        previous = bytearray(snapshot(predecessor_kind, platoon=platoon, member=predecessor_member))
        previous[25] = predecessor_flags
        machine.mem_write(DGROUP + ACTOR, bytes(actor))
        machine.mem_write(DGROUP + PREDECESSOR, bytes(previous))
        if member:
            index = ((platoon * 4) & 0xff00) | ((platoon * 4 + member) & 255)
            previous_pointer = (0, PREDECESSOR, ACTOR)[presence]
            machine.mem_write(DGROUP + ROSTER + 2 * index - 2,
                              struct.pack('<HH', previous_pointer, ACTOR))
        return actor, previous

    def test_complete_both_flag_byte_domains_all_ground_predecessors_and_authored_slots(self):
        machine = self.owner.machine(SEEDS, 3)
        masks = (0, 2, 4, 6, 16, 18, 20, 22)
        count = 0
        for kind, predecessor_kind in itertools.product(range(4), (0, 1, 2, 3, 19)):
            for full, masked in itertools.product(range(256), masks):
                platoon, member = full % 8, 1 + (full // 8) % 3
                for actor_flags, predecessor_flags in ((full, masked), (masked, full)):
                    self.fixture(machine, kind, platoon, member, actor_flags=actor_flags,
                                 predecessor_kind=predecessor_kind, predecessor_flags=predecessor_flags,
                                 predecessor_member=member - 1)
                    self.check(machine, ACTOR)
                    count += 1
        self.assertEqual(count, 81920)
        self.evidence['complete_flag_cases'] = count

    def test_null_wreck_alias_orphans_and_byte_counter_wrap(self):
        machine = self.owner.machine(SEEDS, 3)
        count = 0
        for kind, platoon, member, actor_flags, previous_member in itertools.product(
                range(4), range(8), range(4), (0, 2, 4, 16, 255), (0, 1, 3, 254, 255)):
            for previous_kind, presence in ((0, 0), (23, 1), (0, 2)):
                self.fixture(machine, kind, platoon, member, actor_flags=actor_flags,
                             predecessor_kind=previous_kind, presence=presence,
                             predecessor_member=previous_member)
                # Registry occupancy is irrelevant: these are retained physical references.
                actual, _ = self.check(machine, ACTOR)
                if member and presence == 1:
                    self.assertEqual(actual[PREDECESSOR + 36], (previous_member + 1) & 255)
                if member and presence == 2 and not actor_flags & 0x16:
                    self.assertEqual(actual[ACTOR + 28], member)
                count += 1
        self.assertEqual(count, 9600)
        # The member-zero branch must not inspect even invalid platoon bytes.
        for kind, platoon, flags in itertools.product(range(4), range(256), (0, 255)):
            self.fixture(machine, kind, platoon, 0, actor_flags=flags)
            self.check(machine, ACTOR)
            count += 1
        # Use actual allocation/loading and overwrite the actor's registry entry.
        # Its physical roster pointer remains live and must still be promoted.
        for kind, predecessor_kind in itertools.product(range(4), (0, 1, 2, 3, 23)):
            # Original wreck import deliberately skips roster index zero.
            # Use the second platoon so both real loader branches install it.
            raw_actor = bytearray(snapshot(kind, flags=32, platoon=1, member=1))
            raw_previous = bytearray(snapshot(predecessor_kind, flags=32, platoon=1, member=0))
            allocated, objects = self.owner.prepare_saved(
                [(0, 1, bytes(raw_actor)), (0, 2, bytes(raw_previous))],
                SEEDS, 3, 0, (bytes(2144), bytes(176)))
            orphan = objects[150][2]
            predecessor = objects[0 if predecessor_kind == 23 else 151][2]
            self.assertEqual(word(allocated.mem_read(DGROUP, 65536), 0xdfbc), predecessor)
            self.assertNotEqual(orphan, predecessor)
            self.assertEqual(word(allocated.mem_read(DGROUP, 65536), ROSTER + 10), orphan)
            self.assertEqual(word(allocated.mem_read(DGROUP, 65536), ROSTER + 8), predecessor)
            allocated.mem_write(DGROUP + orphan + 25, b'\0')
            allocated.mem_write(DGROUP + predecessor + 25, b'\x10')
            actual, changed = self.check(allocated, orphan)
            self.assertTrue(changed)
            self.assertEqual(word(actual, ROSTER + 8), orphan)
            self.assertEqual(word(actual, ROSTER + 10), predecessor)
            self.assertEqual(word(actual, 0xdfbc), predecessor)
            count += 1
        self.assertEqual(count, 11668)
        self.evidence['null_wreck_alias_cases'] = count
        self.evidence['actual_allocated_registry_orphans'] = 20
        self.evidence['unused_leader_platoon_byte_domain'] = 256

    def test_malformed_member_address_wrap_and_neighbor_writes(self):
        machine = self.owner.machine(SEEDS, 3)
        outside = wrapped = count = 0
        for platoon, member in itertools.product(range(8), range(4, 256)):
            self.fixture(machine, platoon=platoon, member=member, presence=0)
            before = bytes(machine.mem_read(DGROUP, 65536))
            actual, _ = self.check(machine, ACTOR)
            index = (platoon * 4 + member) & 255
            previous = ROSTER + 2 * index - 2
            outside += not (ROSTER <= previous and previous + 4 <= ROSTER + 64)
            wrapped += platoon * 4 + member >= 256
            self.assertEqual(word(actual, previous), ACTOR)
            self.assertNotEqual(actual[ACTOR + 28], before[ACTOR + 28])
            count += 1
        self.assertEqual(count, 2016)
        self.assertGreater(outside, 0)
        self.assertGreater(wrapped, 0)
        self.evidence['malformed'] = {'cases': count, 'out_of_roster_writes': outside,
                                      'byte_lane_wraps': wrapped,
                                      'c_requirement': 'Reject used member/platoon/index values outside the authored roster before mutation; preserve the unused leader early return.'}

    def test_genuine_parent_both_banks_counter_rng_and_every_admission_byte(self):
        machine = self.owner.machine(SEEDS, 3)
        count = 0
        for kind, automatic, counter, value, cursor in itertools.product(
                range(4), (0, 1), range(12, 256, 16), (0, 1, 2, 32767, 32768, 65535), range(4)):
            self.fixture(machine, kind, kind, 1 + cursor % 3, control=65534 | automatic)
            machine.mem_write(DGROUP + ACTOR + 0x42, bytes([counter]))
            machine.mem_write(DGROUP + 0x978a, b'\0')
            machine.mem_write(DGROUP + 0x7ae0, bytes(2))
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H', 0x1f84 + ((cursor + 3) % 4) * 2,
                                                        value, value ^ 65535, value ^ 32768, value ^ 1))
            self.check(machine, ACTOR, through_parent=True)
            count += 1
        self.assertEqual(count, 3072)
        for admission in range(1, 256):
            self.fixture(machine, member=1)
            machine.mem_write(DGROUP + ACTOR + 0x42, b'\x0c')
            machine.mem_write(DGROUP + 0x978a, bytes([admission]))
            self.check(machine, ACTOR, through_parent=True)
        self.evidence['parent_domain_returns'] = count + 255

    def test_all_pinned_missions_actual_prepared_roster_and_both_parent_banks(self):
        from original_asset_oracle import OriginalAssetOracle
        from original_heightfield_oracle import OriginalHeightfieldOracle
        decoder, scaler = OriginalAssetOracle(), OriginalHeightfieldOracle()
        manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
        terrains = json.loads((ROOT / 'tests/terrain_originals.json').read_text())['files']
        directory = ROOT / 'armoredfist/FISTDATA'
        self.assertEqual(sorted(p.name for p in directory.glob('*.FSG')), sorted(manifest))
        self.assertEqual(len(manifest), 47)
        grouped = {}
        for name, info in manifest.items():
            data = (directory / name).read_bytes()
            self.assertEqual((len(data), hashlib.sha256(data).hexdigest()), (info['size'], info['sha256']))
            offset = 0
            height_name = None
            while offset < len(data):
                tag, size = struct.unpack_from('<4sH', data, offset)
                if tag == b'BINF':
                    height_name = data[offset + 6:offset + 22].split(b'\0')[0].replace(b' ', b'').decode().upper()
                offset += size + 6
            self.assertEqual(offset, len(data))
            self.assertIn(height_name, terrains)
            grouped.setdefault(height_name, []).append((name, data))
        self.assertEqual(len(grouped), 8)
        worlds = actors_total = calls = swaps = 0
        for height_name, missions in grouped.items():
            info = terrains[height_name]
            data = (directory / height_name).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), info['sha256'])
            decoded = decoder.klc(data)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                installed, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(installed, side)
                for name, data in missions:
                    machine, objects = self.owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                                scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    actors = [p for _, _, p, _ in objects.values() if word(self.owner.raw(machine, p), 0) < 4]
                    prepared = bytes(machine.mem_read(DGROUP, 65536))
                    changed = 0
                    for actor in actors:
                        self.assertIn(prepared[actor + 27], range(8))
                        self.assertIn(prepared[actor + 28], range(4))
                        _, promoted = self.check(machine, actor)
                        changed += promoted
                        calls += 1
                    for automatic in (0, 1):
                        machine.mem_write(DGROUP, prepared)
                        machine.mem_write(DGROUP + 0x978a, b'\0')
                        machine.mem_write(DGROUP + 0x7ae0, bytes(2))
                        for actor in actors:
                            machine.mem_write(DGROUP + actor + 0x42, b'\x0c')
                            flags = word(machine.mem_read(DGROUP, 65536), actor + 0x40)
                            machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', (flags & 65534) | automatic))
                            self.check(machine, actor, through_parent=True)
                            calls += 1
                    self.evidence['corpus'].setdefault(name, {})[str(side)] = {'ground_actors': len(actors),
                        'child_roster_changes': changed, 'complete_child_and_parent_returns': len(actors) * 3}
                    worlds += 1
                    actors_total += len(actors)
                    swaps += changed
            print(f'Complete roster corpus: {height_name}, four details', flush=True)
        self.assertEqual((worlds, actors_total, calls), (188, 3840, 11520))
        self.evidence['canonical'] = {'worlds': worlds, 'ground_actors': actors_total,
                                     'complete_child_and_parent_returns': calls, 'child_roster_changes': swaps}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    REVIEW = args.review_dir
    if REVIEW and (not REVIEW.resolve().is_relative_to(pathlib.Path('/tmp')) or REVIEW.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 5 and not program.result.skipped
    if REVIEW and success:
        REVIEW.mkdir(parents=True, exist_ok=True)
        evidence = {**PromotionTests.evidence, 'success': True, 'groups': 5, 'skips': 0,
                    'complete_returns': PromotionTests.calls, 'complete_parent_returns': PromotionTests.parents,
                    'changed_roster_returns': PromotionTests.changes, 'original_image_sha256': IMAGE_SHA256,
                    'output_sha256': PromotionTests.digest.hexdigest()}
        (REVIEW / 'roster-promotion.json').write_text(json.dumps(evidence, indent=2) + '\n')
    raise SystemExit(not success)
