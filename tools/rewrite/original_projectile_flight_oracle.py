"""Actual flight stages, op-54 height handler, allocation and complete effect returns.

The DOS->protected-mode device gate is an explicit register-transfer boundary.
No instruction hooks or patched instructions replace a method. Unit flight stops
before actual damage dispatch; continuation starts after that separate boundary.
Notification/voice/audio device consumers are outside this oracle's scope.
"""
import struct

from original_ground_oracle import DGROUP as KERNEL_DGROUP, OriginalGroundOracle
from original_object_pool_oracle import REGISTRY, SHORT_BASE, OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from test_projectile_flight import line


class OriginalProjectileFlightOracle(OriginalObjectPoolOracle):
    @staticmethod
    def execute(machine, start, stop, code_segment=0):
        # Different observation stops share translated blocks across steps.
        # Drop cached blocks so Unicorn honors each new stop before dispatch.
        machine.ctl_remove_cache(0, 0x60000)
        OriginalObjectPoolOracle.execute(machine, start, stop, code_segment)

    def __init__(self):
        super().__init__()
        self.ground = OriginalGroundOracle()
        self.height_cache = {}
        # Actual update table and M1 templates, not adjacent routines/templates.
        updates = struct.unpack('<28H', self.image[DGROUP + 0xe454:DGROUP + 0xe454 + 56])
        if (updates[8], updates[4], updates[18]) != (0xb5e7, 0xbab4, 0x9bc6):
            raise AssertionError('Original dispatched update methods changed')
        if self.image[DGROUP + 0x9c1d:DGROUP + 0x9c25] != bytes.fromhex('1000160600030000'):
            raise AssertionError('Original type-8 ground template changed')
        if self.image[DGROUP + 0x9c4d:DGROUP + 0x9c55] != bytes.fromhex('14000a0500010400'):
            raise AssertionError('Original type-8 unit-hit template changed')
        ground_templates = struct.unpack('<28H', self.image[DGROUP + 0x9bdd:DGROUP + 0x9bdd + 56])
        if ground_templates[8] != 0x9c1d:
            raise AssertionError('Original type-8 ground template selection changed')

    def height(self, near_position, raw, heights, *, side=2):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_EAX
        key = (side, bytes(heights))
        if key not in self.height_cache:
            self.height_cache[key] = self.ground.prepare(side, key[1])
        machine = self.height_cache[key]
        machine.mem_write(KERNEL_DGROUP + near_position, raw)
        machine.reg_write(UC_X86_REG_DI, near_position)
        # Full actual op-54 handler uses DI, not the EBX inbox from the DOS prefix.
        self.ground.call(machine, 0x11a6)
        if machine.reg_read(UC_X86_REG_DI) != near_position:
            raise AssertionError('Actual op-54 handler failed to preserve DI')
        return machine.reg_read(UC_X86_REG_EAX) % 256

    def imports(self, machine, bodies):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        pointers = []
        for kind, index, value, x, y, altitude, heading, scale, flags, mode in bodies:
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, value)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise AssertionError('Actual original fixture import failed')
            pointer = machine.reg_read(UC_X86_REG_DI)
            pointers.append(pointer)
            machine.mem_write(DGROUP + pointer + 4, struct.pack('<3iH', x, y, altitude, heading))
            machine.mem_write(DGROUP + pointer + 0x14, struct.pack('<H', scale))
            machine.mem_write(DGROUP + pointer + 0x16, bytes([flags]))
            machine.mem_write(DGROUP + pointer + 0x19, bytes([mode]))
        return pointers

    def flight(self, machine, pointer, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_EFLAGS, UC_X86_REG_SP
        raw = bytes(machine.mem_read(DGROUP + pointer, 55))
        machine.reg_write(UC_X86_REG_DI, pointer)
        if (struct.unpack_from('<H', raw, 0x2d)[0] + 1) % 65536 >= 480:
            self.call(machine, 0xb5e7)
            return 3, (65535, 65535, 0, 0)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        # Complete age/tracking and integration prefix. Untargeted b74b returns normally.
        self.execute(machine, 0xb5e7, 0xb637)
        altitude, = struct.unpack('<i', machine.mem_read(DGROUP + pointer + 0xc, 4))
        if (altitude >> 8) % 65536 < 128:
            self.execute(machine, 0xb637, 0xe1c3)
            near_position = machine.reg_read(UC_X86_REG_DI)
            if near_position != pointer + 4:
                raise AssertionError('Actual DOS prefix did not pass the updated own position in DI')
            height = self.height(near_position, bytes(machine.mem_read(DGROUP + near_position, 8)), case['heights'])
            # e339 is the DOS/PM service-transfer boundary, not replaced code.
            machine.reg_write(UC_X86_REG_AX, height)
            self.execute(machine, 0xe1c6, 0xb641)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 128:
                self.execute(machine, 0xb641, 0xb691)
                return 1, (65535, 65535, 0, 0)
            self.execute(machine, 0xb641, 0xb643)
        else:
            self.execute(machine, 0xb637, 0xb643)
        grace, = struct.unpack('<H', machine.mem_read(DGROUP + pointer + 0x23, 2))
        if grace:
            self.execute(machine, 0xb643, 0xeff0)
            return 0, (65535, 65535, 0, 0)
        self.execute(machine, 0xb643, 0xb64c)
        origin, = struct.unpack('<H', machine.mem_read(DGROUP + pointer + 0x27, 2))
        target, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9a25, 2))
        if not machine.reg_read(UC_X86_REG_EFLAGS) & 1 or target == origin:
            self.execute(machine, 0xb64c, 0xeff0)
            return 0, (65535, 65535, 0, 0)
        self.execute(machine, 0xb64c, 0xb673)
        aspect, = struct.unpack('<H', machine.mem_read(DGROUP + 0x9bd7, 2))
        entry = next((index, value) for index in range(182)
                     for address, value in [struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))]
                     if address == target)
        return 2, (self.slot(target), *entry, aspect)

    @staticmethod
    def shell(machine, pointer, phase):
        raw = bytes(machine.mem_read(DGROUP + pointer, 55))
        x, y, altitude, heading = struct.unpack_from('<3iH', raw, 4)
        velocity = struct.unpack_from('<3h', raw, 0x1d)
        age, grace = struct.unpack_from('<H', raw, 0x2d)[0], struct.unpack_from('<H', raw, 0x23)[0]
        origin, = struct.unpack_from('<H', raw, 0x27)
        return line('shell', [x, y, altitude, heading, *velocity, age, grace, raw[0x16], raw[0x18],
                              raw[0x19], OriginalObjectPoolOracle.slot(origin), 65535, phase])

    @staticmethod
    def explosion(machine, pointer, allocation):
        raw = bytes(machine.mem_read(DGROUP + pointer, 55))
        x, y, altitude, model, extent, scale = struct.unpack_from('<3i3H', raw, 4)
        callback, height = struct.unpack_from('<2H', raw, 0x1a)
        return line('explosion', [*allocation, x, y, altitude, model, extent, scale,
                                  callback, height, raw[0x19], raw[0x1e], raw[0x1f], raw[0x20], raw[0x16]])

    def finish(self, machine, pointer, phase):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_SI
        before = [struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
                  for index in range(182)]
        machine.reg_write(UC_X86_REG_DI, pointer)
        machine.reg_write(UC_X86_REG_AX, 0x9c4d if phase == 2 else 0x9c1d)
        self.call(machine, 0xba33)
        after = [struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
                 for index in range(182)]
        created = [(index, address, value) for index, (address, value) in enumerate(after)
                   if (address, value) != before[index]]
        if len(created) > 1:
            raise AssertionError('Actual explosion creation changed multiple registry bindings')
        output = line('impact', [int(bool(created)), phase, 15, int(phase == 2)])
        if created:
            index, address, value = created[0]
            if address != machine.reg_read(UC_X86_REG_SI):
                raise AssertionError('Actual explosion allocation and initializer SI disagree')
            expected = bytearray(55)
            struct.pack_into('<HH', expected, 0, 4, self.slot(address))
            expected[4:16] = machine.mem_read(DGROUP + pointer + 4, 12)
            struct.pack_into('<3H', expected, 16, 20 if phase == 2 else 16,
                             256 if phase == 2 else 768, 2048)
            expected[0x1e:0x21] = bytes([10, 5, 5] if phase == 2 else [22, 6, 6])
            struct.pack_into('<H', expected, 0x1a, 4 if phase == 2 else 0)
            if bytes(machine.mem_read(DGROUP + address, 55)) != expected:
                raise AssertionError('Complete original explosion payload differs from actual template fields')
            output += self.explosion(machine, address, (4, self.slot(address), index, value))
        machine.reg_write(UC_X86_REG_DI, pointer)
        self.call(machine, 0xb6be)
        return output + self.shell(machine, pointer, 3)

    def trace(self, case):
        from unicorn.x86_const import UC_X86_REG_DI
        machine = self.machine(case['seeds'], case['cursor'])
        self.far_call(machine, 0x1b176)
        pointers = self.imports(machine, case['bodies'])
        pointer = pointers[0]
        kind = case['bodies'][0][0]
        _, index, value, *_ = case['bodies'][0]
        allocation = kind, self.slot(pointer), index, value
        operation = case['operation']
        if operation == 0:
            machine.mem_write(DGROUP + pointer + 0x1d, struct.pack('<3h', *case['velocity']))
            machine.mem_write(DGROUP + pointer + 0x23, struct.pack('<H', case['grace']))
            machine.mem_write(DGROUP + pointer + 0x27, struct.pack('<H', pointers[case['origin']]))
            machine.mem_write(DGROUP + pointer + 0x2d, struct.pack('<H', case['age']))
            machine.mem_write(DGROUP + pointer + 0x18, bytes([case['frame']]))
            machine.mem_write(DGROUP + 0xea2e, struct.pack('<H', 0x4000))
            machine.mem_write(DGROUP + 0xea2c, bytes(2))
        elif operation == 1:
            machine.mem_write(DGROUP + pointer + 0x12, struct.pack('<H', 512))
            machine.mem_write(DGROUP + pointer + 0x1a, struct.pack('<HH', case['grace'], case['age']))
            machine.mem_write(DGROUP + pointer + 0x19, bytes([case['frame']]))
            machine.mem_write(DGROUP + pointer + 0x1e, bytes([case['last'], case['period'], case['countdown']]))
        else:
            machine.mem_write(DGROUP + pointer + 0x1a, struct.pack('<H', case['age']))
            machine.mem_write(DGROUP + pointer + 0x19, bytes([case['frame']]))
        initial = bytes(machine.mem_read(DGROUP + pointer, 55))
        # All unrelated physical object records, including overwritten import orphans.
        unrelated = [(address, bytes(machine.mem_read(DGROUP + address,
                          251 if self.type_flags[case['bodies'][ordinal][0]] & 1 else 55)))
                     for ordinal, address in enumerate(pointers) if ordinal != 0]
        mutable = set(range(4, 16)) | {0x16, 0x18, 0x23, 0x24, 0x2d, 0x2e} if operation == 0 else (
                  {0x16, 0x19, 0x1c, 0x1d, 0x20} if operation == 1 else {0x16, 0x19, 0x1a, 0x1b})
        output = ''
        for _ in range(case['ticks']):
            phase = 0
            machine.reg_write(UC_X86_REG_DI, pointer)
            if operation == 0:
                phase, hit = self.flight(machine, pointer, case)
                output += line('hit', hit) + self.shell(machine, pointer, phase)
                words, cursor = self.random_state(machine)
                output += line('random', [cursor, *words])
                if case['finish'] and phase in (1, 2):
                    output += self.finish(machine, pointer, phase)
                    phase = 3
            elif operation == 1:
                self.call(machine, 0xbab4)
                output += self.explosion(machine, pointer, allocation)
            else:
                self.call(machine, 0x9bc6)
                raw = bytes(machine.mem_read(DGROUP + pointer, 55))
                counter, = struct.unpack_from('<H', raw, 0x1a)
                output += line('muzzle', [counter, raw[0x19], raw[0x16]])
            raw = bytes(machine.mem_read(DGROUP + pointer, 55))
            if any(raw[index] != initial[index] for index in range(55) if index not in mutable):
                raise AssertionError('Original update changed an unrelated source payload field')
            if any(bytes(machine.mem_read(DGROUP + address, len(saved))) != saved for address, saved in unrelated):
                raise AssertionError('Original update changed an unrelated world body')
            output += self.state(machine)
            if phase or raw[0x16] & 1:
                break
        return output
