"""Unchanged a904 with actual allocations, readonly roster candidates and PM visibility."""
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_audio_request_oracle import OriginalAudioRequestOracle
from original_ground_phase_oracle import OriginalGroundPhaseOracle
from original_target_acquisition_oracle import MAILBOX
from original_unit_oracle import DGROUP
from original_weapon_control_oracle import DISPATCH
from roster_promotion_contract import word
from target_acquisition_contract import aim_positions
from test_visibility import visible
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_DS,
                              UC_X86_REG_EFLAGS, UC_X86_REG_SP, UC_X86_REG_SS)

owner=OriginalGroundPhaseOracle(OriginalAudioRequestOracle())
side,pixels=512,bytes([32])*512**2
owner.install_height(side,pixels)
machine=owner.fresh()
actors,targets=[],[]
for group,kinds in ((actors,range(4)),(targets,range(28))):
    for kind in kinds:
        machine.reg_write(UC_X86_REG_AX,kind)
        owner.far_call(machine,0x1b1df)
        assert machine.reg_read(UC_X86_REG_EFLAGS)&1==0
        pointer=machine.reg_read(UC_X86_REG_DI)
        assert word(bytes(machine.mem_read(DGROUP+pointer,2)),0)==kind
        group.append(pointer)
machine.mem_write(DGROUP+0xea2c,struct.pack('<HH',0,MAILBOX//16))
machine.mem_write(DGROUP+0x6d3c,bytes(64))
base=bytes(machine.mem_read(0,0x60000))
for _,_,_,table in DISPATCH:
    assert struct.unpack_from('<H',owner.image,table+22)[0]==0xa904
counts=collections.Counter(); digest=hashlib.sha256()


def cases():
    for phase,flags in itertools.product(range(256),range(256)):
        yield 'admission',0,phase,flags,255,255,None,0,False
    for kind,count,cursor,nature in itertools.product(range(4),range(256),(0,254,255),range(4)):
        target=None if nature==0 else targets[23] if nature==1 else targets[0]
        yield 'counters',kind,22,8,count,cursor,target,0,nature==3
    for kind,cursor in itertools.product(range(4),range(256)):
        yield 'self',kind,22,8,128,cursor,actors[kind],0,False
    for kind,target_kind,outside in itertools.product(range(4),range(28),(False,True)):
        yield 'target_types',kind,22,8,127,0,targets[target_kind],3,outside
    for kind,variant,outside in itertools.product(range(4),range(256),(False,True)):
        yield 'variants',kind,22,8,127,0,targets[26],variant,outside


for group,kind,phase,flags,count,cursor,candidate,variant,outside in cases():
    actor=actors[kind]
    raw=bytearray(base[DGROUP+actor:DGROUP+actor+251])
    struct.pack_into('<3i',raw,4,100000,10000,8192)
    raw[0x3d],raw[0x16],raw[0x36],raw[0x37]=phase,flags,count,cursor
    machine.mem_write(0,base)
    machine.mem_write(DGROUP+actor,bytes(raw))
    # A live high-half decoy proves that the callback inspects only slots0..15.
    next_cursor=(cursor+1)&255;index=next_cursor&15
    machine.mem_write(DGROUP+0x6d3c+2*(index+16),struct.pack('<H',targets[1]))
    if candidate is not None:
        machine.mem_write(DGROUP+0x6d3c+2*index,struct.pack('<H',candidate))
        if candidate!=actor:
            target=bytearray(base[DGROUP+candidate:DGROUP+candidate+(251 if owner.type_flags[word(base,DGROUP+candidate)]&1 else 55)])
            struct.pack_into('<3i',target,4,100000+(262144 if outside else -200000),10000,8192)
            target[0x19]=variant
            machine.mem_write(DGROUP+candidate,bytes(target))
    before=bytes(machine.mem_read(0,0x60000)); expected=bytearray(before)
    wanted=bytearray(raw);plan={'transfers':[],'requests':[],'height':None}
    if phase&0xe0==0 and flags&8:
        if wanted[0x36]:wanted[0x36]-=1
        wanted[0x37]=next_cursor
        if candidate is not None and word(before,DGROUP+candidate)!=23:
            target=wanted if candidate==actor else before[DGROUP+candidate:DGROUP+candidate+251]
            source,destination=aim_positions(owner,wanted,target)
            admitted=visible(side,pixels,(source,destination))
            plan['transfers']=[{'source':source,'target':destination,'visible':admitted}]
            if admitted:wanted[0x36]=255
            counts['visible' if admitted else 'occluded']+=1
    expected[DGROUP+actor:DGROUP+actor+251]=wanted
    expected[DGROUP+0x9000:DGROUP+0x9002]=struct.pack('<H',0xeff0)
    if plan['transfers']:
        # e21c/e25c/e286 pushes reuse8ffc; the provider stops before CALL e339.
        # Preserve8ffa unchanged and predict the exact scratch/near return.
        expected[DGROUP+0x8ffc:DGROUP+0x9000]=struct.pack('<2H',actor,0xa935)
        struct.pack_into('<H',expected,DGROUP+0xea10,0x58)
    machine.reg_write(UC_X86_REG_DI,actor)
    machine.reg_write(UC_X86_REG_SP,0x9000)
    machine.mem_write(DGROUP+0x9000,struct.pack('<H',0xeff0))
    transfers,requests=owner.run_child(machine,0xa904,0xeff0,plan,{})
    assert not requests and len(transfers)==len(plan['transfers'])
    assert tuple(machine.reg_read(r) for r in (UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SP,UC_X86_REG_SS))==(actor,0x1c00,0x9002,0x1c00)
    actual=bytes(machine.mem_read(0,0x60000))
    assert actual[:MAILBOX]==bytes(expected[:MAILBOX]) and actual[MAILBOX+4096:]==bytes(expected[MAILBOX+4096:]), (group,kind,phase,flags,count,cursor,candidate,[hex(i) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b and not MAILBOX<=i<MAILBOX+4096][:12])
    # run_child independently checks every mailbox/code/external write and real
    # kernel result; this guard additionally checks all DOS bytes/other objects.
    digest.update(struct.pack('<7H',kind,phase,flags,count,cursor,65535 if candidate is None else candidate,variant)+wanted)
    counts[group]+=1
    if sum(counts[g] for g in ('admission','counters','self','target_types','variants'))%16384==0:print(dict(counts),flush=True)
assert counts['admission']==65536 and counts['counters']==12288 and counts['self']==1024 and counts['target_types']==224 and counts['variants']==2048
receipt={'success':True,'scope':'Complete a904 original roster visibility, actual allocations, full DOS/external/mailbox/PM guards; no shared-class or full game acceptance.','counts':dict(counts),'cases':81120,'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(owner.image).hexdigest(),'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'reproduce':'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-roster-visibility.py'}
Path('/tmp/wasm-fist-0119-class-research/roster-visibility.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt),flush=True)
