"""Actual allocated physical contact subset with independent full memory/ABI."""
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
          delta=(0, 0), velocity=(0, 0), speed=0):
    machine.mem_write(0, base)
    actor, candidate = pointers[kind], pointers[4]
    raw = bytearray((i * 37 + kind * 11) & 255 for i in range(251))
    raw[:4] = base[DGROUP + actor:DGROUP + actor + 4]
    struct.pack_into('<2i', raw, 4, *pose)
    struct.pack_into('<H', raw, 0x14, 256)
    struct.pack_into('<H', raw, 0x40, 65535)
    struct.pack_into('<2h', raw, 0x55, speed, -300)
    struct.pack_into('<2h', raw, 0x59, *velocity)
    raw[0x62], raw[0x93] = 255 if repeated else 253, 0
    machine.mem_write(DGROUP + actor, bytes(raw))
    obstacle = bytearray(base[DGROUP + candidate:DGROUP + candidate + 55])
    struct.pack_into('<2i', obstacle, 4, *(signed(a + b) for a, b in zip(pose, delta)))
    struct.pack_into('<H', obstacle, 0x14, 256)
    obstacle[0x16] = candidate_flags
    machine.mem_write(DGROUP + candidate, bytes(obstacle))
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP + 0x9fdf, bytes(2))
    far = selected
    entry = 0x1a0a4 if far else 0xa631
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if far else struct.pack('<H', 0xeff0))
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for register, value in zip(REGISTERS, initial):
        machine.reg_write(register, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial)
    oracle.execute(machine, entry, 0xeff0, 0xf69 if far else 0)
    actual = bytes(machine.mem_read(0, 0x60000))
    case = (kind, selected, candidate_flags, repeated, pose, delta, velocity, speed)
    assert actual == expected, (case, 'memory', [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:24])
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    assert observed == abi, (case, 'ABI', observed, abi)
    branches[effect['branch']] += 1
    for key in ('geometry_calls', 'sound_calls', 'registry_visits', 'high_word_skips'):
        counts[key] += effect[key]
    counts['complete_returns'] += 1
    digest.update(repr(case).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


for kind, selected, flags, repeated in itertools.product(range(4), (False, True), range(256), (False, True)):
    check(kind, selected, flags, repeated)
positions = (((0, 0), (0, 0)), ((-2**31, 2**31-1), (0, 0)),
             ((2**31-1, -2**31), (0, 10000)), ((0, 0), (0, 65535)), ((0, 0), (0, 65537)), ((0, 0), (0, -65536)))
for kind, selected, (pose, delta), velocity, speed, repeated in itertools.product(
        range(4), (False, True), positions,
        ((-32768, 32767), (-1, 1), (0, 0), (32767, -32768)),
        (-32768, -1, 0, 32767), (False, True)):
    check(kind, selected, pose=pose, delta=delta, velocity=velocity, speed=speed, repeated=repeated)
assert counts['complete_returns'] == 5632
assert counts['high_word_skips'] == 512
assert dict(branches) == {'no_hit': 4096, 'first_non_tree_hit': 768, 'repeated_hit': 768}
result = {'success': True, 'cases': counts['complete_returns'], 'counts': dict(counts),
          'branches': dict(branches), 'output_sha256': digest.hexdigest(),
          'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
          'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
          'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
          'scope': 'Bounded original reference only: actual allocated four classes and type26 obstacle, selected/unselected complete wrappers, every obstacle flag byte, first/repeated/no-hit physical contact, directional proximity high-word skips and signed position/velocity/speed-wrap contexts. Whole0x60000 memory and eleven low-word register/segment/stack returns are independently predicted without scratch masks. Sound owner is explicitly unmatched; no substituted c047/PM method. Predictive continuation, tree damage, admitted sound, complete retained/domain/corpus/source lifetime and shared C/class/game acceptance remain open.'}
(REVIEW / 'contact-direct-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
