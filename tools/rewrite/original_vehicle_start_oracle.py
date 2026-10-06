"""Run complete original c296/class/template initialization and 0291 RNG returns.

No instruction hooks, patches or replacement calls are used. Explicit caller
seeds and restored snapshot bytes are the boundary before original execution.
"""
import struct

from original_unit_oracle import DGROUP, OriginalUnitOracle


class OriginalVehicleStartOracle(OriginalUnitOracle):
    def __init__(self):
        super().__init__()
        self.verify_heading_fields()

    @staticmethod
    def machine_registers():
        from unicorn.x86_const import (UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SP,
                                      UC_X86_REG_SS)
        return UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_SP, UC_X86_REG_SS

    def machine(self, words, next_stream, link=0):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_16
        machine = Uc(UC_ARCH_X86, UC_MODE_16)
        machine.mem_map(0, 0x60000)
        machine.mem_write(0, self.image)
        _, ds, sp, ss = self.machine_registers()
        machine.reg_write(ds, 0x1c00)
        machine.reg_write(ss, 0x1c00)  # 0291 explicitly uses SS, not DS.
        machine.reg_write(sp, 0x9000)
        machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                          0x1f84 + ((next_stream + 3) % 4) * 2, *words))
        machine.mem_write(DGROUP + 0x6dae, bytes([link]))
        return machine

    def call(self, machine, entry):
        _, ds, sp, ss = self.machine_registers()
        machine.reg_write(sp, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        self.execute(machine, entry, 0xeff0)
        if (machine.reg_read(sp), machine.reg_read(ds), machine.reg_read(ss)) != (
                0x9002, 0x1c00, 0x1c00):
            raise RuntimeError('Original complete return/segment contract failed')

    @staticmethod
    def random_state(machine):
        cursor, *words = struct.unpack('<5H', machine.mem_read(DGROUP + 0x1f82, 10))
        if cursor not in (0x1f84, 0x1f86, 0x1f88, 0x1f8a):
            raise RuntimeError('Original random cursor escaped its four words')
        return words, ((cursor - 0x1f84) // 2 + 1) % 4

    def random(self, words, next_stream, *, sweep=False):
        from unicorn.x86_const import UC_X86_REG_AX
        machine = self.machine(words, next_stream)
        values = []
        for previous in range(65536):
            if sweep:
                machine.mem_write(DGROUP + 0x1f82, struct.pack('<H',
                                  0x1f84 + ((next_stream + 3) % 4) * 2))
                machine.mem_write(DGROUP + 0x1f84 + next_stream * 2,
                                  struct.pack('<H', previous))
            self.call(machine, 0x0291)
            value = machine.reg_read(UC_X86_REG_AX)
            if bytes(machine.mem_read(DGROUP + 0x0342, 2)) != struct.pack('<H', value):
                raise RuntimeError('Original RNG result and stored result disagree')
            values.append(value)
        return values, self.random_state(machine)

    def initialize(self, records, words, next_stream, link):
        machine = self.machine(words, next_stream, link)
        di, _, _, _ = self.machine_registers()
        observed = []
        for index, generation, snapshot in records:
            kind, = struct.unpack_from('<H', snapshot)
            if kind >= 4:
                continue
            machine.mem_write(DGROUP + 0x7000, snapshot)
            machine.reg_write(di, 0x7000)
            self.call(machine, 0xc296)
            if machine.reg_read(di) != 0x7000:
                raise RuntimeError('Original class initialization changed DI')
            state = bytes(machine.mem_read(DGROUP + 0x7000, 251))
            observed.append((index, generation, state))
        return observed, self.random_state(machine)

    def verify_heading_fields(self):
        # Actual complete turret wrappers identify which word is absolute and
        # which is hull-relative. A settled offset avoids target/aim updates.
        machine = self.machine((0, 0, 0, 0), 0)
        di, _, _, _ = self.machine_registers()
        for kind, entry in ((0, 0x7d0f), (1, 0x8917), (3, 0x9911)):
            for hull in (0, 65535, 49152):
                for offset in (16384, 65535, 32768):
                    raw = bytearray(251)
                    struct.pack_into('<H', raw, 0, kind)
                    struct.pack_into('<H', raw, 0x26, hull)
                    struct.pack_into('<HH', raw, 0x89, offset, offset)
                    machine.mem_write(DGROUP + 0x7000, bytes(raw))
                    machine.reg_write(di, 0x7000)
                    self.call(machine, entry)
                    struct.pack_into('<H', raw, 0x10, (hull + offset) % 65536)
                    raw[0xa9] = 0x80
                    if bytes(machine.mem_read(DGROUP + 0x7000, 251)) != bytes(raw):
                        raise RuntimeError('Original hull/absolute turret field contract failed')
