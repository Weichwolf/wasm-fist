#!/usr/bin/env python3
"""Require actual DOS child observations for every prepared canonical stimulus.

This validates reference inputs and typed predictions before C consumption. It
does not accept the C implementation, a command parent, queued strikes or PCM.
"""
import argparse
import collections
import hashlib
import json
import pathlib
import struct
import time

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_remaining_ground_oracle import OriginalRemainingGroundOracle
from original_target_acquisition_oracle import MAILBOX
from original_unit_oracle import DGROUP
from remaining_ground_canonical_contract import CASES, cases, encoded_case, predict
from remaining_ground_corpus import Corpus, DETAILS, SEEDS


def fingerprint(data):
    return hashlib.sha256(data).hexdigest()


def comparable(data):
    # Existing observers independently bound the near-call stack. The fixture
    # begins before that call and does not predict its transient stack bytes.
    return bytes(data[:0x8fc0] + data[0x9004:])


def verify():
    from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                  UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                  UC_X86_REG_EBP)
    begin = time.monotonic()
    audio = OriginalAudioRequestOracle()
    owner = OriginalRemainingGroundOracle(audio)
    corpus = Corpus()
    corpus.owner = owner
    counts, kinds, branches = collections.Counter(), collections.Counter(), collections.Counter()
    predictions, observations, encoded = hashlib.sha256(), hashlib.sha256(), hashlib.sha256()
    records, resources = {}, collections.Counter()
    registers = (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                 UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
    contexts = ((0, 1, False), (32, 1, False), (128, 1, False),
                (0, 0, False), (0, 1, True))
    for world in corpus.worlds():
        owner.install_height(world.side, world.pixels)
        machine = owner.machine(SEEDS, 3, 0)
        world_counts = collections.Counter()
        for entries in (world.artillery(0), world.artillery(1)):
            resources.update(variant for _, variant, _ in entries)
        for f in cases(world):
            output, expected, effect = predict(world, f)
            packet = encoded_case(f)
            if len(packet) != 331 or struct.unpack_from('<2H', packet, 76) != (
                    f['actor_slot'], f['target_slot']) or packet[80:] != f['raw']:
                raise AssertionError('Incomplete canonical probe input')
            prefix = f'{world.name}:{world.side}:{f["actor_slot"]}:{f["name"]}\n'.encode()
            predictions.update(prefix + output.encode())
            encoded.update(prefix + packet)
            counts['stimuli'] += 1
            world_counts[f['name']] += 1
            kinds[f['kind']] += 1
            branches['typed_' + effect.get('support', 'station')] += 1
            for offset, enabled, missing in (contexts if f['source'] else contexts[:1]):
                audio.place('DSOUNDS.BIN', offset)
                audio_fixture = audio.fixture(enabled=enabled, missing_effects=missing)
                machine.mem_write(DGROUP, f['before'])
                machine.mem_write(MAILBOX, bytes(4096))
                for register in registers:
                    machine.reg_write(register, 0)
                actual, observed = owner.observe(machine, f['actor_pointer'], audio_fixture,
                                                 entry=0xae5c if f['operation'] == 0 else 0xb0be)
                # The source-unmatched original is the proved stable smoke
                # rule. Matched callers additionally expose the deliberate
                # bank/device repair, so their original differences are kept.
                if not f['source'] and comparable(actual) != comparable(expected):
                    differences = [hex(i) for i, (a, b) in enumerate(zip(actual, expected))
                                   if a != b and not 0x8fc0 <= i < 0x9004]
                    raise AssertionError(f'{prefix!r}: original/model differs at {differences[:24]}')
                if f['source'] and observed['allocation'] != effect['allocation']:
                    raise AssertionError('Audio context changed the smoke allocation/constructor boundary')
                if observed['allocation'] is not None:
                    pointer = observed['allocation'][0]
                    if actual[pointer:pointer + 55] != expected[pointer:pointer + 55]:
                        raise AssertionError('Canonical original marker constructor differs from typed prediction')
                counts['dos_returns'] += 1
                counts['kernel_audio_returns'] += int(observed['audio'])
                counts['height_returns'] += int(observed['allocation'] is not None)
                if not f['source']:
                    counts['whole_state_prediction_pairs'] += 1
                else:
                    counts['matched_source_contexts'] += 1
                    branches[f'original_{offset}_{enabled}_{int(missing)}_' + observed['support']] += 1
                observations.update(prefix + bytes((offset, enabled, missing)) + comparable(actual))
            audio.verify_assets()
        actor_count = len(world.ground_actors())
        if dict(world_counts) != {name: actor_count for name in CASES}:
            raise AssertionError('Missing canonical child stimulus')
        records.setdefault(world.name, {})[str(world.side)] = {
            'terrain': world.terrain, 'ground_actors': actor_count,
            'prepared_data_sha256': fingerprint(world.data), 'stimuli': dict(world_counts)}
        counts['worlds'] += 1
        counts['ground_actors'] += actor_count
        print(f'Canonical DOS children: {world.name} {world.side}; '
              f'{actor_count} actors, {sum(world_counts.values())} stimuli', flush=True)
    required = {'worlds': 188, 'ground_actors': 3840, 'stimuli': 26880, 'dos_returns': 42240,
                'whole_state_prediction_pairs': 23040, 'matched_source_contexts': 19200}
    if any(counts[key] != value for key, value in required.items()):
        raise AssertionError(f'Incomplete canonical original coverage: {dict(counts)}')
    if len(records) != 47 or any(set(v) != set(map(str, DETAILS)) for v in records.values()):
        raise AssertionError('Missing authored mission or detail')
    if set(kinds) != {0, 1, 2, 3} or set(resources) != {0, 1} or sum(resources.values()) != 944:
        raise AssertionError('Missing ground class or prepared artillery variant')
    if not any(k.endswith('air_confirmed') for k in branches if k.startswith('original_0_1_0_')):
        raise AssertionError('No actual bank-induced AIR observation')
    if not any(k.endswith('artillery_confirmed') for k in branches if k.startswith('original_32_1_0_')):
        raise AssertionError('No actual bank-induced ART observation')
    if not branches['original_128_1_0_not_called']:
        raise AssertionError('No actual bank-induced declined support observation')
    return {'success': True, 'scope': 'Complete canonical DOS child stimuli, original whole-state '
            'pairs and matched-source audio-dependence evidence; C consumers/lifetimes remain unaccepted',
            'counts': dict(counts), 'classes': dict(kinds), 'resource_variants': dict(resources),
            'branches': dict(branches), 'prediction_sha256': predictions.hexdigest(),
            'original_observation_sha256': observations.hexdigest(),
            'encoded_input_sha256': encoded.hexdigest(), 'seconds': time.monotonic() - begin,
            'corpus': records}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--result-json', type=pathlib.Path, required=True)
    args = parser.parse_args()
    if not args.result_json.resolve().is_relative_to(pathlib.Path('/tmp')):
        parser.error('Disposable evidence belongs under /tmp')
    args.result_json.unlink(missing_ok=True)
    result = verify()
    args.result_json.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'corpus'}, sort_keys=True), flush=True)
