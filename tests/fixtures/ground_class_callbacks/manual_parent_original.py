"""Unchanged complete manual parent; independent full memory and ABI prediction."""
import hashlib
import itertools
import json
from pathlib import Path
import struct
import sys

from manual_control_contract import (DRIVE_BANK, WEAPON_BANK, VIEW_METHODS,
    VIEW_COMPONENTS, VIEW_DISPLAY, predict)
from ground_throttle_contract import DISPLAY_OFFSETS, SETTERS
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS, UC_X86_REG_EFLAGS)

ROOT = Path('/tmp/wasm-fist-0125-review')
REGISTERS = (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
             UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
             UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)


def expected_memory(before, actor, selected, mode, ax=0x1234, bx=0x88):
    raw = before[DGROUP+actor:DGROUP+actor+251]
    clock, = struct.unpack_from('<H', before, DGROUP+0x452)
    controls = struct.unpack_from('<3H', before, DGROUP+0x9600)
    selector, = struct.unpack_from('<H', before, DGROUP+0x9746)
    wanted, end, selector, evidence = predict(raw, selected, mode, clock, controls, selector)
    expected = bytearray(before)
    expected[DGROUP+actor:DGROUP+actor+251] = wanted
    struct.pack_into('<3H', expected, DGROUP+0x9600, *end)
    struct.pack_into('<H', expected, DGROUP+0x9746, selector)
    if not selected:
        return expected, ax, bx, evidence
    drive = evidence['drive_mode']
    kind = int.from_bytes(raw[:2], 'little')
    if drive is not None:
        struct.pack_into('<H', expected, DGROUP+0x8ffe, 0xa591)
    if evidence['drive']:
        struct.pack_into('<2H', expected, DGROUP+0x8ffa, DRIVE_BANK[drive]+5, 0)
        ax = evidence['demand'] & 65535
        profile = evidence['profile']
        if profile is not None:
            for offset in DISPLAY_OFFSETS:
                expected[DGROUP+offset] = 3
            struct.pack_into('<5H', expected, DGROUP+0x8ff0,
                SETTERS[kind]+14, 0, 0xa1a6, 0xab45 if profile == 3 else 0xab3d, 0xf69)
            ax = (ax & 0xff00) | profile
        if evidence['view']:
            ax = (ax & 0xff00) | raw[0xa4]
            for offset in VIEW_DISPLAY:
                expected[DGROUP+offset] = 3
            struct.pack_into('<2H', expected, DGROUP+0x8ffa, 0xa623, 0)
    struct.pack_into('<H', expected, DGROUP+0x8ffe, 0xa59a)
    action = evidence['weapon']
    bx = action
    refresh = int.from_bytes(raw[0x40:0x42], 'little') & 1
    if action in (2, 4):
        right = action == 4
        struct.pack_into('<3H', expected, DGROUP+0x8ff8,
            0xa3ac if right else 0xa37a, 0xa3af if right else 0xa37d,
            0xa5af if right else 0xa5a5)
        if refresh:
            struct.pack_into('<3H', expected, DGROUP+0x8ff2, 0xaafd, ax, actor)
        ax, bx = evidence['turret_step'], evidence['turret_index']
    elif action in (6, 8):
        if refresh:
            struct.pack_into('<3H', expected, DGROUP+0x8ff4, 0xaafd, ax, actor)
        struct.pack_into('<2H', expected, DGROUP+0x8ffa,
            0xa5b4 if action == 6 else 0xa5d1, 0xa5ba if action == 6 else 0xa5d7)
        ax, bx = evidence['elevation_step'], controls[0]
    return expected, ax, bx, evidence


def setup():
    oracle = OriginalObjectPoolOracle()
    assert struct.unpack_from('<6H', oracle.image, DGROUP+0x976c) == DRIVE_BANK
    assert struct.unpack_from('<5H', oracle.image, DGROUP+0x9778) == WEAPON_BANK
    methods = tuple(segment*16+offset for offset,segment in
                    struct.iter_unpack('<HH', oracle.image[DGROUP+0x971e:DGROUP+0x972e]))
    assert methods == VIEW_METHODS
    for address, offsets in zip(methods, VIEW_COMPONENTS):
        assert oracle.image[address:address+16] == b''.join(
            b'\xc6\x85'+struct.pack('<H',offset)+b'\x03' for offset in offsets)+b'\xcb'
    assert oracle.image[0x1701b:0x1702b] == b''.join(
        b'\xc6\x06'+struct.pack('<H',offset)+b'\x03' for offset in VIEW_DISPLAY)+b'\xcb'
    machine = oracle.fresh()
    actors = []
    for kind in range(4):
        machine.reg_write(UC_X86_REG_AX, kind)
        machine.reg_write(UC_X86_REG_BX, kind)
        machine.reg_write(UC_X86_REG_CX, 1)
        oracle.far_call(machine, 0x1b1a2)
        assert not machine.reg_read(UC_X86_REG_EFLAGS)&1
        actors.append(machine.reg_read(UC_X86_REG_DI))
    return oracle, machine, actors


def run(machine, actor, selected, mode):
    machine.mem_write(DGROUP+0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP+0x8b43, struct.pack('<H', mode))
    machine.mem_write(DGROUP+0x9000, struct.pack('<H', 0xeff0))
    for reg,value in zip(REGISTERS,(0x1234,0x88,0x1111,0x2222,0x3333,0x4444,
                                    actor,0x1c00,0x1c00,0x9000,0)):
        machine.reg_write(reg,value)
    before = bytes(machine.mem_read(0,0x60000))
    wanted,ax,bx,evidence = expected_memory(before,actor,selected,mode)
    OriginalGroundManeuverOracle.execute(machine,0xa57a,0xeff0,0)
    observed = tuple(machine.reg_read(r) for r in REGISTERS)
    expected = (ax,bx,0x1111,0x2222,0x3333,0x4444,actor,0x1c00,0x1c00,0x9002,0)
    assert observed == expected,(selected,mode,hex(actor),observed,expected,evidence)
    actual = bytes(machine.mem_read(0,0x60000))
    assert actual == wanted,(selected,mode,hex(actor),evidence,
        [hex(i) for i,(a,b) in enumerate(zip(actual,wanted)) if a!=b][:24])
    return actual[DGROUP+actor:DGROUP+actor+251],evidence


