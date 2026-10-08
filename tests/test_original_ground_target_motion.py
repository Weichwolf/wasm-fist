#!/usr/bin/env python3
"""Require genuine target-aware class motion before canonical C integration."""
import argparse
import collections
import hashlib
import itertools
import json
import struct
from pathlib import Path

from ground_target_motion_contract import motion, turret
from original_ground_target_motion_oracle import OriginalGroundTargetMotionOracle
from original_unit_oracle import DGROUP
from roster_promotion_contract import store, word
from test_original_target_discovery import record
from test_vehicle_motion import start

ROOT = Path(__file__).resolve().parents[1]


def prepare(owner, kind, target_kind=26):
    machine, objects = owner.prepare_saved(
        [record(kind, 0, flags=4), record(target_kind, 1, x=-4096, y=7001, mode=2)],
        (1, 2, 32768, 65535), 3, 0, (bytes(2144), bytes(176)))
    actor = objects[150][2]
    target = next(value[2] for value in objects.values() if value[0] == 1)
    raw = bytearray(start(kind, hull=65000, request=2000, offset=17000,
                          turret_request=31000, speed=51, throttle=121, x=-1234, y=7890))
    store(raw, 0x97, target)
    store(raw, 0x9b, 9000)
    store(raw, 0x99, 65000)
    store(raw, 0x38, 17000)
    machine.mem_write(DGROUP + actor, bytes(raw))
    return machine, actor, target, bytes(raw)


def check(owner, machine, actor, raw, *, move=False, coarse=0):
    pointer = word(raw, 0x97)
    objects = {pointer: owner.raw(machine, pointer)} if pointer else {}
    predicted, events = (motion(owner, raw, objects, coarse, actor) if move else
                         turret(owner, raw, objects, coarse))
    machine.mem_write(DGROUP + actor, raw)
    actual, dirty = owner.step(machine, actor, move=move, coarse=coarse)
    if actual != predicted:
        changes = [hex(i) for i, (a, b) in enumerate(zip(actual, predicted)) if a != b]
        raise AssertionError((word(raw, 0), move, coarse, changes))
    changed = any(events[1:]) if move else events
    assert dirty == (3 if changed else 0), (dirty, events)
    return predicted


def domains(owner):
    counts = collections.Counter()
    digest = hashlib.sha256()
    for kind in range(4):
        machine, actor, target, initial = prepare(owner, kind)
        for value in range(65536):
            raw = bytearray(initial)
            # Every signed slew difference for M1/M3; every hull word for
            # T80/BMP, whose actual fresh geometry overwrites stale +9b.
            store(raw, 0x9b if kind < 2 else 0x26, value)
            raw = bytes(raw)
            output = check(owner, machine, actor, raw)
            digest.update(output)
            counts['complete_turret_returns'] += 1
        counts['classes'] += 1
        print('Complete target turret domain:', kind, dict(counts), flush=True)
    assert counts['complete_turret_returns'] == 262144 and counts['classes'] == 4
    return {'counts': dict(counts), 'output_sha256': digest.hexdigest()}


def types_and_presence(owner):
    counts = collections.Counter()
    digest = hashlib.sha256()
    for kind, target_kind, coarse in itertools.product(range(4), range(28), (0, 1)):
        machine, actor, target, initial = prepare(owner, kind, target_kind)
        for present, recenter in itertools.product((False, True), (False, True)):
            raw = bytearray(initial)
            store(raw, 0x97, target if present else 0)
            raw[0x1a] = 128 if recenter else 0
            output = check(owner, machine, actor, bytes(raw), coarse=coarse)
            digest.update(output)
            counts['complete_turret_returns'] += 1
            if coarse == 0:
                output = check(owner, machine, actor, bytes(raw), move=True)
                digest.update(output)
                counts['complete_ordered_motion_returns'] += 1
    assert counts['complete_turret_returns'] == 896
    assert counts['complete_ordered_motion_returns'] == 448
    return {'counts': dict(counts), 'output_sha256': digest.hexdigest()}


