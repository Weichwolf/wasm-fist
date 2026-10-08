"""Original a46e composition, independently predicted smoke/pool/RNG.

The paired unchanged child proves full-memory caller composition, including
scratch. Child semantic state is independently predicted by the existing smoke
contract. This is not a shared C class-update or world-scheduling gate.
"""
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_destruction_oracle import OriginalDestructionOracle
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP, SERVICE_CS
from original_weapon_control_oracle import DISPATCH
from test_destruction import expected, fixture
from test_projectile_flight import line
from unicorn.x86_const import (
    UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
    UC_X86_REG_SI, UC_X86_REG_DI, UC_X86_REG_BP, UC_X86_REG_SP,
    UC_X86_REG_CS, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_EFLAGS)

ROOT = Path('/tmp/wasm-fist-0119-class-research')
GENERAL = (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
           UC_X86_REG_SI, UC_X86_REG_DI, UC_X86_REG_BP)
oracle = OriginalDestructionOracle()
for _, _, _, table in DISPATCH:
    assert struct.unpack_from('<H', oracle.image, table+28)[0] == 0xa46e
counts = collections.Counter()
digest = hashlib.sha256()


def cases():
    for kind, enabled in itertools.product(range(4), range(256)):
        yield 'quality', kind, dict(enabled=enabled)
    for kind, phase in itertools.product(range(4), range(256)):
        yield 'phase', kind, dict(phase=phase)
    for kind, fill in itertools.product(range(4), range(151)):
        yield 'occupancy', kind, dict(fill=fill)
    for kind, status in itertools.product(range(4), range(256)):
        yield 'status', kind, dict(phase=128, status=status)
    seeds = [(0,0,0,0), (1,2,3,4), (65535,65535,65535,65535),
             (0,65535,32768,1), (2,76,78,1), (63,64,127,128),
             (32767,32768,65534,65535), (12345,54321,22222,44444)]
    for kind, cursor, words in itertools.product(range(4), range(4), seeds):
        yield 'random', kind, dict(cursor=cursor, seeds=words)
    poses = [(0,0,z) for z in (0,767,768,65535,65536,-1,-768,-2147483648,2147482880)]
    poses += [(2147483647,-2147483648,2147483647)]
    for kind, pose in itertools.product(range(4), poses):
        yield 'pose', kind, dict(pose=pose)
    for kind, extra in itertools.product(range(4), (0,1,30,31)):
        yield 'long_occupancy', kind, dict(extra=extra, fill=119)


def registers(machine, actor, ax, sp):
    for reg in GENERAL:
        machine.reg_write(reg, 0)
    machine.reg_write(UC_X86_REG_AX, ax)
    machine.reg_write(UC_X86_REG_DI, actor)
    machine.reg_write(UC_X86_REG_SP, sp)
    machine.reg_write(UC_X86_REG_DS, 0x1c00)
    machine.reg_write(UC_X86_REG_SS, 0x1c00)
    machine.reg_write(UC_X86_REG_EFLAGS, 2)


