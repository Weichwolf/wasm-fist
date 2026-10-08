#!/usr/bin/env python3
"""Require original and shared target-motion returns in complete named scopes.

Corpus consumes actual C mission restoration/preparation and retains each world
across updates. Target captures are explicit caller fixtures; phase scheduling,
acquisition, contacts and the complete class callback are separate contracts.
Retained exercises moving synthetic actors against all 28 target types. Neither
scope substitutes for allocator lifetime, atomic failure or sanitizer acceptance.
"""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import time

from ground_target_motion_contract import motion
from original_ground_target_motion_oracle import OriginalGroundTargetMotionOracle
from original_unit_oracle import DGROUP
from remaining_ground_corpus import Corpus, DETAILS, DIRECTORY
from roster_promotion_contract import store, word
from test_ground_target_motion import encode
from test_original_ground_target_motion import prepare
from test_target_acquisition import fixture
from test_target_discovery import NONE, physical, pointer
from test_vehicle_start import state_lines

ROOT = Path(__file__).resolve().parents[1]


def framed(case, retained):
    data = bytearray(encode(case))
    # Header byte25 is the operation; the following word is retained mode.
    struct.pack_into('<H', data, 26, int(retained))
    return bytes(data)


def observation(raw, events, slot, identity):
    normalized = bytearray(raw)
    store(normalized, 0x97, 0)
    store(normalized, 0x9d, 0)
    return (state_lines([(*identity, bytes(normalized))]) + 'ground_motion ' +
            ' '.join(map(str, (slot, physical(word(raw, 0x97)), word(raw, 0x9b),
                              word(raw, 0x99), *events))) + '\n')


