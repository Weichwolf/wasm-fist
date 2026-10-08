"""Required full original analog consumer, real allocations and exact memory/ABI."""
import collections
import hashlib
import json
from pathlib import Path
import struct

from analog_drive_contract import COUNTS, domains, predict
from ground_throttle_contract import DISPLAY_OFFSETS, SETTERS
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_ground_throttle_oracle import OriginalGroundThrottleOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP,
    UC_X86_REG_CS, UC_X86_REG_EFLAGS)

root = Path('/tmp/wasm-fist-0119-class-research')
original = OriginalGroundThrottleOracle()  # Pins complete class setter/display instructions.
oracle = OriginalObjectPoolOracle()
assert original.image == oracle.image
machine = oracle.fresh()
actors = []
for kind in range(4):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, kind)
    machine.reg_write(UC_X86_REG_CX, 1)
    oracle.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    actors.append(machine.reg_read(UC_X86_REG_DI))
base = bytes(machine.mem_read(0, 0x60000))
counts = collections.Counter()
digest = hashlib.sha256()
for group, kind, steer, pedal, heading, requested, speed, mode in domains():
    actor = actors[kind]
    raw = bytearray(base[DGROUP + actor:DGROUP + actor + 251])
    struct.pack_into('<H', raw, 0x26, heading)
    struct.pack_into('<H', raw, 0x30, requested)
    struct.pack_into('<h', raw, 0x55, speed)
    struct.pack_into('<h', raw, 0x57, 12345)
    struct.pack_into('<bb', raw, 0xa1, steer, pedal)
    raw[0x90] = mode
    wanted, profile, demand = predict(raw)
    machine.mem_write(0, base)
    machine.mem_write(DGROUP + actor, bytes(raw))
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
    machine.reg_write(UC_X86_REG_DI, actor)
    machine.reg_write(UC_X86_REG_AX, 0x1234)
    machine.reg_write(UC_X86_REG_BX, 0x88)
    machine.reg_write(UC_X86_REG_SP, 0x9000)
    expected = bytearray(machine.mem_read(0, 0x60000))
    expected[DGROUP + actor:DGROUP + actor + 251] = wanted
    expected_ax = demand & 65535
    if profile is not None:
        for offset in DISPLAY_OFFSETS:
            expected[DGROUP + offset] = 3
        struct.pack_into('<5H', expected, DGROUP + 0x8ff6,
            SETTERS[kind] + 14, 0, 0xa1a6, 0xab45 if profile == 3 else 0xab3d, 0xf69)
        expected_ax = (expected_ax & 0xff00) | profile
    OriginalGroundManeuverOracle.execute(machine, 0x1a174, 0xeff0, 0xf69)
    assert tuple(machine.reg_read(r) for r in (UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)) == (actor, 0x1c00, 0x1c00, 0x9004, 0)
    assert machine.reg_read(UC_X86_REG_AX) == expected_ax, (group, kind, steer, pedal, mode, speed, hex(machine.reg_read(UC_X86_REG_AX)), hex(expected_ax))
    assert machine.reg_read(UC_X86_REG_BX) == (kind * 2 if profile is not None else 0x88)
    actual = bytes(machine.mem_read(0, 0x60000))
    assert actual == expected, (group, kind, steer, pedal, heading, requested, speed, mode,
        [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:16])
    digest.update(bytes(raw) + wanted + struct.pack('<H', expected_ax))
    counts[group] += 1
    if sum(counts.values()) % 16384 == 0:
        print(dict(counts), flush=True)
assert counts == COUNTS
receipt = {'success': True, 'cases': sum(counts.values()), 'counts': dict(counts),
    'scope': 'Full original signed-axis consumer with actual allocated four-class actors and reused independently proved complete profile setter. Exact whole 0x60000 memory, AX/BX/DI/segments/stack; no producer, manual bank, C or full-class acceptance.',
    'output_sha256': digest.hexdigest(), 'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'reproduce': 'PYTHONPATH=tests PYTHONPYCACHEPREFIX=/tmp/wasm-fist-python-cache /tmp/wasm-fist-decoder-oracle/bin/python tests/fixtures/ground_class_callbacks/analog_drive.py',
    'contract_sha256': hashlib.sha256(Path(predict.__code__.co_filename).read_bytes()).hexdigest()}
root.joinpath('analog-drive-original.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
