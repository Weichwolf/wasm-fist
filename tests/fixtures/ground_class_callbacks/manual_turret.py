"""Full unchanged curved manual turret helpers; independent exact memory and ABI."""
import collections,hashlib,json,struct
from pathlib import Path
from manual_turret_contract import COUNTS,CURVE,METHODS,domains,predict
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
base=bytes(machine.mem_read(0,0x60000));counts=collections.Counter();digest=hashlib.sha256()
for group,kind,right,selector,view,target,flags,requested in domains():
 actor=actors[kind];raw=bytearray(base[DGROUP+actor:DGROUP+actor+251])
 struct.pack_into('<H',raw,0x40,flags);struct.pack_into('<H',raw,0x97,target)
 struct.pack_into('<h',raw,0x38,-1234);struct.pack_into('<H',raw,0x8b,requested)
 raw[0x86]=view
 wanted,index,step=predict(raw,selector,right)
 machine.mem_write(0,base);machine.mem_write(DGROUP+actor,bytes(raw))
 machine.mem_write(DGROUP+0x9746,struct.pack('<H',selector))
 machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
 machine.reg_write(UC_X86_REG_DI,actor);machine.reg_write(UC_X86_REG_AX,0x1234)
 machine.reg_write(UC_X86_REG_BX,0x88);machine.reg_write(UC_X86_REG_SP,0x9000)
 expected=bytearray(machine.mem_read(0,0x60000))
 expected[DGROUP+actor:DGROUP+actor+251]=wanted
 if flags&1: struct.pack_into('<3H',expected,DGROUP+0x8ff6,0xaafd,0x1234,actor)
 struct.pack_into('<2H',expected,DGROUP+0x8ffc,0xa3ac if right else 0xa37a,0xa3af if right else 0xa37d)
 OriginalGroundManeuverOracle.execute(machine,0xa3a8 if right else 0xa376,0xeff0,0)
 assert tuple(machine.reg_read(r) for r in (UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_DI,
  UC_X86_REG_DS,UC_X86_REG_SS,UC_X86_REG_SP,UC_X86_REG_CS))==(step,index,actor,0x1c00,0x1c00,0x9002,0)
 actual=bytes(machine.mem_read(0,0x60000))
 assert actual==expected,(group,kind,right,selector,view,target,flags,requested,
  [hex(i) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b][:16])
 digest.update(bytes(raw)+wanted+struct.pack('<2H',index,step));counts[group]+=1
 if sum(counts.values())%16384==0: print(dict(counts),flush=True)
assert counts==COUNTS
receipt={'success':True,'cases':sum(counts.values()),'counts':dict(counts),
 'scope':'Complete a376/a3a8 curved manual turret helpers on four actual allocated classes. Whole memory/AX/BX/DI/segments/exact call scratch predictions; complete device/manual/class C remains required.',
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'contract_sha256':hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest()}
Path('/tmp/wasm-fist-0119-class-research/manual-turret-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
