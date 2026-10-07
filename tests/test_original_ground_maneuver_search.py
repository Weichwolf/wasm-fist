#!/usr/bin/env python3
"""Observe every maneuver search exit with actual original physical allocation."""
import argparse
import hashlib
import itertools
import json
import pathlib
import struct
import unittest

from ground_maneuver_contract import OFFSETS
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP, IMAGE_SHA256
from roster_promotion_contract import store, word
from test_original_ground_maneuver import SEEDS
from test_units import snapshot
from test_vehicle_motion import rotate, start


class SearchTests(unittest.TestCase):
    evidence = None

    def test_every_first_clear_index_and_exhaustion_with_real_allocated_bodies(self):
        owner = OriginalGroundManeuverOracle()
        digest = hashlib.sha256()
        contexts = calls = samples = hits = 0
        exits = set()
        for kind, coarse, blocked in itertools.product(range(4), (0, 1, 255), range(16)):
            actor = bytearray(start(kind, control=1))
            actor[22], actor[0x45], actor[0x46] = 64, 2, 1
            records = [(0, 1, bytes(actor))]
            for index in range(blocked):
                # A declared wrapped-radius fixture isolates this exact sampled ray.
                # 1024 + 1024 + 63488 wraps to zero; this is not an authored-size claim.
                point = tuple(value * 72 for value in rotate(OFFSETS[index], 32, coarse))
                candidate = bytearray(snapshot(26, flags=64, pose=(*point, 0)))
                store(candidate, 0x14, 63488)
                records.append((index + 1, 1, bytes(candidate)))
            machine, objects = owner.prepare_saved(records, SEEDS, 3, 0,
                                                    (bytes(2144), bytes(176)))
            receiver = objects[150][2]
            self.assertEqual(len(objects), blocked + 1)
            self.assertEqual(owner.bindings(machine)[::2].count(0), 181 - blocked)
            machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
            after, effect = owner.observe(machine, receiver)
            self.assertEqual(effect['search_index'], blocked)
            self.assertEqual(effect['searches'], min(blocked + 1, 15))
            self.assertEqual(effect['prediction_hits'], blocked)
            self.assertEqual(after[receiver + 0x45:receiver + 0x47], b'\x04\x04')
            self.assertEqual(word(after, receiver + 0x47), OFFSETS[blocked + 8] & 65535)
            self.assertEqual(word(after, 0x97a8), blocked * 2)
            self.assertEqual(word(after, 0x97a6), blocked * 2 + 16)
            digest.update(after[:0x8fc0]); digest.update(after[0x9000:])
            contexts += 1
            calls += effect['prediction_calls']
            samples += effect['prediction_samples']
            hits += effect['prediction_hits']
            exits.add(blocked)
        self.assertEqual(contexts, 192)
        self.assertEqual(exits, set(range(16)))
        ordered = 0
        for kind, reverse in itertools.product(range(4), (0, 1)):
            actor = bytearray(start(kind, control=1))
            actor[22], actor[0x45], actor[0x46] = 64, 2, 1
            records = [(0, 1, bytes(actor))]
            points = ((0, 3000), (400, 3000))
            for index, point in enumerate(points):
                body = bytearray(snapshot(26 + index, flags=64, pose=(*point, 0)))
                store(body, 0x14, 1024)
                registry = 1 if index == reverse else 181
                records.append((registry, 65535 if index == 0 else 1, bytes(body)))
            machine, objects = owner.prepare_saved(records, SEEDS, 3, 0,
                                                    (bytes(2144), bytes(176)))
            receiver = objects[150][2]
            first = owner.bindings(machine)[2]
            self.assertEqual(struct.unpack('<2i', machine.mem_read(DGROUP + first + 4, 8)), points[reverse])
            after, effect = owner.observe(machine, receiver)
            self.assertEqual(effect['search_index'], 15)
            self.assertEqual(struct.unpack_from('<2i', after, 0x9690), points[reverse])
            self.assertEqual(word(after, 0x97a2), 0xdfc4)
            self.assertEqual(word(after, 0x97a4), 180)
            digest.update(after[:0x8fc0]); digest.update(after[0x9000:])
            calls += effect['prediction_calls']; samples += effect['prediction_samples']
            hits += effect['prediction_hits']; ordered += 1
        self.assertEqual(ordered, 8)
        type(self).evidence = {'success': True, 'groups': 1, 'skips': 0, 'contexts': contexts,
            'complete_original_returns': contexts + ordered, 'ordered_competing_body_contexts': ordered,
            'first_registry_body_precedes_nearest_body': True, 'search_indices': sorted(exits),
            'actual_actor_classes': 4, 'coarse_bytes': [0, 1, 255], 'max_allocated_bodies': 16,
            'prediction_calls': calls, 'prediction_samples': samples, 'prediction_hits': hits,
            'fixture': 'Actual pool/registry imports; constructed uint16 radius wrap isolates sampled rays',
            'scope': 'Complete original search-exit coverage; shared C remains open',
            'original_image_sha256': IMAGE_SHA256, 'output_sha256': digest.hexdigest(),
            'complete_wasm_streak': 0}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=pathlib.Path)
    args = parser.parse_args()
    if args.review_dir and (not args.review_dir.resolve().is_relative_to(pathlib.Path('/tmp'))
                            or args.review_dir.resolve() == pathlib.Path('/tmp')):
        parser.error('Evidence belongs in a dedicated /tmp directory')
    program = unittest.main(argv=[__file__], exit=False)
    success = program.result.wasSuccessful() and program.result.testsRun == 1 and not program.result.skipped
    if success and args.review_dir:
        args.review_dir.mkdir(parents=True, exist_ok=True)
        (args.review_dir / 'ground-maneuver-search.json').write_text(json.dumps(SearchTests.evidence, indent=2) + '\n')
        print(json.dumps(SearchTests.evidence, sort_keys=True), flush=True)
    raise SystemExit(not success)
