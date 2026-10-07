"""Complete original bc06/bed2 returns, including in-place pixels and dimensions.

The buffered KLC oracle supplies decoded inputs separately. These routines run
without replacement hooks or patched instructions in the pinned extender image.
"""
import struct

from original_asset_oracle import OriginalAssetOracle

BUFFER = 0x1000000
STACK = 0x3000000
RETURN = 0x80000
MAX_SIDE = 4096


class OriginalHeightfieldOracle(OriginalAssetOracle):
    def resample(self, side, pixels, targets):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
        from unicorn.x86_const import UC_X86_REG_EDI, UC_X86_REG_EIP, UC_X86_REG_ESP
        if not targets or len(pixels) != side * side:
            raise ValueError("Expected a complete square input and target sequence")
        maximum = max(side, *targets)
        if not 0 < maximum <= MAX_SIDE:
            raise ValueError("Oracle memory bound exceeded")
        for target in targets:
            smaller, larger = sorted((side, target))
            ratio, remainder = divmod(larger, smaller)
            if remainder or ratio & (ratio - 1):
                raise ValueError("Expected an exact power-of-two ratio")
        machine = Uc(UC_ARCH_X86, UC_MODE_32)
        machine.mem_map(0, (len(self.image) + 4095) & ~4095)
        machine.mem_write(0, self.image)
        machine.mem_map(RETURN, 4096)
        machine.mem_map(BUFFER, (maximum * maximum + 4095) & ~4095)
        machine.mem_write(BUFFER, pixels)
        machine.mem_map(STACK, 4096)
        machine.mem_write(0x5578, struct.pack("<II", side, side))
        for target in targets:
            while side != target:
                entry = 0xbc06 if side < target else 0xbed2
                expected_side = side * 2 if side < target else side // 2
                machine.reg_write(UC_X86_REG_EDI, BUFFER)
                machine.reg_write(UC_X86_REG_ESP, STACK + 0xff0)
                machine.mem_write(STACK + 0xff0, struct.pack("<I", RETURN))
                machine.emu_start(entry, RETURN, timeout=60_000_000,
                                  count=128 * max(side, expected_side)**2 + 4096)
                if (machine.reg_read(UC_X86_REG_EIP) != RETURN or
                        machine.reg_read(UC_X86_REG_ESP) != STACK + 0xff4):
                    raise RuntimeError("Original resampling routine did not return completely")
                dimensions = struct.unpack("<II", machine.mem_read(0x5578, 8))
                if dimensions != (expected_side, expected_side):
                    raise RuntimeError("Original resampling dimension publication differs")
                side = expected_side
        return side, bytes(machine.mem_read(BUFFER, side * side))
