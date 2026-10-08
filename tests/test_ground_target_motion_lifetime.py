"""Require complete lifecycle, projection and atomic-failure observations."""
import argparse
import hashlib
import json
import pathlib
import subprocess


EXPECTED = b'ground_motion_contracts 40 104 8 40 48\n'
COUNTS = {
    'repairs': 40,
    'atomic_rejections': 104,
    'projection_cases': 8,
    'live_reference_cases': 40,
    'stage_rejections': 48,
}


def verify(command):
    result = subprocess.run(command, capture_output=True, check=True, timeout=30)
    if result.stdout != EXPECTED or result.stderr:
        raise RuntimeError(
            f'Incomplete or unexpected lifecycle output: '
            f'{result.stdout!r}; stderr={result.stderr!r}')
    return {
        'counts': COUNTS,
        'cases': sum(COUNTS.values()),
        'output_sha256': hashlib.sha256(result.stdout).hexdigest(),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path,
                        default=pathlib.Path('/tmp/wasm-fist-rewrite'))
    parser.add_argument('--native-probe', type=pathlib.Path)
    args = parser.parse_args()
    results = {}
    if args.target in ('native', 'all'):
        probe = args.native_probe or (
            args.build_root / 'native/fist_ground_target_motion_lifetime_probe')
        results['native'] = verify([str(probe.resolve())])
    if args.target in ('wasm', 'all'):
        probe = args.build_root / 'wasm/fist_ground_target_motion_lifetime_probe.js'
        results['wasm'] = verify(['node', str(probe.resolve())])
    print(json.dumps({'success': True, 'targets': results}, sort_keys=True))


if __name__ == '__main__':
    main()
