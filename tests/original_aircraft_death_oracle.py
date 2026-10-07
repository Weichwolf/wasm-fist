"""Untouched 9e2b/a03f, actual op54 transfer, exact effects and lost-emitter evidence.

The original machine is never repaired or patched. Only observations of smoke
whose constructor demonstrably erased its retired emitter use captured emission
coordinates for the intentional C lifetime repair. Every other byte stays exact.
"""
import struct

from original_other_damage_oracle import OriginalOtherDamageOracle
from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_unit_oracle import DGROUP
from test_aircraft_death import effect_lines
from test_collision import delta
from test_destruction import smoke_lines
from test_other_damage import actor_lines
from test_projectile_flight import line


class OriginalAircraftDeathOracle(OriginalOtherDamageOracle):
    def __init__(self):
        super().__init__()
        updates = struct.unpack_from('<28H', self.image, DGROUP + 0xe454)
        if (updates[5], updates[6]) != (0x9e2b, 0x9e2b):
            raise AssertionError('Original aircraft updates changed')
        if struct.unpack_from('<H', self.image, 0x9f0f + 12)[0] != 0xa03f:
            raise AssertionError('Original byte-offset behavior-12 callback changed')
        self.flight = OriginalProjectileFlightOracle()
        self.repaired_emissions = 0

    def prepare(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.machine(case['seeds'], case['cursor'])
        self.far_call(machine, 0x1b176)
        pointers = []
        for kind, index, value in case['bindings']:
            machine.reg_write(UC_X86_REG_AX, kind); machine.reg_write(UC_X86_REG_BX, index); machine.reg_write(UC_X86_REG_CX, value)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1: raise AssertionError('Original import failed')
            pointers.append(machine.reg_read(UC_X86_REG_DI))
        for ordinal in case['releases']:
            if ordinal != 65535:
                machine.reg_write(UC_X86_REG_AX, case['bindings'][ordinal][1]); self.far_call(machine, 0x1b2ef)
        parent = pointers[case['target']]
        raw = bytearray(case['raw']); raw[2:4] = machine.mem_read(DGROUP + parent + 2, 2)
        machine.mem_write(DGROUP + parent, bytes(raw))
        machine.mem_write(DGROUP + 0x2040, bytes([case['coarse']]))
        machine.mem_write(DGROUP + 0x8b4f, bytes([case['enabled']]))
        machine.mem_write(DGROUP + 0x92f2, struct.pack('<2i', *case['wind']))
        machine.mem_write(DGROUP + 0xea2e, struct.pack('<H', 0x4000)); machine.mem_write(DGROUP + 0xea2c, bytes(2))
        machine.mem_write(DGROUP + 0x9fea, b'\xff'); machine.mem_write(DGROUP + 0x6da2, bytes(2))
        return machine, pointers

    def shared(self, machine):
        words, cursor = self.random_state(machine)
        return line('random', [cursor, *words]) + self.state(machine)

    def effect(self, machine, entry):
        pointer, index, value = entry
        return effect_lines(((4, self.slot(pointer), index, value), bytearray(machine.mem_read(DGROUP + pointer, 55))))

    def initialized_effect(self, machine, entry, source, template):
        pointer, _, _ = entry
        expected = bytearray(55); struct.pack_into('<HH', expected, 0, 4, self.slot(pointer))
        expected[4:16] = source[4:16]
        model, last, period, extent, callback = struct.unpack_from('<HBBHH', self.image, DGROUP + template)
        struct.pack_into('<3H', expected, 16, model, extent, 2048)
        struct.pack_into('<H', expected, 26, callback); expected[30:33] = bytes((last, period, period))
        if bytes(machine.mem_read(DGROUP + pointer, 55)) != bytes(expected):
            raise AssertionError('Original complete effect constructor differs')
        return self.effect(machine, entry)

    def smoke(self, machine, entry, correction):
        pointer, index, value = entry
        observed = bytearray(machine.mem_read(DGROUP + pointer, 55))
        if correction is not None:
            x, y, z = struct.unpack_from('<3i', observed, 4)
            corrected = (delta(x + correction[0], 0), delta(y + correction[1], 0),
                         delta((correction[2] & 0xffff0000) | ((z - 768 + correction[2]) & 65535), 0))
            struct.pack_into('<3i', observed, 4, *corrected)
        return smoke_lines(observed, (17, self.slot(pointer), index, value))

    def initialized_smoke(self, machine, entry, source, aliases):
        pointer, _, _ = entry
        observed = bytes(machine.mem_read(DGROUP + pointer, 55))
        extent, scale = struct.unpack_from('<2H', observed, 18)
        expected = bytearray(55); struct.pack_into('<HH', expected, 0, 17, self.slot(pointer))
        x, y, z = struct.unpack_from('<3i', source, 4)
        if aliases: x, y, z = 0, 0, 0
        struct.pack_into('<3iH2H', expected, 4, x, y, delta(z + 768, 0), 0, extent, scale)
        if observed != bytes(expected) or scale != extent * 4 or not 384 <= extent <= 447:
            raise AssertionError('Original complete smoke constructor differs beyond emitter loss')
        correction = None
        if aliases:
            self.repaired_emissions += 1
            cx, cy, cz = struct.unpack_from('<3i', source, 4)
            correction = cx, cy, delta(cz + 768, 0) % 2**32
        return self.smoke(machine, entry, correction), correction

    def advance(self, machine, parent, case, tick, animation):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_SP
        before = self.bindings(machine)
        machine.mem_write(DGROUP + 0x6cde, struct.pack('<H', tick)); machine.mem_write(DGROUP + 0x6d14, struct.pack('<H', animation))
        machine.reg_write(UC_X86_REG_DI, parent); machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        old = bytes(machine.mem_read(DGROUP + parent, 55))
        if tick & 3 == 0:
            self.execute(machine, 0x9e2b, 0xe1eb)
            near = machine.reg_read(UC_X86_REG_DI)
            if near != parent + 4: raise AssertionError('Original aircraft height position differs')
            height = self.flight.height(near, bytes(machine.mem_read(DGROUP + near, 8)), case['pixels'], side=case['side'])
            machine.reg_write(UC_X86_REG_AX, height); self.execute(machine, 0xe1ee, 0x9e66)
        else: self.execute(machine, 0x9e2b, 0x9e66)
        countdown = (struct.unpack_from('<H', old, 35)[0] - 1) % 65536
        has_callback = countdown & 31 == 0
        self.execute(machine, 0x9e66, 0x9ec8 if has_callback else 0x9ed2)
        sound = 255
        if has_callback:
            grounded = machine.mem_read(DGROUP + parent + 24, 1)[0] == 0
            if grounded:
                self.execute(machine, 0x9ec8, 0xbe8b)
                sound = machine.reg_read(UC_X86_REG_AX)
                if sound != 9: raise AssertionError('Original aircraft destruction sound differs')
                self.execute(machine, 0xbe8b, 0x9ed2)
            else: self.execute(machine, 0x9ec8, 0x9ed2)
        retired = bytes(machine.mem_read(DGROUP + parent, 55))
        effects = self.created(machine, before)
        effect_output = ''.join(self.initialized_effect(machine, entry, retired, 0x9c65) for entry in effects)
        kind, index, value = case['bindings'][case['target']]
        released = self.bindings(machine)[index][0] == 0
        before = self.bindings(machine)
        self.execute(machine, 0x9ed2, 0x9eef)
        smokes = self.created(machine, before)
        if len(smokes) > 1 or len(effects) > 1: raise AssertionError('Multiple aircraft emissions')
        aliases = bool(smokes and smokes[0][0] == parent)
        if aliases and not released: raise AssertionError('Smoke replaced a live aircraft')
        smoke_output, correction = self.initialized_smoke(machine, smokes[0], retired, aliases) if smokes else ('', None)
        replacement = bytes(machine.mem_read(DGROUP + parent, 55)) if aliases else None
        self.execute(machine, 0x9eef, 0xeff0)
        observed = retired if aliases else bytes(machine.mem_read(DGROUP + parent, 55))
        allowed = {*range(4, 12), 13, 16, 17, 22, 24, 26, 27, 28, 35, 36, 46, 47, 48}
        if any(observed[i] != old[i] for i in range(55) if i not in allowed):
            raise AssertionError('Unrelated aircraft payload byte changed')
        if aliases and bytes(machine.mem_read(DGROUP + parent, 55)) != replacement:
            raise AssertionError('Replacement smoke tail changed')
        output = line('tick', [tick, animation, 1, int(released), sound, int(bool(effects)), int(bool(smokes))])
        output += actor_lines(observed, (kind, self.slot(parent), index, value)) + effect_output + smoke_output
        return output, released, effects, smokes, correction

    def trace(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI
        machine, pointers = self.prepare(case); parent = pointers[case['target']]
        kind, index, value = case['bindings'][case['target']]
        allocation = kind, self.slot(parent), index, value
        excluded = {parent, *(pointers[ordinal] for ordinal in case['releases'] if ordinal != 65535)}
        if case['operation']: excluded.add(pointers[0])
        unrelated = [(pointer, bytes(machine.mem_read(DGROUP + pointer, 251 if self.type_flags[typ] & 1 else 55)))
                     for pointer, (typ, _, _) in zip(pointers, case['bindings']) if pointer not in excluded]
        output, smokes, effects, live = '', {}, {}, True
        raw = bytes(machine.mem_read(DGROUP + parent, 55))
        if case['operation']:
            source = pointers[0]; machine.mem_write(DGROUP + source + 4, raw[4:18]); machine.mem_write(DGROUP + source + 42, b'\x05')
            machine.mem_write(DGROUP + 0x9a25, struct.pack('<H', parent)); machine.mem_write(DGROUP + 0x9bd7, bytes(2))
            machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H', 256, 256)); machine.mem_write(DGROUP + 0x6d34, bytes(2))
            machine.mem_write(DGROUP + 0x6d3c, bytes(64)); machine.mem_write(DGROUP + 0x6ce6, b'\2')
            before = self.bindings(machine); saved_source = bytes(machine.mem_read(DGROUP + source, 55))
            voices, sound, destruction, selected = self.dispatch(machine, source, voice_entries=(0xbefb,))
            if voices or sound != 255 or selected or bytes(machine.mem_read(DGROUP + source, 55)) != saved_source:
                raise AssertionError('Unexpected original source/UI mutation')
            raw = bytes(machine.mem_read(DGROUP + parent, 55)); live = self.bindings(machine)[index][0] != 0
            output += line('damage', [(raw[51] - case['raw'][51]) % 256, int(not live or raw[37] == 12), int(not live), destruction])
            for entry in self.created(machine, before):
                output += self.initialized_effect(machine, entry, raw, 0x9c65); effects[self.slot(entry[0])] = entry
            before = self.bindings(machine); machine.reg_write(UC_X86_REG_DI, source); machine.reg_write(UC_X86_REG_AX, 0x9c4d)
            self.call(machine, 0xba33); created = self.created(machine, before)
            for entry in created:
                output += self.initialized_effect(machine, entry, saved_source, 0x9c4d); effects[self.slot(entry[0])] = entry
            output += line('impact', [int(bool(created))])
            machine.reg_write(UC_X86_REG_DI, source); self.call(machine, 0xb6be)
        output += actor_lines(raw, allocation) + self.shared(machine)
        for clock in range(case['ticks']):
            tick, animation = (case['tick'] + clock) % 65536, (case['animation'] + clock) % 65536
            if live:
                result, released, born_effects, born_smokes, correction = self.advance(machine, parent, case, tick, animation)
                output += result; live = not released
                for entry in born_effects: effects[self.slot(entry[0])] = entry
                for entry in born_smokes: smokes[self.slot(entry[0])] = entry, correction
            else: output += line('tick', [tick, animation, 0, 0, 255, 0, 0])
            for slot, (entry, correction) in sorted(list(smokes.items())):
                pointer, entry_index, _ = entry
                old = bytes(machine.mem_read(DGROUP + pointer, 55))
                machine.reg_write(UC_X86_REG_DI, pointer); self.call(machine, 0x9b11)
                current = bytes(machine.mem_read(DGROUP + pointer, 55))
                if any(current[i] != old[i] for i in range(55) if i not in {*range(4,14),22,25,26,27}):
                    raise AssertionError('Unrelated smoke payload byte changed')
                output += self.smoke(machine, entry, correction)
                if self.bindings(machine)[entry_index][0] == 0: del smokes[slot]
            for slot, entry in sorted(list(effects.items())):
                pointer, entry_index, _ = entry
                old = bytes(machine.mem_read(DGROUP + pointer, 55))
                machine.reg_write(UC_X86_REG_DI, pointer); self.call(machine, 0xbab4)
                current = bytes(machine.mem_read(DGROUP + pointer, 55))
                if any(current[i] != old[i] for i in range(55) if i not in {22,25,28,29,32}):
                    raise AssertionError('Unrelated effect payload byte changed')
                output += self.effect(machine, entry)
                if self.bindings(machine)[entry_index][0] == 0: del effects[slot]
            if any(bytes(machine.mem_read(DGROUP + pointer,len(saved))) != saved for pointer,saved in unrelated):
                raise AssertionError('Unrelated imported payload changed')
            output += self.shared(machine)
        return output
