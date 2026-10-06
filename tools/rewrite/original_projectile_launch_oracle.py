"""Complete original M1 launch at a registered physical actor, without hooks."""
import struct

from original_object_pool_oracle import COUNT, LONG_BASE, REGISTRY, SHORT_BASE, SHORT_SLOTS, OriginalObjectPoolOracle
from original_unit_oracle import DGROUP



class OriginalProjectileLaunchOracle(OriginalObjectPoolOracle):
    def launch(self, raw, bindings, origin, steps, coarse, releases=()):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.fresh()
        actor = None
        for ordinal, (kind, index, value) in enumerate(bindings):
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, value)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise AssertionError('Original fixture import failed')
            if ordinal == origin:
                actor = machine.reg_read(UC_X86_REG_DI)
        if actor is None or len(raw) != 251 or struct.unpack_from('<H', raw)[0] or struct.unpack_from('<H', raw, 0x97)[0]:
            raise AssertionError('Complete untargeted registered M1 required')
        for index in releases:
            machine.reg_write(UC_X86_REG_AX, index)
            self.far_call(machine, 0x1b2ef)
        raw = bytearray(raw)
        struct.pack_into('<H', raw, 2, self.slot(actor) - SHORT_SLOTS)
        machine.mem_write(DGROUP + actor, bytes(raw))
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        # Keep dispatch at its nonselected-player boundary: no device/sample call.
        # The handler's actual AX=12 dispatch invocation still executes in full.
        machine.mem_write(DGROUP + 0x9fdf, bytes(2))
        observed = []
        for _ in range(steps):
            before = bytes(machine.mem_read(DGROUP + SHORT_BASE, REGISTRY - SHORT_BASE))
            used = bytes(machine.mem_read(DGROUP + 0xe2f7, SHORT_SLOTS))
            machine.reg_write(UC_X86_REG_DI, actor)
            self.far_call(machine, 0x17745)
            if machine.reg_read(UC_X86_REG_DI) != actor:
                raise AssertionError('Actual handler changed its origin')
            carry = machine.reg_read(UC_X86_REG_EFLAGS) & 1
            objects = []
            for index in range(COUNT):
                address, value = struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
                if address and address < LONG_BASE and not used[self.slot(address)]:
                    slot = self.slot(address)
                    objects.append((slot, index, value, bytes(machine.mem_read(DGROUP + address, 55))))
            after = bytes(machine.mem_read(DGROUP + SHORT_BASE, REGISTRY - SHORT_BASE))
            permitted = set()
            for slot, _, _, _ in objects:
                permitted.update(range(slot * 55, (slot + 1) * 55))
            permitted.update(range(actor - SHORT_BASE, actor - SHORT_BASE + 251))
            if any(left != right and offset not in permitted for offset, (left, right) in enumerate(zip(before, after))):
                raise AssertionError('Actual launch modified an unrelated arena payload')
            observed.append((bytes(machine.mem_read(DGROUP + actor, 251)), self.state(machine), objects, carry))
        return observed
