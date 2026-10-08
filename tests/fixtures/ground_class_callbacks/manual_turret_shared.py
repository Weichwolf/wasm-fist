"""Two complete original fixed-input turret callbacks retain four real actors."""
import hashlib,json,struct
from pathlib import Path
from manual_turret_contract import CURVE,METHODS,predict
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_CX,UC_X86_REG_DI,
 UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS,UC_X86_REG_EFLAGS)
oracle=OriginalObjectPoolOracle()
assert struct.unpack_from('<16H',oracle.image,DGROUP+0x9748)==CURVE
assert struct.unpack_from('<4H',oracle.image,DGROUP+0x995a)==METHODS
machine=oracle.fresh();actors=[]
for kind in range(4):
 machine.reg_write(UC_X86_REG_AX,kind);machine.reg_write(UC_X86_REG_BX,kind)
 machine.reg_write(UC_X86_REG_CX,1);oracle.far_call(machine,0x1b1a2)
 assert not machine.reg_read(UC_X86_REG_EFLAGS)&1
 actors.append(machine.reg_read(UC_X86_REG_DI))
digest=hashlib.sha256();cases=0
for tick in range(512):
 for kind,actor in enumerate(actors):
  right=bool((tick//256+kind)&1)
  caller=0xa5a6 if right else 0xa59c;selector=232 if right else 88
  # Explicit current view/control/target boundaries, all other actor/world state retained.
  machine.mem_write(DGROUP+actor+0x86,bytes([tick&255]))
  machine.mem_write(DGROUP+actor+0x40,struct.pack('<H',0xffff if tick&1 else 0))
  machine.mem_write(DGROUP+actor+0x97,struct.pack('<H',actors[(kind+1)%4] if tick%3==0 else 0))
  raw=bytes(machine.mem_read(DGROUP+actor,251))
  wanted,index,step=predict(raw,selector,right)
  machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
  machine.reg_write(UC_X86_REG_DI,actor);machine.reg_write(UC_X86_REG_AX,0x1234)
  machine.reg_write(UC_X86_REG_BX,0x88);machine.reg_write(UC_X86_REG_SP,0x9000)
  expected=bytearray(machine.mem_read(0,0x60000))
  expected[DGROUP+actor:DGROUP+actor+251]=wanted
  struct.pack_into('<H',expected,DGROUP+0x9746,selector)
  if raw[0x40]&1: struct.pack_into('<3H',expected,DGROUP+0x8ff4,0xaafd,0x1234,actor)
  struct.pack_into('<3H',expected,DGROUP+0x8ffa,0xa3ac if right else 0xa37a,
   0xa3af if right else 0xa37d,0xa5af if right else 0xa5a5)
  OriginalGroundManeuverOracle.execute(machine,caller,0xeff0,0)
  assert tuple(machine.reg_read(r) for r in (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_DI,
   UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS))==(step,index,actor,0x1c00,0x1c00,0x9002,0)
  assert bytes(machine.mem_read(0,0x60000))==expected,(tick,kind,right)
  digest.update(struct.pack('<HB',caller,kind)+wanted);cases+=1
assert cases==2048
receipt={'success':True,'cases':cases,
 'scope':'Complete a59c/a5a6 original wrappers, all view bytes and both directions across retained four allocated actors, exact whole memory and ABI. Not full a57a/input/device/C/class acceptance.',
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'contract_sha256':hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest()}
Path('/tmp/wasm-fist-0119-class-research/manual-turret-shared.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
