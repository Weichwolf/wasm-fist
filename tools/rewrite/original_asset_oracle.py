"""Execute frozen original decoder instructions with already buffered input.

This is an independent data oracle, not a port of the KLC algorithm. No instruction
hooks replace decoding. DOS open/read/close, paging/refill, terrain resampling and
mission palette remapping are outside this buffered-decoder contract.
"""
import hashlib
import importlib.metadata
import pathlib
import struct

ROOT = pathlib.Path(__file__).resolve().parents[2]
IMAGE_SHA256 = "102a5465e03f3b0397041b1bd783d33992ac2c14bb6e42a713af610ec08c3ab1"
INPUT = 0x100000
NAME = 0x180000
OUTPUT = 0x200000
STACK = 0x300000


class OriginalAssetOracle:
    def __init__(self):
        if importlib.metadata.version("unicorn") != "2.1.4":
            raise RuntimeError("Install the pinned optional oracle_requirements.txt")
        self.image = (ROOT / "re_out/fist_image.bin").read_bytes()
        if hashlib.sha256(self.image).hexdigest() != IMAGE_SHA256:
            raise RuntimeError("Frozen original instruction image does not match its pin")

    def machine(self, data, output_size):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_PROT_READ
        from unicorn.x86_const import UC_X86_REG_ESP
        machine = Uc(UC_ARCH_X86, UC_MODE_32)
        machine.mem_map(0, (len(self.image) + 4095) & ~4095)
        machine.mem_write(0, self.image)
        machine.mem_map(INPUT, (len(data) + 4095) & ~4095)
        machine.mem_write(INPUT, data)
        machine.mem_protect(INPUT, (len(data) + 4095) & ~4095, UC_PROT_READ)
        machine.mem_map(OUTPUT, (output_size + 4095) & ~4095)
        machine.mem_map(STACK, 4096)
        machine.reg_write(UC_X86_REG_ESP, STACK + 0xff0)
        return machine

    @staticmethod
    def execute(machine, start, stop):
        from unicorn.x86_const import UC_X86_REG_EIP
        machine.emu_start(start, stop, timeout=10_000_000, count=10_000_000)
        if machine.reg_read(UC_X86_REG_EIP) != stop:
            raise RuntimeError(f"Original instructions did not complete at {stop:#x}")

    def klc(self, data):
        from unicorn.x86_const import UC_X86_REG_ESI
        width, height = struct.unpack_from("<II", data, 4)
        if data[:4] != b"KLC1" or not 0 < width * height <= 512 * 1024:
            raise RuntimeError("Oracle accepts the pinned valid original corpus only")
        machine = self.machine(data, width * height)
        machine.mem_write(0x5568, struct.pack("<I", INPUT + len(data)))
        machine.mem_write(0x556c, struct.pack("<I", OUTPUT))
        machine.reg_write(UC_X86_REG_ESI, INPUT)
        self.execute(machine, 0x646d, 0x6853)
        if machine.reg_read(UC_X86_REG_ESI) != INPUT + len(data):
            raise RuntimeError("Original decoder did not consume the entire KLC file")
        dimensions = struct.unpack("<II", machine.mem_read(0x5578, 8))
        if dimensions != (width, height):
            raise RuntimeError("Original decoded dimensions disagree with file metadata")
        palette = bytes(machine.mem_read(0x5598, 768))
        pixels = bytes(machine.mem_read(OUTPUT, width * height))
        return f"{width} {height}\n".encode("ascii") + palette + pixels

    def resource(self, data, name):
        from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_EAX
        machine = self.machine(data, 4096)
        machine.mem_map(NAME, 4096)
        machine.mem_write(NAME, name.encode("ascii") + b"\0")
        machine.reg_write(UC_X86_REG_ESI, NAME)
        self.execute(machine, 0x6250, 0x6289)  # Actual DOS name folding and XOR key.
        machine.mem_write(0x623c, struct.pack("<I", INPUT))
        self.execute(machine, 0x62dc, 0x6303)  # Actual resource header and count reads.
        machine.mem_write(0x623c, struct.pack("<I", INPUT + 16))
        self.execute(machine, 0x6331, 0x635f)  # Actual encoded-name directory search.
        self.execute(machine, 0x636a, 0x6375)  # Original selected payload offset.
        offset = machine.reg_read(UC_X86_REG_EAX)
        machine.mem_write(0x927, struct.pack("<I", OUTPUT))
        self.execute(machine, 0x6397, 0x63a6)  # Original next-offset member length.
        length, = struct.unpack("<I", machine.mem_read(OUTPUT + 0x1a, 4))
        if offset < 16 or length == 0 or offset + length > len(data):
            raise RuntimeError("Original selected an invalid pinned resource member")
        return bytes(machine.mem_read(INPUT + offset, length))
