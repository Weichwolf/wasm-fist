"""Actual allocated four-class full start/reset retain all saved manual bytes."""
import hashlib
import json
from pathlib import Path
import struct
from manual_parent_original import setup
from original_mission_ready_oracle import RESET_BANK
from original_unit_oracle import DGROUP
from test_original_mission_ready import ground_reset
from test_vehicle_start import initialized
from unicorn.x86_const import UC_X86_REG_DI

ROOT=Path('/tmp/wasm-fist-0125-review')
oracle,machine,actors=setup();base=bytes(machine.mem_read(0,0x60000))
seeds=(1,2,32768,65535);counts={'class_start':0,'readiness':0};digest=hashlib.sha256()
for link in (0,2):
 for kind,actor in enumerate(actors):
  for value in range(256):
   raw=bytearray((i*37+value)&255 for i in range(251))
   struct.pack_into('<HH',raw,0,kind,kind)
   raw[0xa0],raw[0xa4]=value,255-value
   raw=bytes(raw)
   for operation in ('class_start','readiness'):
    machine.mem_write(0,base)
    machine.mem_write(DGROUP+0x1f82,struct.pack('<5H',0x1f8a,*seeds))
    machine.mem_write(DGROUP+0x6dae,bytes([link]))
    machine.mem_write(DGROUP+actor,raw)
    machine.reg_write(UC_X86_REG_DI,actor)
    if operation=='class_start':
     records,end=initialized([(kind,1,raw)],seeds,0,link)
     expected=records[0][2]
     oracle.call(machine,0xc296)
    else:
     expected=ground_reset(raw,link);end=(list(seeds),0)
     oracle.call(machine,RESET_BANK[kind])
    actual=bytes(machine.mem_read(DGROUP+actor,251))
    assert actual==expected,(operation,link,kind,value)
    assert actual[0xa0]==value and actual[0xa4]==255-value
    assert oracle.random_state(machine)==end
    assert machine.reg_read(UC_X86_REG_DI)==actor
    for other in actors:
     if other!=actor:assert bytes(machine.mem_read(DGROUP+other,251))==base[DGROUP+other:DGROUP+other+251]
    counts[operation]+=1;digest.update(bytes([link,kind,value])+actual)
assert counts=={'class_start':2048,'readiness':2048}
receipt={'success':True,'cases':4096,'counts':counts,'fields':[0xa0,0xa4],
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'scope':'Complete unchanged class initialization/readiness on actual allocated four-class actors, every saved selector/decoded-view byte and both link boundaries. Independent full actor/RNG, other actors and DI/segment/stack return; initializer/reset algorithms remain owned by existing proven models. Not full original configuration/device or class scheduling acceptance.'}
(ROOT/'manual-retention-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
