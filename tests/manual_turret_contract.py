"""Complete original curved manual turret helpers and their two direct callers."""
import struct
CURVE=(30,30,30,30,36,45,60,91,182,122,212,242,364,364,364,364)
REFRESH=(0xc7,0xc4,0xe6,0xd7)
METHODS=(0x784d,0x7faf,0x8cc8,0x9407)

def predict(original,selector,right):
 raw=bytearray(original);kind=int.from_bytes(raw[:2],'little')
 if len(raw)!=251 or kind>=4 or not 0<=selector<=65535 or right not in (False,True):
  raise ValueError('Complete ground actor, selector word and direction required')
 flags=int.from_bytes(raw[0x40:0x42],'little')
 if flags&1:
  struct.pack_into('<H',raw,0x40,flags&65534);raw[REFRESH[kind]]=3
 if int.from_bytes(raw[0x97:0x99],'little'):
  struct.pack_into('<H',raw,0x97,0);struct.pack_into('<h',raw,0x38,0)
 index=(((selector-160) if right else (160-selector))&65535)>>2&30
 step=CURVE[index//2]
 if raw[0x86]!=1:
  step>>=1
  if raw[0x86]>3: step>>=1
 requested=int.from_bytes(raw[0x8b:0x8d],'little')
 struct.pack_into('<H',raw,0x8b,(requested+(step if right else -step))&65535)
 return bytes(raw),index,step

def domains():
 for kind in range(4):
  for right in (False,True):
   for selector in range(160,288,8):
    for view in range(256):
     for target in (0,65535):
      for flags in (0,65535):
       yield 'views',kind,right,selector,view,target,flags,65535
 for right in (False,True):
  for selector in range(65536):
   yield 'selector_words',selector&3,right,selector,(selector>>8)&255,selector,selector,0x1234
 for right in (False,True):
  for requested in range(65536):
   yield 'requested_words',requested&3,right,232 if right else 88,1,0,requested,requested
COUNTS={'views':131072,'selector_words':131072,'requested_words':131072}
