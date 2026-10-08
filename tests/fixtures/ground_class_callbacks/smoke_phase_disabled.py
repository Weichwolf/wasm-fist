"""Complete unchanged a46e admission and hit-byte decay with explicit smoke setting0.

The actual far producer is executed when admitted; constructor-enabled coupling
is a separate required scope and is never accepted by this disabled context.
"""
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from original_vehicle_maintenance_oracle import OriginalVehicleMaintenanceOracle
from original_weapon_control_oracle import DISPATCH
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_CS, UC_X86_REG_DI,
                              UC_X86_REG_DS, UC_X86_REG_SP, UC_X86_REG_SS)

ENTRY, ACTOR = 0xa46e, 0x7000
oracle = OriginalVehicleMaintenanceOracle()
machine = oracle.machine()
machine.mem_write(DGROUP + 0x8b4f, bytes(1))
base = bytes(machine.mem_read(0, 0x60000))
for _, _, _, table in DISPATCH:
    assert struct.unpack_from('<H', oracle.image, table + 28)[0] == ENTRY
counts = collections.Counter()
digest = hashlib.sha256()


def cases():
    for phase, flags in itertools.product(range(256), range(256)):
        yield 'admission', 0, phase, flags, 255
    for kind, value, phase in itertools.product(range(4), range(256), (0, 28, 32, 128)):
        yield 'hit_byte', kind, phase, 255, value


for group, kind, phase, flags, value in cases():
    raw = bytearray((i * 37 + flags * 11 + value) & 255 for i in range(251))
    struct.pack_into('<H', raw, 0, kind)
    raw[0x3d], raw[0x1a], raw[0x96] = phase, flags, value
    wanted = bytearray(raw)
    wanted[0x96] >>= 1
    admitted = phase & 0x60 == 0 and flags & 0x40 != 0
    machine.mem_write(0, base)
    machine.mem_write(DGROUP + ACTOR, bytes(raw))
    machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
    machine.reg_write(UC_X86_REG_DI, ACTOR)
    machine.reg_write(UC_X86_REG_AX, 0x1200)
    machine.reg_write(UC_X86_REG_SP, 0x9000)
    expected = bytearray(base)
    expected[DGROUP+ACTOR:DGROUP+ACTOR+251] = wanted
    struct.pack_into('<H', expected, DGROUP+0x9000, 0xeff0)
    if admitted:
        expected[DGROUP+0x8ffc:DGROUP+0x9000] = struct.pack('<HH', 0xa482, 0)
        counts['actual_disabled_smoke_producer'] += 1
    OriginalGroundManeuverOracle.execute(machine, ENTRY, 0xeff0, 0)
    assert tuple(machine.reg_read(reg) for reg in (
        UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP,
        UC_X86_REG_CS)) == (ACTOR, 0x1c00, 0x1c00, 0x9002, 0)
    assert machine.reg_read(UC_X86_REG_AX) == (0x300 if admitted else 0x1200)
    actual = bytes(machine.mem_read(0, len(base)))
    assert actual == bytes(expected), (
        group, kind, phase, flags, value,
        [hex(i) for i, (a,b) in enumerate(zip(actual,expected)) if a != b][:16])
    digest.update(struct.pack('<4B', kind, phase, flags, value)+wanted)
    counts[group] += 1
    if counts['admission']+counts['hit_byte'] == 65536:
        print(dict(counts), flush=True)
assert counts == {'admission': 65536, 'actual_disabled_smoke_producer': 11264, 'hit_byte': 4096}
receipt = {
    'success': True, 'cases': 69632, 'counts': dict(counts),
    'scope': 'Complete original a46e phase/operating admission and every hit byte, real explicitly disabled smoke-producer far returns, whole memory and exact ABI. Enabled constructor coupling and shared/full class acceptance remain required.',
    'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'output_sha256': digest.hexdigest(),
    'reproduce': 'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python /tmp/wasm-fist-original-ground-smoke-phase-disabled.py'}
Path('/tmp/wasm-fist-0119-class-research/smoke-phase-disabled.json').write_text(json.dumps(receipt, indent=2)+'\n')
print(json.dumps(receipt), flush=True)
