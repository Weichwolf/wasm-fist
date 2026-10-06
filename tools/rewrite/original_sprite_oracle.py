"""Original unfiltered sprite scan, priority insertion and anchor sign instructions.

Integer source/destination steps are supplied after perspective preparation to
isolate the authored-resolution texel plane. The original mode-6 body executes
unmodified. Full perspective setup is separately entered for anchor sign proof;
mission palette interpolation and final scene projection are excluded.
"""
import hashlib
import struct

from original_model_oracle import OriginalModelOracle
from original_unit_oracle import DGROUP, ROOT

KERNEL_SHA256 = '102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1'
SOURCE = 0x30000
FRAME = 0x50000
TCB = 0x20000
STOP = 0x70000


class OriginalSpriteOracle:
    def __init__(self):
        self.original = OriginalModelOracle()
        self.image = (ROOT / 're_out/fist_image.bin').read_bytes()
        if hashlib.sha256(self.image).hexdigest() != KERNEL_SHA256:
            raise RuntimeError('Frozen original kernel image does not match its pin')
        self.cache = {}

    def kernel(self):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
        from unicorn.x86_const import UC_X86_REG_ESP
        machine = Uc(UC_ARCH_X86, UC_MODE_32)
        machine.mem_map(0, 0x80000)
        machine.mem_write(0, self.image)
        machine.reg_write(UC_X86_REG_ESP, 0x7fff0)
        return machine

    @staticmethod
    def word(machine, address, value):
        machine.mem_write(address, struct.pack('<H', value))

    @staticmethod
    def dword(machine, address, value):
        machine.mem_write(address, struct.pack('<I', value & 0xffffffff))

    def anchor(self, x, y):
        from unicorn.x86_const import (UC_X86_REG_ECX, UC_X86_REG_EDX,
                                      UC_X86_REG_ESI, UC_X86_REG_EIP)
        machine = self.kernel()
        self.dword(machine, 0xc93, TCB)
        self.word(machine, TCB + 0x480, 32768)
        self.word(machine, TCB + 0xca, 1)
        quotient = 0xffffffff // 32768
        factor = ((65536 << 20) + quotient - 1) // quotient
        self.dword(machine, 0x4224, factor)
        self.word(machine, TCB + 0x476, 128)
        self.word(machine, TCB + 0x478, 128)
        self.dword(machine, 0x9450, 0)  # Zero shear at the projection-input boundary.
        machine.reg_write(UC_X86_REG_ECX, 0x0101)
        machine.reg_write(UC_X86_REG_EDX, (x & 255) | ((y & 255) << 8))
        machine.reg_write(UC_X86_REG_ESI, SOURCE)
        machine.emu_start(0xad1e, 0xae40, count=1000)
        if machine.reg_read(UC_X86_REG_EIP) != 0xae40:
            raise RuntimeError('Original perspective anchor setup did not complete')
        actual_x = machine.reg_read(UC_X86_REG_ECX) & 65535
        actual_y = machine.reg_read(UC_X86_REG_EDX) & 65535
        inverse, = struct.unpack('<I', machine.mem_read(0xad0a, 4))
        if inverse != 1:
            raise RuntimeError('Original anchor setup did not select one-row source stepping')
        return actual_x - 128, actual_y - 128

    def sprite(self, sprite, mirrored):
        _, width, height, pixels = sprite
        key = (width, height, pixels, mirrored)
        if key in self.cache:
            return self.cache[key]
        from unicorn.x86_const import (UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI,
                                      UC_X86_REG_ESP, UC_X86_REG_EIP)
        machine = self.kernel()
        machine.mem_write(SOURCE, pixels)
        machine.mem_write(FRAME, bytes([37]) * 65536)
        self.dword(machine, 0x3918, FRAME)
        self.dword(machine, 0xad02, 0x60000)
        self.dword(machine, 0xacfa, SOURCE)
        self.dword(machine, 0xacfe, SOURCE + width * height)
        self.dword(machine, 0xace6, height)
        self.dword(machine, 0xad0e, height)
        self.dword(machine, 0xad0a, 1)
        self.dword(machine, 0xad16, 0)
        self.dword(machine, 0xad06, 0)
        self.dword(machine, 0xacf2, 0)
        self.dword(machine, 0xacee, 0)
        machine.mem_write(0xad1a, bytes((height, int(mirrored))))
        machine.reg_write(UC_X86_REG_ESI, SOURCE)
        machine.reg_write(UC_X86_REG_ECX, 0)
        machine.reg_write(UC_X86_REG_EDX, 0)
        stack = machine.reg_read(UC_X86_REG_ESP)
        self.dword(machine, stack, STOP)
        machine.emu_start(0xb31d, STOP, timeout=2_000_000, count=3_000_000)
        if machine.reg_read(UC_X86_REG_EIP) != STOP or machine.reg_read(UC_X86_REG_ESP) != stack + 4:
            raise RuntimeError('Original complete sprite scan did not return')
        frame = bytes(machine.mem_read(FRAME, 65536))
        expected = bytearray([37] * 65536)
        output = bytearray()
        for row in range(height):
            for column in range(width):
                source_column = width - column - 1 if mirrored else column
                value = pixels[source_column * height + row]
                if value:
                    expected[column * 256 + row] = value
                output.append(value)
        # Original framebuffer addressing is column-major (BH=X, BL=Y).
        if frame != expected:
            raise RuntimeError('Original column scan, mirror or transparent background differs')
        self.cache[key] = bytes(output)
        return self.cache[key]

    def order(self, priorities):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_CX, UC_X86_REG_ES, UC_X86_REG_SI
        machine = self.original.machine()
        machine.mem_write(DGROUP + 0x6adc, bytes(4))
        self.word(machine, DGROUP + 0x6ad6, 0x6ae4)
        for index, priority in enumerate(priorities):
            machine.reg_write(UC_X86_REG_AX, priority * 256)
            machine.reg_write(UC_X86_REG_CX, 0x6adc)  # The actual pre-search list predecessor.
            machine.reg_write(UC_X86_REG_ES, 0x4000)
            machine.reg_write(UC_X86_REG_SI, index)
            self.original.original.execute(machine, 0x2fcf, 0x2fd2)
            self.original.original.execute(machine, 0x2fd4, 0x2fff)
        pointer, = struct.unpack('<H', machine.mem_read(DGROUP + 0x6adc, 2))
        order = []
        while pointer:
            successor, _, index, segment = struct.unpack('<4H', machine.mem_read(DGROUP + pointer, 8))
            if segment != 0x4000 or index >= len(priorities) or index in order:
                raise RuntimeError('Original sorted part list is incomplete/cyclic')
            order.append(index)
            pointer = successor
        if len(order) != len(priorities):
            raise RuntimeError('Original sorting lost a part')
        return order
