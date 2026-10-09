"""Complete physical collision sound requests coupled to original read-only banks."""
import argparse
import collections
import hashlib
import itertools
import json
from pathlib import Path
import struct

from audio_request_contract import request
from physical_contact_contract import predict
from original_audio_request_oracle import OriginalAudioRequestOracle, BANK_BASES
from original_ground_maneuver_oracle import OriginalGroundManeuverOracle
from original_unit_oracle import DGROUP
from test_original_ground_maneuver import SEEDS
from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
    UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_BP, UC_X86_REG_DI, UC_X86_REG_DS,
    UC_X86_REG_SS, UC_X86_REG_SP, UC_X86_REG_CS, UC_X86_REG_EFLAGS, UC_X86_REG_EAX)

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
owner = OriginalGroundManeuverOracle()
audio = OriginalAudioRequestOracle()
for offset in (0, 4, 252):
    assert audio.effects_allocation(offset) == BANK_BASES['DSOUNDS.BIN'] + offset
machine = owner.machine(SEEDS, 0)
owner.far_call(machine, 0x1b176)
pointers = []
for index, kind in enumerate((0, 1, 2, 3, 21, 26, 8)):
    machine.reg_write(UC_X86_REG_AX, kind)
    machine.reg_write(UC_X86_REG_BX, index)
    machine.reg_write(UC_X86_REG_CX, 1)
    owner.far_call(machine, 0x1b1a2)
    assert not machine.reg_read(UC_X86_REG_EFLAGS) & 1
    pointers.append(machine.reg_read(UC_X86_REG_DI))
base = bytes(machine.mem_read(0, 0x60000))
assert struct.unpack_from('<HB', base, DGROUP + 0x9fe1 + 45) == (13, 0)
assert struct.unpack_from('<HB', base, DGROUP + 0x9fe1 + 48) == (270, 0)
counts = collections.Counter()
digest = hashlib.sha256()


def check(kind, selected, obstacle_kind, match, enabled, missing, placement, attenuation, packet=None):
    machine.mem_write(0, base)
    actor = pointers[kind]
    obstacle = pointers[4 if obstacle_kind == 21 else 5]
    source = pointers[-1]
    for p in pointers[4:6]:
        machine.mem_write(DGROUP + p + 0x16, b'\0')
    machine.mem_write(DGROUP + actor + 0x14, struct.pack('<H', 256))
    machine.mem_write(DGROUP + actor + 0x40, struct.pack('<H', 8))
    machine.mem_write(DGROUP + actor + 0x55, struct.pack('<2h', -3, -1))
    machine.mem_write(DGROUP + actor + 0x59, struct.pack('<2h', -1, 1))
    machine.mem_write(DGROUP + obstacle + 0x14, struct.pack('<H', 256))
    machine.mem_write(DGROUP + obstacle + 0x16, b'\x40')
    machine.mem_write(DGROUP + 0x930a, struct.pack('<H', 1))
    machine.mem_write(DGROUP + 0xe3ae, struct.pack('<3H', 3, 256, source))
    machine.mem_write(DGROUP + source + 0x16, bytes([8 if kind & 1 else 0]))
    machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor if selected else 0))
    machine.mem_write(DGROUP + 0x9fdf, struct.pack('<H', actor if match else 0))
    machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
    selector = 48 if obstacle_kind == 21 else 45
    if packet is None:
        packet = 270 if obstacle_kind == 21 else 13
    machine.mem_write(DGROUP + 0x9fe1 + selector, struct.pack('<HB', packet, attenuation))
    audio.place('DSOUNDS.BIN', placement)
    kernel_before, _ = audio.fixture(enabled=enabled, missing_effects=missing)
    initial = (0x1234, 0x88, 0x1111, 0x2222, 0x3333, 0x4444,
               actor, 0x1c00, 0x1c00, 0x9000, 0)
    for reg, value in zip(REGISTERS, initial):
        machine.reg_write(reg, value)
    machine.reg_write(UC_X86_REG_EFLAGS, 3)
    entry = 0x1a0a4 if selected else 0xa631
    machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0) if selected else struct.pack('<H', 0xeff0))
    before = bytes(machine.mem_read(0, 0x60000))
    expected, abi, effect = predict(before, actor, entry, initial, sound=True)
    if effect['audio_requests']:
        owner.execute(machine, entry, 0xe2db, 0xf69 if selected else 0)
        registers = {name: machine.reg_read(reg) for name, reg in audio.registers.items()}
        mailbox_bx = struct.unpack('<I', machine.mem_read(0x403f2, 4))[0]
        assert mailbox_bx == selector
        registers['ebx'] = mailbox_bx
        assert registers['eax'] & 65535 == effect['audio_packet'] == packet
        assert registers['edx'] & 255 == effect['audio_attenuation'] == attenuation >> 1
        assert registers['ecx'] == 0
        assert machine.mem_read(DGROUP + 0xea10, 2) == b'\x64\0'
        kernel_expected, register_expected, audio_effect = request(kernel_before, registers, audio.banks)
        for name, value in registers.items():
            audio.machine.reg_write(audio.registers[name], value)
        kernel_actual, returned, _ = audio.observe(kernel_before, registers)
        assert kernel_actual == kernel_expected and returned == register_expected
        # Actual returned EAX only, through the existing DOS/PM transfer ABI.
        machine.reg_write(UC_X86_REG_EAX, returned['eax'])
        owner.execute(machine, 0xe2de, 0xeff0)
        counts['actual_kernel_returns'] += 1
        counts['kernel_' + audio_effect['branch']] += 1
        digest.update(kernel_actual)
        digest.update(struct.pack('<7I', *returned.values()))
    else:
        owner.execute(machine, entry, 0xeff0, 0xf69 if selected else 0)
        counts['no_kernel_request_returns'] += 1
    actual = bytes(machine.mem_read(0, 0x60000))
    case = (kind, selected, obstacle_kind, match, enabled, missing, placement, attenuation, packet)
    assert actual == expected, (case, 'DOS memory', [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:24])
    observed = [machine.reg_read(reg) for reg in REGISTERS]
    assert observed == abi, (case, 'DOS ABI', observed, abi)
    counts['contact_returns'] += 1
    counts['matched_sources' if match else 'unmatched_sources'] += 1
    counts['tree_released'] += effect['tree_released']
    digest.update(repr(case).encode())
    digest.update(actual)
    digest.update(struct.pack('<11H', *observed))


