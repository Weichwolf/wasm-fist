"""Unchanged a55d decoded record transfer; no platform device producer acceptance."""
import hashlib
import json
from pathlib import Path
import struct
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SS, UC_X86_REG_SP,
    UC_X86_REG_CS, UC_X86_REG_EFLAGS)

oracle = OriginalObjectPoolOracle()
assert oracle.image[0xa55d:0xa57a] == bytes.fromhex(
    '8a47038885a1008a47048885a2008a47058885a3008a47068885a400c3')
machine = oracle.fresh()
actors = []
for kind in range(4):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, kind)
    machine.reg_write(UC_X86_REG_CX, 1)
    oracle.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    actors.append(machine.reg_read(UC_X86_REG_DI))
record_pointer = 0x7000
assert all(not (a <= record_pointer + 7 and record_pointer < a + 251) for a in actors)
base = bytes(machine.mem_read(0, 0x60000))
digest = hashlib.sha256()
cases = 0
for actor in actors:
    for field in range(4):
        for value in range(256):
            record = bytearray((0x81, 0x23, 0xff, 0x55, 0xaa, 0x7f, 0x80))
            record[3 + field] = value
            machine.mem_write(0, base)
            machine.mem_write(DGROUP + record_pointer, bytes(record))
            machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            machine.reg_write(UC_X86_REG_AX, 0x1234)
            machine.reg_write(UC_X86_REG_BX, record_pointer)
            machine.reg_write(UC_X86_REG_DI, actor)
            machine.reg_write(UC_X86_REG_SP, 0x9000)
            expected = bytearray(machine.mem_read(0, 0x60000))
            expected[DGROUP + actor + 0xa1:DGROUP + actor + 0xa5] = record[3:7]
            OriginalGroundManeuverOracle.execute(machine, 0xa55d, 0xeff0, 0)
            assert tuple(machine.reg_read(r) for r in (
                UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_DI, UC_X86_REG_DS,
                UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)) == (
                0x1200 | record[6], record_pointer, actor, 0x1c00, 0x1c00, 0x9002, 0)
            assert bytes(machine.mem_read(0, 0x60000)) == expected
            digest.update(bytes(record) + bytes(expected[DGROUP + actor:DGROUP + actor + 251]))
            cases += 1
assert cases == 4096
receipt = {'success': True, 'cases': cases,
    'scope': 'Complete original decoded-record byte transfer at a55d on four actual allocated actors, every value of each source channel. Exact whole memory and AX/BX/DI/segments/stack; no keyboard/joystick/PM/device producer or full manual-bank acceptance.',
    'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(oracle.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
Path('/tmp/wasm-fist-0119-class-research/analog-input-copy.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps(receipt), flush=True)
