"""Unchanged M3/BMP secondary refill callbacks with real muted voice returns."""
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

ACTOR = 0x7000
CLASSES = ((1, 0x8886, 0xad, 2, 0xd6, 0x88c7),
           (3, 0x987b, 0xb1, 4, 0xe4, 0x98bc))
oracle = OriginalVehicleMaintenanceOracle()
machine = oracle.machine()
machine.mem_write(DGROUP + 0x6da2, bytes(2))
machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', ACTOR))
base = bytes(machine.mem_read(0, 0x60000))
for kind, entry, *_ in CLASSES:
    table = DISPATCH[kind][3]
    assert struct.unpack_from('<H', oracle.image, table + (20 if kind == 1 else 16))[0] == entry
counts = collections.Counter()
digest = hashlib.sha256()


def predict(original, tick, ammo_offset, capacity, component):
    raw = bytearray(original)
    voices = 0
    if tick & 0xff0 == 0:
        ammo = struct.unpack_from('<H', raw, ammo_offset)[0]
        if raw[0x19] & 1:
            while ammo != capacity and raw[0xbb] != 0:
                raw[0x19] &= 254
                ammo = (ammo + 1) & 65535
                raw[0xbb] -= 1
                raw[component] = 3
                voices += 1
            struct.pack_into('<H', raw, ammo_offset, ammo)
        elif ammo != capacity and raw[0xbb] != 0:
            raw[0x19] |= 1
    return bytes(raw), voices


def cases(capacity):
    for tick in range(65536):
        yield 'tick', tick, 1, 0, 2
    for ammo in range(65536):
        yield 'ammunition', 0, 1, ammo, 1
    edges = sorted({0, 1, capacity - 1, capacity, capacity + 1, 65534, 65535})
    for flags, ammo, reserve, tick in itertools.product(
            range(256), edges, (0, 1, 2, 7), (0, 16)):
        yield 'flags', tick, flags, ammo, reserve
    for reserve, ammo in itertools.product(
            range(256), (0, capacity - 1, capacity, capacity + 1, 65535)):
        yield 'reserve', 0, 1, ammo, reserve


for kind, entry, ammo_offset, capacity, component, voice_return in CLASSES:
    for group, tick, flags, ammo, reserve in cases(capacity):
        raw = bytearray((i * 37 + flags * 11 + reserve) & 255 for i in range(251))
        struct.pack_into('<H', raw, 0, kind)
        raw[0x19], raw[0xbb] = flags, reserve
        struct.pack_into('<H', raw, ammo_offset, ammo)
        wanted, voices = predict(raw, tick, ammo_offset, capacity, component)
        machine.mem_write(0, base)
        machine.mem_write(DGROUP + ACTOR, bytes(raw))
        machine.mem_write(DGROUP + 0x6cde, struct.pack('<H', tick))
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        machine.reg_write(UC_X86_REG_DI, ACTOR)
        machine.reg_write(UC_X86_REG_AX, 0x1200)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        expected = bytearray(base)
        expected[DGROUP+ACTOR:DGROUP+ACTOR+251] = wanted
        struct.pack_into('<H', expected, DGROUP + 0x6cde, tick)
        struct.pack_into('<H', expected, DGROUP + 0x9000, 0xeff0)
        start = entry
        for _ in range(voices):
            OriginalGroundManeuverOracle.execute(machine, start, 0xbf3c, 0)
            assert machine.reg_read(UC_X86_REG_AX) == 0x1217
            assert machine.reg_read(UC_X86_REG_SP) == 0x8ffc
            assert bytes(machine.mem_read(DGROUP + 0x8ffc, 4)) == struct.pack('<HH', voice_return, 0)
            # Real mute admission and RETF; no callback substitution or PC skip.
            OriginalGroundManeuverOracle.execute(machine, 0xbf3c, 0xbf76, 0)
            OriginalGroundManeuverOracle.execute(machine, 0xbf76, voice_return, 0)
            start = voice_return
        OriginalGroundManeuverOracle.execute(machine, start, 0xeff0, 0)
        if voices:
            expected[DGROUP+0x8ffc:DGROUP+0x9000] = struct.pack('<HH', voice_return, 0)
        assert tuple(machine.reg_read(r) for r in (
            UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SP, UC_X86_REG_SS,
            UC_X86_REG_CS)) == (ACTOR, 0x1c00, 0x9002, 0x1c00, 0)
        actual = bytes(machine.mem_read(0, len(base)))
        assert actual == bytes(expected), (
            kind, group, tick, flags, ammo, reserve,
            [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:16])
        digest.update(struct.pack('<BHBHBH', kind, tick, flags, ammo, reserve, voices) + wanted)
        counts[group] += 1
        counts['voice_23'] += voices
        if sum(counts[g] for g in ('tick', 'ammunition', 'flags', 'reserve')) % 16384 == 0:
            print(dict(counts), flush=True)
assert {g: counts[g] for g in ('tick', 'ammunition', 'flags', 'reserve')} == {
    'tick': 131072, 'ammunition': 131072, 'flags': 26624, 'reserve': 2560}
receipt = {
    'success': True, 'cases': 291328, 'counts': dict(counts),
    'scope': 'Complete original M3/BMP secondary rack callbacks and real logical voice23 returns in explicitly muted contexts; no PCM or shared-class acceptance.',
    'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'reproduce': 'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-secondary-rack.py'}
Path('/tmp/wasm-fist-0119-class-research/secondary-rack.json').write_text(json.dumps(receipt, indent=2)+'\n')
print(json.dumps(receipt), flush=True)
