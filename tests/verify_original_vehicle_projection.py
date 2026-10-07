#!/usr/bin/env python3
"""Execute original bearing, model/camera scale posting and sprite projection.

The new scene deliberately uses continuous atan2, softgl perspective and MAL
colors. These checks recover its world axes/scale and original scan-row order;
they do not claim complete original rasterization or gameplay initialization.
"""
import math
import struct

from original_model_oracle import OriginalModelOracle
from original_sprite_oracle import OriginalSpriteOracle, SOURCE, TCB, STOP
from original_unit_oracle import DGROUP, SERVICE_CS


def main():
    from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_SI,
                                  UC_X86_REG_DI, UC_X86_REG_SS, UC_X86_REG_SP,
                                  UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI,
                                  UC_X86_REG_EAX, UC_X86_REG_ESP,
                                  UC_X86_REG_EIP)
    original = OriginalModelOracle()
    sprite = OriginalSpriteOracle()
    vectors = [(0, 100, 0), (100, 100, 8192), (100, 0, 16384),
               (100, -100, 24576), (0, -100, 32768), (-100, -100, 40960),
               (-100, 0, 49152), (-100, 100, 57344), (0, 0, 0)]
    for origin in ((0, 0), (-1061797, 1816527), (583982, 1142557)):
        for x, y, expected in vectors:
            machine = original.machine()
            machine.reg_write(UC_X86_REG_SS, 0x1c00)
            machine.reg_write(UC_X86_REG_SP, 0x9000)
            machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            machine.mem_write(DGROUP + 0x7000, struct.pack('<4i', *origin,
                              origin[0] + x, origin[1] + y))
            machine.mem_write(DGROUP + 0x2040, b'\1')  # Actual 059a coarse branch.
            machine.reg_write(UC_X86_REG_SI, 0x7000)
            machine.reg_write(UC_X86_REG_DI, 0x7008)
            original.original.execute(machine, 0x731, 0xeff0)
            if machine.reg_read(UC_X86_REG_AX) != expected:
                raise RuntimeError('Original observer-to-object bearing axes differ')
            continuous = round(math.atan2(x, y) * 65536 / math.tau) % 65536
            if continuous != expected:
                raise RuntimeError('New continuous bearing uses different axes')

    machine = original.machine()
    original.original.execute(machine, 0x15f15, 0x15fc9, SERVICE_CS)
    machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
    original.original.execute(machine, 0xde70, 0xde85)
    camera_scale, = struct.unpack('<H', machine.mem_read(0x40000 + 0xca, 2))
    if camera_scale != 176:
        raise RuntimeError('Original TCB camera sprite scale differs')
    for code, expected in zip((4, 16, 28, 40), (272, 272, 336, 304)):
        machine.mem_write(DGROUP + 0x7005, bytes((code,)))
        machine.reg_write(UC_X86_REG_BX, 0x7000)
        original.original.execute(machine, 0x3ed4, 0x3ef0)
        scale, = struct.unpack('<H', machine.mem_read(0x40000 + 0x480, 2))
        if scale != expected:
            raise RuntimeError('Original selected C-model scale posting differs')
        for depth in (1 << 21, 1 << 22, 1 << 23):
            kernel = sprite.kernel()
            sprite.dword(kernel, 0xc93, TCB)
            sprite.word(kernel, TCB + 0x480, scale)
            sprite.word(kernel, TCB + 0xca, camera_scale)
            sprite.dword(kernel, 0x4224, depth)
            kernel.reg_write(UC_X86_REG_ECX, 0x0101)
            kernel.reg_write(UC_X86_REG_EDX, 0)
            kernel.reg_write(UC_X86_REG_ESI, SOURCE)
            kernel.emu_start(0xad1e, 0xada1, count=1000)
            inverse = kernel.reg_read(UC_X86_REG_EAX)
            expected_inverse = ((0xffffffff // (scale * camera_scale)) * depth) >> 20
            if kernel.reg_read(UC_X86_REG_EIP) != 0xada1 or inverse != expected_inverse:
                raise RuntimeError('Original complete perspective scale setup differs')
            kernel.emu_start(0xada1, 0xadd7, count=1000)
            vertical, = struct.unpack('<I', kernel.mem_read(0xad06, 4))
            horizontal, = struct.unpack('<I', kernel.mem_read(0xad16, 4))
            if horizontal != (vertical * 2) & 0xffffffff:
                raise RuntimeError('Original horizontal source step is not twice vertical')
            world_height = scale * camera_scale / 131072
            projected_height = world_height * (2**21 / depth)
            # The rewrite removes fixed-point division bias, not physical scale.
            quotient = 0xffffffff // (scale * camera_scale)
            rounding_bound = projected_height * (1 / quotient + 1 / inverse + 1 / 0xffffffff)
            if abs(65536 / inverse - projected_height) > rounding_bound:
                raise RuntimeError('Recovered world texel scale disagrees with instructions')

    results = []
    for height in (50, 51):
        kernel = sprite.kernel()
        sprite.dword(kernel, 0x90e0, 0)
        kernel.emu_start(0x6997, 0x69f1, count=1000)  # Actual heading-zero object basis.
        sprite.dword(kernel, 0x3a20, 2)
        kernel.mem_write(0x4624, struct.pack('<2I', 0xffffffff, 0xffffffff))
        kernel.mem_write(0x4224, struct.pack('<2I', 1 << 21, 1 << 21))
        kernel.mem_write(SOURCE, struct.pack('<2iH', 0, 65536, height << 8))
        kernel.mem_write(0x90df, bytes((100,)))
        kernel.reg_write(UC_X86_REG_ESI, SOURCE)
        kernel.reg_write(UC_X86_REG_EDX, 0)
        stack = kernel.reg_read(UC_X86_REG_ESP)
        sprite.dword(kernel, stack, STOP)
        kernel.emu_start(0xba7d, STOP, count=5000)
        if kernel.reg_read(UC_X86_REG_EIP) != STOP or kernel.reg_read(UC_X86_REG_EDX) == 0xffffffff:
            raise RuntimeError('Original complete object projection did not succeed/return')
        results.append(kernel.reg_read(UC_X86_REG_EAX) & 65535)
    if results != [128, 129]:
        raise RuntimeError('Original frame Y does not increase upward with world height')
    print('Original vehicle projection: 27 complete bearings, 4 posted C-model scales, '
          '12 perspective/step ratios, 2 complete height projections pass')


if __name__ == '__main__':
    main()
