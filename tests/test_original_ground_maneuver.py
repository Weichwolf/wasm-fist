#!/usr/bin/env python3
"""Required complete original maneuver, obstacle producer, idle turret and parents."""
import argparse
import collections
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from ground_maneuver_contract import DISPATCH, OFFSETS
from orders_contract import scenario_order_blocks
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_units import records_from_scenario, snapshot
from test_vehicle_motion import start

ROOT = pathlib.Path(__file__).resolve().parents[1]
ACTOR, CANDIDATE = 0x7000, 0x7100
SEEDS = (1, 2, 32768, 65535)


def seed_for_draw(draw):
    following = (draw + 1) & 65535
    odd = following >> 15
    return (((following ^ (0xb400 if odd else 0)) << 1) | odd) & 65535


class ManeuverTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner = OriginalGroundManeuverOracle()
        cls.machine = cls.owner.machine(SEEDS, 3)
        cls.machine.mem_write(DGROUP + 0xdfbc, bytes(728))
        cls.digest = hashlib.sha256()
        cls.counts = collections.Counter()
        cls.states = collections.Counter()
        cls.search_indices = collections.Counter()
        cls.draws = collections.Counter()
        cls.corpus = {}
        cls.groups = {}

    def check(self, machine, actor=ACTOR, *, operation='maneuver', candidate=0):
        actual, effect = self.owner.observe(machine, actor, operation=operation, candidate=candidate)
        self.digest.update(actual[:0x8fc0])
        self.digest.update(actual[0x9000:])
        self.counts[operation] += 1
        for field in ('searches', 'registry_visits', 'prediction_calls', 'prediction_samples', 'prediction_hits'):
            self.counts[field] += effect.get(field, 0)
        if 'state' in effect:
            self.states[effect['state']] += 1
        if effect.get('search_index') is not None:
            self.search_indices[effect['search_index']] += 1
        if 'random_draws' in effect:
            self.draws[effect['random_draws']] += 1
        if 'parent_callback' in effect:
            self.counts['parent_' + str(effect['parent_callback'])] += 1
        return actual, effect

    def fixture(self, kind=0, *, selector=0, remaining=0, blocked_count=0, flags=0,
                heading=0, turret_heading=0, pose=(0, 0), extent=1024, velocity=(0, 0),
                target=0, cursor=0, first_draw=None, second_draw=None, coarse=0):
        raw = bytearray(start(kind))
        raw[0x45], raw[0x46], raw[0x51] = selector, remaining, blocked_count
        raw[22] = 64
        struct.pack_into('<2i', raw, 4, *pose)
        for offset, value in ((0x40, flags), (0x26, heading), (0x10, turret_heading),
                              (0x14, extent), (0x97, target), (0x8b, 12345)):
            store(raw, offset, value)
        struct.pack_into('<2h', raw, 0x59, *velocity)
        words = list(SEEDS)
        if first_draw is not None:
            words[cursor] = seed_for_draw(first_draw)
        if second_draw is not None:
            words[(cursor + 1) % 4] = seed_for_draw(second_draw)
        self.machine.mem_write(DGROUP + ACTOR, bytes(raw))
        self.machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
            0x1f84 + ((cursor + 3) % 4) * 2, *words))
        self.machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        return raw

    def candidate(self, kind=0, *, flags=64, pose=(0, 0), extent=1024, pointer=CANDIDATE):
        raw = bytearray(snapshot(kind, flags=flags, pose=(*pose, 0)))
        store(raw, 0x14, extent)
        self.machine.mem_write(DGROUP + pointer, bytes(raw))
        return raw

    def test_01_complete_selector_counter_control_and_blocked_byte_domains(self):
        count = 0
        for selector, remaining in itertools.product(range(256), repeat=2):
            self.fixture((selector + remaining) % 4, selector=selector, remaining=remaining,
                         flags=8 if remaining & 1 else 0, blocked_count=selector)
            self.check(self.machine)
            count += 1
        for flags in range(65536):
            self.fixture(flags % 4, selector=flags & 255, remaining=(flags >> 8) & 255,
                         flags=flags, blocked_count=(flags * 17) & 255)
            self.check(self.machine)
            count += 1
        for kind, blocked_count, remaining in itertools.product(range(4), range(256), (0, 1, 2, 255)):
            self.fixture(kind, selector=0, remaining=remaining, flags=8, blocked_count=blocked_count)
            self.check(self.machine)
            count += 1
        self.assertEqual(count, 135168)
        self.assertEqual(set(self.states), {0, 2, 4, 6})
        self.groups['selector_counter_control_domains'] = count

    def test_02_complete_prediction_registry_filters_order_and_search_exhaustion(self):
        count = 0
        self.machine.mem_write(DGROUP + 0xdfbc, struct.pack('<HH', CANDIDATE, 1) + bytes(724))
        for flags in range(256):
            for kind in range(4):
                self.fixture(kind, selector=2, remaining=1, turret_heading=32768,
                             heading=(flags * 257) & 65535)
                self.candidate(flags % 28, flags=flags)
                self.check(self.machine)
                count += 1
        for kind, candidate_kind, flags in itertools.product(range(4), range(28), (0, 1, 8, 64, 65, 255)):
            self.fixture(kind, selector=2, remaining=1, turret_heading=32768)
            self.candidate(candidate_kind, flags=flags)
            self.check(self.machine)
            count += 1
        # A forward, stationary predicted point proves every wrapped radius word.
        self.candidate(26, pose=(0, 4608))
        for extent in range(65536):
            self.fixture(extent % 4, extent=extent, flags=extent, velocity=(0, 0))
            self.check(self.machine, operation='motion_obstacle', candidate=CANDIDATE)
            count += 1
        positions = ((0, 0), (-2**31, 2**31 - 1), (2**31 - 1, -2**31), (65535, -65537))
        for kind, pose, delta, velocity, selector, coarse in itertools.product(
                range(4), positions, ((0, 0), (0, 1), (1, 0), (0, 65536), (-65537, 65537)),
                ((-32768, 32767), (-64, 64), (-1, 0), (0, 0), (1, -1), (64, 64)),
                (0, 1, 2, 255), (0, 1, 255)):
            from test_geometry import measure, signed
            target = tuple(signed(a + b) for a, b in zip(pose, delta))
            bearing = (measure((target, pose, coarse))[0] + 32768) & 65535
            self.fixture(kind, pose=pose, velocity=velocity, selector=selector,
                         turret_heading=bearing, heading=(kind * 16384 + 65535) & 65535, coarse=coarse)
            self.candidate(23, pose=target, extent=65535)
            self.check(self.machine, operation='motion_obstacle', candidate=CANDIDATE)
            count += 1
        # Slot order, generation irrelevance, repeated pointers and last-cell admission.
        for index, flags, kind in itertools.product((0, 1, 90, 181), (0, 64, 65), range(4)):
            registry = bytearray(728)
            if index:
                struct.pack_into('<HH', registry, 0, ACTOR, 65535)
            struct.pack_into('<HH', registry, index * 4, CANDIDATE, index ^ 65535)
            self.machine.mem_write(DGROUP + 0xdfbc, bytes(registry))
            self.fixture(kind, selector=2, remaining=1, turret_heading=32768)
            self.candidate(26, flags=flags)
            self.check(self.machine)
            count += 1
        self.machine.mem_write(DGROUP + 0xdfbc, bytes(728))
        for position in ((0, 3000), (0, 5000), (0, 7000), (3000, 4000), (-3000, 4000),
                         (0, 7680), (0, 7681), (0, -3000), (0, 0)):
            for heading, coarse in itertools.product((0, 1, 16384, 32768, 65535), (0, 1)):
                self.fixture(selector=2, remaining=1, heading=heading)
                self.candidate(0, pose=position, extent=1024)
                self.machine.mem_write(DGROUP + 0xdfbc, struct.pack('<HH', CANDIDATE, 1) + bytes(724))
                self.machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
                self.check(self.machine)
                count += 1
        # Actual allocation leaves the overwritten actor live but out of the registry.
        orphans = 0
        for kind in range(4):
            actor = bytearray(start(kind)); actor[22] = 64; actor[0x45] = 2; actor[0x46] = 1
            store(actor, 0x10, 32768)
            previous = bytearray(start(1)); previous[22] = 64
            machine, objects = self.owner.prepare_saved([(0, 1, bytes(actor)), (0, 2, bytes(previous))],
                SEEDS, 3, 0, (bytes(2144), bytes(176)))
            orphan, visible = objects[150][2], objects[151][2]
            self.assertEqual(word(machine.mem_read(DGROUP, 65536), 0xdfbc), visible)
            self.assertNotEqual(orphan, visible)
            _, effect = self.check(machine, orphan)
            self.assertEqual(effect['search_index'], 15)
            orphans += 1
            count += 1
        self.machine.mem_write(DGROUP + 0xdfbc, bytes(728))
        self.assertIn(0, self.search_indices)
        self.assertIn(15, self.search_indices)
        self.assertGreater(len(self.search_indices), 2)
        self.groups['prediction_registry_domains'] = count
        self.groups['actual_allocated_registry_orphans'] = orphans

    def test_03_complete_first_second_random_control_and_target_word_domains(self):
        count = 0
        for value in range(65536):
            self.fixture(value % 4, cursor=value % 4, first_draw=value, second_draw=value ^ 65535)
            self.check(self.machine, operation='idle_turret')
            count += 1
        for value in range(65536):
            self.fixture(value % 4, cursor=(value >> 2) % 4, first_draw=65, second_draw=value)
            actual, effect = self.check(self.machine, operation='idle_turret')
            self.assertEqual(effect['random_draws'], 2)
            self.assertEqual(word(actual, ACTOR + 0x8b), ((value & 0x3fff) - 8192) & 65535)
            count += 1
        for value in range(65536):
            self.fixture(value % 4, flags=value, first_draw=value, cursor=value % 4)
            self.check(self.machine, operation='idle_turret')
            count += 1
        for value in range(65536):
            self.fixture(value % 4, target=value, first_draw=0, cursor=value % 4)
            actual, effect = self.check(self.machine, operation='idle_turret')
            self.assertEqual(effect['random_draws'], int(value == 0))
            if value:
                self.assertEqual(word(actual, ACTOR + 0x8b), 12345)
            count += 1
        self.assertEqual(count, 262144)
        self.assertEqual(set(self.draws), {0, 1, 2})
        self.groups['idle_turret_word_domains'] = count

    def test_04_genuine_parent_banks_counter_rng_and_global_admission(self):
        count = 0
        self.machine.mem_write(DGROUP + 0x978a, b'\0')
        self.machine.mem_write(DGROUP + 0x7ae0, bytes(2))
        for kind, automatic, entry, high, cursor, value in itertools.product(
                range(4), (0, 1), (7, 11, 15), range(16), range(4), (0, 1, 65, 32767, 32768, 65535)):
            raw = self.fixture(kind, selector=2 if cursor & 1 else 0, remaining=1,
                               flags=automatic, first_draw=value, second_draw=65, cursor=cursor)
            raw[0x42] = high * 16 + entry - 1
            raw[27] = kind
            self.machine.mem_write(DGROUP + ACTOR, bytes(raw))
            self.check(self.machine, operation='parent')
            count += 1
        for admission, kind, automatic, entry in itertools.product(range(1, 256), range(4), (0, 1), (7, 11, 15)):
            raw = self.fixture(kind, selector=255, remaining=255, flags=automatic, cursor=kind,
                               first_draw=admission * 257)
            raw[27], raw[0x42] = 255, entry - 1  # Both are unused after global rejection.
            self.machine.mem_write(DGROUP + ACTOR, bytes(raw))
            self.machine.mem_write(DGROUP + 0x978a, bytes([admission]))
            actual, effect = self.check(self.machine, operation='parent')
            self.assertIsNone(effect['parent_callback'])
            self.assertEqual(actual[ACTOR:ACTOR + 251], bytes(raw))
            count += 1
        self.machine.mem_write(DGROUP + 0x978a, b'\0')
        self.assertEqual(count, 15336)
        self.groups['genuine_parent_domains'] = count

    def test_05_all_original_prepared_worlds_and_reaching_obstacle_consumers(self):
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
        worlds = ground = complete = reaching = forced = 0
        for height_name, missions in grouped.items():
            info = terrains[height_name]
            source = (directory / height_name).read_bytes()
            self.assertEqual(hashlib.sha256(source).hexdigest(), info['sha256'])
            decoded = decoder.klc(source)
            self.assertEqual(hashlib.sha256(decoded).hexdigest(), info['decoded_sha256'])
            plane = decoded.split(b'\n', 1)[1][768:]
            for side in (512, 1024, 2048, 4096):
                actual_side, pixels = scaler.resample(info['width'], plane, [side])
                self.assertEqual(actual_side, side)
                for name, data in missions:
                    machine, objects = self.owner.prepare_saved(records_from_scenario(data), SEEDS, 3, 0,
                                                                 scenario_order_blocks(data))
                    self.owner.reset(machine, pixels, side=side)
                    prepared = bytes(machine.mem_read(DGROUP, 65536))
                    actors = [pointer for _, _, pointer, _ in objects.values() if word(prepared, pointer) < 4]
                    observer = getattr(self, 'begin_prepared_mission', None)
                    if observer is not None:
                        observer(name, side, pixels, machine, objects)
                    local = collections.Counter()
                    for actor in actors:
                        self.check(machine, actor)
                        self.check(machine, actor, operation='idle_turret')
                        local['unchanged_saved_child_returns'] += 2
                        current = bytes(machine.mem_read(DGROUP, 65536))
                        candidate = next((word(current, 0xdfbc + index * 4) for index in range(182)
                            if word(current, 0xdfbc + index * 4) not in (0, actor)
                            and word(current, word(current, 0xdfbc + index * 4)) != 21
                            and current[word(current, 0xdfbc + index * 4) + 22] & 64), None)
                        if candidate is not None:
                            self.check(machine, actor, operation='motion_obstacle', candidate=candidate)
                            self.check(machine, actor)
                            local['reaching_motion_and_maneuver_returns'] += 2
                        # Declared consuming search boundary in the real physical world.
                        machine.mem_write(DGROUP + actor + 0x45, b'\x02\x01')
                        self.check(machine, actor)
                        local['constructed_full_search_returns'] += 1
                    for automatic in (0, 1):
                        machine.mem_write(DGROUP, prepared)
                        machine.mem_write(DGROUP + 0x978a, b'\0')
                        machine.mem_write(DGROUP + 0x7ae0, bytes(2))
                        for actor in actors:
                            for entry in (7, 11, 15):
                                machine.mem_write(DGROUP + actor + 0x42, bytes([entry - 1]))
                                flags = word(machine.mem_read(DGROUP, 65536), actor + 0x40)
                                machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', (flags & 65534) | automatic))
                                self.check(machine, actor, operation='parent')
                                local['genuine_parent_returns'] += 1
                    self.assertEqual(local['genuine_parent_returns'], len(actors) * 6)
                    self.corpus.setdefault(name, {})[str(side)] = {'ground_actors': len(actors), **dict(local)}
                    worlds += 1
                    ground += len(actors)
                    complete += sum(local.values())
                    reaching += local['reaching_motion_and_maneuver_returns']
                    forced += local['constructed_full_search_returns']
            print(f'Complete maneuver corpus: {height_name}, four details', flush=True)
        self.assertEqual((worlds, ground, forced), (188, 3840, 3840))
        self.assertGreater(reaching, 0)
        self.groups['canonical'] = {'worlds': worlds, 'ground_actors': ground, 'complete_returns': complete,
                                    'reaching_motion_and_maneuver_returns': reaching, 'constructed_full_search_returns': forced}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    if args.review_dir and (not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp'))
                            or args.review_dir.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 5 and not program.result.skipped
    if success and args.review_dir:
        args.review_dir.mkdir(parents=True, exist_ok=True)
        result = {'success': True, 'groups': 5, 'skips': 0, 'scope': 'Complete original maneuvers/idle turret/predictor and diagnostic-unselected parent entries 7/11/15; shared C/full parent/class/battle/PCM remain open',
                  'counts': dict(ManeuverTests.counts), 'maneuver_states': dict(ManeuverTests.states),
                  'search_indices': dict(ManeuverTests.search_indices), 'idle_draw_counts': dict(ManeuverTests.draws),
                  'coverage': ManeuverTests.groups, 'corpus': ManeuverTests.corpus,
                  'dispatch': DISPATCH, 'offsets': OFFSETS, 'original_image_sha256': IMAGE_SHA256,
                  'output_sha256': ManeuverTests.digest.hexdigest(), 'complete_wasm_streak': 0}
        (args.review_dir / 'ground-maneuver.json').write_text(json.dumps(result, indent=2) + '\n')
        print(json.dumps({k:v for k,v in result.items() if k != 'corpus'}, sort_keys=True), flush=True)
    raise SystemExit(not success)
