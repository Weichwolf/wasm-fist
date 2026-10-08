#!/usr/bin/env python3
"""Require real prepared-world station/support consumers on production targets.

Independent inputs/predictions are accepted by closed0109. The program must load
and prepare the actual FSG, keep its captured resource lifetimes, destroy source
storage and preserve every unrelated world byte while consuming each child.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import time

from original_object_pool_oracle import REGISTRY
from remaining_ground_canonical_contract import CASES, cases, encoded_case, predict, seeded_case
from remaining_ground_corpus import Corpus, DETAILS, DIRECTORY
from remaining_ground_probe_input import NONE, line
from roster_promotion_contract import store, word


def commands(args):
    result = {}
    if args.target in ('native', 'all'):
        result['native'] = [str(args.native_probe or args.build_root / 'native/fist_remaining_ground_probe')]
    if args.target in ('wasm', 'all'):
        result['wasm'] = ['node', str(args.build_root / 'wasm/fist_remaining_ground_probe.js')]
    return result


def check_output(actual, expected, context):
    if actual != expected:
        left, right = actual.splitlines(), expected.splitlines()
        index = next((i for i, (a, b) in enumerate(zip(left, right)) if a != b), min(len(left), len(right)))
        raise AssertionError(f'{context}: output differs at line {index}: '
                             f'{left[index:index+1]!r} != {right[index:index+1]!r}; '
                             f'{len(left)} vs {len(right)} lines')


def ownership_cases(world, inputs):
    """Every prepared side1 resource, and each present bound ground class."""
    bound = {}
    for f in inputs:
        index, generation = f['identity']
        if (f['name'] == 'air' and word(world.data, REGISTRY + 4 * index) == f['actor_pointer']
                and word(world.data, REGISTRY + 4 * index + 2) == generation):
            bound.setdefault(f['kind'], f)
    if not bound:
        raise AssertionError('Actual prepared mission has no bound ground requester')
    for f in bound.values():
        for post in (1, 2):
            yield dict(f, post_requester=post, ownership='requester', ownership_class=f['kind'])
    slot = next(iter(bound.values()))['actor_slot']
    # A mission's first physical target may itself be its first artillery gun.
    # Keep a separate real ground target while expiring resources so the test
    # reaches the resource scan rather than the proved stale-target admission.
    target = next(other for other in world.ground_actors() if other != slot)
    artillery = seeded_case(world, slot, target, 'artillery')
    for index in range(len(artillery['resources'])):
        for retained, operation in ((4, 'release'), (12, 'reuse'), (16, 'orphan')):
            yield dict(artillery, retained=retained, gun_index=index, ownership=operation)


def ownership_prediction(world, f):
    if f['ownership'] == 'requester':
        output = predict(world, f)[0]
        reuse = f['post_requester'] == 2
        return output + line('requester_lifetime', [f['actor_slot'], int(reuse), 0, int(reuse)])
    index = f['gun_index']
    old_slot = f['resources'][index][0]
    expired = f['ownership'] != 'orphan'
    before = bytearray(f['before'])
    if expired:
        # The original scan has no allocation lifetimes. Suppress only this
        # expired resource in its independent decision model; the real C
        # successor/released payload must retain its original five rounds.
        store(before, world.physical()[old_slot] + 31, 0)
    output = predict(world, dict(f, before=bytes(before)))[0]
    rows = output.splitlines(keepends=True)
    result = next(row for row in rows if row.startswith('result ')).split()
    spent = int(result[4])
    guns = [f'{slot}:{5-int(slot == spent)}:{int(not expired or slot != old_slot)}'
            for slot, _, _ in f['resources']]
    guns += [f'{NONE}:0:0'] * (4 - len(guns))
    delta = -1 if f['ownership'] == 'release' else 1 if f['ownership'] == 'orphan' else 0
    if f['ownership'] == 'reuse':
        # Real preparation can leave earlier pool holes. The fixture uses
        # actual normal constructors to fill them before release, proving
        # reuse of this exact physical slot rather than assuming allocation.
        physical = world.physical()
        delta += sum(slot not in physical for slot in range(old_slot))
    for row, value in enumerate(rows):
        if value.startswith('guns '):
            rows[row] = line('guns', guns)
        elif value.startswith('pool '):
            rows[row] = line('pool', [word(world.data, 0xe294) + delta, word(world.data, 0xe296)])
    return ''.join(rows)


def verify(args):
    begin = time.monotonic()
    corpus = Corpus()
    programs = commands(args)
    counts = {target: collections.Counter() for target in programs}
    digests = {target: hashlib.sha256() for target in programs}
    records = {}
    ownership_required = collections.Counter()
    requester_classes = set()
    with tempfile.TemporaryDirectory(prefix='fist-canonical-support-', dir='/tmp') as temporary:
        request = pathlib.Path(temporary) / 'request.bin'
        for world in corpus.worlds():
            inputs = list(cases(world))
            ownership = list(ownership_cases(world, inputs))
            ownership_required.update(f['ownership'] for f in ownership)
            requester_classes.update(f['ownership_class'] for f in ownership if f['ownership'] == 'requester')
            payload = struct.pack('<2I', world.side, len(inputs) + len(ownership)) + world.pixels
            payload += b''.join(encoded_case(f) for f in inputs + ownership)
            expected = 'prepared\n' + world.after_observation + 'children\n'
            expected += ''.join(predict(world, f)[0] for f in inputs)
            expected += ''.join(ownership_prediction(world, f) for f in ownership)
            request.write_bytes(payload)
            for target, program in programs.items():
                process = subprocess.run([*program, '--canonical', str(DIRECTORY / world.name), str(request)],
                                         text=True, capture_output=True, timeout=300)
                context = f'{target} {world.name} {world.side}'
                if process.returncode != 0:
                    raise AssertionError(f'{context}: exit {process.returncode}; '
                                         f'{process.stdout[-1200:]} {process.stderr[-1200:]}')
                check_output(process.stdout, expected, context)
                counts[target]['worlds'] += 1
                counts[target]['actors'] += len(world.ground_actors())
                counts[target]['children'] += len(inputs)
                counts[target]['prepared_resources'] += sum(len(world.artillery(side)) for side in (0, 1))
                counts[target].update(f['ownership'] for f in ownership)
                digests[target].update(f'{world.name}:{world.side}\n'.encode() + process.stdout.encode())
            records.setdefault(world.name, {})[str(world.side)] = {
                'actors': len(world.ground_actors()), 'children': len(inputs),
                'ownership': dict(collections.Counter(f['ownership'] for f in ownership)),
                'expected_output_sha256': hashlib.sha256(expected.encode()).hexdigest()}
            print(f'Canonical C children: {world.name} {world.side}; {len(inputs)} ordinary '
                  f'and {len(ownership)} ownership returns', flush=True)
        if len(records) != 47 or any(set(v) != set(map(str, DETAILS)) for v in records.values()):
            raise AssertionError('Missing mission or detail')
        required = {'worlds': 188, 'actors': 3840, 'children': 26880, 'prepared_resources': 944}
        required.update(ownership_required)
        if any(dict(value) != required for value in counts.values()):
            raise AssertionError(f'Incomplete target coverage: {counts}')
        if requester_classes != {0, 1, 2, 3} or any(ownership_required[k] == 0 for k in (
                'requester', 'release', 'reuse', 'orphan')):
            raise AssertionError('Incomplete canonical resource/requester lifetime coverage')
        if len(set(value.hexdigest() for value in digests.values())) != 1:
            raise AssertionError('Native/WASM complete canonical observations differ')
        # Empty, incomplete and excess frames must fail rather than produce a
        # plausible prefix. Keep the final valid mission/height as real input.
        invalid = (b'', bytes(8), payload[:-1], payload + b'\0',
                   struct.pack('<2I', 513, 1) + payload[8:])
        for target, program in programs.items():
            for malformed in invalid:
                request.write_bytes(malformed)
                process = subprocess.run([*program, '--canonical', str(DIRECTORY / world.name), str(request)],
                                         capture_output=True, timeout=30)
                if process.returncode == 0:
                    raise AssertionError(f'{target}: malformed canonical input admitted')
                counts[target]['malformed_rejections'] += 1
    return {'success': True, 'scope': 'Complete actual prepared-world C station/support children, '
            'prepared resource release/reuse/orphans and admitted queue requester lifetimes; '
            'memory/scene/complete regression and full-game gates remain separate',
            'targets': {target: {'counts': dict(value), 'output_sha256': digests[target].hexdigest()}
                        for target, value in counts.items()}, 'corpus': records,
            'requester_classes': sorted(requester_classes),
            'seconds': time.monotonic() - begin, 'groups': len(CASES) + 5, 'skips': 0}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=pathlib.Path('/tmp/wasm-fist-rewrite'))
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--result-json', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.result_json.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    args.result_json.unlink(missing_ok=True)
    result = verify(args)
    args.result_json.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'corpus'}, sort_keys=True), flush=True)
