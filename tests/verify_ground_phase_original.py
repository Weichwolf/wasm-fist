#!/usr/bin/env python3
"""Require full original parent comparisons for the named acceptance scope.

Each scope has fixed complete coverage and no filter/skip mode. Corpus and
domains are separate required gates; neither alone accepts WI0081 or the game.
Original files and the pinned Unicorn environment must be provisioned explicitly.
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

from ground_phase_probe_contract import encode, fixture, observation, world_observation
from mission_ready_contract import observed_world
from orders_contract import scenario_order_blocks
from original_audio_request_oracle import OriginalAudioRequestOracle
from original_ground_phase_oracle import OriginalGroundPhaseOracle
from remaining_ground_corpus import Corpus, DETAILS, DIRECTORY
from roster_promotion_contract import store, word


class Verification:
    def __init__(self, args):
        self.args = args
        self.commands = {}
        self.programs = []
        if args.target in ('native', 'all'):
            program = args.native_probe or args.build_root / 'native/fist_ground_phase_probe'
            self.commands['native'] = [str(program)]
            self.programs.append(program)
        if args.target in ('wasm', 'all'):
            program = args.build_root / 'wasm/fist_ground_phase_probe.js'
            self.commands['wasm'] = ['node', str(program)]
            self.programs.extend((program, program.with_suffix('.wasm')))
        self.pins = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in self.programs}
        self.corpus = Corpus()
        self.owner = OriginalGroundPhaseOracle(OriginalAudioRequestOracle())
        self.counts, self.banks = collections.Counter(), collections.Counter()
        self.digest = hashlib.sha256()
        self.missions, self.terrains = {}, set()

    def install(self, world):
        self.world = world
        self.owner.install_height(world.side, world.pixels)
        machine = self.owner.prepared_machine(world.data)
        state = observed_world(self.corpus.owner, machine, world.objects, True)
        self.prepared = 'prepared\n' + world_observation(state, scenario_order_blocks(world.scenario))

    def check(self, cases, group, request):
        wanted = self.prepared
        for index, f in enumerate(cases):
            output, effect = observation(self.corpus, self.owner, self.world, f, index)
            wanted += output
            self.counts['original_parent_returns'] += 1
            self.counts['original_child_returns'] += int(effect['entry'] is not None)
            for key in ('heading_samples', 'selected_diagnostics'):
                self.counts[key] += effect[{'heading_samples': 'heading_sampled',
                                           'selected_diagnostics': 'diagnostic'}[key]]
            for key in ('height_returns', 'visibility_returns', 'audio_returns'):
                self.counts[key] += effect[key]
            if effect['entry'] is not None:
                self.banks[f"{'automatic' if effect['automatic'] else 'controlled'}_{effect['callback']}"] += 1
        request.write_bytes(struct.pack('<2I', self.world.side, len(cases)) +
                            self.world.pixels + b''.join(map(encode, cases)))
        for target, command in self.commands.items():
            r = subprocess.run([*command, str(DIRECTORY / self.world.name), str(request)],
                               capture_output=True, timeout=180)
            if r.returncode or r.stderr or r.stdout != wanted.encode():
                actual, expected = r.stdout.decode(errors='replace').splitlines(), wanted.splitlines()
                first = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                             min(len(actual), len(expected)))
                failure = {'target': target, 'group': group, 'mission': self.world.name,
                           'detail': self.world.side, 'exit_code': r.returncode, 'line': first,
                           'actual': actual[first:first+2], 'expected': expected[first:first+2],
                           'stderr': r.stderr.decode(errors='replace'), 'counts': self.counts}
                (self.args.review_dir / f'{self.args.scope}-failure.json').write_text(
                    json.dumps(failure, indent=2) + '\n')
                raise AssertionError(failure)
            self.counts[f'{target}_parent_returns'] += len(cases)
        self.counts[group] += len(cases)
        self.counts['batches'] += 1
        self.digest.update(wanted.encode())

    def corpus_scope(self, request):
        for world in self.corpus.worlds():
            self.install(world)
            for slot in world.ground_actors():
                self.check([fixture(world, slot, callback, automatic)
                            for automatic in (0, 1) for callback in range(16)], 'canonical', request)
                self.counts['actors'] += 1
            self.missions.setdefault(world.name, set()).add(world.side)
            self.terrains.add(world.terrain)
            self.counts['worlds'] += 1
            print(world.name, world.side, dict(self.counts), flush=True)
        assert self.counts['worlds'] == 188 and self.counts['actors'] == 3840
        assert self.counts['canonical'] == 122880
        assert len(self.missions) == 47 and len(self.terrains) == 8
        assert all(sides == set(DETAILS) for sides in self.missions.values())
        assert set(self.banks) == {f'{bank}_{i}' for bank in ('automatic', 'controlled') for i in range(16)}
        assert all(count == 3840 for count in self.banks.values())

    def domains_scope(self, request):
        world = next(self.corpus.worlds())
        self.install(world)
        actors = {}
        for slot, near in world.ground_actors().items():
            actors.setdefault(word(world.data, near), slot)
        assert set(actors) == {0, 1, 2, 3}
        pending = []
        for slot in actors.values():
            for automatic in (0, 1):
                for counter in range(256):
                    f = fixture(world, slot, (counter + 1) % 16, automatic,
                                diagnostic=slot if counter & 1 else 65535, cursor=counter % 4)
                    raw = bytearray(f['raw'])
                    raw[0x42] = counter
                    f['raw'] = bytes(raw)
                    pending.append(f)
                    if len(pending) == 64:
                        self.check(pending, 'counter', request)
                        pending = []
        assert not pending and self.counts['counter'] == 2048
        for offset in (0x26, 0x28, 0x2a, 0x2c):
            for value in range(65536):
                f = fixture(world, actors[0], 0, value & 1, diagnostic=65535)
                raw = bytearray(f['raw'])
                for lane, fixed in ((0x26, 32768), (0x28, 65535), (0x2a, 32767), (0x2c, 1)):
                    store(raw, lane, fixed)
                store(raw, offset, value)
                store(raw, 0x2e, value ^ 0xa55a)
                raw[0x42] = 255 if value & 1 else 15
                f['raw'] = bytes(raw)
                pending.append(f)
                if len(pending) == 64:
                    self.check(pending, f'heading_{offset:02x}', request)
                    pending = []
                if value and value % 8192 == 0:
                    print(hex(offset), value, dict(self.counts), flush=True)
            assert not pending and self.counts[f'heading_{offset:02x}'] == 65536


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--scope', choices=('corpus', 'domains'), required=True)
    parser.add_argument('--target', choices=('all', 'native', 'wasm'), default='all')
    parser.add_argument('--build-root', type=pathlib.Path, default=pathlib.Path('/tmp/wasm-fist-rewrite'))
    parser.add_argument('--native-probe', type=pathlib.Path)
    parser.add_argument('--review-dir', type=pathlib.Path, required=True)
    args = parser.parse_args()
    args.review_dir = args.review_dir.resolve()
    if not args.review_dir.is_relative_to(pathlib.Path('/tmp')) or args.review_dir == pathlib.Path('/tmp'):
        parser.error('Use a dedicated /tmp evidence directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    receipt = args.review_dir / f'{args.scope}.json'
    receipt.unlink(missing_ok=True)
    begin = time.monotonic()
    verification = Verification(args)
    with tempfile.TemporaryDirectory(prefix='fist-parent-original-', dir='/tmp') as temporary:
        getattr(verification, f'{args.scope}_scope')(pathlib.Path(temporary) / 'request.bin')
    expected = 122880 if args.scope == 'corpus' else 264192
    assert verification.counts['original_parent_returns'] == expected
    assert all(verification.counts[f'{target}_parent_returns'] == expected
               for target in verification.commands)
    for path, sha in verification.pins.items():
        assert hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest() == sha
    result = {'success': True, 'scope': args.scope, 'counts': verification.counts,
              'banks': verification.banks, 'skips': 0, 'programs_sha256': verification.pins,
              'output_sha256': verification.digest.hexdigest(), 'seconds': time.monotonic() - begin,
              'complete_game_wasm_streak': 0}
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, sort_keys=True), flush=True)


if __name__ == '__main__':
    main()
