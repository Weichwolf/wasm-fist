"""Required contact admission, word widths and genuine registry binding domains."""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct

from ground_maneuver_contract import motion_obstacle
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from physical_contact_contract import predict
from test_geometry import signed
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
owner = OriginalGroundManeuverOracle()
machine = owner.machine(SEEDS, 0)
owner.far_call(machine, 0x1b176)
actors = []
for kind in range(4):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, kind)
    machine.reg_write(UC_X86_REG_CX, 1)
    owner.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    actors.append(machine.reg_read(UC_X86_REG_DI))
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
branches = collections.Counter()
digest = hashlib.sha256()


def observe(actor, selected, case, *, ignored=False):
    entry = 0x1a0a4 if selected else 0xa631
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected != ignored else 0))
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if selected else struct.pack('<H', 0xeff0))
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for reg, value in zip(REGISTERS, initial):
        machine.reg_write(reg, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial, near_obstacle=motion_obstacle)
    owner.execute(machine, entry, 0xeff0, 0xf69 if selected else 0)
    actual = bytes(machine.mem_read(0, 0x60000))
    if effect['near_obstacle_calls']:
        common_sp = 0x9000 if selected else 0x8ffc
        lo, hi = DGROUP + common_sp - 38, DGROUP + common_sp
        assert actual[:lo] == expected[:lo] and actual[hi:] == expected[hi:], (case, 'semantic memory')
        compared = (1, 2, 4, 6, 7, 8, 9, 10)
    else:
        assert actual == expected, (case, 'full memory')
        compared = tuple(range(11))
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    assert [observed[i] for i in compared] == [abi[i] for i in compared], (case, 'ABI', observed, abi)
    counts['complete_returns'] += 1
    counts['scratch_excluded_returns' if effect['near_obstacle_calls'] else 'full_memory_returns'] += 1
    counts['near_obstacle_calls'] += effect['near_obstacle_calls']
    branches[effect['branch']] += 1
    digest.update(repr(case).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


# Full control-word domain, crossed saved contact/cooldown bytes, rotating both
# genuine admitted wrappers and all four allocated classes. No obstacle exists.
for value in range(65536):
    machine.mem_write(0, base)
    actor = actors[value % 4]
    machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', value))
    machine.mem_write(DGROUP + actor + 0x62, bytes([value & 255]))
    machine.mem_write(DGROUP + actor + 0x93, bytes([value >> 8]))
    observe(actor, bool(value & 256), ('admission', value))
counts['control_word_and_contact_cooldown_pair_domains'] = 65536
# Both selection rejections preserve controls/cooldown, actor and every register.
for kind, actor in enumerate(actors):
    for selected in (False, True):
        for cooldown in (0, 255):
            machine.mem_write(0, base)
            machine.mem_write(DGROUP + actor + 0x40, b'\xff\xff')
            machine.mem_write(DGROUP + actor + 0x62, b'\xff')
            machine.mem_write(DGROUP + actor + 0x93, bytes([cooldown]))
            observe(actor, selected, ('ignored', kind, selected, cooldown), ignored=True)
counts['ignored_wrapper_returns'] = 16
# Genuine requested-index constructor binds every generation word and visits
# all182 registry cells. Overwritten ground bindings remain actual live orphans.
for value in range(65536):
    machine.mem_write(0, base)
    index = value % 182
    machine.reg_write(UC_X86_REG_AX, 26)
    machine.reg_write(UC_X86_REG_BX, index)
    machine.reg_write(UC_X86_REG_CX, value)
    owner.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    obstacle = machine.reg_read(UC_X86_REG_DI)
    assert machine.mem_read(DGROUP + 0xdfbc + index * 4, 4) == struct.pack('<HH', obstacle, value)
    assert bytes(machine.mem_read(DGROUP + obstacle, 55)) == struct.pack('<HH', 26, 0) + bytes(51)
    counts['genuine_registry_constructor_returns'] += 1
    actor = actors[value % 4]
    pose = (2147483647, -2147483648) if value & 256 else (-2147483648, 2147483647)
    velocity = ((value + 32768) % 65536 - 32768,
                ((value ^ 32768) + 32768) % 65536 - 32768)
    machine.mem_write(DGROUP + actor + 4, struct.pack('<2i', *pose))
    machine.mem_write(DGROUP + actor + 0x14, struct.pack('<H', 256))
    machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', value))
    machine.mem_write(DGROUP + actor + 0x55, struct.pack('<2h', velocity[0], velocity[1]))
    machine.mem_write(DGROUP + actor + 0x59, struct.pack('<2h', *velocity))
    machine.mem_write(DGROUP + obstacle + 0x16, b'\x40')
    initialized = bytes(machine.mem_read(0, 0x60000))
    for mode in ('first', 'repeat', 'radius_edge'):
        machine.mem_write(0, initialized)
        extent = 256 if mode == 'first' else value
        margin = ((256 + extent + 256) & 65535) >> 1
        distance = margin if mode == 'radius_edge' else max(0, margin - 1)
        machine.mem_write(DGROUP + obstacle + 4, struct.pack('<2i', pose[0], signed(pose[1] - distance)))
        machine.mem_write(DGROUP + obstacle + 0x14, struct.pack('<H', extent))
        machine.mem_write(DGROUP + actor + 0x62, b'\xff' if mode == 'repeat' else b'\xfd')
        observe(actor, bool(value & 256), (mode, value, index))
        counts[mode + '_word_domain_returns'] += 1
assert counts['complete_returns'] == 262160
assert counts['genuine_registry_constructor_returns'] == 65536
assert all(counts[mode + '_word_domain_returns'] == 65536 for mode in ('first', 'repeat', 'radius_edge'))
assert branches['ignored'] == 16
assert branches['cooldown'] == 65280
assert branches['first_non_tree_hit'] == 65536
assert branches['repeated_hit'] == 65534
assert branches['no_hit'] == 65794
result = {'success': True, 'cases': counts['complete_returns'], 'counts': dict(counts),
    'branches': dict(branches), 'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(owner.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scope': 'Complete control-word and crossed saved contact/cooldown byte domains, both ignored/admitted wrappers, genuine constructor generation words/all182 physical indices, full signed speed/throttle/velocity word domains on first contact, every wrapped extent word at repeated/contact and exact non-contact boundary. Classes/wrappers/coordinate extremes rotate. Full memory/eleven low words except declared38-byte numeric stack/three scratch registers on b059 continuations. Geometry/prediction reuse complete proved owners; no stub/device provider or instruction hook. This is original contact evidence, not shared C or full living battle/PCM/game acceptance.'}
(REVIEW / 'contact-domains-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
