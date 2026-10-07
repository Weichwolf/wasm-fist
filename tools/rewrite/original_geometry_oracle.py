"""Complete original 0541 bearing/distance returns with owned caller coordinates."""
import math
import struct

from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle


class OriginalGeometryOracle(OriginalVehicleStartOracle):
    def __init__(self):
        super().__init__()
        self.angle_table = struct.unpack_from('<257H', self.image, DGROUP + 0x2448)
        authored = tuple(round(math.atan(index / 256) * 65536 * 8 / math.tau) % 65536
                         for index in range(257))
        if self.angle_table != authored:
            raise AssertionError('Original complete atan table differs from the independent formula')
        if self.image[0x541:0x54c] != bytes.fromhex('e8 ed 01 50 e8 df 03 8b c8 58 c3'):
            raise AssertionError('Original complete planar helper call chain differs')

    def cases(self, cases):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_CX, UC_X86_REG_DX, UC_X86_REG_SI, UC_X86_REG_DI
        machine = self.machine((1, 2, 32768, 65535), 3)
        results = []
        for source, target, coarse in cases:
            machine.mem_write(DGROUP + 0x7000, struct.pack('<2i', *source))
            machine.mem_write(DGROUP + 0x7010, struct.pack('<2i', *target))
            machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
            machine.reg_write(UC_X86_REG_SI, 0x7000)
            machine.reg_write(UC_X86_REG_DI, 0x7010)
            before = bytes(machine.mem_read(DGROUP, 65536))
            self.call(machine, 0x541)
            if (machine.reg_read(UC_X86_REG_SI), machine.reg_read(UC_X86_REG_DI)) != (0x7000, 0x7010):
                raise AssertionError('Original planar helper changed input identity')
            after = bytes(machine.mem_read(DGROUP, 65536))
            # Actual 0927 owns this ten-byte numeric scratch; call-stack writes
            # are declared separately. Every input/RNG/other DGROUP byte stays.
            offset = 0
            for begin, end in ((0x2034, 0x203e), (0x8fc0, 0x9002), (65536, 65536)):
                if before[offset:begin] != after[offset:begin]:
                    raise AssertionError('Original planar helper changed unrelated memory')
                offset = end
            results.append((machine.reg_read(UC_X86_REG_AX),
                            machine.reg_read(UC_X86_REG_CX) | (machine.reg_read(UC_X86_REG_DX) << 16)))
        return results
