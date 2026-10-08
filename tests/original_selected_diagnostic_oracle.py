"""Unchanged b152, its complete text services and genuine relocation setup.

No instruction hooks, patched instructions, fake callbacks or omitted services.
Only the declared DOS text surface and caller arguments are host-provided.
"""
import struct

from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_unit_oracle import DGROUP
from selected_diagnostic_contract import STACK_BEGIN, STACK_END, panel, parent, without_stack

TEXT_BYTES = 4000
NEAR_TABLE = 0x33520
NEAR_SECTION = 0x8c
VECTORS = {0x26e: 0x1b6, 0x284: 0x1ec, 0x286: 0x1f2, 0x298: 0x228}


class OriginalSelectedDiagnosticOracle(OriginalMissionReadyOracle):
    def __init__(self):
        super().__init__()
        if self.image[0xf692:0xf694] != struct.pack('<H', DGROUP // 16):
            raise AssertionError('Original service header has a different DGROUP')
        table = NEAR_TABLE + NEAR_SECTION
        if self.image[table:table + 2] != bytes(2):
            raise AssertionError('Original near relocation addend differs')
        self.near_pairs = []
        offset = table + 2
        while True:
            destination, = struct.unpack_from('<H', self.image, offset)
            offset += 2
            if destination == 0:
                break
            value, = struct.unpack_from('<H', self.image, offset)
            self.near_pairs.append((destination, value))
            offset += 2
            if len(self.near_pairs) > 23:
                raise AssertionError('Original near relocation section exceeds its pinned bounds')
        if len(self.near_pairs) != 23 or any(dict(self.near_pairs).get(a) != b for a, b in VECTORS.items()):
            raise AssertionError('Original text vectors differ')
        self.near_end = offset - NEAR_TABLE

    def relocated(self, data=None, *, text_segment=0x5000):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_ES,
                                      UC_X86_REG_SI)
        machine = self.machine()
        if data is not None:
            if len(data) != 65536:
                raise ValueError('A complete prepared DGROUP is required')
            machine.mem_write(DGROUP, data)
        before = bytes(machine.mem_read(0, 0x60000))
        machine.reg_write(UC_X86_REG_BX, NEAR_TABLE // 16)
        machine.reg_write(UC_X86_REG_SI, NEAR_SECTION)
        self.far_call(machine, 0xf7ef)
        expected = bytearray(before)
        for destination, value in self.near_pairs:
            expected[DGROUP + destination:DGROUP + destination + 2] = struct.pack('<H', value)
        after = bytes(machine.mem_read(0, 0x60000))
        lo, hi = DGROUP + 0x8ffa, DGROUP + 0x9004
        expected[lo:hi] = after[lo:hi]  # Declared complete far-call stack only.
        if after != bytes(expected):
            raise AssertionError('Genuine relocation changed memory outside its 23 destinations/stack')
        if (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_BX),
                machine.reg_read(UC_X86_REG_SI), machine.reg_read(UC_X86_REG_ES)) != (
                0, 0, self.near_end, DGROUP // 16):
            raise AssertionError('Genuine relocation return differs')
        if text_segment not in (0x4000, 0x5000):
            raise ValueError('Text storage must stay outside original code/data')
        machine.mem_write(DGROUP + 0x29e, struct.pack('<H', text_segment))
        return machine

    @staticmethod
    def screen_address(machine):
        return int.from_bytes(machine.mem_read(DGROUP + 0x29e, 2), 'little') * 16

    def observe(self, machine, actor, *, entry=0xb152, screen=None):
        from unicorn.x86_const import UC_X86_REG_DI
        address = self.screen_address(machine)
        if screen is not None:
            if len(screen) != TEXT_BYTES:
                raise ValueError('Incomplete original text surface')
            machine.mem_write(address, screen)
        machine.reg_write(UC_X86_REG_DI, actor)
        before = bytes(machine.mem_read(0, 0x60000))
        expected, output = (panel if entry == 0xb152 else parent)(
            before[DGROUP:DGROUP + 65536], actor, before[address:address + TEXT_BYTES])
        if entry not in (0xb152, 0xab03):
            raise ValueError('Only complete diagnostic or declared genuine parent entry')
        self.call(machine, entry)
        after = bytes(machine.mem_read(0, 0x60000))
        if machine.reg_read(UC_X86_REG_DI) != actor:
            raise AssertionError('Complete diagnostic/parent return lost the physical actor')
        data = after[DGROUP:DGROUP + 65536]
        if without_stack(data) != without_stack(expected):
            index = next(i for i, (a, b) in enumerate(zip(data, expected))
                         if a != b and not STACK_BEGIN <= i < STACK_END)
            raise AssertionError(f'Complete DGROUP differs at {index:#x}')
        if after[address:address + TEXT_BYTES] != output:
            raise AssertionError('Complete diagnostic character/attribute surface differs')
        whole = bytearray(before)
        whole[DGROUP:DGROUP + 65536] = expected
        whole[address:address + TEXT_BYTES] = output
        whole[DGROUP + STACK_BEGIN:DGROUP + STACK_END] = data[STACK_BEGIN:STACK_END]
        if bytes(whole) != after:
            raise AssertionError('Original code, external memory or text guard changed')
        return data, output
