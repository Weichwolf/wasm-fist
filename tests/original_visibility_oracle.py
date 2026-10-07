"""Complete unchanged PM op-58 returns on the original installed height plane."""
import struct

from original_ground_oracle import BUFFER, DGROUP, ROSTER, STACK, OriginalGroundOracle


class OriginalVisibilityOracle(OriginalGroundOracle):
    def prepare(self, side, pixels):
        from unicorn import UC_PROT_READ
        machine = super().prepare(side, pixels)
        machine.mem_write(0xc93, struct.pack('<I', ROSTER))
        machine.mem_protect(BUFFER, (len(pixels) + 4095) & ~4095, UC_PROT_READ)
        if (bytes(machine.mem_read(0xd0b, 4)) != struct.pack('<I', 0x1103) or
                bytes(machine.mem_read(0x8105, 1)) != bytes([side.bit_length() - 1]) or
                bytes(machine.mem_read(0x8109, 1)) != bytes([side.bit_length() - 1])):
            raise RuntimeError('Original visibility dispatch/installed detail differs')
        return machine

    def visible(self, machine, cases):
        from unicorn.x86_const import UC_X86_REG_EAX
        result = []
        for source, target in cases:
            machine.mem_write(ROSTER + 0xd2, struct.pack('<6i', *source, *target))
            before_image = bytes(machine.mem_read(0, len(self.image)))
            before_mailbox = bytes(machine.mem_read(ROSTER, 4096))
            before_dgroup = bytes(machine.mem_read(DGROUP, 65536))
            # call() installs the declared return before execution. The only
            # additional stack write is CALL 8030's actual return at +fec.
            before_stack = bytes(machine.mem_read(STACK, 4096))
            self.call(machine, 0x1103)
            value = machine.reg_read(UC_X86_REG_EAX)
            if value not in (0, 0xffffffff):
                raise RuntimeError('Original visibility returned a non-boolean DWORD')
            after_image = bytearray(machine.mem_read(0, len(self.image)))
            after_image[0x8020:0x802c] = before_image[0x8020:0x802c]
            after_stack = bytearray(machine.mem_read(STACK, 4096))
            after_stack[0xfec:0xff4] = before_stack[0xfec:0xff4]
            if (bytes(after_image) != before_image or
                    bytes(machine.mem_read(ROSTER, 4096)) != before_mailbox or
                    bytes(machine.mem_read(DGROUP, 65536)) != before_dgroup or
                    bytes(after_stack) != before_stack):
                raise RuntimeError('Original visibility changed undeclared memory')
            result.append(bool(value))
        return result