for args in itertools.product(range(4), (False, True), (21, 26), (False, True),
        (0, 1, 255), (False, True), (0, 4, 252), (0, 1, 254, 255)):
    check(*args)
for value in range(256):
    check(value % 4, bool(value & 1), 21 if value & 2 else 26,
          True, 1, False, 0, value)
for kind, selected, obstacle_kind, packet in itertools.product(range(4),
        (False, True), (21, 26), (16, 255, 65535)):
    check(kind, selected, obstacle_kind, True, 1, False, 0, 255, packet=packet)
assert counts['contact_returns'] == 2608
assert counts['actual_kernel_returns'] > 0 and counts['no_kernel_request_returns'] > 0
assert all(counts['kernel_' + key] > 0 for key in ('disabled', 'bank_absent', 'direct'))
audio.verify_assets()
result = {'success': True, 'cases': counts['contact_returns'], 'counts': dict(counts), 'output_sha256': digest.hexdigest(),
    'image_sha256': hashlib.sha256(owner.image).hexdigest(),
    'fixture_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'model_sha256': hashlib.sha256(MODEL.read_bytes()).hexdigest(),
    'scope': 'Genuine complete selected/unselected contact with authored c047 selectors45/48, actual transport e2c2, unchanged kernel op64 and pinned read-only DSOUNDS. Independent whole DOS image/eleven low-word returns, entire kernel bytes/seven full general registers and bank guards. Actual kernel EAX response is transferred at the existing DOS/PM ABI boundary; it is not replaced with a sample address. All attenuation bytes, matched/unmatched source, disabled/enabled, absent/present bank, both caller selection, three aligned bank placements independently proved by actual complete original loader allocations and explicit malformed packet skip values are covered. Tree source pointer is an explicit allocated input here; real capture/retirement/reuse is a separate required gate. PCM/mixer/device initialization/shared C/full contact-family/game acceptance remain open.'}
(REVIEW / 'contact-sound-original.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result), flush=True)
