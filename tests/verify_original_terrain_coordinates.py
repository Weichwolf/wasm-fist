#!/usr/bin/env python3
"""Execute frozen original terrain sampling and camera setup contracts (optional oracle)."""
import struct

from original_asset_oracle import INPUT, OUTPUT, OriginalAssetOracle


def main():
    from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_EDX, UC_X86_REG_ESI
    oracle = OriginalAssetOracle()
    samples = [(583982, 1142557), (-(2**31), 2**31 - 1), (0, 0), (524287, -524287)]
    for x, y in samples:
        # The original samples its expanded 1024-square plane. Place a single
        # sentinel at the independently recovered address, retaining all other zeros.
        plane = bytearray(1024 * 1024)
        index = ((-y % 524288) // 512) * 1024 + (x % 524288) // 512
        plane[index] = 173
        machine = oracle.machine(bytes(plane), 4096)
        machine.reg_write(UC_X86_REG_EBX, (x << 13) & 0xffffffff)
        machine.reg_write(UC_X86_REG_EDX, (-y << 13) & 0xffffffff)
        machine.reg_write(UC_X86_REG_ESI, INPUT)
        oracle.execute(machine, 0x8480, 0x848d)
        if machine.reg_read(UC_X86_REG_EAX) & 255 != 173:
            raise RuntimeError(f"Original sampling disagrees at {x}, {y}")
    camera_cases = [(583982, 1142557, 44 * 256, 26729),
                    (-(2**31), 2**31 - 1, 0, 0), (524287, -524287, 0x8100, 65535)]
    for x, y, altitude, heading in camera_cases:
        machine = oracle.machine(bytes(256), 4096)
        tcb = bytearray(256)
        struct.pack_into("<3i", tcb, 0x2c, x, y, altitude)
        struct.pack_into("<H", tcb, 0x38, heading)
        struct.pack_into("<H", tcb, 0x3e, 256)
        machine.mem_write(OUTPUT, bytes(tcb))
        machine.mem_write(0xc93, struct.pack("<I", OUTPUT))
        oracle.execute(machine, 0x85d0, 0x8641)
        transformed = struct.unpack("<4I", machine.mem_read(0x90d4, 16))
        expected = ((x << 13) & 0xffffffff, (-y << 13) & 0xffffffff,
                    (min(altitude, 0x7f00) << 17) & 0xffffffff, (-heading << 16) & 0xffffffff)
        if transformed != expected:
            raise RuntimeError(f"Original camera setup disagrees: {transformed} != {expected}")
    # The kernel uses sin(-heading), cos(-heading) as the two terrain-ray axes.
    # Its cardinal sine table proves that heading zero follows decreasing map row.
    cardinal = struct.unpack_from("<i", oracle.image, 0x9450)[0], struct.unpack_from("<i", oracle.image, 0x9650)[0]
    if cardinal != (0, 0x7fffffff):
        raise RuntimeError("Original heading basis changed")
    print(f"Original terrain coordinates: {len(samples)} complete sampler cases, "
          f"{len(camera_cases)} complete camera cases, heading basis verified")


if __name__ == "__main__":
    main()
