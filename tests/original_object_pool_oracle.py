"""Run actual original pool reset/import/allocation/release methods without hooks."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_motion_oracle import OriginalVehicleMotionOracle

SHORT_BASE, LONG_BASE, REGISTRY = 0xa022, 0xc05c, 0xdfbc
SHORT_SLOTS, LONG_SLOTS, COUNT = 150, 32, 182


class OriginalObjectPoolOracle(OriginalVehicleMotionOracle):
    def fresh(self):
        machine = self.machine()
        self.far_call(machine, 0x1b176)
        return machine

    @staticmethod
    def slot(pointer):
        if SHORT_BASE <= pointer < LONG_BASE and (pointer - SHORT_BASE) % 55 == 0:
            return (pointer - SHORT_BASE) // 55
        if LONG_BASE <= pointer < REGISTRY and (pointer - LONG_BASE) % 251 == 0:
            return SHORT_SLOTS + (pointer - LONG_BASE) // 251
        raise AssertionError(f'Original pointer {pointer:#x} is outside its actual arenas')

    def state(self, machine):
        counts = struct.unpack('<HH', machine.mem_read(DGROUP + 0xe294, 4))
        used = bytes(machine.mem_read(DGROUP + 0xe2f7, SHORT_SLOTS)) + bytes(
            machine.mem_read(DGROUP + 0xe38d, LONG_SLOTS))
        slots = []
        for index, active in enumerate(used):
            pointer = SHORT_BASE + index * 55 if index < SHORT_SLOTS else LONG_BASE + (index - SHORT_SLOTS) * 251
            kind, = struct.unpack('<H', machine.mem_read(DGROUP + pointer, 2))
            slots.append((active, kind if active else 0))
        registry = []
        for index in range(COUNT):
            pointer, value = struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
            registry.append((self.slot(pointer) if pointer else 65535, value))
        return (f'counts {counts[0]} {counts[1]}\nslots' + ''.join(f' {used}:{kind}' for used, kind in slots) +
                '\nregistry' + ''.join(f' {slot}:{value}' for slot, value in registry) + '\n')

    def trace(self, commands):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.fresh()
        result = self.state(machine)
        allocation = (28, 65535, 182, 65535)
        for operation, low_priority, kind, index, value in commands:
            status = 0
            if operation == 0:
                self.far_call(machine, 0x1b176)
            elif operation in (1, 2):
                machine.reg_write(UC_X86_REG_AX, kind)
                machine.reg_write(UC_X86_REG_BX, index)
                machine.reg_write(UC_X86_REG_CX, value)
                entry = 0x1b1a2 if operation == 2 else (0x1b1d6 if low_priority else 0x1b1df)
                self.far_call(machine, entry)
                status = machine.reg_read(UC_X86_REG_EFLAGS) & 1
                if not status:
                    pointer = machine.reg_read(UC_X86_REG_DI)
                    slot = self.slot(pointer)
                    registry_index = machine.reg_read(UC_X86_REG_AX)
                    saved_value, = struct.unpack('<H', machine.mem_read(DGROUP + REGISTRY + registry_index * 4 + 2, 2))
                    size = 251 if self.type_flags[kind] & 1 else 55
                    raw = bytes(machine.mem_read(DGROUP + pointer, size))
                    if raw != struct.pack('<HH', kind, slot - SHORT_SLOTS if slot >= SHORT_SLOTS else slot) + bytes(size - 4):
                        raise AssertionError('Actual constructor failed to initialize the full payload')
                    allocation = (kind, slot, registry_index, saved_value)
            elif operation == 3:
                pointer, value = struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
                status = int(pointer == 0)
                if pointer:
                    kind, = struct.unpack('<H', machine.mem_read(DGROUP + pointer, 2))
                    allocation = kind, self.slot(pointer), index, value
                    size = 251 if self.type_flags[kind] & 1 else 55
                    before = bytearray(machine.mem_read(DGROUP + pointer, size))
                    before[0x16] |= 1
                machine.reg_write(UC_X86_REG_AX, index)
                self.far_call(machine, 0x1b2ef)
                if pointer and bytes(machine.mem_read(DGROUP + pointer, size)) != bytes(before):
                    raise AssertionError('Actual release mutated payload beyond its deletion flag')
            else:
                raise ValueError('Original gate only accepts valid operations')
            result += f'result {status} ' + ' '.join(map(str, allocation)) + '\n' + self.state(machine)
        return result

    def exhaustion_corruption(self):
        """Prove the original's two writes beyond its fixed metadata/storage bounds."""
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.fresh()
        for index in range(LONG_SLOTS + 1):
            machine.reg_write(UC_X86_REG_AX, 0)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, 1)
            self.far_call(machine, 0x1b1a2)
        pointer = machine.reg_read(UC_X86_REG_DI)
        carry = machine.reg_read(UC_X86_REG_EFLAGS) & 1
        first_binding = bytes(machine.mem_read(DGROUP + REGISTRY, 4))
        if pointer != REGISTRY or carry or first_binding != struct.pack('<HH', 0, LONG_SLOTS):
            raise AssertionError('The observed 33rd extended allocation no longer reaches the registry overwrite')
        machine = self.fresh()
        machine.mem_write(DGROUP + REGISTRY, struct.pack('<HH', 0, 1) * COUNT)
        machine.reg_write(UC_X86_REG_AX, 8)
        self.far_call(machine, 0x1b1df)
        registry_index = machine.reg_read(UC_X86_REG_AX)
        if registry_index < COUNT or machine.reg_read(UC_X86_REG_EFLAGS) & 1:
            raise AssertionError('The observed exhausted registry no longer reaches its unbounded write')
        return pointer, registry_index
