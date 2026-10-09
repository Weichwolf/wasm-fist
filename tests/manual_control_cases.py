"""Exact domain inputs from the proved complete original parent fixture.

Pure Python; production tests do not require optional original-emulation tools.
"""
import itertools
import struct


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
