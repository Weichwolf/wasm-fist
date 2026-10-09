"""Original physical contact and real predictive obstacle composition discovery.

Semantic memory is independently predicted; the exact maximum nested stack
write interval and three numerical scratch registers are explicitly excluded.
This is not the full-memory/full-eleven-register non-tree gate.
"""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from physical_contact_contract import predict
from ground_maneuver_contract import motion_obstacle
from test_geometry import signed
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS, UC_X86_REG_EFLAGS)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--review-dir', type=Path, required=True)
REVIEW = parser.parse_args().review_dir.resolve()
if not REVIEW.is_relative_to(Path('/tmp')):
    parser.error('Disposable review evidence must live under /tmp')
REVIEW.mkdir(parents=True, exist_ok=True)
MODEL = Path(__file__).resolve().parents[2] / 'physical_contact_contract.py'
REGISTERS = (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
             UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
             UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)
oracle = OriginalObjectPoolOracle()
machine = oracle.fresh()
pointers = []
for index, kind in enumerate((0, 1, 2, 3, 26)):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, index)
    machine.reg_write(UC_X86_REG_CX, 1)
    oracle.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    pointers.append(machine.reg_read(UC_X86_REG_DI))
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
branches = collections.Counter()
digest = hashlib.sha256()


def check(kind, selected, candidate_flags=64, repeated=False, pose=(0, 0),
          delta=(0, 0), velocity=(0, 0), speed=0, heading=0,
          maneuver=0, radii=(256, 256), coarse=0):
    machine.mem_write(0, base)
    actor, candidate = pointers[kind], pointers[4]
    raw = bytearray((i * 37 + kind * 11) & 255 for i in range(251))
    raw[:4] = base[DGROUP + actor:DGROUP + actor + 4]
    struct.pack_into('<2i', raw, 4, *pose)
    struct.pack_into('<H', raw, 0x14, radii[0])
    struct.pack_into('<H', raw, 0x10, heading)
    struct.pack_into('<H', raw, 0x26, heading)
    raw[0x45] = maneuver
    struct.pack_into('<H', raw, 0x40, 65535)
    struct.pack_into('<2h', raw, 0x55, speed, -300)
    struct.pack_into('<2h', raw, 0x59, *velocity)
    raw[0x62], raw[0x93] = 255 if repeated else 253, 0
    machine.mem_write(DGROUP + actor, bytes(raw))
    obstacle = bytearray(base[DGROUP + candidate:DGROUP + candidate + 55])
    struct.pack_into('<2i', obstacle, 4, *(signed(a + b) for a, b in zip(pose, delta)))
    struct.pack_into('<H', obstacle, 0x14, radii[1])
    obstacle[0x16] = candidate_flags
    machine.mem_write(DGROUP + candidate, bytes(obstacle))
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP + 0x9fdf, bytes(2))
    machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
    far = selected
    entry = 0x1a0a4 if far else 0xa631
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if far else struct.pack('<H', 0xeff0))
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for register, value in zip(REGISTERS, initial):
        machine.reg_write(register, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial, near_obstacle=motion_obstacle)
    oracle.execute(machine, entry, 0xeff0, 0xf69 if far else 0)
    actual = bytes(machine.mem_read(0, 0x60000))
    case = (kind, selected, candidate_flags, repeated, pose, delta, velocity, speed, heading, maneuver, radii, coarse)
    common_sp = 0x9000 if far else 0x8ffc
    if effect['near_obstacle_calls']:
        # Maximum depth from unchanged calls: physical pair4 + b059 far4
        # + b2a0 far4 + savedSI/DI4 + sampleCX2 + a19a far4 + 0927 near2
        # + baf near2 + five saved words10 + square-root iterationCX2 =38.
        lo, hi = DGROUP + common_sp - 38, DGROUP + common_sp
        assert actual[:lo] == expected[:lo] and actual[hi:] == expected[hi:], (case, 'semantic memory', [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b and not lo <= i < hi][:24])
    else:
        assert actual == expected, (case, 'complete memory', [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:24])
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    compared = (1, 2, 4, 6, 7, 8, 9, 10) if effect['near_obstacle_calls'] else tuple(range(11))
    assert [observed[i] for i in compared] == [abi[i] for i in compared], (case, 'declared ABI', observed, abi)
    counts['scratch_excluded_returns' if effect['near_obstacle_calls'] else 'full_returns'] += 1
    branches[effect['branch']] += 1
    for key in ('geometry_calls', 'sound_calls', 'registry_visits', 'high_word_skips', 'near_obstacle_calls', 'prediction_calls', 'prediction_samples', 'prediction_hits'):
        counts[key] += effect[key]
    counts['complete_returns'] += 1
    digest.update(repr(case).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


for kind, selected, delta, heading, mode, velocity, coarse in itertools.product(
        range(4), (False, True),
        ((0, -384), (0, -385), (0, -8064), (0, -8065),
         (0, 384), (0, 385), (0, 8065), (0, 8066),
         (-384, 0), (-385, 0), (384, 0), (385, 0)),
        (0, 16384, 32768, 49152), (0, 1, 255),
        ((-64, 64), (0, 0), (64, -64)), (0, 1, 255)):
    check(kind, selected, delta=delta, heading=heading, maneuver=mode,
          velocity=velocity, coarse=coarse)
for kind, selected, radii, delta in itertools.product(range(4), (False, True),
        ((0, 0), (32767, 32767), (32768, 32768), (65535, 65535), (65535, 1)),
        ((0, -128), (0, -129), (0, -7808), (0, -7809))):
    check(kind, selected, radii=radii, delta=delta, velocity=(0, -64))
assert counts['complete_returns'] == 10528
assert counts['near_obstacle_calls'] > 0
assert counts['prediction_calls'] > counts['prediction_hits'] > 0
assert counts['scratch_excluded_returns'] == counts['near_obstacle_calls']
result = {'success': True, 'cases': counts['complete_returns'], 'counts': dict(counts),
          'branches': dict(branches), 'output_sha256': digest.hexdigest(),
          'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
          'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
          'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
          'scope': 'Bounded original reference only: actual allocated four ground classes and type26 obstacle. Genuine selected/unselected physical wrappers and unmodified b059, b112, rotation, b2a0 and nested Euclidean samples execute. Independently predicted all0x60000 memory outside exact maximum38-byte stack write interval on predictive calls, and eight returned identity/loop/segment/stack registers; other branches compare all memory and eleven low-word registers. The interval is declared before execution and no observed bytes are copied into expected state. Radius wrapping, directional margin and gap edges, cardinal bearing, all nonzero maneuver admission and both numeric modes have representative coverage. Sound owner is unmatched. Complete contact-family domains, tree/admitted sound/source-lifetime/corpus and shared C/class/game acceptance remain open.'}
(REVIEW / 'contact-prediction-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