class Verification:
    def __init__(self, args):
        self.args = args
        self.commands, programs = {}, []
        if args.target in ('native', 'all'):
            program = args.native_probe or args.build_root / 'native/fist_target_discovery_probe'
            self.commands['native'] = [str(program)]
            programs.append(program)
        if args.target in ('wasm', 'all'):
            program = args.build_root / 'wasm/fist_target_discovery_probe.js'
            self.commands['wasm'] = ['node', str(program)]
            programs.extend((program, program.with_suffix('.wasm')))
        self.pins = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in programs}
        self.sources = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                        for p in sorted((ROOT / 'tests').glob('*.py'))}
        self.owner = OriginalGroundTargetMotionOracle()
        self.counts, self.classes, self.targets = (collections.Counter() for _ in range(3))
        self.digest = hashlib.sha256()

    def advance(self, machine, case, identity, retained):
        slot = case['actor']
        raw = case['raw'][slot]
        objects = {pointer(other): state for other, state in case['raw'].items()}
        expected, events = motion(self.owner, raw, objects, case['coarse'], pointer(slot))
        # Input checks include the entire retained raw actor, not just its pose.
        assert self.owner.raw(machine, pointer(slot)) == raw
        actual, dirty = self.owner.step(machine, pointer(slot), move=True, coarse=case['coarse'])
        if actual != expected or dirty != (3 if any(events[1:]) else 0):
            changed = [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
            raise AssertionError(('original', slot, case['coarse'], changed, dirty, events))
        wire = framed(case, retained)
        output = observation(expected, events, slot, identity)
        case['raw'][slot] = expected
        self.counts['original_motion_returns'] += 1
        self.counts['retained_returns'] += retained
        self.counts['translated_returns'] += expected[4:12] != raw[4:12]
        self.classes[word(raw, 0)] += 1
        target = word(raw, 0x97)
        if target:
            self.targets[word(objects[target], 0)] += 1
        return wire, output

    def check(self, side, pixels, cases, wanted, path, mission=None):
        path.write_bytes(struct.pack('<II', side, len(cases)) + pixels + b''.join(cases))
        for target, command in self.commands.items():
            args = [*command, '--ground-motion']
            if mission is not None:
                args.append(str(DIRECTORY / mission))
            result = subprocess.run([*args, str(path)], capture_output=True, timeout=180)
            if result.returncode or result.stderr or result.stdout != wanted.encode():
                actual, expected = result.stdout.decode(errors='replace').splitlines(), wanted.splitlines()
                first = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                             min(len(actual), len(expected)))
                failure = {'target': target, 'mission': mission, 'detail': side,
                           'exit_code': result.returncode, 'line': first,
                           'actual': actual[first:first+2], 'expected': expected[first:first+2],
                           'stderr': result.stderr.decode(errors='replace'), 'counts': self.counts}
                (self.args.review_dir / (self.args.scope + '-failure.json')).write_text(
                    json.dumps(failure, indent=2) + '\n')
                raise AssertionError(failure)
            self.counts[target + '_motion_returns'] += len(cases)
        self.digest.update(wanted.encode())
        self.counts['batches'] += 1

    def corpus_scope(self, path):
        missions, terrains = {}, set()
        for world in Corpus().worlds():
            machine = self.owner.machine()
            machine.mem_write(DGROUP, world.data)
            registry = [(physical(near), value) for near, value in
                        struct.iter_unpack('<HH', world.data[0xdfbc:0xdfbc + 728])]
            seeds, cursor = self.owner.random_state(machine)
            physicals = world.physical()
            raw = {slot: self.owner.raw(machine, near) for slot, near in physicals.items()}
            cases, outputs = [], []
            for slot in world.ground_actors():
                index, generation = world.objects[slot][:2]
                others = sorted(other for other in raw if other != slot)
                assert others, world.name
                # Authored speed/throttle/phase remain intact. Each declared
                # capture begins an episode; its next three updates retain C state.
                for coarse in (0, 1):
                    for target in (NONE, slot, others[slot % len(others)]):
                        supplied = bytearray(raw[slot])
                        store(supplied, 0x97, 0 if target == NONE else pointer(target))
                        raw[slot] = bytes(supplied)
                        machine.mem_write(DGROUP + pointer(slot), raw[slot])
                        case = {'actor': slot, 'raw': raw, 'registry': registry,
                                'selected': 0, 'clock': 0, 'gate': 0, 'last': 0,
                                'link': 0, 'coarse': coarse, 'cursor': cursor, 'seeds': seeds}
                        for tick in range(4):
                            wire, output = self.advance(machine, case, (index, generation), tick != 0)
                            cases.append(wire)
                            outputs.append(output)
                self.counts['actors'] += 1
            self.check(world.side, world.pixels, cases, ''.join(outputs), path, world.name)
            missions.setdefault(world.name, set()).add(world.side)
            terrains.add(world.terrain)
            self.counts['worlds'] += 1
            print(world.name, world.side, dict(self.counts), flush=True)
        assert len(missions) == 47 and len(terrains) == 8
        assert all(sides == set(DETAILS) for sides in missions.values())
        assert self.counts['worlds'] == 188 and self.counts['actors'] == 3840
        assert self.counts['original_motion_returns'] == 92160
        assert self.counts['retained_returns'] == 69120
        assert set(self.classes) == set(range(4))

    def retained_scope(self, path):
        for kind in range(4):
            for target_kind in range(28):
                for coarse in (0, 1, 255):
                    machine, actor, target, initial = prepare(self.owner, kind, target_kind)
                    case = fixture(kind, target_kind, old=-1, operation=70)
                    assert pointer(case['actor']) == actor and pointer(case['candidate']) == target
                    case['raw'][case['actor']] = initial
                    case['raw'][case['candidate']] = self.owner.raw(machine, target)
                    case['coarse'] = coarse
                    cases, outputs = [], []
                    for tick in range(32):
                        wire, output = self.advance(machine, case, (0, 1), tick != 0)
                        cases.append(wire)
                        outputs.append(output)
                    self.check(1, b'\0', cases, ''.join(outputs), path)
            print('Retained class', kind, dict(self.counts), flush=True)
        assert self.counts['original_motion_returns'] == 10752
        assert self.counts['retained_returns'] == 10416
        assert self.counts['translated_returns'] > 0
        assert set(self.classes) == set(range(4)) and set(self.targets) == set(range(28))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--scope', choices=('corpus', 'retained'), required=True)
    parser.add_argument('--target', choices=('native', 'wasm', 'all'), default='all')
    parser.add_argument('--build-root', type=Path, default=Path('/tmp/wasm-fist-rewrite'))
    parser.add_argument('--native-probe', type=Path)
    parser.add_argument('--review-dir', type=Path, required=True)
    args = parser.parse_args()
    args.review_dir = args.review_dir.resolve()
    if args.review_dir == Path('/tmp') or not args.review_dir.is_relative_to(Path('/tmp')):
        parser.error('Use a dedicated /tmp evidence directory')
    args.review_dir.mkdir(parents=True, exist_ok=True)
    receipt = args.review_dir / (args.scope + '.json')
    receipt.unlink(missing_ok=True)
    begin = time.monotonic()
    verification = Verification(args)
    with tempfile.TemporaryDirectory(prefix='fist-target-motion-original-', dir='/tmp') as temporary:
        getattr(verification, args.scope + '_scope')(Path(temporary) / 'request.bin')
    expected = 92160 if args.scope == 'corpus' else 10752
    assert all(verification.counts[target + '_motion_returns'] == expected
               for target in verification.commands)
    for name, expected in {**verification.pins,
                           **{str(ROOT / p): v for p, v in verification.sources.items()}}.items():
        assert hashlib.sha256(Path(name).read_bytes()).hexdigest() == expected, name
    result = {'success': True, 'scope': args.scope, 'counts': verification.counts,
              'classes': verification.classes, 'target_types': verification.targets,
              'skips': 0, 'programs_sha256': verification.pins,
              'source_sha256': verification.sources, 'output_sha256': verification.digest.hexdigest(),
              'seconds': time.monotonic() - begin, 'complete_game_wasm_streak': 0}
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'source_sha256'}, sort_keys=True), flush=True)


if __name__ == '__main__':
    main()
