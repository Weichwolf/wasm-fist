"""Actual M1 damage/destruction methods, observed without hooks or code patches.

Configured speech is busy, positional sound is unfocused and destruction sound's
resource is unloaded. Producer requests are observed before these device gates.
Selected fatal damage pauses at a97a and resumes at a990: the intervening player
UI/camera/takeover flow is an explicit, unimplemented consumer boundary.
"""
import struct

from original_object_pool_oracle import OriginalObjectPoolOracle, REGISTRY
from original_unit_oracle import DGROUP
from test_projectile_flight import line
from test_vehicle_damage import vehicle_lines


class OriginalVehicleDamageOracle(OriginalObjectPoolOracle):
    @staticmethod
    def execute(machine, start, stop, code_segment=0):
        machine.ctl_remove_cache(0, 0x60000)
        OriginalObjectPoolOracle.execute(machine, start, stop, code_segment)

    def __init__(self):
        super().__init__()
        actions = struct.unpack('<28H', self.image[DGROUP + 0xe550:DGROUP + 0xe588])
        if actions[:4] != (0xc336,) * 4:
            raise AssertionError('Original ground damage dispatch changed')
        for offset, expected in ((0x9c5d, '14000a07c0010400'), (0x9c3d, '1300150600030200')):
            if self.image[DGROUP + offset:DGROUP + offset + 8] != bytes.fromhex(expected):
                raise AssertionError('Original destruction template changed')
        if self.image[DGROUP + 0xe47a:DGROUP + 0xe47c] != bytes.fromhex('bac0'):
            raise AssertionError('Original type-19 update changed')

    @staticmethod
    def bindings(machine):
        return [struct.unpack('<HH', machine.mem_read(DGROUP + REGISTRY + index * 4, 4))
                for index in range(182)]

    def prepare(self, case):
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
        machine.mem_write(DGROUP + source + 4, bytes(raw[4:16]))
        machine.mem_write(DGROUP + source + 0x16, bytes([case['source_flags']]))
        machine.mem_write(DGROUP + source + 0x2a, b'\x05')
        machine.mem_write(DGROUP + 0x9a25, struct.pack('<H', target))
        machine.mem_write(DGROUP + 0x9bd7, struct.pack('<H', case['aspect']))
        machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H', *case['scales']))
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', target if case['selected'] else 0))
        machine.mem_write(DGROUP + 0x6d3c, struct.pack('<32H', *[
            0 if ordinal == 65535 else pointers[ordinal] for ordinal in case['roster']]))
        machine.mem_write(DGROUP + 0x73e, bytes([case['flash']]))
        for offset, value in zip((0x799e, 0x799a, 0x79a2), case['counters']):
            machine.mem_write(DGROUP + offset, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0x7a14, struct.pack('<4H', *case['sizes']))
        # Actual configured device gates; no handler instructions are substituted.
        machine.mem_write(DGROUP + 0x9fdf, bytes(2))
        machine.mem_write(DGROUP + 0x6da2, bytes(2))
        machine.mem_write(DGROUP + 0x9fea, b'\xff')
        return machine, pointers

    def dispatch(self, machine, source):
        from unicorn.x86_const import UC_X86_REG_CS, UC_X86_REG_IP, UC_X86_REG_DI, UC_X86_REG_SP, UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_DS, UC_X86_REG_SS
        exits = (0xbf3c, 0xc047, 0xbe8b, 0xa97a, 0xeff0)
        machine.reg_write(UC_X86_REG_CS, 0)
        machine.reg_write(UC_X86_REG_IP, 0xbbb7)
        machine.reg_write(UC_X86_REG_DI, source)
        machine.reg_write(UC_X86_REG_AX, 5)
        machine.reg_write(UC_X86_REG_BX, 0)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        voices, sounds, destroyed_sounds = [], [], []
        selected_loss = False
        machine.ctl_exits_enabled(True)
        machine.ctl_set_exits(exits)
        for _ in range(12):
            address = machine.reg_read(UC_X86_REG_CS) * 16 + machine.reg_read(UC_X86_REG_IP)
            machine.ctl_remove_cache(0, 0x60000)
            machine.emu_start(address, 0, timeout=1_000_000, count=100_000)
            reached = machine.reg_read(UC_X86_REG_CS) * 16 + machine.reg_read(UC_X86_REG_IP)
            if reached == 0xeff0:
                break
            if reached not in exits:
                raise AssertionError(f'Original damage stopped unexpectedly at {reached:#x}')
            value = machine.reg_read(UC_X86_REG_AX)
            if reached == 0xbf3c:
                voices.append(value % 256)
            elif reached == 0xc047:
                sounds.append(value)
            elif reached == 0xbe8b:
                destroyed_sounds.append(value)
            else:
                selected_loss = True
                # Whole balanced player UI/takeover block remains outside this contract.
                machine.reg_write(UC_X86_REG_IP, 0xa990)
                continue
            # Execute the real entry instruction once, then observe the next exit.
            machine.ctl_exits_enabled(False)
            machine.emu_start(reached, 0x60000, count=1)
            machine.ctl_exits_enabled(True)
            machine.ctl_set_exits(exits)
        else:
            raise AssertionError('Original damage did not complete')
        machine.ctl_exits_enabled(False)
        if (machine.reg_read(UC_X86_REG_SP), machine.reg_read(UC_X86_REG_DI),
            machine.reg_read(UC_X86_REG_DS), machine.reg_read(UC_X86_REG_SS)) != (0x9002, source, 0x1c00, 0x1c00):
            raise AssertionError('Actual damage return/segment/source contract failed')
        if len(sounds) > 1 or len(destroyed_sounds) > 1 or len(voices) > 3:
            raise AssertionError('Unexpected original producer requests')
        if (bytes(machine.mem_read(DGROUP + 0x7b1e, 1)), bytes(machine.mem_read(DGROUP + 0x87d5, 1)),
            bytes(machine.mem_read(DGROUP + 0x87bd, 1))) != (b'\0', b'\x03', b'\x03'):
            raise AssertionError('Original damage display refresh missing')
        return voices, sounds[0] if sounds else 255, destroyed_sounds[0] if destroyed_sounds else 255, selected_loss

    def created(self, machine, before):
        return [(address, index, value) for index, (address, value) in enumerate(self.bindings(machine))
                if (address, value) != before[index] and address]

    def effect(self, machine, address, index, value):
        raw = bytes(machine.mem_read(DGROUP + address, 55))
        pose = struct.unpack_from('<3i', raw, 4)
        model, extent, scale = struct.unpack_from('<3H', raw, 16)
        callback, height = struct.unpack_from('<2H', raw, 0x1a)
        expected = bytearray(55)
        struct.pack_into('<HH', expected, 0, 4, self.slot(address))
        expected[4:22] = raw[4:22]
        expected[0x19:0x21] = raw[0x19:0x21]
        if raw != expected or scale != 2048 or height != 0 or raw[0x19] or raw[0x20] != raw[0x1f]:
            raise AssertionError('Unexpected original complete initial explosion payload')
        if (model, extent, callback, raw[0x1e], raw[0x1f]) not in (
            (20, 448, 4, 10, 7), (19, 768, 2, 21, 6), (20, 256, 4, 10, 5)):
            raise AssertionError('Original effect differs from reached template')
        return line('effect', [self.slot(address), index, value, *pose, model, extent, scale,
                              callback, height, raw[0x19], raw[0x1e], raw[0x1f], raw[0x20], raw[0x16]])

    def wreck(self, machine, address, index, value, target_before):
        raw = bytes(machine.mem_read(DGROUP + address, 55))
        expected = bytearray(55)
        kind, = struct.unpack_from('<H', target_before)
        struct.pack_into('<HH', expected, 0, 23, self.slot(address))
        expected[4:16] = target_before[4:16]
        expected[16:18] = target_before[0x26:0x28]
        struct.pack_into('<H', expected, 0x14, 256)
        expected[0x16:0x18] = bytes([64, 4])
        struct.pack_into('<HH', expected, 0x1b, (10, 22, 34, 46)[kind], kind)
        struct.pack_into('<H', expected, 0x21, 768)
        expected[0x23:0x25] = target_before[0x1b:0x1d]
        if raw != expected:
            raise AssertionError('Complete original wreck payload differs')
        return line('wreck', [self.slot(address), index, value, *struct.unpack_from('<3iH', raw, 4),
                             *struct.unpack_from('<HH', raw, 0x1b), struct.unpack_from('<H', raw, 0x14)[0],
                             struct.unpack_from('<H', raw, 0x21)[0], raw[0x23], raw[0x24], raw[0x16], raw[0x17]])

    def combat(self, machine):
        selected, = struct.unpack('<H', machine.mem_read(DGROUP + 0x6d34, 2))
        counters = [struct.unpack('<H', machine.mem_read(DGROUP + offset, 2))[0]
                    for offset in (0x799e, 0x799a, 0x79a2)]
        roster = struct.unpack('<32H', machine.mem_read(DGROUP + 0x6d3c, 64))
        sizes = struct.unpack('<4H', machine.mem_read(DGROUP + 0x7a14, 8))
        words, cursor = self.random_state(machine)
        return (line('combat', [self.slot(selected) if selected else 65535,
                               machine.mem_read(DGROUP + 0x73e, 1)[0], *counters]) +
                line('roster', [self.slot(pointer) if pointer else 65535 for pointer in roster]) +
                line('platoons', sizes) + line('random', [cursor, *words]))

    def trace(self, case):
        machine, pointers = self.prepare(case)
        return self.trace_prepared(machine, pointers, case)

    def trace_prepared(self, machine, pointers, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI
        source, target = pointers[0], pointers[case['target']]
        kind, index, value = case['bindings'][case['target']]
        unrelated = [(p, bytes(machine.mem_read(DGROUP + p, 251 if self.type_flags[t] & 1 else 55)))
                     for p, (t, _, _) in zip(pointers, case['bindings']) if p != target]
        mutable = {0, 1, 0x16, 0x17, 0x19, 0x1a, 0x3a, 0x40, 0x41, 0x55, 0x56, 0x57, 0x58, 0x95}
        output = ''
        for _ in range(case['steps']):
            before = self.bindings(machine)
            old = bytes(machine.mem_read(DGROUP + target, 251))
            voices, sound, destruction_sound, selected = self.dispatch(machine, source)
            raw = bytes(machine.mem_read(DGROUP + target, 251))
            if any(raw[i] != old[i] for i in range(251) if i not in mutable):
                raise AssertionError('Original damage changed an unrelated target field')
            if any(bytes(machine.mem_read(DGROUP + p, len(saved))) != saved for p, saved in unrelated):
                raise AssertionError('Original damage changed the projectile/unrelated live or orphan record')
            destroyed = struct.unpack_from('<H', raw)[0] == 19
            effects, wrecks = [], []
            for address, entry, saved in self.created(machine, before):
                new_type, = struct.unpack('<H', machine.mem_read(DGROUP + address, 2))
                if new_type == 4:
                    effects.append(self.effect(machine, address, entry, saved))
                elif new_type == 23:
                    wrecks.append(self.wreck(machine, address, entry, saved, old))
                else:
                    raise AssertionError('Unexpected original destruction allocation')
            if len(effects) > 2 or len(wrecks) > 1:
                raise AssertionError('Unexpected number of destruction allocations')
            output += line('result', [(raw[0x3a] - old[0x3a]) % 256, int(destroyed), int(bool(wrecks)),
                                      int(selected), 1, sound, destruction_sound, len(effects), len(voices)])
            output += line('voices', voices).rstrip() + '\n' + ''.join(effects + wrecks)
            output += vehicle_lines(raw, kind, index, value) + self.combat(machine) + self.state(machine)
            if destroyed:
                break
        if case['finish']:
            before = self.bindings(machine)
            machine.reg_write(UC_X86_REG_DI, source)
            machine.reg_write(UC_X86_REG_AX, 0x9c4d)
            self.call(machine, 0xba33)
            created = self.created(machine, before)
            if len(created) > 1:
                raise AssertionError('Unexpected original impact allocation')
            output += line('impact', [int(bool(created)), 2, 15, 1])
            for address, entry, saved in created:
                output += self.effect(machine, address, entry, saved)
            machine.reg_write(UC_X86_REG_DI, source)
            self.call(machine, 0xb6be)
            output += self.state(machine)
        if destroyed:
            for _ in range(case['retire']):
                machine.reg_write(UC_X86_REG_DI, target)
                self.call(machine, 0xc0ba)
                remaining = machine.mem_read(DGROUP + target + 0x17, 1)[0]
                output += line('retire', [remaining, int(remaining == 0)]) + self.state(machine)
                if remaining == 0:
                    break
        return output

    def pipeline(self):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI
        from original_projectile_flight_oracle import OriginalProjectileFlightOracle
        from test_vehicle_damage import fixture
        from test_vehicle_start import state_lines
        machine = self.machine((0, 0, 0, 0), 0)
        self.far_call(machine, 0x1b176)
        actors = []
        for kind, index, pose in ((0, 3, (0, 0, 65536)), (2, 4, (0, 2556, 66560))):
            machine.reg_write(UC_X86_REG_AX, kind)
            machine.reg_write(UC_X86_REG_BX, index)
            machine.reg_write(UC_X86_REG_CX, 1)
            self.far_call(machine, 0x1b1a2)
            pointer = machine.reg_read(UC_X86_REG_DI)
            machine.mem_write(DGROUP + pointer + 4, struct.pack('<3i', *pose))
            machine.mem_write(DGROUP + pointer + 0x16, b'\x40')
            self.call(machine, 0xc296)
            actors.append(pointer)
        origin, target = actors
        machine.mem_write(DGROUP + 0x9fdf, bytes(2))
        machine.reg_write(UC_X86_REG_DI, origin)
        self.far_call(machine, 0x17745)
        bindings = self.bindings(machine)
        source, muzzle = bindings[0][0], bindings[1][0]
        if not source or not muzzle or struct.unpack('<H', machine.mem_read(DGROUP + source, 2))[0] != 8:
            raise AssertionError('Original reaching launch failed')
        output = 'pipeline_launch 0 1 12\n' + state_lines([(3, 1, bytes(machine.mem_read(DGROUP + origin, 251)))])
        for _ in range(3):
            phase, hit = OriginalProjectileFlightOracle.flight(self, machine, source, {})
            raw = bytes(machine.mem_read(DGROUP + source, 55))
            output += line('pipeline_flight', [phase, hit[0], hit[3], *struct.unpack_from('<3i', raw, 4),
                                               struct.unpack_from('<H', raw, 0x2d)[0],
                                               struct.unpack_from('<H', raw, 0x23)[0]])
        if phase != 2 or hit != (151, 4, 1, 0) or (
            machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_BX)) != (5, 0):
            raise AssertionError('Original reaching flight did not establish the M1 damage inputs')
        machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H', 256, 256))
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', origin))
        machine.mem_write(DGROUP + 0x6d3c, struct.pack('<32H', target, origin, *([0] * 30)))
        machine.mem_write(DGROUP + 0x73e, b'\0')
        machine.mem_write(DGROUP + 0x7a14, bytes(8))
        machine.mem_write(DGROUP + 0x6da2, bytes(2))
        machine.mem_write(DGROUP + 0x9fea, b'\xff')
        for offset in (0x799e, 0x799a, 0x79a2):
            machine.mem_write(DGROUP + offset, bytes(2))
        case = fixture(2, bindings=[(8, 0, 1), (0, 3, 1), (2, 4, 1), (18, 1, 1)], target=2)
        output += self.trace_prepared(machine, [source, origin, target, muzzle], case)
        effect_pointers = [bindings[0] for bindings in self.bindings(machine) if bindings[0] and
                           struct.unpack('<H', machine.mem_read(DGROUP + bindings[0], 2))[0] == 4]
        if len(effect_pointers) != 3:
            raise AssertionError('Original reaching destruction/impact did not produce all three effects')
        for _ in range(132):
            for pointer in (*effect_pointers, muzzle):
                if not machine.mem_read(DGROUP + pointer + 0x16, 1)[0] & 1:
                    machine.reg_write(UC_X86_REG_DI, pointer)
                    self.call(machine, 0x9bc6 if pointer == muzzle else 0xbab4)
        flags = [machine.mem_read(DGROUP + pointer + 0x16, 1)[0] for pointer in (*effect_pointers, muzzle)]
        return output + line('pipeline_effects', flags) + self.state(machine)
