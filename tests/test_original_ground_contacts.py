#!/usr/bin/env python3
"""Required original contact, tree, sound, source lifetime and corpus references.

This verifies the original contact boundary; shared C, complete living classes,
PCM/device initialization and full game acceptance require separate gates.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from original_unit_oracle import IMAGE_SHA256

ROOT = Path(__file__).resolve().parents[1]
REVIEW = Path('/tmp/wasm-fist-ground-contact-reference')


class OriginalContactTests(unittest.TestCase):
    def check_fixture(self, name, result_name, count, digest, required_counts):
        fixture = ROOT / 'tests/fixtures/ground_class_callbacks' / name
        model = ROOT / 'tests/physical_contact_contract.py'
        REVIEW.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix='contact-', dir=REVIEW) as temporary:
            environment = dict(os.environ)
            environment['PYTHONPATH'] = str(ROOT / 'tests')
            environment['PYTHONPYCACHEPREFIX'] = '/tmp/wasm-fist-python-cache'
            result = subprocess.run(
                [sys.executable, str(fixture), '--review-dir', temporary],
                cwd=ROOT, env=environment, capture_output=True, text=True, timeout=2400)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, '')
            lines = result.stdout.splitlines()
            self.assertTrue(lines)
            for line in lines[:-1]:
                self.assertTrue(line.startswith('Original contact worlds: '), line)
            observed = json.loads(lines[-1])
            self.assertEqual(observed, json.loads((Path(temporary) / result_name).read_text()))
            self.assertIs(observed['success'], True)
            self.assertEqual(observed['cases'], count)
            if 'branches' in observed:
                self.assertEqual(sum(observed['branches'].values()), count)
            if digest is not None:
                self.assertEqual(observed['output_sha256'], digest)
            self.assertRegex(observed['output_sha256'], r'^[0-9a-f]{64}$')
            self.assertEqual(observed['image_sha256'], IMAGE_SHA256)
            self.assertEqual(observed['fixture_sha256'], hashlib.sha256(fixture.read_bytes()).hexdigest())
            self.assertEqual(observed['model_sha256'], hashlib.sha256(model.read_bytes()).hexdigest())
            for key, value in required_counts.items():
                self.assertEqual(observed['counts'][key], value, key)
            (REVIEW / result_name).write_text(json.dumps(observed, indent=2) + '\n')
        print(f'{name}: {count} required complete original returns; {digest}', flush=True)

    def test_direct_contacts_full_memory_and_eleven_registers(self):
        self.check_fixture(
            'contact_direct_original.py', 'contact-direct-original.json', 5632,
            '9b6b91c2b42f99fc232bb8765cbc51ac79ad9c5439570dd42c07fb3ea74d182b',
            {'geometry_calls': 2560, 'sound_calls': 768,
             'registry_visits': 753152, 'high_word_skips': 512})

    def test_near_obstacle_composition_with_declared_numeric_scratch(self):
        self.check_fixture(
            'contact_prediction_original.py', 'contact-prediction-original.json', 10528,
            '01a78957f004716c47a57d2231418b5575fffb8e4859bc07548b53149fe5f5d1',
            {'geometry_calls': 10528, 'sound_calls': 1728, 'registry_visits': 1610240,
             'near_obstacle_calls': 7016, 'prediction_calls': 3456,
             'prediction_samples': 69696, 'prediction_hits': 576,
             'scratch_excluded_returns': 7016, 'full_returns': 3512})

    def test_tree_health_scale_and_full_word_domains(self):
        self.check_fixture(
            'contact_tree_original.py', 'contact-tree-original.json', 131632,
            '96a5d954d31f71e5a5f0a02ba07f4d077baf8a3cce001f6ff66b4305e41030dd',
            {'returns': 131632, 'released': 121216, 'retained': 10416,
             'wrapped_retained': 9565, 'joint_authored_health_scale_returns': 65536,
             'raw_factor_speed_throttle_rng_seed_returns': 65536})

    def test_all_prepared_original_worlds_at_four_details(self):
        self.check_fixture(
            'contact_world_original.py', 'contact-world-original.json', 15360,
            'bc60014d549ad6a92ebd34e1837ce5490fd76075c3850699259f900e2605fc0c',
            {'returns': 15360, 'prepared_worlds': 188, 'ground_actors': 3840,
             'tree_calls': 7456, 'overlap_candidate_type_21': 7456,
             'overlap_candidate_type_2': 224, 'full_memory_returns': 13424,
             'scratch_excluded_returns': 1936, 'prediction_samples': 11544})

    def test_real_flight_capture_retirement_and_constructor_reuse(self):
        self.check_fixture(
            'contact_source_flight_original.py', 'contact-source-flight-original.json', 48,
            'e107dec2ea8a1c3830ae90fb145f6bf1b62a8beff069d751f1a4bdbf36bce6d1',
            {'contact_returns': 48, 'genuine_capture_returns': 8,
             'complete_flight_updates': 24, 'complete_normal_impact_retirements': 8,
             'genuine_normal_successor_constructors': 8,
             'reachable_reuse_changes_source_side': 8})

    def test_admitted_collision_sound_to_real_original_kernel(self):
        self.check_fixture(
            'contact_sound_original.py', 'contact-sound-original.json', 2608,
            'a502e544890c83927cb085c25ba84f706ff486aa43a8ee3cde57d90e2d45fd23',
            {'contact_returns': 2608, 'actual_kernel_returns': 1408,
             'no_kernel_request_returns': 1200, 'kernel_disabled': 384,
             'kernel_bank_absent': 384, 'kernel_direct': 640})

    def test_saved_ground_contact_bytes_survive_start_and_readiness(self):
        self.check_fixture(
            'contact_retention_original.py', 'contact-retention-original.json', 4096,
            'aac2ef7a8b98c0e91804ced18c5d319183e9e347de9fc6e3fd474ac46ed0ea86',
            {'class_start': 2048, 'readiness': 2048})

    def test_saved_tree_health_survives_full_ready_and_height_transfer(self):
        self.check_fixture(
            'tree_retention_original.py', 'tree-retention-original.json', 2048,
            '891f489e5f51a232fe75186e9812d7f2e2a038e0dd94ff2b623569b9089b3b6c',
            {'restored_payloads': 2048, 'complete_ready_returns': 2048,
             'height_transfers': 2048, 'rng_draws': 4096})

    def test_full_admission_velocity_extent_and_registry_generation_domains(self):
        self.check_fixture(
            'contact_domains_original.py', 'contact-domains-original.json', 262160, None,
            {'complete_returns': 262160,
             'control_word_and_contact_cooldown_pair_domains': 65536,
             'ignored_wrapper_returns': 16,
             'genuine_registry_constructor_returns': 65536,
             'first_word_domain_returns': 65536,
             'repeat_word_domain_returns': 65536,
             'radius_edge_word_domain_returns': 65536})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--review-dir', type=Path, default=REVIEW)
    args = parser.parse_args()
    REVIEW = args.review_dir.resolve()
    if not REVIEW.is_relative_to(Path('/tmp')):
        parser.error('Disposable review evidence must live under /tmp')
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or bool(program.result.skipped))