def retained(owner):
    counts = collections.Counter()
    digest = hashlib.sha256()
    witnesses = []
    for kind in range(4):
        machine, actor, target, raw = prepare(owner, kind)
        target_raw = bytearray(owner.raw(machine, target))
        for tick in range(256):
            # Keep the independently predicted actor as the next input and move
            # a real target. Alternate target presence without replacing +9b.
            struct.pack_into('<i', target_raw, 4, -4096 + tick * 937)
            struct.pack_into('<i', target_raw, 8, 7001 - tick * 541)
            machine.mem_write(DGROUP + target, bytes(target_raw))
            supplied = bytearray(raw)
            store(supplied, 0x97, 0 if tick % 17 == 0 else target)
            supplied[0x3d] = (supplied[0x3d] + 2) % 256
            raw = check(owner, machine, actor, bytes(supplied), move=True)
            digest.update(raw)
            counts['complete_ordered_motion_returns'] += 1
            if tick in (0, 1, 16, 17, 18, 255):
                witnesses.append({'class': kind, 'tick': tick, 'target_heading': word(raw, 0x9b),
                                  'request': word(raw, 0x8b), 'hull': word(raw, 0x26),
                                  'offset': word(raw, 0x89)})
        print('Retained target motion:', kind, dict(counts), flush=True)
    assert counts['complete_ordered_motion_returns'] == 1024
    return {'counts': dict(counts), 'output_sha256': digest.hexdigest(), 'witnesses': witnesses}


def branch_domains_and_lifetimes(owner):
    from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
    from original_object_pool_oracle import REGISTRY
    counts = collections.Counter()
    digest = hashlib.sha256()
    witnesses = []
    for kind in range(4):
        machine, actor, target, initial = prepare(owner, kind)
        for flags, phase, recenter in itertools.product(range(256), (0, 1, 4, 12, 16, 255), (0, 128)):
            raw = bytearray(initial)
            raw[0x19], raw[0x3d], raw[0x1a] = flags, phase, recenter
            output = check(owner, machine, actor, bytes(raw), move=True)
            digest.update(output)
            counts['motion_flag_phase_returns'] += 1
        for profile in range(256):
            raw = bytearray(initial)
            raw[0x90] = profile
            output = check(owner, machine, actor, bytes(raw), move=True)
            digest.update(output)
            counts['profile_returns'] += 1
        target_raw = bytearray(owner.raw(machine, target))
        for variant, coarse in itertools.product(range(256), (0, 1)):
            target_raw[25] = variant
            machine.mem_write(DGROUP + target, bytes(target_raw))
            output = check(owner, machine, actor, initial, coarse=coarse)
            digest.update(output)
            counts['variant_returns'] += 1
        machine, actor, target, initial = prepare(owner, kind, 0)
        saved_binding = bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4))
        machine.reg_write(UC_X86_REG_AX, 1)
        owner.far_call(machine, 0x1b2ef)
        assert machine.mem_read(DGROUP + 0xe38d + owner.slot(target) - 150, 1) == b'\0'
        output = check(owner, machine, actor, initial, move=True)
        digest.update(output)
        counts['released_target_returns'] += 1
        machine.reg_write(UC_X86_REG_AX, 2)
        owner.far_call(machine, 0x1b1df)
        assert machine.reg_read(UC_X86_REG_EFLAGS) & 1 == 0
        assert machine.reg_read(UC_X86_REG_DI) == target
        assert bytes(machine.mem_read(DGROUP + REGISTRY + 4, 4)) == saved_binding
        machine.mem_write(DGROUP + target + 4, struct.pack('<3i', 77001, -90001, 19000))
        output = check(owner, machine, actor, initial, move=True)
        digest.update(output)
        counts['reused_target_returns'] += 1
        witnesses.append({'class': kind, 'same_near_pointer': True, 'same_saved_binding': True,
                          'original_ignores_release': True, 'freshly_aims_successor': kind >= 2,
                          'rewrite_requires_captured_live_reference': True})
    assert counts == {'motion_flag_phase_returns': 12288, 'profile_returns': 1024,
                      'variant_returns': 2048, 'released_target_returns': 4,
                      'reused_target_returns': 4}
    return {'counts': dict(counts), 'output_sha256': digest.hexdigest(), 'witnesses': witnesses}


