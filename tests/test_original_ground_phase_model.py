#!/usr/bin/env python3
"""Require independent synthetic parent models and actual reached TRAIN1 prefixes.

These three original-only groups prove the models used by shared-C tests, not
acceptance of a C parent or complete living class. All recorded calls execute
unchanged instructions; missing inputs, incomplete returns and skips fail.
"""
import argparse
import collections
import copy
import hashlib
import itertools
import json
import struct
from pathlib import Path
from types import SimpleNamespace

from ground_command_phase_contract import prefix
from ground_phase_model import child, model_owner, random_state, synthetic_world
from ground_phase_model import observation as model_observation
from ground_phase_probe_contract import (complete_world, fixture, original_input,
                                        result_observation, world_observation)
from orders_contract import scenario_order_blocks
from original_audio_request_oracle import OriginalAudioRequestOracle
from original_controlled_m1_oracle import OriginalControlledM1Oracle
from original_ground_phase_oracle import OriginalGroundPhaseOracle
from original_mission_orders_oracle import OriginalMissionOrdersOracle
from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_unit_oracle import DGROUP
from remaining_ground_corpus import PreparedWorld
from roster_promotion_contract import store, word
from test_driving import history_update, maintenance_update, weapon_update, update
from test_ground import installed
from test_units import records_from_scenario

ROOT = Path(__file__).resolve().parents[1]


def safe_fixture(world, slot, callback, automatic):
    """Declared valid mode/maneuver input for the synthetic full-bank cases."""
    f = fixture(world, slot, callback, automatic)
    raw = bytearray(f['raw'])
    raw[0x43] = raw[0x45] = 0
    f['raw'] = bytes(raw)
    return f


def complete_banks():
    """Check every class and both complete banks against genuine parent returns."""
    w, state = synthetic_world()
    owner = OriginalGroundPhaseOracle(OriginalAudioRequestOracle())
    owner.install_height(w.side, w.pixels)
    corpus = SimpleNamespace(owner=OriginalMissionReadyOracle())
    from ground_phase_probe_contract import observation as original_observation
    digest = hashlib.sha256()
    counts = collections.Counter()
    for slot, automatic, callback in itertools.product(w.ground_actors(), (0, 1), range(16)):
        f = safe_fixture(w, slot, callback, automatic)
        predicted = model_observation(w, f)[0]
        actual, effect = original_observation(corpus, owner, w, f)
        assert actual == predicted, (slot, automatic, callback)
        counts['original_parent_returns'] += 1
        counts['original_child_returns'] += 1
        digest.update(predicted.encode())
    assert counts['original_parent_returns'] == 128
    return {'success': True, 'counts': dict(counts), 'output_sha256': digest.hexdigest()}


def retained_banks():
    """Keep the independent model as next input; retain actual relocated service vectors."""
    w, state = synthetic_world()
    owner = OriginalGroundPhaseOracle(OriginalAudioRequestOracle())
    owner.install_height(w.side, w.pixels)
    pool_owner = OriginalMissionReadyOracle()
    counts = collections.Counter()
    digest = hashlib.sha256()
    for slot, automatic in itertools.product(w.ground_actors(), (0, 1)):
        initial = safe_fixture(w, slot, 0, automatic)
        before = original_input(w, initial)
        saved = {other: initial['raw'] if other == slot else w.data[address:address + 251] for other, address in w.ground_actors().items()}
        machine = owner.prepared_machine(before)
        before = bytes(machine.mem_read(DGROUP, 65536))
        for index in range(512):
            f = dict(initial, retain=int(index != 0), tick=index * 31, inhibition=int(index % 37 == 0))
            before = bytearray(before)
            store(before, 0x452, f['tick'])
            before[0x978a] = f['inhibition']
            f.update(voice_prior=word(before, 0x9fca), notice_ticks=word(before, 0x969e), message_ticks=word(before, 0x7a50), advisory_until=word(before, 0x9fd7), advisory_code=before[0x9fd6])
            predicted, model_after, effect = model_observation(w, f, index, before=bytes(before), saved=saved)
            machine.mem_write(DGROUP, bytes(before))
            actual, e = owner.observe(machine, w.ground_actors()[slot])
            fields = copy.deepcopy(f)
            output = f'case {index} 0\n' + result_observation(owner, bytes(before), actual, w.ground_actors()[slot], e, fields)
            state = complete_world(pool_owner, machine, w)
            runtime = {other: bytes(raw) for other, (_, raw) in state[1].items()}
            for other, (_, raw) in state[1].items():
                if word(raw, 0) < 4:
                    store(raw, 0x97, word(saved[other], 0x97))
                    store(raw, 0x9d, word(saved[other], 0x9d))
            orders = tuple((b''.join((actual[word(actual, table + p * 2):word(actual, table + p * 2) + size] for p in range(8))) for table, size in ((0x7d2a, 268), (0x85a0, 22))))
            output += world_observation(state, orders, data=actual, f=fields, original_raw=runtime)
            if output != predicted:
                a, b = (output.splitlines(), predicted.splitlines())
                i = next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), min(len(a), len(b)))
                raise AssertionError((slot, automatic, index, i, a[i:i + 2], b[i:i + 2]))
            digest.update(predicted.encode())
            counts['original_parent_returns'] += 1
            counts['original_child_returns'] += int(e['entry'] is not None)
            counts['heading_samples'] += e['heading_sampled']
            counts['audio_returns'] += e['audio_returns']
            before = model_after
        print('Retained independent model:', slot, automatic, dict(counts), flush=True)
    assert counts['original_parent_returns'] == 4096
    return {'success': True, 'counts': dict(counts), 'output_sha256': digest.hexdigest()}


