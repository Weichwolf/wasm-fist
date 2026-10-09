"""Complete original parent retains actual four actors and shared controls."""
import collections
from manual_control_contract import predict
import hashlib
import json
from pathlib import Path
import struct

from manual_parent_original import setup,run,snapshot
from original_unit_oracle import DGROUP

ROOT=Path('/tmp/wasm-fist-0125-review')
oracle,machine,actors=setup();base=bytes(machine.mem_read(0,0x60000))
digest=hashlib.sha256();counts=collections.Counter();branches=collections.Counter()
for seed,controls,selector in ((0,(0,0,0),0),(65530,(65535,363,65535),65535),
                               (32768,(0,65535,18),32768)):
 machine.mem_write(0,base)
 machine.mem_write(DGROUP+0x9600,struct.pack('<3H',*controls))
 machine.mem_write(DGROUP+0x9746,struct.pack('<H',selector))
 for kind,actor in enumerate(actors):
  machine.mem_write(DGROUP+actor,snapshot(kind,0,0,1,actors[(kind+1)%4],requested=kind*16384))
 for tick in range(1024):
  for kind,actor in enumerate(actors):
   selected=(tick+kind)%13!=0
   action=(0,2,4,6,8)[(tick+kind)%5] if selected else 255
   machine.mem_write(DGROUP+actor+0xa0,bytes([action]))
   machine.mem_write(DGROUP+actor+0xa4,bytes([(tick*37+kind*53)&255]))
   axes=((-128,-128),(-24,-25),(-23,-24),(0,-23),(23,0),(24,23),(127,24),(127,127))[(tick+kind)%8]
   machine.mem_write(DGROUP+actor+0xa1,struct.pack('<bb',*axes))
   machine.mem_write(DGROUP+actor+0x55,struct.pack('<h',(-32768,-1,0,32767)[tick%4]))
   if tick%5==0:
    machine.mem_write(DGROUP+actor+0x97,struct.pack('<H',actors[(kind+1)%4]))
   mode=(tick//32+kind)%6
   if tick%31==0:
    flags=int.from_bytes(machine.mem_read(DGROUP+actor+0x40,2),'little')|1
    machine.mem_write(DGROUP+actor+0x40,struct.pack('<H',flags));mode=65535
   if not selected:mode=65535
   if tick&128 and mode!=65535:mode+=32768
   clock=(seed+tick*(1,19,20,65535)[tick%4]+kind)&65535
   machine.mem_write(DGROUP+0x452,struct.pack('<H',clock))
   before=bytes(machine.mem_read(DGROUP+actor,251))
   actual,evidence=run(machine,actor,selected,mode)
   assert evidence['drive_mode'] is None or not int.from_bytes(before[0x40:0x42],'little')&1
   for name in ('drive_mode','weapon','profile','view'):
    branches[name+':'+str(evidence[name])]+=1
   branches['selected:'+str(selected)]+=1
   controls=struct.unpack('<3H',machine.mem_read(DGROUP+0x9600,6))
   selector=int.from_bytes(machine.mem_read(DGROUP+0x9746,2),'little')
   digest.update(struct.pack('<4H',clock,*controls)+struct.pack('<H',selector))
   for other in actors:digest.update(bytes(machine.mem_read(DGROUP+other,251)))
   counts['held_parent']+=1
assert counts=={'held_parent':12288}
for mode in range(6):assert branches['drive_mode:'+str(mode)]>0
for action in (0,2,4,6,8):assert branches['weapon:'+str(action)]>0
for profile in (None,0,3):assert branches['profile:'+str(profile)]>0
for value in (True,False):
 assert branches['view:'+str(value)]>0 and branches['selected:'+str(value)]>0
receipt={'success':True,'cases':12288,'counts':dict(counts),'branches':dict(branches),
 'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
 'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
 'contract_sha256':hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest(),
 'scope':'Three complete 1024-tick sequences across four actually allocated retained actors, shared selector/adaptive controls/device-clock boundaries, genuine full parent banks, selected/unselected and inhibited high/alias modes. Actor flags are retained between calls except explicit periodic caller admission boundaries. Independent whole 0x60000 memory and eleven-register ABI; no full hardware/class/world acceptance.'}
(ROOT/'manual-parent-shared.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
