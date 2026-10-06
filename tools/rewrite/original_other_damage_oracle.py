"""Untouched original 5/6/23/26/27 M1 damage, reaching flight and effect cleanup.

Producer requests are observed before real configured device returns. Voice's
6ce6 byte is 2; sound 9 has an unloaded resource. No instruction hooks, patched
instructions or replacement methods execute. Later non-effect class updates
remain outside this damage contract.
"""
import struct

from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_unit_oracle import DGROUP
from original_vehicle_damage_oracle import OriginalVehicleDamageOracle
from test_other_damage import actor_lines
from test_projectile_flight import line


class OriginalOtherDamageOracle(OriginalVehicleDamageOracle):
    def __init__(self):
        super().__init__()
        actions = struct.unpack('<28H', self.image[DGROUP + 0xe550:DGROUP + 0xe588])
        if tuple(actions[index] for index in (5, 6, 23, 26, 27)) != (0xa0c8, 0xa0c8, 0xc335, 0xbd09, 0xb396):
            raise AssertionError('Original remaining-target damage methods changed')
        models = self.image[DGROUP + 0xe48c:DGROUP + 0xe48c + 28]
        if tuple(models[index] for index in (5, 6, 26, 27)) != (50, 52, 62, 66):
            raise AssertionError('Original byte-indexed model family assignment changed')
        for offset, expected in ((0x9c65, '14000a0900030400'),
                                 (0x9c6d, '14000a0b00050400'), (0x9c2d, '1000160a00080000')):
            if self.image[DGROUP + offset:DGROUP + offset + 8] != bytes.fromhex(expected):
                raise AssertionError('Original reached effect template changed')

    def prepare_other(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.machine(case['seeds'], case['cursor'])
        self.far_call(machine, 0x1b176)
        pointers = []
        for kind, index, value in case['bindings']:
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, value)
            self.far_call(machine, 0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS) & 1:
                raise AssertionError('Original fixture import failed')
            pointers.append(machine.reg_read(UC_X86_REG_DI))
        source, target = pointers[0], pointers[case['target']]
        raw = bytearray(case['raw'])
        raw[2:4] = machine.mem_read(DGROUP + target + 2, 2)
        machine.mem_write(DGROUP + target, bytes(raw))
        machine.mem_write(DGROUP + source + 4, bytes(raw[4:18]))
        machine.mem_write(DGROUP + source + 0x16, bytes([case['source_flags']]))
        machine.mem_write(DGROUP + source + 0x2a, b'\x05')
        machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H', *case['scales']))
        machine.mem_write(DGROUP + 0x9a25, struct.pack('<H', target))
        machine.mem_write(DGROUP + 0x9bd7, struct.pack('<H', case['aspect']))
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', 0 if case['selected'] == 65535 else pointers[case['selected']]))
        machine.mem_write(DGROUP + 0x6d3c, struct.pack('<32H', *[
            0 if ordinal == 65535 else pointers[ordinal] for ordinal in case['roster']]))
        machine.mem_write(DGROUP + 0x7a14, bytes(8))
        machine.mem_write(DGROUP + 0x73e, bytes([case['flash']]))
        for offset, value in zip((0x799e, 0x799a, 0x79a2, 0x79a0, 0x799c), case['counters']):
            machine.mem_write(DGROUP + offset, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0x9fdf, bytes(2))
        machine.mem_write(DGROUP + 0x9fea, b'\xff')
        machine.mem_write(DGROUP + 0x6ce6, b'\x02')
        return machine, pointers

    def reach(self, machine, pointers, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX
        source, target = pointers[0], pointers[case['target']]
        x, y, altitude = struct.unpack('<3i', machine.mem_read(DGROUP + target + 4, 12))
        def wrap(value):
            return (value + 2**31) % 2**32 - 2**31
        for pointer in pointers:
            if pointer not in (source, target):
                machine.mem_write(DGROUP + pointer + 4, struct.pack('<3i', wrap(x + 1000000), wrap(y + 1000000), altitude))
        machine.mem_write(DGROUP + source + 4, struct.pack('<3iH', x, wrap(y - 2556), altitude, 0))
        machine.mem_write(DGROUP + source + 0x1d, struct.pack('<3h', 0, 852, 0))
        machine.mem_write(DGROUP + source + 0x23, struct.pack('<H', 2))
        machine.mem_write(DGROUP + source + 0x27, struct.pack('<H', 0xc05c))
        machine.mem_write(DGROUP + 0xea2e, struct.pack('<H', 0x4000))
        machine.mem_write(DGROUP + 0xea2c, bytes(2))
        output = ''
        for _ in range(3):
            phase, hit = OriginalProjectileFlightOracle.flight(self, machine, source, {})
            raw = bytes(machine.mem_read(DGROUP + source, 55))
            output += line('flight', [phase, hit[0], hit[3], *struct.unpack_from('<3i', raw, 4),
                                      struct.unpack_from('<H', raw, 0x2d)[0], struct.unpack_from('<H', raw, 0x23)[0]])
        if phase != 2 or hit[0] != self.slot(target) or (
            machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_BX)) != (5, 0):
            raise AssertionError('Original flight did not reach the requested M1 target/profile/parameter')
        return output

    def write_shared(self, machine):
        selected, = struct.unpack('<H', machine.mem_read(DGROUP + 0x6d34, 2))
        counters = [struct.unpack('<H', machine.mem_read(DGROUP + address, 2))[0]
                    for address in (0x799e, 0x799a, 0x79a2, 0x79a0, 0x799c)]
        roster = struct.unpack('<32H', machine.mem_read(DGROUP + 0x6d3c, 64))
        sizes = struct.unpack('<4H', machine.mem_read(DGROUP + 0x7a14, 8))
        words, cursor = self.random_state(machine)
        return (line('combat', [self.slot(selected) if selected else 65535, machine.mem_read(DGROUP + 0x73e, 1)[0], *counters]) +
                line('roster', [self.slot(pointer) if pointer else 65535 for pointer in roster]) +
                line('platoons', sizes) + line('random', [cursor, *words]) + self.state(machine))

    def effect_state(self, machine, address, index, value):
        raw = bytes(machine.mem_read(DGROUP + address, 55))
        return line('effect', [self.slot(address), index, value, *struct.unpack_from('<3i3H', raw, 4),
                              *struct.unpack_from('<2H', raw, 0x1a), raw[0x19], raw[0x1e], raw[0x1f], raw[0x20], raw[0x16]])

    def trace(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI
        machine, pointers = self.prepare_other(case)
        source, target = pointers[0], pointers[case['target']]
        allocation = case['bindings'][case['target']]
        kind, index, value = allocation
        output = self.reach(machine, pointers, case) if case['reach'] else ''
        unrelated = [(pointer, bytes(machine.mem_read(DGROUP + pointer, 251 if self.type_flags[typ] & 1 else 55)))
                     for pointer, (typ, _, _) in zip(pointers, case['bindings']) if pointer != target]
        allowed = {5: {22, 23, 29, 30, 37, 50, 51}, 6: {22, 23, 29, 30, 37, 50, 51},
                   23: set(), 26: {20, 21, 22, 23, 25, 26, 28, 29},
                   27: {18, 19, 22, 23, 25, 26, 29, 30, 31, 32}}[kind]
        effects = []
        for _ in range(case['steps']):
            before = self.bindings(machine)
            old = bytes(machine.mem_read(DGROUP + target, 55))
            voices, sound, destruction_sound, selected_loss = self.dispatch(machine, source, voice_entries=(0xbefb,))
            if sound != 255 or selected_loss or len(voices) > 1:
                raise AssertionError('Unexpected remaining-target producer/UI request')
            raw = bytes(machine.mem_read(DGROUP + target, 55))
            if any(raw[pos] != old[pos] for pos in range(55) if pos not in allowed):
                raise AssertionError('Original damage changed an unrelated target payload field')
            if any(bytes(machine.mem_read(DGROUP + pointer, len(saved))) != saved for pointer, saved in unrelated):
                raise AssertionError('Original damage changed source or unrelated live/orphan payload')
            released = self.bindings(machine)[index][0] == 0
            destroyed = released or (kind in (5, 6) and old[37] != 12 and raw[37] == 12) or (
                kind == 26 and not old[25] & 4 and raw[25] & 4) or (kind == 27 and old[25] == 0 and raw[25] == 1)
            damage_offset = 51 if kind in (5, 6) else 26
            damage = (raw[damage_offset] - old[damage_offset]) % 256 if kind != 23 else 0
            created = self.created(machine, before)
            if len(created) > 1:
                raise AssertionError('Unexpected original remaining-target effect allocation count')
            output += line('result', [damage, int(bool(destroyed)), int(released), int(bool(created)),
                                      destruction_sound, voices[0] if voices else 255, 1])
            for address, entry, saved in created:
                output += self.effect(machine, address, entry, saved)
                effects.append((address, entry, saved))
            output += actor_lines(raw, (kind, self.slot(target), index, value)) + self.write_shared(machine)
            if destroyed or released:
                break
        if case['finish']:
            before = self.bindings(machine)
            machine.reg_write(UC_X86_REG_DI, source)
            machine.reg_write(UC_X86_REG_AX, 0x9c4d)
            self.call(machine, 0xba33)
            created = self.created(machine, before)
            output += line('impact', [int(bool(created)), 2, 15, 1])
            for address, entry, saved in created:
                output += self.effect(machine, address, entry, saved)
                effects.append((address, entry, saved))
            machine.reg_write(UC_X86_REG_DI, source)
            self.call(machine, 0xb6be)
            output += self.state(machine)
        initial_effects = [(pointer, bytes(machine.mem_read(DGROUP + pointer, 55))) for pointer, _, _ in effects]
        effect_writes = {22, 25, 28, 29, 32}
        for _ in range(case['effects']):
            for address, entry, saved in effects:
                if not machine.mem_read(DGROUP + address + 22, 1)[0] & 1:
                    machine.reg_write(UC_X86_REG_DI, address)
                    self.call(machine, 0xbab4)
                output += self.effect_state(machine, address, entry, saved)
            for address, initial in initial_effects:
                current = bytes(machine.mem_read(DGROUP + address, 55))
                if any(current[pos] != initial[pos] for pos in range(55) if pos not in effect_writes):
                    raise AssertionError('Original effect update changed an unrelated payload field')
            output += self.state(machine)
        return output
