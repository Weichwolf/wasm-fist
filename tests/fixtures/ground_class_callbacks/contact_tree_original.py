"""Full memory/eleven-register tree contacts with genuine cached-source capture."""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from physical_contact_contract import predict
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from test_original_ground_maneuver import SEEDS
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
assert struct.unpack_from('<H', oracle.image, DGROUP + 0xe550 + 21 * 2)[0] == 0x9c76
assert struct.unpack_from('<H', oracle.image, DGROUP + 0xe550 + 23 * 2)[0] == 0xc335
assert oracle.image[DGROUP + 0x9b5d:DGROUP + 0x9b5f] == bytes((100, 0))
machine = oracle.fresh()
pointers = []
for index, kind in enumerate((0, 1, 2, 3, 21, 8, 23)):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, index)
    machine.reg_write(UC_X86_REG_CX, 1)
    oracle.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    pointers.append(machine.reg_read(UC_X86_REG_DI))
actors, tree, source, wreck = pointers[:4], *pointers[4:]
# Real bbb7->c31e->type23 RET->60f4, independently predicted before execution.
machine.mem_write(DGROUP + 0x9a25, struct.pack('<H', wreck))
machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
initial = (5, 0, 0x1111, 0x2222, 0x3333, 0x4444,
           source, 0x1c00, 0x1c00, 0x9000, 0)
for reg, value in zip(REGISTERS, initial):
    machine.reg_write(reg, value)
before = bytes(machine.mem_read(0, 0x60000))
expected = bytearray(before)
for offset, value in ((0xe3b2, source), (0x9bd9, 0), (0x9bdb, 5)):
    struct.pack_into('<H', expected, DGROUP + offset, value)
expected[DGROUP + 0x7b1e] = 0
expected[DGROUP + 0x87d5] = expected[DGROUP + 0x87bd] = 3
struct.pack_into('<5H', expected, DGROUP + 0x8ff6, 0xc330, 0xc334, 0xbbc4, 0, source)
abi = list(initial)
abi[4], abi[9] = 46, 0x9002
oracle.execute(machine, 0xbbb7, 0xeff0)
assert bytes(machine.mem_read(0, 0x60000)) == expected, 'Full genuine source producer memory'
assert [machine.reg_read(reg) for reg in REGISTERS] == abi, 'Full genuine source producer ABI'
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
digest = hashlib.sha256()


def check(kind, selected, side, factor, damage, speed=-3, throttle=-1, seed=1, cursor=0):
    machine.mem_write(0, base)
    actor = actors[kind]
    raw = bytearray((i * 37 + kind * 11) & 255 for i in range(251))
    raw[:4] = base[DGROUP + actor:DGROUP + actor + 4]
    struct.pack_into('<2i', raw, 4, 0, 0)
    struct.pack_into('<H', raw, 0x14, 256)
    struct.pack_into('<H', raw, 0x40, 65535)
    struct.pack_into('<2h', raw, 0x55, speed, throttle)
    raw[0x62], raw[0x93] = 253, 0
    machine.mem_write(DGROUP + actor, bytes(raw))
    obstacle = bytearray(base[DGROUP + tree:DGROUP + tree + 55])
    struct.pack_into('<2i', obstacle, 4, 0, 0)
    struct.pack_into('<H', obstacle, 0x14, 256)
    obstacle[0x16], obstacle[0x1a] = 64, damage
    machine.mem_write(DGROUP + tree, bytes(obstacle))
    machine.mem_write(DGROUP + source + 0x16, bytes([side]))
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP + 0x9fdf, bytes(2))
    machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H',
        37 if side & 8 else factor, factor if side & 8 else 37))
    machine.mem_write(DGROUP + 0x930a, struct.pack('<H', damage * 257))
    seeds = list(SEEDS)
    seeds[cursor] = seed
    machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
        0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
    entry = 0x1a0a4 if selected else 0xa631
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if selected else struct.pack('<H', 0xeff0))
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for reg, value in zip(REGISTERS, initial):
        machine.reg_write(reg, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial)
    oracle.execute(machine, entry, 0xeff0, 0xf69 if selected else 0)
    actual = bytes(machine.mem_read(0, 0x60000))
    case = (kind, selected, side, factor, damage, speed, throttle, seed, cursor)
    assert actual == expected, (case, 'memory', [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:24])
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    assert observed == abi, (case, 'ABI', observed, abi)
    assert effect['tree_calls'] == 1
    counts['returns'] += 1
    counts['released' if effect['tree_released'] else 'retained'] += 1
    counts['wrapped_retained'] += effect['tree_wrapped_retained']
    digest.update(repr(case).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


for kind, selected, side, factor, damage in itertools.product(range(4), (False, True),
        (0, 8), (0, 1, 3, 50, 128, 256, 65535), (0, 19, 20, 155, 255)):
    check(kind, selected, side, factor, damage)
counts['pilot_returns'] = counts['returns']
for damage, factor in itertools.product(range(256), range(1, 257)):
    side = (damage * 17 + factor * 2) & 255
    check((damage + factor) % 4, bool(factor & 1), side, factor, damage,
          seed=(damage * 257 + factor) & 65535, cursor=damage % 4)
counts['joint_authored_health_scale_returns'] = counts['returns'] - counts['pilot_returns']
for value in range(65536):
    speed = (value + 32768) % 65536 - 32768
    throttle = ((value ^ 32768) + 32768) % 65536 - 32768
    check(value % 4, bool(value & 256), value & 255, value, value >> 8,
          speed=speed, throttle=throttle, seed=value, cursor=value % 4)
counts['raw_factor_speed_throttle_rng_seed_returns'] = 65536
assert counts['returns'] == 131632
assert counts['joint_authored_health_scale_returns'] == 65536
assert counts['released'] > 0 and counts['retained'] > 0 and counts['wrapped_retained'] > 0

result = {'success': True, 'cases': counts['returns'], 'counts': dict(counts),
    'output_sha256': digest.hexdigest(), 'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scope': 'Complete original tree transition reference: genuine bbb7 source capture with independently predicted whole image and eleven-register ABI, actual four classes/selected/unselected/type21/source8/wreck23 allocation. Entire0x60000 memory, eleven returned low words, damage/RNG, physical pool/release/census and display effects independently predicted without scratch exclusions or instruction hooks. Sound owner explicitly unmatched. Every tree-health byte crossed with every authored source factor1..256, plus full raw factor/signed speed/signed throttle/RNG seed word domains and all source flag bytes with rotating classes/wrappers/cursors are covered. Whole contact family, saved/lifetime/corpus/admitted sound/shared C/class/game acceptance remains open.'}
(REVIEW / 'contact-tree-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
