"""Retained four-actor analog consumer through the three genuine manual callers."""
import hashlib
import json
from pathlib import Path
import struct

from analog_drive_contract import predict
from ground_throttle_contract import DISPLAY_OFFSETS, SETTERS
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_ground_throttle_oracle import OriginalGroundThrottleOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP,
    UC_X86_REG_CS, UC_X86_REG_EFLAGS)

root = Path('/tmp/wasm-fist-0119-class-research')
original = OriginalGroundThrottleOracle()
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
callers = (0xa5eb, 0xa624, 0xa62a)
for caller in callers:
    assert oracle.image[caller:caller + 6] == bytes.fromhex('9a e4 aa 69 0f c3')
assert struct.unpack_from('<6H', oracle.image, DGROUP + 0x976c) == (
    0xa5ea, 0xa5eb, 0xa5f1, 0xa62a, 0xa624, 0xa630)
assert struct.unpack_from('<5H', oracle.image, DGROUP + 0x9778) == (
    0xa59b, 0xa59c, 0xa5a6, 0xa5b0, 0xa5cd)
assert oracle.image[0xa5ea] == oracle.image[0xa630] == oracle.image[0xa59b] == 0xc3
digest = hashlib.sha256()
profiles = []
cases = 0
for step in range(96):
    for kind, actor in enumerate(actors):
        # Explicit decoded input and movement-result boundary. All other actor,
        # display, pool and RNG state remains retained from the previous call.
        steer = (-128, -24, -23, 0, 23, 24, 127)[(step + kind) % 7]
        pedal = (-128, -25, -24, -23, 0, 23, 24, 127)[step % 8]
        speed = (-1, 0, 32767, -32768)[step % 4]
        machine.mem_write(DGROUP + actor + 0xa1, struct.pack('<bb', steer, pedal))
        machine.mem_write(DGROUP + actor + 0x55, struct.pack('<h', speed))
        raw = bytes(machine.mem_read(DGROUP + actor, 251))
        wanted, profile, demand = predict(raw)
        caller = callers[step % 3]
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_AX, 0x1234)
        machine.reg_write(UC_X86_REG_BX, 0x88)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        expected = bytearray(machine.mem_read(0, 0x60000))
        expected[DGROUP + actor:DGROUP + actor + 251] = wanted
        struct.pack_into('<2H', expected, DGROUP + 0x8ffc, caller + 5, 0)
        expected_ax = demand & 65535
        if profile is not None:
            for offset in DISPLAY_OFFSETS:
                expected[DGROUP + offset] = 3
            struct.pack_into('<5H', expected, DGROUP + 0x8ff2,
                SETTERS[kind] + 14, 0, 0xa1a6,
                0xab45 if profile == 3 else 0xab3d, 0xf69)
            expected_ax = (expected_ax & 0xff00) | profile
        OriginalGroundManeuverOracle.execute(machine, caller, 0xeff0, 0)
        assert tuple(machine.reg_read(r) for r in (
            UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)
            ) == (actor, 0x1c00, 0x1c00, 0x9002, 0)
        assert machine.reg_read(UC_X86_REG_AX) == expected_ax
        assert machine.reg_read(UC_X86_REG_BX) == (kind * 2 if profile is not None else 0x88)
        assert bytes(machine.mem_read(0, 0x60000)) == expected, (step, kind, caller)
        digest.update(struct.pack('<HB', caller, kind) + wanted)
        profiles.append(profile)
        cases += 1
assert cases == 384 and set(profiles) == {None, 0, 3}
receipt = {'success': True, 'cases': cases, 'manual_callers': list(callers),
    'scope': 'Actual allocated retained actors through three unchanged complete manual analog callbacks, full memory and AX/BX/DI/segments/stack predictions. No axis producer, whole a57a bank, C or full-class acceptance.',
    'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
root.joinpath('analog-drive-shared.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