def snapshot(kind,action,view,flags,target,heading=65535,requested=65535,speed=-1,profile=255):
    raw = bytearray((i*37+kind*11)&255 for i in range(251))
    struct.pack_into('<H',raw,0,kind)
    struct.pack_into('<H',raw,0x40,flags)
    struct.pack_into('<H',raw,0x97,target)
    struct.pack_into('<H',raw,0x26,heading)
    struct.pack_into('<H',raw,0x30,0x1234)
    struct.pack_into('<H',raw,0x8b,requested)
    struct.pack_into('<h',raw,0x38,-32768)
    struct.pack_into('<h',raw,0x55,speed)
    struct.pack_into('<bb',raw,0xa1,24,-128)
    raw[0x90]=profile
    raw[0xa0]=action
    raw[0xa4]=view
    raw[0x86]=3
    return bytes(raw)



def complete_cases():
    for kind,mode,action,flags,target,view in itertools.product(
            range(4),range(6),(0,2,4,6,8),(0,1,65534,65535),(0,65535),range(256)):
        raw=bytearray(snapshot(kind,action,view,flags,target));raw[0x86]=255-view
        yield 'banks_views',kind,True,mode,bytes(raw),65535,(65530,363,65535),65535
    pairs=((-128,-128),(-24,-25),(-23,-24),(0,-23),(23,0),(24,23),(127,24),(127,127))
    for kind,mode,action,pair,speed,profile,flag,target in itertools.product(
            range(4),range(6),(0,2,4,6,8),pairs,(-32768,-1,0,32767),
            (0,1,2,3,255),(0,1),(0,65535)):
        raw=bytearray(snapshot(kind,action,160,flag,target,speed=speed,profile=profile))
        struct.pack_into('<bb',raw,0xa1,*pair)
        yield 'axes_profiles',kind,True,mode,bytes(raw),20,(0,18,111),32768
    elevations=(0,9099,9100,9101,-5459,-5460,-5461,32767,-32768,65535)
    for kind,mode,action,value,step,elapsed,flag,target in itertools.product(
            range(4),range(6),(6,8),elevations,(0,17,18,363,364,365,728,32767,32768,65535),
            (0,19,20,65535),(0,1),(0,65535)):
        raw=bytearray(snapshot(kind,action,79,flag,target))
        struct.pack_into('<H',raw,0x38,value&65535)
        yield 'elevation_order',kind,True,mode,bytes(raw),(65530+elapsed)&65535,(65530,step,111),65535
    for mode in range(65536):
        kind=mode&3;raw=bytearray(snapshot(kind,mode&255,mode>>8,mode,mode))
        yield 'unselected_words',kind,False,mode,bytes(raw),mode,(mode,mode^65535,123),mode
        raw[0xa0]=(0,2,4,6,8)[mode%5]
        struct.pack_into('<H',raw,0x40,mode|1)
        yield 'inhibited_words',kind,True,mode,bytes(raw),mode,(mode,mode^65535,123),mode
    for kind,mode,action,view,flag in itertools.product(
            range(4),range(32768,32774),(0,2,4,6,8),range(256),(0,1)):
        raw=bytearray(snapshot(kind,action,view,flag,65535));raw[0x86]=255-view
        yield 'wrapped_aliases',kind,True,mode,bytes(raw),0,(65535,363,18),0


COUNTS={'banks_views':245760,'axes_profiles':76800,'elevation_order':76800,
        'unselected_words':65536,'inhibited_words':65536,'wrapped_aliases':61440}


def main():
    import collections
    oracle,machine,actors=setup();base=bytes(machine.mem_read(0,0x60000))
    digest=hashlib.sha256();counts=collections.Counter()
    for group,kind,selected,mode,raw,clock,controls,selector in complete_cases():
        actor=actors[kind];machine.mem_write(0,base)
        machine.mem_write(DGROUP+actor,raw)
        machine.mem_write(DGROUP+0x452,struct.pack('<H',clock))
        machine.mem_write(DGROUP+0x9600,struct.pack('<3H',*controls))
        machine.mem_write(DGROUP+0x9746,struct.pack('<H',selector))
        actual,evidence=run(machine,actor,selected,mode)
        digest.update(struct.pack('<HBBH4H',mode,kind,selected,clock,*controls,selector)+raw+actual)
        counts[group]+=1
        if sum(counts.values())%16384==0:print(dict(counts),flush=True)
    assert counts==COUNTS,(counts,COUNTS)
    receipt={'success':True,'cases':sum(counts.values()),'counts':dict(counts),
             'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),
             'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
             'contract_sha256':hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest(),
             'scope':'Complete unchanged allocated four-class selected/unselected a57a banks, all view bytes, axis/profile/elevation boundaries, full ignored/inhibited word domains and wrapping aliases; independent whole 0x60000 memory and eleven-register ABI. Retained parent, source corpus, saved retention and selector producers remain separately required.'}
    (ROOT/'manual-parent-original.json').write_text(json.dumps(receipt,indent=2)+'\n')
    print(json.dumps(receipt),flush=True)

if __name__=='__main__':main()