for group, kind, opts in cases():
    phase, enabled = opts.get('phase', 0), opts.get('enabled', 1)
    status, fill, extra = opts.get('status', 255), opts.get('fill', 0), opts.get('extra', 0)
    flags = 0x40
    raw = bytearray((i*37+kind*11+status) & 255 for i in range(251))
    struct.pack_into('<H', raw, 0, kind)
    struct.pack_into('<3i', raw, 4, *opts.get('pose', (2147483647,-2147483648,65535)))
    raw[0x3d], raw[0x1a], raw[0x96] = phase, flags, status
    bindings = [(kind,181,7)] + [(21,i,1) for i in range(fill)]
    bindings += [((kind+i+1)%4,150+i,1) for i in range(extra)]
    case = fixture(kind, operation=0, ticks=0, raw=raw, bindings=bindings,
                   enabled=enabled, seeds=opts.get('seeds', (2,76,78,1)),
                   cursor=opts.get('cursor', 0), strength=768)
    machine, pointers = oracle.prepare(case)
    actor = pointers[0]
    source = bytes(machine.mem_read(DGROUP+actor,251))
    machine.mem_write(DGROUP+0x9000, struct.pack('<H',0xeff0))
    base = bytes(machine.mem_read(0,0x60000))
    before = oracle.bindings(machine)
    admitted = phase & 0x60 == 0 and flags & 0x40 != 0
    registers(machine, actor, 0x1200, 0x9000)
    OriginalGroundManeuverOracle.execute(machine,0xa46e,0xeff0,0)
    actual = bytes(machine.mem_read(0,0x60000))
    wanted_source = bytearray(source)
    wanted_source[0x96] >>= 1
    assert actual[DGROUP+actor:DGROUP+actor+251] == wanted_source
    assert tuple(machine.reg_read(r) for r in (
        UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS)) == (
            actor,0x1c00,0x1c00,0x9002,0)
    assert actual[:DGROUP] == base[:DGROUP]
    assert actual[DGROUP+65536:] == base[DGROUP+65536:]
    created = oracle.allocations(machine,before)
    assert len(created) <= 1
    output = line('creation',[int(not created)])
    for entry in created:
        output += oracle.initial_smoke(machine,entry,source,768)
    output += oracle.shared(machine)
    child_case = dict(case, raw=case['raw'][:55], enabled=enabled if admitted else 0)
    wanted_output = expected(child_case)
    assert output == wanted_output, (group, kind, opts, output, wanted_output)
    for pointer, (typ, _, _) in zip(pointers[1:],bindings[1:]):
        size = 251 if oracle.type_flags[typ]&1 else 55
        assert actual[DGROUP+pointer:DGROUP+pointer+size] == base[DGROUP+pointer:DGROUP+pointer+size]
    # Declared far-child entry snapshot: reproduce its real constructor and
    # call scratch at the independently predicted stack/AX/DI boundary. Compare
    # all memory, then apply only the caller's independently predicted status shift.
    machine.mem_write(0,base)
    if admitted:
        registers(machine,actor,0x300,0x8ffc)
        machine.mem_write(DGROUP+0x8ffc,struct.pack('<HH',0xa482,0))
        OriginalGroundManeuverOracle.execute(machine,0x19caa,0xa482,SERVICE_CS)
        assert tuple(machine.reg_read(r) for r in (
            UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS)) == (
                actor,0x1c00,0x1c00,0x9000,0)
        counts['actual_child'] += 1
    composed = bytearray(machine.mem_read(0,0x60000))
    composed[DGROUP+actor+0x96] = status >> 1
    assert actual == composed, (group,kind,opts,
        [hex(i) for i,(a,b) in enumerate(zip(actual,composed)) if a != b][:16])
    counts[group] += 1
    counts['created'] += len(created)
    if admitted and enabled == 1 and not created:
        counts['low_priority_refused'] += 1
    digest.update(struct.pack('<4BH',kind,phase,enabled,status,fill)+wanted_source+output.encode())
    if sum(counts[g] for g in ('quality','phase','occupancy','status','random','pose','long_occupancy')) % 256 == 0:
        print(dict(counts),flush=True)
groups = {'quality':1024,'phase':1024,'occupancy':604,'status':1024,
          'random':128,'pose':40,'long_occupancy':16}
assert all(counts[k] == v for k,v in groups.items())
assert sum(groups.values()) == 3860
receipt = {'success':True,'cases':3860,'counts':dict(counts),
           'scope':'Complete original a46e returns with actual allocated actors, enabled/disabled/low-priority-refused smoke and independently predicted payload/pool/RNG. All memory matches paired unchanged child composition and predicted caller shift. No shared C class API, world scheduling, every original child scratch prediction or PCM acceptance.',
           'output_sha256':digest.hexdigest(),
           'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
           'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
           'reproduce':'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-smoke-phase-coupled.py'}
(ROOT/'smoke-phase-coupled.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