def reaching_prefixes():
    """Compose the actual command before terrain contact in the original M1 pre-engine prefix."""
    path = ROOT / 'armoredfist/FISTDATA/TRAIN1.FSG'
    scenario = path.read_bytes()
    manifest = json.loads((ROOT / 'tests/scenario_originals.json').read_text())
    assert hashlib.sha256(scenario).hexdigest() == manifest['TRAIN1.FSG']['sha256']
    records = records_from_scenario(scenario)
    orders = scenario_order_blocks(scenario)
    counts = collections.Counter()
    observations = []
    digest = hashlib.sha256()
    for side in (512, 1024, 2048, 4096):
        pixels = bytes([32]) * side ** 2
        for constructed in (False, True):
            patches = ((0x49, bytes(8)), (0x40, bytes(2))) if constructed else ()
            with OriginalControlledM1Oracle(records, side, pixels, patches=patches) as original:
                loader = OriginalMissionOrdersOracle()
                loader.load(original.machine, *orders)
                data = bytes(original.machine.mem_read(DGROUP, 65536))
                world = PreparedWorld('TRAIN1.FSG', 'constant', side, pixels, scenario, data, {}, '', '')
                owner = model_owner(world)
                actor = original.pointer
                identity = original.state[:2]
                assert len(original.objects) == 85
                for tick in range(1, 55):
                    predicted = bytearray(data)
                    raw = bytearray(data[actor:actor + 251])
                    raw[0x0d] = raw[0x1d]
                    raw, _ = weapon_update(raw, 3, 0)
                    raw, _ = update(raw)
                    raw = bytearray(raw)
                    raw[0x3d] = (raw[0x3d] + 2) % 256
                    raw, _ = weapon_update(raw, 4, 0)
                    raw = maintenance_update(history_update(raw))
                    predicted[actor:actor + 251] = raw
                    if raw[0x3d] & 0x1e in (14, 30):
                        predicted, effect = prefix(predicted, actor)
                        predicted = child(predicted, actor, effect, owner)
                        counts['composed_parent_returns'] += 1
                    wanted = installed(side, pixels, [(*identity, bytes(predicted[actor:actor + 251]))])[0]
                    actual = original.step()
                    if actual != wanted:
                        differences = [hex(i) for i, (a, b) in enumerate(zip(actual[2], wanted[2])) if a != b]
                        raise AssertionError((side, constructed, tick, differences))
                    assert original.random_state() == random_state(predicted)
                    assert original.unchanged_others()
                    counts['complete_reached_prefixes'] += 1
                    digest.update(actual[2])
                    digest.update(struct.pack('<4HB', *original.random_state()[0], original.random_state()[1]))
                    predicted[actor:actor + 251] = wanted[2]
                    data = bytes(predicted)
                    if tick in (7, 15, 23, 31, 39, 47, 54):
                        observations.append({'side': side, 'constructed': constructed, 'tick': tick, 'raw': actual[2].hex(), 'rng': original.random_state()})
    assert counts['complete_reached_prefixes'] == 432 and counts['composed_parent_returns'] == 48
    return {'success': True, 'counts': dict(counts), 'mission_sha256': manifest['TRAIN1.FSG']['sha256'], 'observations': observations, 'output_sha256': digest.hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=Path, required=True)
    args = parser.parse_args()
    review = args.review_dir.resolve()
    if not review.is_relative_to(Path('/tmp')) or review == Path('/tmp'):
        parser.error('Use a dedicated /tmp evidence directory')
    review.mkdir(parents=True, exist_ok=True)
    receipt = review / 'ground-parent-model.json'
    receipt.unlink(missing_ok=True)
    files = ('ground_phase_model.py', 'ground_phase_probe_contract.py', 'test_original_ground_phase_model.py')
    pins = {name: hashlib.sha256((ROOT / 'tests' / name).read_bytes()).hexdigest() for name in files}
    groups = {}
    for name, verify in (('complete_banks', complete_banks), ('retained_banks', retained_banks), ('reaching_prefixes', reaching_prefixes)):
        print('Required independent parent-model group:', name, flush=True)
        groups[name] = verify()
    assert len(groups) == 3 and all((group['success'] for group in groups.values()))
    for name, sha in pins.items():
        assert hashlib.sha256((ROOT / 'tests' / name).read_bytes()).hexdigest() == sha
    result = {'success': True, 'groups': groups, 'skips': 0, 'source_sha256': pins, 'complete_game_wasm_streak': 0, 'scope': 'Synthetic whole-world/event parent models and reached original TRAIN1 M1 pre-engine prefixes; C/full class acceptance remains open'}
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({key: value for key, value in result.items() if key != 'groups'},
                     sort_keys=True), flush=True)
if __name__ == '__main__':
    main()
