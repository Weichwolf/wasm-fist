"""Discovery: complete allocated class start/reset retain saved contact bytes."""
import argparse
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

REVIEW=None
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--review-dir',type=Path,required=True)
REVIEW=parser.parse_args().review_dir.resolve()
if not REVIEW.is_relative_to(Path('/tmp')):
 parser.error('Disposable review evidence must live under /tmp')
REVIEW.mkdir(parents=True,exist_ok=True)
MODEL=Path(__file__).resolve().parents[2]/'physical_contact_contract.py'
oracle,machine,actors=setup();base=bytes(machine.mem_read(0,0x60000))
seeds=(1,2,32768,65535);counts={'class_start':0,'readiness':0};digest=hashlib.sha256()
for link in (0,2):
 for kind,actor in enumerate(actors):
  for value in range(256):
   raw=bytearray((i*37+value)&255 for i in range(251))
   struct.pack_into('<HH',raw,0,kind,kind)
   raw[0x62],raw[0x93]=value,255-value
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
    assert actual[0x62]==value and actual[0x93]==255-value
    assert oracle.random_state(machine)==end
    assert machine.reg_read(UC_X86_REG_DI)==actor
    for other in actors:
     if other!=actor:assert bytes(machine.mem_read(DGROUP+other,251))==base[DGROUP+other:DGROUP+other+251]
    counts[operation]+=1;digest.update(bytes([link,kind,value])+actual)
assert counts=={'class_start':2048,'readiness':2048}
receipt={'success':True,'cases':4096,'counts':counts,'fields':[0x62,0x93],
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'model_sha256':hashlib.sha256(MODEL.read_bytes()).hexdigest(),
 'scope':'Complete unchanged class initialization/readiness on actual allocated four-class actors, every saved +62/+93 contact byte and both link boundaries. Independent full actor/RNG, other actors and DI/segment/stack return; initializer/reset algorithms remain owned by existing proven models. Retention reference only; not full contact/whole memory/eleven-register ABI, cached tree-damage-source producer/reuse, shared C or class scheduling acceptance.'}
(REVIEW/'contact-retention-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
