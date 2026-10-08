"""All required unchanged original elevation domains, whole memory and ABI."""
import collections
import hashlib
import json
from pathlib import Path
import struct

from ground_elevation_contract import (PHASE, RAISE, LOWER, QUICK_LOWER, CENTER,
    DOMAIN_COUNTS, domain_cases, predict, snapshot)
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from original_weapon_control_oracle import DISPATCH
from unicorn.x86_const import (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_CX,
    UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS,UC_X86_REG_EFLAGS)

ROOT = Path('/tmp/wasm-fist-0122-review')
ROOT.mkdir(exist_ok=True)
oracle = OriginalObjectPoolOracle()
machine = oracle.fresh()
actors = []
for kind in range(4):
    machine.reg_write(UC_X86_REG_AX,kind)
    machine.reg_write(UC_X86_REG_BX,kind)
    machine.reg_write(UC_X86_REG_CX,1)
    oracle.far_call(machine,0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS)&1
    actors.append(machine.reg_read(UC_X86_REG_DI))
base = bytes(machine.mem_read(0,0x60000))
for _,_,_,table in DISPATCH:
    for slot in (1,4,9,13):
        assert struct.unpack_from('<H',oracle.image,table+slot*2)[0] == 0xa202
entries = (0xa202,0xa1e9,0xa26d,0xa265,0xa25b)
counts = collections.Counter()
digest = hashlib.sha256()
for group,kind,flags,target,elevation,clock,controls,action in domain_cases():
    raw = snapshot(kind,flags,target,elevation)
    wanted, end, applied = predict(raw,clock,controls,action)
    actor = actors[kind]
    machine.mem_write(0,base)
    machine.mem_write(DGROUP+actor,raw)
    machine.mem_write(DGROUP+0x452,struct.pack('<H',clock))
    machine.mem_write(DGROUP+0x9600,struct.pack('<3H',*controls))
    machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
    machine.reg_write(UC_X86_REG_DI,actor)
    machine.reg_write(UC_X86_REG_AX,0x1234)
    machine.reg_write(UC_X86_REG_BX,0x88)
    machine.reg_write(UC_X86_REG_SP,0x9000)
    expected = bytearray(machine.mem_read(0,0x60000))
    expected[DGROUP+actor:DGROUP+actor+251] = wanted
    struct.pack_into('<3H',expected,DGROUP+0x9600,*end)
    if action == PHASE and applied is not None:
        returns = (0xa1ef,0xa21c,controls[1]) if flags&32 else (0xa273,0xa22e,controls[1])
        struct.pack_into('<3H',expected,DGROUP+0x8ffa,*returns)
    elif action in (RAISE,LOWER,QUICK_LOWER,CENTER):
        struct.pack_into('<H',expected,DGROUP+0x8ffe,0xa1ef if action==RAISE else 0xa25e if action==CENTER else 0xa273)
    OriginalGroundManeuverOracle.execute(machine,entries[action],0xeff0,0)
    assert tuple(machine.reg_read(r) for r in (UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS)) == (actor,0x1c00,0x1c00,0x9002,0)
    assert machine.reg_read(UC_X86_REG_AX) == (0x1234 if applied is None else applied)
    assert machine.reg_read(UC_X86_REG_BX) == (controls[0] if action in (PHASE,RAISE,LOWER) and applied is not None else 0x88)
    actual = bytes(machine.mem_read(0,0x60000))
    assert actual == expected, (group,kind,flags,target,elevation,clock,controls,action,[hex(i) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b][:16])
    digest.update(struct.pack('<B4H',action,clock,*controls)+wanted+struct.pack('<3H',*end))
    counts[group] += 1
    if sum(counts.values()) % 16384 == 0:
        print(dict(counts),flush=True)
assert counts == DOMAIN_COUNTS
receipt = {'success':True,'cases':sum(counts.values()),'counts':dict(counts),
    'scope':'Complete actual allocated original phase/manual elevation returns, all required groups, independent whole-memory/AX/BX/DI/segment/stack predictions. Shared C, device producers and full class scheduling remain separately required.',
    'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'contract_sha256':hashlib.sha256(Path(__file__).parents[2].joinpath('ground_elevation_contract.py').read_bytes()).hexdigest(),
    'reproduce':'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/elevation.py'}
(ROOT/'original-elevation.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
