"""Complete original M1 launch at a registered physical actor, without hooks."""
import struct

from original_object_pool_oracle import COUNT, LONG_BASE, REGISTRY, SHORT_BASE, SHORT_SLOTS, OriginalObjectPoolOracle
from original_unit_oracle import DGROUP



class OriginalProjectileLaunchOracle(OriginalObjectPoolOracle):
    def launch(self, raw, bindings, origin, steps, coarse, releases=(), *, fire=False, tick=0, failed=0):
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
        if fire:
            machine.mem_write(DGROUP + 0x6da2, bytes(2))
            machine.mem_write(DGROUP + 0x9fce, struct.pack('<H', failed))
            if self.image[DGROUP + 0x8f06] != 12:
                raise AssertionError('Original primary successful-shot cue changed')
        observed = []
        for _ in range(steps):
            before = bytes(machine.mem_read(DGROUP + SHORT_BASE, REGISTRY - SHORT_BASE))
            used = bytes(machine.mem_read(DGROUP + 0xe2f7, SHORT_SLOTS))
            machine.reg_write(UC_X86_REG_DI, actor)
            current = bytes(machine.mem_read(DGROUP + actor, 251))
            requested = bool(current[0x92] or current[0x17] & 128)
            dispatched = requested and current[0xa8] == 0
            plates = [bytes(machine.mem_read(DGROUP + offset, 1)) for offset in (0x8e62, 0x8e66, 0x8e6a, 0x8e6e)]
            old_failed, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9fce, 2))
            if fire:
                from unicorn.x86_const import UC_X86_REG_SP
                machine.mem_write(DGROUP + 0x6cde, struct.pack('<H', tick))
                stack = machine.reg_read(UC_X86_REG_SP)
                self.execute(machine, 0x7c65, 0x7c7b, 0)
                if machine.reg_read(UC_X86_REG_SP) != stack:
                    raise AssertionError('Original complete fire branch changed its outer stack')
            else:
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
            actual = bytes(machine.mem_read(DGROUP + actor, 251))
            if fire:
                actual_plates = [bytes(machine.mem_read(DGROUP + offset, 1)) for offset in (0x8e62, 0x8e66, 0x8e6a, 0x8e6e)]
                if actual_plates != ([b'\x03'] * 4 if requested else plates):
                    raise AssertionError('Original complete fire branch weapon panel writes differ')
                end_failed, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9fce, 2))
                fired = any(int.from_bytes(record[:2], 'little') == 8 for _, _, _, record in objects)
                voice = 12 if fired else 13 if end_failed != old_failed else 255
                if voice != 255 and machine.reg_read(UC_X86_REG_AX) & 255 != voice:
                    raise AssertionError('Original actual voice-call byte differs')
                observed.append((actual, self.state(machine), objects, requested, dispatched, requested, voice, end_failed))
                tick = (tick + 1) % 65536
            else:
                observed.append((actual, self.state(machine), objects, carry))
        return observed
