"""Execute original DOS unit allocation, pose copies and normal-side roster writes.

No instruction hooks, patches or replacement implementations are used. The host
provides snapshot bytes at the DOS read boundary. Vehicle initialization c296 and
side swapping are deliberately outside this immutable-definition contract.
"""
import importlib.metadata
import struct

from reference_images import load_image

IMAGE_SHA256 = "d46f480dd2214b2693b4192a42aa79fb7bffb8d692fd67ddc3d746dfdea9c5e5"
DGROUP = 0x1c000
SERVICE_CS = 0xf69


class OriginalUnitOracle:
    def __init__(self):
        if importlib.metadata.version("unicorn") != "2.1.4":
            raise RuntimeError("Install the pinned optional oracle_requirements.txt")
        self.image = load_image('fist_dat_image.bin', IMAGE_SHA256)
        # Method/type flags, immediately followed by executable thunks in DGROUP.
        self.type_flags = self.image[DGROUP + 0xe614:DGROUP + 0xe614 + 28]

    @staticmethod
    def execute(machine, start, stop, code_segment=0):
        from unicorn.x86_const import UC_X86_REG_CS, UC_X86_REG_IP
        machine.reg_write(UC_X86_REG_CS, code_segment)
        machine.emu_start(start, stop, timeout=1_000_000, count=100_000)
        reached = machine.reg_read(UC_X86_REG_CS) * 16 + machine.reg_read(UC_X86_REG_IP)
        if reached != stop:
            raise RuntimeError(f"Original instructions stopped at {reached:#x}, wanted {stop:#x}")

    def definitions(self, records):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_16
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
                                      UC_X86_REG_DI, UC_X86_REG_DS, UC_X86_REG_EFLAGS,
                                      UC_X86_REG_SP, UC_X86_REG_SS)
        machine = Uc(UC_ARCH_X86, UC_MODE_16)
        machine.mem_map(0, 0x60000)
        machine.mem_write(0, self.image)
        machine.reg_write(UC_X86_REG_DS, 0x1c00)
        machine.reg_write(UC_X86_REG_SS, 0x5000)
        machine.reg_write(UC_X86_REG_SP, 0xff0)
        self.execute(machine, 0x1b176, 0x1b1a1, SERVICE_CS)  # Real pool/registry reset.
        self.execute(machine, 0x4413, 0x442f)  # Real 32-slot roster reset, before RET.
        pointers = {}
        observed = []
        for index, (registry_index, generation, snapshot) in enumerate(records):
            kind, saved_pool = struct.unpack_from("<HH", snapshot)
            expected_size = 251 if self.type_flags[kind] & 1 else 55
            if len(snapshot) != expected_size:
                raise RuntimeError("Snapshot does not match original type allocation flags")
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, registry_index)
            machine.reg_write(UC_X86_REG_CX, generation)
            self.execute(machine, 0x1b1a2, 0x1b1d5, SERVICE_CS)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise RuntimeError("Original allocation failed")
            pointer = machine.reg_read(UC_X86_REG_DI)
            constructor = bytes(machine.mem_read(DGROUP + pointer, expected_size))
            actual_kind, runtime_pool = struct.unpack_from("<HH", constructor)
            if actual_kind != kind or constructor[4:] != bytes(expected_size - 4):
                raise RuntimeError("Original constructor did not initialize the complete state")
            if pointer in pointers:
                raise RuntimeError("Original reused a live allocation unexpectedly")
            pointers[pointer] = index
            # d82e..d847: preserve newly allocated pool index around DOS state read.
            machine.mem_write(DGROUP + pointer + 2, snapshot[2:])
            machine.mem_write(DGROUP + pointer + 2, struct.pack("<H", runtime_pool))
            # Execute actual XY and Z/heading copies independently, for both pool classes.
            self.execute(machine, 0x1a866, 0x1a876, SERVICE_CS)
            self.execute(machine, 0x1a884, 0x1a892, SERVICE_CS)
            pose = struct.unpack("<3iH", bytes(machine.mem_read(DGROUP + 0x155a, 8)) +
                                 bytes(machine.mem_read(DGROUP + 0x1566, 4)) +
                                 bytes(machine.mem_read(DGROUP + 0x1558, 2)))
            runtime = bytes(machine.mem_read(DGROUP + pointer, expected_size))
            expected_runtime = snapshot[:2] + struct.pack("<H", runtime_pool) + snapshot[4:]
            if runtime != expected_runtime:
                raise RuntimeError("Original snapshot restoration changed unexpected bytes")
            observed.append((kind, saved_pool, pose, runtime[0x16], runtime[0x17], len(runtime)))
            if kind == 23:
                self.execute(machine, 0x43c1, 0x43e9)
            elif runtime[0x16] & 0x20:
                self.execute(machine, 0x43c1, 0x43ea)  # Actual participation branch.
                self.execute(machine, 0x4402, 0x4412)  # Actual assignment after init.
            else:
                self.execute(machine, 0x43c1, 0x43cc)  # Actual nonparticipant return.
        registry = []
        for slot in range(182):
            pointer, generation = struct.unpack("<HH", machine.mem_read(DGROUP + 0xdfbc + slot * 4, 4))
            registry.append((pointers[pointer], generation) if pointer else (65535, 0))
        roster = []
        for slot in range(32):
            pointer, = struct.unpack("<H", machine.mem_read(DGROUP + 0x6d3c + slot * 2, 2))
            roster.append(pointers[pointer] if pointer else 65535)
        return observed, registry, roster
