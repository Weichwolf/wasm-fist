"""Complete unchanged a9a0 timer/flag/voice producer; muted device context."""
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_unit_oracle import DGROUP
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_vehicle_maintenance_oracle import OriginalVehicleMaintenanceOracle
from original_weapon_control_oracle import DISPATCH
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_CS, UC_X86_REG_DI,
                              UC_X86_REG_DS, UC_X86_REG_SP, UC_X86_REG_SS)

ENTRY, ACTOR = 0xa9a0, 0x7000
oracle = OriginalVehicleMaintenanceOracle()
machine = oracle.machine()
machine.mem_write(DGROUP + 0x6da2, bytes(2))
machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', ACTOR))
base = bytes(machine.mem_read(0, 0x60000))
for _, _, _, table in DISPATCH:
    assert struct.unpack_from('<H', oracle.image, table + 24)[0] == ENTRY
counts = collections.Counter()
digest = hashlib.sha256()


def predict(original):
    raw = bytearray(original)
    voice = None
    if raw[0x3d] & 0xe0 == 0:
        if raw[0x19] & 6:
            raw[0xa6] = (raw[0xa6] + 1) & 255
            if raw[0xa6] >= 64:
                raw[0xa6] = 0
                raw[0x19] &= 249
                if raw[0x19] & 16 == 0:
                    voice = 27
        if raw[0x19] & 16:
            raw[0x52] = (raw[0x52] + 1) & 255
            if raw[0x52] >= 112:
                raw[0x52] = 0
                raw[0x19] &= 239
                assert voice is None
                voice = 35
    return bytes(raw), voice


def cases():
    for phase, flags, pair in itertools.product(
            range(256), (0, 2, 4, 6, 16, 18, 20, 22, 255),
            ((0,0),(62,110),(63,111),(64,112),(127,127),(254,254),(255,255),(255,111))):
        yield 'phase', phase, flags, *pair
    for flags, value in itertools.product(range(256), range(256)):
        yield 'turn_counter', 24, flags, value, 111
        yield 'immobile_counter', 24, flags, 63, value


for group, phase, flags, turn, immobile in cases():
    raw = bytearray((i * 37 + flags * 11 + turn * 3 + immobile) & 255 for i in range(251))
    struct.pack_into('<H', raw, 0, 0)
    raw[0x3d],raw[0x19],raw[0xa6],raw[0x52] = phase,flags,turn,immobile
    wanted, voice = predict(raw)
    machine.mem_write(0, base)
    machine.mem_write(DGROUP + ACTOR, bytes(raw))
    machine.reg_write(UC_X86_REG_DI, ACTOR)
    machine.reg_write(UC_X86_REG_AX, 0x1200)
    machine.reg_write(UC_X86_REG_SP, 0x9000)
    machine.mem_write(DGROUP + 0x9000, struct.pack('<H',0xeff0))
    expected = bytearray(base)
    expected[DGROUP+ACTOR:DGROUP+ACTOR+251] = wanted
    expected[DGROUP+0x9000:DGROUP+0x9002] = struct.pack('<H',0xeff0)
    start = ENTRY
    if voice is not None:
        OriginalGroundManeuverOracle.execute(machine,start,0xbf3c,0)
        assert machine.reg_read(UC_X86_REG_AX)==0x1200 | voice
        assert machine.reg_read(UC_X86_REG_SP)==0x8ffc
        # Execute the unchanged mute comparison/branch and actual far return.
        # No PC skip, instruction patch, hook or stand-in return is used.
        OriginalGroundManeuverOracle.execute(machine,0xbf3c,0xbf76,0)
        expected[DGROUP+0x8ffc:DGROUP+0x9000] = struct.pack('<HH',0xa9cc if voice==27 else 0xa9e9,0)
        start=0xbf76
        counts['voice_'+str(voice)]+=1
    OriginalGroundManeuverOracle.execute(machine,start,0xeff0,0)
    assert tuple(machine.reg_read(r) for r in (UC_X86_REG_DI,UC_X86_REG_DS,UC_X86_REG_SP,UC_X86_REG_SS,UC_X86_REG_CS)) == (ACTOR,0x1c00,0x9002,0x1c00,0)
    actual = bytes(machine.mem_read(0,len(base)))
    assert actual==bytes(expected), (group,phase,flags,turn,immobile,[hex(i) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b][:16])
    digest.update(struct.pack('<4B',phase,flags,turn,immobile)+wanted+bytes([255 if voice is None else voice]))
    counts[group]+=1
    if sum(counts[g] for g in ('phase','turn_counter','immobile_counter')) % 16384==0:
        print(dict(counts),flush=True)
assert counts['phase']==18432 and counts['turn_counter']==65536 and counts['immobile_counter']==65536
receipt={'success':True,'scope':'Complete original a9a0 timer/flag/logical voice callbacks in explicitly muted device contexts; not admitted PCM or shared-class acceptance.','cases':149504,'counts':dict(counts),'output_sha256':digest.hexdigest(),'image_sha256':hashlib.sha256(oracle.image).hexdigest(),'fixture_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'reproduce':'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-status-timers.py'}
Path('/tmp/wasm-fist-0119-class-research/status-timers.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt),flush=True)
