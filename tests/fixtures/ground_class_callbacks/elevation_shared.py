"""Nine actual cross-actor returns share one clock/step/held owner without resets."""
import hashlib
import json
from pathlib import Path
import struct

from ground_elevation_contract import PHASE,RAISE,LOWER,QUICK_LOWER,CENTER,predict
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_CX,UC_X86_REG_DI,
    UC_X86_REG_SP,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_CS,UC_X86_REG_EFLAGS)

oracle = OriginalObjectPoolOracle()
machine = oracle.fresh()
actors = []
for kind in range(4):
    machine.reg_write(UC_X86_REG_AX,kind)
    machine.reg_write(UC_X86_REG_BX,kind)
    machine.reg_write(UC_X86_REG_CX,1)
    oracle.far_call(machine,0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS)&1
    actor = machine.reg_read(UC_X86_REG_DI)
    actors.append(actor)
    machine.mem_write(DGROUP+actor+0x19,bytes([96]))
controls = (0,123,18)
machine.mem_write(DGROUP+0x9600,struct.pack('<3H',*controls))
entries = (0xa202,0xa1e9,0xa26d,0xa265,0xa25b)
sequence = [(k,PHASE,10+k) for k in range(4)] + [
    (2,RAISE,14),(3,QUICK_LOWER,999),(0,PHASE,65535),(1,CENTER,20),(2,LOWER,0)]
digest = hashlib.sha256()
observations = []
for kind,action,clock in sequence:
    actor = actors[kind]
    assert bytes(machine.mem_read(DGROUP+0x9600,6)) == struct.pack('<3H',*controls)
    if action == CENTER:
        machine.mem_write(DGROUP+actor+0x97,struct.pack('<H',actors[0]))
    raw = bytes(machine.mem_read(DGROUP+actor,251))
    wanted,end,applied = predict(raw,clock,controls,action)
    machine.mem_write(DGROUP+0x452,struct.pack('<H',clock))
    machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
    machine.reg_write(UC_X86_REG_DI,actor)
    machine.reg_write(UC_X86_REG_SP,0x9000)
    expected = bytearray(machine.mem_read(0,0x60000))
    expected[DGROUP+actor:DGROUP+actor+251] = wanted
    struct.pack_into('<3H',expected,DGROUP+0x9600,*end)
    if action == PHASE:
        struct.pack_into('<3H',expected,DGROUP+0x8ffa,0xa1ef,0xa21c,controls[1])
    else:
        struct.pack_into('<H',expected,DGROUP+0x8ffe,0xa1ef if action==RAISE else 0xa25e if action==CENTER else 0xa273)
    OriginalGroundManeuverOracle.execute(machine,entries[action],0xeff0,0)
    assert tuple(machine.reg_read(r) for r in (UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS)) == (actor,0x1c00,0x1c00,0x9002,0)
    assert bytes(machine.mem_read(0,0x60000)) == expected
    controls = end
    elevations = [struct.unpack('<h',machine.mem_read(DGROUP+a+0x38,2))[0] for a in actors]
    observations.append({'actor':kind,'action':action,'clock':clock,'controls':controls,'elevations':elevations})
    digest.update(wanted+struct.pack('<3H',*end))
assert controls == (0,728,18) and elevations == [37,0,-585,-709]
receipt = {'success':True,'cases':9,'observations':observations,
    'scope':'Actual allocated cross-actor original calls share unchanged retained controls. Whole-memory and return ABI match independent predictions; C probe repeats exact sequence. Not full clock-device/class/world acceptance.',
    'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'reproduce':'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/elevation_shared.py'}
Path('/tmp/wasm-fist-0122-review/elevation-shared-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