def both_angle_modes(owner):
    counts = collections.Counter()
    digest = hashlib.sha256()
    angles = (0, 1, 31, 32, 33, 63, 64, 8192, 16351, 16352, 16383, 16384,
              16385, 32767, 32768, 49151, 49152, 49153, 65535)
    for kind in range(4):
        machine, actor, target, initial = prepare(owner, kind)
        for coarse, self_target, present, heading, speed, direction in itertools.product(
                (0, 1, 255), (False, True), (False, True), angles,
                (-32768, -32767, -1, 0, 1, 321, 32767), (0, 2, 4, 6, 16)):
            raw = bytearray(initial)
            store(raw, 0x97, (actor if self_target else target) if present else 0)
            store(raw, 0x26, heading)
            store(raw, 0x30, heading)
            store(raw, 0x55, speed)
            raw[0x19] = direction
            raw[0x3d] = 1  # Retain this supplied speed at the motion boundary.
            struct.pack_into('<i', raw, 4, 2147483647)
            struct.pack_into('<i', raw, 8, -2147483648)
            output = check(owner, machine, actor, bytes(raw), move=True, coarse=coarse)
            digest.update(output)
            counts['complete_ordered_motion_returns'] += 1
            counts['coarse_%d' % coarse] += 1
            counts['self_target_returns'] += bool(self_target and present)
        print('Complete both-mode/self/wrap motion:', kind, dict(counts), flush=True)
    assert counts['complete_ordered_motion_returns'] == 31920
    assert counts['self_target_returns'] == 7980
    assert all(counts['coarse_%d' % mode] == 10640 for mode in (0, 1, 255))
    return {'counts': dict(counts), 'output_sha256': digest.hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', action='store_true', required=True)
    parser.add_argument('--review-dir', type=Path, required=True)
    args = parser.parse_args()
    review = args.review_dir.resolve()
    if not review.is_relative_to(Path('/tmp')) or review == Path('/tmp'):
        parser.error('Use a dedicated /tmp evidence directory')
    review.mkdir(parents=True, exist_ok=True)
    receipt = review / 'target-motion.json'
    receipt.unlink(missing_ok=True)
    names = ('ground_target_motion_contract.py', 'original_ground_target_motion_oracle.py',
             'test_original_ground_target_motion.py')
    pins = {name: hashlib.sha256((ROOT / 'tests' / name).read_bytes()).hexdigest() for name in names}
    owner = OriginalGroundTargetMotionOracle()
    groups = {}
    for name, verify in (('types_and_presence', types_and_presence), ('domains', domains),
                         ('retained', retained), ('branch_domains_and_lifetimes', branch_domains_and_lifetimes),
                         ('both_angle_modes', both_angle_modes)):
        print('Required target-motion group:', name, flush=True)
        groups[name] = verify(owner)
    assert len(groups) == 5
    for name, expected in pins.items():
        assert hashlib.sha256((ROOT / 'tests' / name).read_bytes()).hexdigest() == expected
    result = {'success': True, 'groups': groups, 'skips': 0, 'source_sha256': pins,
              'scope': 'Complete class turret and ordered motion in both angle modes; C/full class remains open',
              'complete_game_wasm_streak': 0}
    receipt.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({key: value for key, value in result.items() if key != 'groups'}, sort_keys=True))


if __name__ == '__main__':
    main()
