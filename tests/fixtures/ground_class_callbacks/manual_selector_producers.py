"""Complete key-selection tail and explicit UI/config setting-store boundaries."""
import collections
import hashlib
import json
from pathlib import Path
import struct

from manual_parent_original import REGISTERS,setup,run,snapshot
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_DI,UC_X86_REG_SP)

ROOT=Path('/tmp/wasm-fist-0125-review');KEYS=0x7000
oracle,machine,actors=setup();counts=collections.Counter();digest=hashlib.sha256()
assert oracle.image[DGROUP+0x8b5f:DGROUP+0x8b65]==bytes(range(6))
assert struct.unpack_from('<6H',oracle.image,0x6b2d)==(0x6b55,)*5+(0x6b60,)


def registers(actor,bx=0x88):
 values=(0x1234,bx,0x1111,0x2222,0x3333,0x4444,actor,0x1c00,0x1c00,0x9000,0)
 for register,value in zip(REGISTERS,values):machine.reg_write(register,value)
 machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
 return values


def key_case(kind,bits,previous):
 actor=actors[kind]
 machine.mem_write(DGROUP+actor,snapshot(kind,previous,0,0,65535))
 machine.mem_write(DGROUP+0x976a,struct.pack('<H',KEYS))
 machine.mem_write(DGROUP+KEYS,struct.pack('<H',bits))
 values=registers(actor);before=bytes(machine.mem_read(0,0x60000));expected=bytearray(before)
 action=previous
 for bit,value in ((16,0),(32,2),(128,4),(256,6),(64,8)):
  if bits&bit:action=value
 expected[DGROUP+actor+0xa0]=action
 OriginalGroundManeuverOracle.execute(machine,0xa521,0xeff0)
 assert bytes(machine.mem_read(0,0x60000))==expected,(kind,bits,previous,action)
 assert tuple(machine.reg_read(r) for r in REGISTERS)==(values[0],KEYS,*values[2:9],0x9002,0)
 digest.update(struct.pack('<HBBB',bits,kind,previous,action))
 counts['key_words' if previous==255 else 'retained_action_bytes']+=1
 if bits&0x1f0 and bits%1024==0x1f0:
  run(machine,actor,True,2)
  counts['key_parent_coupling']+=1


for kind in range(4):
 for bits in range(65536):key_case(kind,bits,255)
 for previous in range(255):key_case(kind,0,previous)
# Include 255 in the separate retained-byte inventory: the word sweep already
# proves it, but record this explicit boundary separately for all-byte coverage.
for kind in range(4):
 key_case(kind,0,255);counts['key_words']-=1;counts['retained_action_bytes']+=1

for kind,actor in enumerate(actors):
 for value in range(256):
  machine.mem_write(DGROUP+0x8aa8,bytes([value]));values=registers(actor)
  before=bytes(machine.mem_read(0,0x60000));expected=bytearray(before)
  parsed=(value-48)&255;parsed=parsed if parsed<=4 else 0
  struct.pack_into('<H',expected,DGROUP+0x8b43,parsed)
  OriginalGroundManeuverOracle.execute(machine,0x6ef5,0x6f05)
  assert bytes(machine.mem_read(0,0x60000))==expected
  assert tuple(machine.reg_read(r) for r in REGISTERS)==(parsed,*values[1:])
  counts['config_store_prefix']+=1;digest.update(bytes([kind,value,parsed]))
  machine.mem_write(DGROUP+actor,snapshot(kind,2,value,0,65535))
  run(machine,actor,True,parsed);counts['config_parent_coupling']+=1
 for item in range(5):
  values=registers(actor,item*2);before=bytes(machine.mem_read(0,0x60000));expected=bytearray(before)
  struct.pack_into('<H',expected,DGROUP+0x8b43,item)
  struct.pack_into('<2H',expected,DGROUP+0x8ffc,0x6b5f,0x6b2c)
  OriginalGroundManeuverOracle.execute(machine,0x6b27,0x6c3b)
  assert bytes(machine.mem_read(0,0x60000))==expected
  assert tuple(machine.reg_read(r) for r in REGISTERS)==(item,item,*values[2:9],0x8ffc,0)
  counts['ui_store_prefix']+=1;digest.update(bytes([kind,item]))
  machine.mem_write(DGROUP+actor,snapshot(kind,4,160,0,65535))
  run(machine,actor,True,item);counts['ui_parent_coupling']+=1
assert counts=={'key_words':262144,'retained_action_bytes':1024,'key_parent_coupling':256,
               'config_store_prefix':1024,'config_parent_coupling':1024,
               'ui_store_prefix':20,'ui_parent_coupling':20},counts
receipt={'success':True,'cases':sum(counts.values()),'counts':dict(counts),
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'scope':'Whole memory/eleven-register ABI for complete unchanged a521 key-selector tail and its parent couplings. All five original UI setting producers execute genuine 6b27/6b55 calls through the 6c38 store at named stop6c3b; config bytes execute 6ef5 through named completed store6f05. Both producers only produce modes0..4; the sixth complete drive-bank entry5 has no producer asserted here. These prefixes prove produced modes and complete setting-store behavior, not remaining widget callbacks/config parsing or full hardware/UI returns; no provider is replaced or bypassed as a successful full return.'}
(ROOT/'manual-selector-producers.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
