"""Real M1 flight/capture/impact retirement/constructor reuse before tree contact."""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from physical_contact_contract import predict
from original_other_damage_oracle import OriginalOtherDamageOracle
from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_unit_oracle import DGROUP
from test_other_damage import fixture
from test_projectile_flight import line
from test_original_support_initialization import catalog, CATALOG, MAILBOX
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS, UC_X86_REG_EFLAGS)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--review-dir', type=Path, required=True)
REVIEW = parser.parse_args().review_dir.resolve()
if not REVIEW.is_relative_to(Path('/tmp')):
    parser.error('Disposable review evidence must live under /tmp')
REVIEW.mkdir(parents=True, exist_ok=True)
MODEL = Path(__file__).resolve().parents[2] / 'physical_contact_contract.py'
REGISTERS = (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DX,
    UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS)
owner = OriginalOtherDamageOracle()
flight_owner = OriginalProjectileFlightOracle()
assert flight_owner.image == owner.image
counts = collections.Counter()
digest = hashlib.sha256()
for kind, source_side in itertools.product(range(4), (0, 8)):
    case = fixture(kind=23, source_flags=source_side, selected=2, finish=0, reach=1,
                   bindings=[(8, 181, 1), (23, 5, 7), (kind, 180, 1)])
    machine, pointers = owner.prepare_other(case)
    source, wreck, actor = pointers
    record = bytearray(253)
    record[0x33], record[0x34] = 2, 255
    machine.mem_write(DGROUP + CATALOG, bytes(record))
    machine.mem_write(DGROUP + 0x6db6, bytes(2))
    machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, MAILBOX // 16))
    # Genuine catalog producer. The complete catalog model is already owned;
    # verify all memory before following its real scale words through flight.
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
    before = bytes(machine.mem_read(0, 0x60000))
    group, mailbox = catalog(before[DGROUP:DGROUP + 65536], before[MAILBOX:MAILBOX + 4096])
    expected = bytearray(before)
    expected[DGROUP:DGROUP + 65536] = group
    expected[MAILBOX:MAILBOX + 4096] = mailbox
    owner.far_call(machine, 0x1bf45)
    assert bytes(machine.mem_read(0, 0x60000)) == expected
    assert struct.unpack('<2H', machine.mem_read(DGROUP + 0xe3ae, 4)) == (3, 256)
    counts['catalog_producers'] += 1
    # Actual complete three-update original flight reaches type23 at its real
    # unit-impact boundary. Geometry and collision instructions are unchanged.
    flight = owner.reach(machine, pointers, case)
    expected_flight = ''.join(line('flight', [2 if tick == 3 else 0,
        owner.slot(wreck) if tick == 3 else 65535, 0, 0, -852 * (3 - tick),
        65536, tick, max(2 - tick, 0)]) for tick in range(1, 4))
    assert flight == expected_flight, (kind, source_side, flight, expected_flight)
    assert (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_BX)) == (5, 0)
    counts['complete_flight_updates'] += 3
    machine.reg_write(UC_X86_REG_DI, source)
    machine.reg_write(UC_X86_REG_SP, 0x9000)
    machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
    initial = [machine.reg_read(reg) for reg in REGISTERS]
    before = bytes(machine.mem_read(0, 0x60000))
    expected = bytearray(before)
    for offset, value in ((0xe3b2, source), (0x9bd9, 0), (0x9bdb, 5)):
        struct.pack_into('<H', expected, DGROUP + offset, value)
    expected[DGROUP + 0x7b1e] = 0
    expected[DGROUP + 0x87d5] = expected[DGROUP + 0x87bd] = 3
    struct.pack_into('<5H', expected, DGROUP + 0x8ff6, 0xc330, 0xc334, 0xbbc4, 0, source)
    abi = list(initial)
    abi[4], abi[9] = 46, 0x9002
    owner.execute(machine, 0xbbb7, 0xeff0)
    assert bytes(machine.mem_read(0, 0x60000)) == expected
    assert [machine.reg_read(reg) for reg in REGISTERS] == abi
    counts['genuine_capture_returns'] += 1
    # Normal allocation supplies a separate genuine type21 physical tree.
    machine.reg_write(UC_X86_REG_AX, 21)
    machine.reg_write(UC_X86_REG_BX, 6)
    machine.reg_write(UC_X86_REG_CX, 1)
    owner.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    tree = machine.reg_read(UC_X86_REG_DI)
    machine.mem_write(DGROUP + tree + 4, struct.pack('<2i', 10000, 10000))
    machine.mem_write(DGROUP + tree + 0x14, struct.pack('<H', 256))
    machine.mem_write(DGROUP + tree + 0x16, b'\x40')
    machine.mem_write(DGROUP + 0x930a, struct.pack('<H', 1))
    for stage in ('flight_capture', 'normal_impact_retirement', 'normal_successor_constructor'):
        if stage == 'normal_impact_retirement':
            # The existing complete actual impact owner checks its canonical
            # explosion payload, normal allocation and genuine source release.
            output = flight_owner.finish(machine, source, 2)
            assert output.startswith('impact 1 2 15 1\n')
            assert int.from_bytes(machine.mem_read(DGROUP + 0xdfbc + 181 * 4, 2), 'little') == 0
            assert machine.mem_read(DGROUP + source + 0x16, 1)[0] == source_side | 1
            assert machine.mem_read(DGROUP + 0xe2f7, 1)[0] == 0
            counts['complete_normal_impact_retirements'] += 1
        elif stage == 'normal_successor_constructor':
            machine.reg_write(UC_X86_REG_AX, 8)
            owner.far_call(machine, 0x1b1df)
            assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
            assert machine.reg_read(UC_X86_REG_DI) == source
            assert bytes(machine.mem_read(DGROUP + source, 55)) == struct.pack('<HH', 8, 0) + bytes(51)
            counts['genuine_normal_successor_constructors'] += 1
        assert struct.unpack('<H', machine.mem_read(DGROUP + 0xe3b2, 2))[0] == source
        baseline = bytes(machine.mem_read(0, 0x60000))
        for selected in (False, True):
            machine.mem_write(0, baseline)
            machine.mem_write(DGROUP + actor + 4, struct.pack('<2i', 10000, 10000))
            machine.mem_write(DGROUP + actor + 0x14, struct.pack('<H', 256))
            machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', 8))
            machine.mem_write(DGROUP + actor + 0x55, struct.pack('<2h', -3, -1))
            machine.mem_write(DGROUP + actor + 0x62, b'\0')
            machine.mem_write(DGROUP + actor + 0x93, b'\0')
            machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
            initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
                       actor, 0x1c00, 0x1c00, 0x9000, 0)
            for reg, value in zip(REGISTERS, initial):
                machine.reg_write(reg, value)
            machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if selected else struct.pack('<H', 0xeff0))
            entry = 0x1a0a4 if selected else 0xa631
            before = bytes(machine.mem_read(0, 0x60000))
            expected, abi, effect = predict(before, actor, entry, initial)
            owner.execute(machine, entry, 0xeff0, 0xf69 if selected else 0)
            actual = bytes(machine.mem_read(0, 0x60000))
            assert actual == expected, (kind, source_side, stage, 'memory')
            observed = [machine.reg_read(reg) for reg in REGISTERS]
            assert observed == abi, (kind, source_side, stage, observed, abi)
            assert effect['tree_calls'] == 1
            released = bool(source_side and stage != 'normal_successor_constructor')
            assert bool(effect['tree_released']) == released
            counts['contact_returns'] += 1
            counts['tree_released' if released else 'tree_retained'] += 1
            counts['reachable_reuse_changes_source_side'] += int(bool(source_side) and stage == 'normal_successor_constructor')
            digest.update(repr((kind, source_side, stage, selected)).encode())
            digest.update(actual)
            digest.update(struct.pack('<11H', *observed))
        machine.mem_write(0, baseline)
assert counts['contact_returns'] == 48
assert counts['genuine_capture_returns'] == 8
assert counts['complete_flight_updates'] == 24
assert counts['reachable_reuse_changes_source_side'] == 8
result = {'success': True, 'cases': counts['contact_returns'], 'counts': dict(counts), 'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(owner.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scope': 'Genuine original decoded-catalog producer then complete three-update M1 flight to persistent wreck, full-memory/eleven-register source-capture return, normal unit-impact/effect creation/source retirement and genuine normal successor constructor. Selected/unselected tree contacts after each source stage on all four classes match independently predicted entire image and eleven low words. Original cached pointer survives actual retirement and same-slot constructor reuse, causing an opposite-side source to change tree damage from101 to1 without a new capture. Consuming overlaps are declared caller poses, not proof of full battle scheduling. Admitted sound/PCM/shared C policy/full contact-family/game acceptance remain open.'}
(REVIEW / 'contact-source-flight-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
