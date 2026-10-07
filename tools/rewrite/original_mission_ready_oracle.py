"""Actual complete d755 reset pass with an explicit original op-54 device transfer.

Only the DOS saved-byte boundary and DOS/protected-mode register transfer are
host adapters. All allocation, loader continuation, registry/class methods and
kernel height instructions execute unchanged. No instruction hooks are used.
"""
import struct

from original_mission_orders_oracle import OriginalMissionOrdersOracle
from original_object_pool_oracle import REGISTRY
from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_unit_oracle import DGROUP

RESET_BANK = (0x7bcf, 0x8791, 0x8fde, 0x9787, *([0xc30f] * 12), 0xb51a,
              *([0xc30f] * 4), 0x9cd3, 0xc30f, 0xbc2c, 0xc30f, 0x9af6, 0xbcbf, 0xb433)
HEIGHT_TYPES = frozenset((21, 23, 25, 26, 27))


class OriginalMissionReadyOracle(OriginalProjectileFlightOracle):
    def __init__(self):
        super().__init__()
        self.orders = OriginalMissionOrdersOracle()
        if struct.unpack_from('<28H', self.image, DGROUP + 0xe668) != RESET_BANK:
            raise RuntimeError('Original complete mission-ready class bank changed')

    def prepare_saved(self, records, seeds, cursor, link, orders):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX,
                                      UC_X86_REG_DI, UC_X86_REG_EFLAGS)
        machine = self.machine(seeds, cursor, link)
        machine.mem_write(DGROUP + 0x6db4, bytes(2))  # Normal-side loading.
        self.far_call(machine, 0x1b176)
        self.call(machine, 0x4413)
        objects = {}
        counts = [0, 0]
        for index, generation, saved in records:
            kind = int.from_bytes(saved[:2], 'little')
            if kind >= 28 or len(saved) != (251 if self.type_flags[kind] & 1 else 55):
                raise ValueError('Saved allocation does not match the actual type bank')
            if kind >= 4 and kind != 23 and saved[22] & 32:
                raise ValueError('This corpus loader boundary requires ground-only participants')
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, generation)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise AssertionError('Original saved allocation failed')
            pointer = machine.reg_read(UC_X86_REG_DI)
            slot = self.slot(pointer)
            extended = int(bool(self.type_flags[kind] & 1))
            expected_slot = counts[extended] + (150 if extended else 0)
            if slot != expected_slot:
                raise AssertionError('Original fresh allocation did not use the next physical slot')
            constructor = bytes(machine.mem_read(DGROUP + pointer, len(saved)))
            if constructor != struct.pack('<HH', kind, counts[extended]) + bytes(len(saved) - 4):
                raise AssertionError('Original saved constructor changed')
            counts[extended] += 1
            if slot in objects:
                raise AssertionError('Original constructor reused a live physical record')
            machine.mem_write(DGROUP + pointer, saved[:2] + constructor[2:4] + saved[4:])
            self.call(machine, 0xd84a)
            objects[slot] = (index, generation, pointer, len(saved))
        self.orders.load(machine, *orders)
        return machine, objects

    @staticmethod
    def bindings(machine):
        return struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))

    def reset(self, machine, pixels, *, side=2):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_DS,
                                      UC_X86_REG_SP, UC_X86_REG_SS)
        bindings = self.bindings(machine)
        expected = [(index, pointer) for index, pointer in enumerate(bindings[::2])
                    if pointer and int.from_bytes(machine.mem_read(DGROUP + pointer, 2), 'little')
                    in HEIGHT_TYPES]
        # Declared caller counters and a separate DOS/PM mailbox. These are
        # inputs to the reset boundary, not a claim that full e006 is configured.
        for address in (0x930a, 0x9c89, 0x9ccb, 0x9ccd):
            machine.mem_write(DGROUP + address, bytes(2))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        entry = 0xd755
        transfers = []
        for index, pointer in expected:
            self.execute(machine, entry, 0xe1eb)
            near_position = machine.reg_read(UC_X86_REG_DI)
            if near_position != pointer + 4:
                raise AssertionError('Registry height-service order/position differs')
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != struct.pack('<H', 0x54):
                raise AssertionError('Original height wrapper selected another PM operation')
            position = bytes(machine.mem_read(DGROUP + near_position, 8))
            height = self.height(near_position, position, pixels, side=side)
            machine.reg_write(UC_X86_REG_AX, height)
            transfers.append((index, pointer, height))
            # CALL e339 is the declared device-transfer boundary. Resume the
            # actual e1d1 wrapper after that call, as in the accepted flight ABI.
            entry = 0xe1ee
        self.execute(machine, entry, 0xeff0)
        if (machine.reg_read(UC_X86_REG_SP), machine.reg_read(UC_X86_REG_DS),
                machine.reg_read(UC_X86_REG_SS)) != (0x9002, 0x1c00, 0x1c00):
            raise AssertionError('Complete mission-ready return/segment contract differs')
        return before, bytes(machine.mem_read(DGROUP, 65536)), transfers
