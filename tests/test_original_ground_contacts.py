#!/usr/bin/env python3
"""Required bounded original direct-contact and predictive-composition references.

This does not accept tree damage, admitted sound, all-47 contact coverage, shared
C or complete living ground classes. No runtime build is needed for this gate.
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
                cwd=ROOT, env=environment, capture_output=True, text=True, timeout=600)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, '')
            observed = json.loads(result.stdout)
            self.assertEqual(observed, json.loads((Path(temporary) / result_name).read_text()))
            self.assertIs(observed['success'], True)
            self.assertEqual(observed['cases'], count)
            self.assertEqual(observed['counts']['complete_returns'], count)
            self.assertEqual(sum(observed['branches'].values()), count)
            self.assertEqual(observed['output_sha256'], digest)
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


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--review-dir', type=Path, default=REVIEW)
    args = parser.parse_args()
    REVIEW = args.review_dir.resolve()
    if not REVIEW.is_relative_to(Path('/tmp')):
        parser.error('Disposable review evidence must live under /tmp')
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or bool(program.result.skipped))
