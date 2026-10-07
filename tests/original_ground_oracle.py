"""Original map setup, complete 7fa0/8480 and 32-entry op 1c returns."""
import struct

from original_asset_oracle import OriginalAssetOracle

BUFFER = 0x1000000
STACK = 0x3000000
ROSTER = 0x3100000
DGROUP = 0x3200000
RETURN = 0x80000


def signed_word(value):
    return (value + 32768) % 65536 - 32768


class OriginalGroundOracle(OriginalAssetOracle):
    def prepare(self, side, pixels):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
        from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP
        if side < 1 or side > 4096 or side & (side - 1) or len(pixels) != side * side:
            raise ValueError('Expected a complete bounded power-of-two plane')
        machine = Uc(UC_ARCH_X86, UC_MODE_32)
        machine.mem_map(0, (len(self.image) + 4095) & ~4095)
        machine.mem_write(0, self.image)
        machine.mem_map(BUFFER, (len(pixels) + 4095) & ~4095)
        machine.mem_write(BUFFER, pixels)
        machine.mem_map(STACK, 4096)
        machine.mem_map(ROSTER, 4096)
        machine.mem_map(DGROUP, 65536)
        machine.mem_map(RETURN, 4096)
        machine.mem_write(0x85bc, struct.pack('<I', BUFFER))
        machine.mem_write(0xc99, struct.pack('<I', ROSTER))
        machine.mem_write(0xca1, struct.pack('<I', DGROUP))
        bits = side.bit_length() - 1
        machine.mem_write(0x8490, struct.pack('<III', bits, side, side * side))
        machine.reg_write(UC_X86_REG_EAX, bits)
        # Execute the original map-loader stores into all sampler immediates.
        machine.emu_start(0x8aa7, 0x8b0b, timeout=10_000_000, count=1000)
        if machine.reg_read(UC_X86_REG_EIP) != 0x8b0b:
            raise RuntimeError('Original installed-detail setup did not complete')
        return machine

    @staticmethod
    def call(machine, entry):
        from unicorn.x86_const import UC_X86_REG_EIP, UC_X86_REG_ESP
        machine.reg_write(UC_X86_REG_ESP, STACK + 0xff0)
        machine.mem_write(STACK + 0xff0, struct.pack('<I', RETURN))
        machine.emu_start(entry, RETURN, timeout=10_000_000, count=100_000)
        if (machine.reg_read(UC_X86_REG_EIP) != RETURN or
                machine.reg_read(UC_X86_REG_ESP) != STACK + 0xff4):
            raise RuntimeError('Original ground routine did not return completely')

    def samples(self, machine, poses):
        from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX,
                                      UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI)
        result = []
        for x, y, heading in poses:
            machine.reg_write(UC_X86_REG_EBX, (x << 13) % 4294967296)
            machine.reg_write(UC_X86_REG_EDX, (-y << 13) % 4294967296)
            machine.reg_write(UC_X86_REG_EAX, (-heading << 16) % 4294967296)
            self.call(machine, 0x7fa0)
            roll = signed_word(machine.reg_read(UC_X86_REG_EBX) >> 16)
            pitch = signed_word(machine.reg_read(UC_X86_REG_ECX) >> 16)
            machine.reg_write(UC_X86_REG_EBX, (x << 13) % 4294967296)
            machine.reg_write(UC_X86_REG_EDX, (-y << 13) % 4294967296)
            machine.reg_write(UC_X86_REG_ESI, BUFFER)
            self.call(machine, 0x8480)
            result.append((machine.reg_read(UC_X86_REG_EAX) % 256, roll, pitch))
        return result

    def contact(self, machine, records):
        observed = []
        for offset in range(0, len(records), 32):
            group = records[offset:offset + 32]
            slots = [0] * 32
            for index, (_, _, raw) in enumerate(group):
                slots[index] = (index + 1) * 256
                machine.mem_write(DGROUP + slots[index], raw)
            machine.mem_write(ROSTER, struct.pack('<32H', *slots))
            self.call(machine, 0x1109)
            for slot, (identity, generation, _) in zip(slots, group):
                observed.append((identity, generation, bytes(machine.mem_read(DGROUP + slot, 251))))
        return observed
