"""Actual c0e5 prefix/c105 traversal and complete declared class calls.

Unmuted voice stops at the real e2c2 producer boundary. The caller's post-device
instructions are checked separately; no audio service return is fabricated.
Traversal-only cases pause before class calls. Lifecycle cases execute the actual
c117 call through its real c11b return, preserving the original outer loop stack.
"""
import struct

from original_destruction_oracle import OriginalDestructionOracle
from original_object_pool_oracle import LONG_BASE, REGISTRY, SHORT_BASE
from original_unit_oracle import DGROUP, SERVICE_CS
from test_aircraft_death import effect_lines
from test_destruction import parent_lines, smoke_lines
from test_projectile_flight import line
from test_world_step import muzzle_lines


class OriginalWorldStepOracle(OriginalDestructionOracle):
    def __init__(self):
        super().__init__()
        self.updates = struct.unpack_from('<28H', self.image, DGROUP + 0xe454)
        if tuple(self.updates[kind] for kind in (4, 17, 18, 23, 26, 27)) != (
                0xbab4, 0x9b11, 0x9bc6, 0xbc0c, 0xbc46, 0xb355):
            raise AssertionError('Original lifecycle update table changed')
        self.metadata_cache = {}
        self.complete_class_calls = self.voice_commands = self.traversal_visits = 0

    def metadata(self, machine):
        # Cache only exact complete metadata/type identity, using the existing
        # canonical observer on every new key. No payload state is cached.
        short = bytes(machine.mem_read(DGROUP + SHORT_BASE, 150 * 55))
        long = bytes(machine.mem_read(DGROUP + LONG_BASE, 32 * 251))
        types = b''.join(short[n * 55:n * 55 + 2] for n in range(150))
        types += b''.join(long[n * 251:n * 251 + 2] for n in range(32))
        key = (bytes(machine.mem_read(DGROUP + 0xe294, 4)),
               bytes(machine.mem_read(DGROUP + 0xe2f7, 150)),
               bytes(machine.mem_read(DGROUP + 0xe38d, 32)),
               bytes(machine.mem_read(DGROUP + REGISTRY, 182 * 4)), types)
        if key not in self.metadata_cache: self.metadata_cache[key] = self.state(machine)
        return self.metadata_cache[key]

    def clock(self, machine):
        word = lambda address: struct.unpack('<H', machine.mem_read(DGROUP + address, 2))[0]
        return [word(0x6cde), word(0x969e), word(0x969c), word(0x9fd7), word(0x9fca),
                *machine.mem_read(DGROUP + 0x6da6, 3), machine.mem_read(DGROUP + 0x9fd6, 1)[0],
                machine.mem_read(DGROUP + 0x6ce6, 1)[0]]

    def shared_world(self, machine):
        words, cursor = self.random_state(machine)
        return line('clock', self.clock(machine)) + line('random', [cursor, *words]) + self.metadata(machine)

    def standalone(self, machine, entry, *, far=False):
        from unicorn.x86_const import UC_X86_REG_SP, UC_X86_REG_SS
        # Run fixture allocation commands on independent caller stack storage;
        # preserve the actual c105 loop frame, which remains paused at c117/c11b.
        saved_ss, saved_sp = machine.reg_read(UC_X86_REG_SS), machine.reg_read(UC_X86_REG_SP)
        machine.reg_write(UC_X86_REG_SS, 0x5000); machine.reg_write(UC_X86_REG_SP, 0xff0)
        machine.mem_write(0x50ff0, struct.pack('<HH', 0xeff0, 0))
        self.execute(machine, entry, 0xeff0, SERVICE_CS if far else 0)
        if machine.reg_read(UC_X86_REG_SP) != 0xff0 + (4 if far else 2):
            raise AssertionError('Original fixture command failed its complete return')
        machine.reg_write(UC_X86_REG_SS, saved_ss); machine.reg_write(UC_X86_REG_SP, saved_sp)

    def begin_tick(self, machine, timer):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DX, UC_X86_REG_ECX, UC_X86_REG_SP, UC_X86_REG_SS
        old_ss, old_sp = machine.reg_read(UC_X86_REG_SS), machine.reg_read(UC_X86_REG_SP)
        machine.reg_write(UC_X86_REG_SS, 0x5000); machine.reg_write(UC_X86_REG_SP, 0xff0)
        machine.mem_write(0x50ff0, struct.pack('<H', 0xeff0))
        machine.mem_write(DGROUP + 0x452, struct.pack('<H', timer))
        self.execute(machine, 0xc0e5, 0xbeb7)
        current = self.clock(machine)
        due = current[8] != 255 and current[0] == current[3]
        voice = 65535
        if due:
            self.execute(machine, 0xbeb7, 0xbefb)
            raw_voice = machine.reg_read(UC_X86_REG_AX) & 255
            if raw_voice != current[8]: raise AssertionError('Original scheduled voice differs')
            if current[9] == 2:
                self.execute(machine, 0xbefb, 0xc0ef)
            else:
                self.execute(machine, 0xbefb, 0xe2c2)
                voice = machine.reg_read(UC_X86_REG_AX)
                if (voice, machine.reg_read(UC_X86_REG_DX) & 255, machine.reg_read(UC_X86_REG_ECX)) != (
                        0x280 | raw_voice, 0, 0):
                    raise AssertionError('Original scheduled voice command contract differs')
                self.voice_commands += 1
                # Explicit post-device caller fragment, not an emulated audio
                # return: actual service playback is outside this producer API.
                self.execute(machine, 0xbece, 0xbedb)
        else:
            self.execute(machine, 0xbeb7, 0xc0ef)
        self.execute(machine, 0xc0ef, 0xc105)
        machine.reg_write(UC_X86_REG_SS, old_ss); machine.reg_write(UC_X86_REG_SP, old_sp)
        return int(due), voice

    def payload(self, machine, allocation):
        kind, slot, index, value = allocation
        address = SHORT_BASE + slot * 55
        raw = bytearray(machine.mem_read(DGROUP + address, 55))
        if kind in (23, 26, 27): return parent_lines(raw, allocation)
        if kind == 17: return smoke_lines(raw, allocation)
        if kind == 18: return muzzle_lines(raw, allocation)
        if kind == 4: return effect_lines((allocation, raw))
        raise AssertionError('Undeclared original class execution')

    def trace(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS, UC_X86_REG_SI, UC_X86_REG_SP, UC_X86_REG_SS
        machine = self.machine(case['seeds'], case['cursor'])
        self.standalone(machine, 0x1b176, far=True)
        values = case['clock']
        for address, value in zip((0x6cde, 0x969e, 0x969c, 0x9fd7, 0x9fca), values[:5]):
            machine.mem_write(DGROUP + address, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0x6da6, bytes(values[5:8]))
        machine.mem_write(DGROUP + 0x9fd6, bytes([values[8]])); machine.mem_write(DGROUP + 0x6ce6, bytes([values[9]]))
        machine.mem_write(DGROUP + 0x8b4f, bytes([case['enabled']]))
        machine.mem_write(DGROUP + 0x92f2, struct.pack('<2i', *case['wind']))
        visit, cursor, started, completed_call = (28, 65535, 182, 65535), 0, False, False
        owned = set()
        output = self.shared_world(machine)
        for op, low, kind, index, value, raw in case['commands']:
            status, due, voice = 0, 0, 65535
            if op == 0:
                self.standalone(machine, 0x1b176, far=True); owned = set()
            elif op in (1, 2):
                machine.reg_write(UC_X86_REG_AX, kind)
                machine.reg_write(UC_X86_REG_BX, index); machine.reg_write(UC_X86_REG_CX, value)
                self.standalone(machine, 0x1b1a2 if op == 2 else 0x1b1d6 if low else 0x1b1df, far=True)
                status = machine.reg_read(UC_X86_REG_EFLAGS) & 1
                if not status: owned.discard(machine.reg_read(UC_X86_REG_DI))
            elif op == 3:
                pointer = self.bindings(machine)[index][0]
                machine.reg_write(UC_X86_REG_AX, index); self.standalone(machine, 0x1b2ef, far=True)
                status = int(pointer == 0)
            elif op == 4:
                cursor, started, completed_call = 0, False, False
                machine.reg_write(UC_X86_REG_SS, 0x1c00); machine.reg_write(UC_X86_REG_SP, 0x9000)
                machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
            elif op == 5:
                any_live = any(pointer for pointer, _ in self.bindings(machine)[cursor:])
                if cursor == 182: status = 1
                else:
                    stop = 0xc117 if any_live else 0xc122
                    self.execute(machine, 0xc105 if not started else 0xc11b, stop)
                    started, completed_call = True, False
                    if not any_live: cursor = 182; status = 1
                    else:
                        pointer = machine.reg_read(UC_X86_REG_DI)
                        current = (machine.reg_read(UC_X86_REG_SI) - REGISTRY) // 4
                        typ = struct.unpack('<H', machine.mem_read(DGROUP + pointer, 2))[0]
                        actual_method = machine.reg_read(UC_X86_REG_BX)
                        if actual_method != typ * 2 or machine.reg_read(UC_X86_REG_CX) != 182 - current:
                            raise AssertionError('Original current-type/index traversal contract differs')
                        _, saved = self.bindings(machine)[current]
                        visit = typ, self.slot(pointer), current, saved; cursor = current + 1
                        self.traversal_visits += 1
            elif op == 6:
                due, voice = self.begin_tick(machine, index)
            elif op == 7:
                pointer, _ = self.bindings(machine)[index]
                if not pointer: raise AssertionError('Original restore requires a live fixture')
                saved = bytes(machine.mem_read(DGROUP + pointer + 2, 2))
                machine.mem_write(DGROUP + pointer, raw[:2] + saved + raw[4:]); owned.add(pointer)
            elif op == 8:
                typ, slot, current, saved = visit
                pointer = SHORT_BASE + slot * 55
                if pointer not in owned or self.bindings(machine)[current] != (pointer, saved):
                    raise AssertionError('Original fixture requested an undeclared/stale method')
                before = self.bindings(machine)
                old = bytes(machine.mem_read(DGROUP + pointer, 55))
                unrelated = [(address, bytes(machine.mem_read(DGROUP + address, 251 if self.type_flags[t] & 1 else 55)))
                             for address, t in self.physical(machine) if address != pointer]
                self.execute(machine, 0xc117, 0xc11b)
                completed_call = True; self.complete_class_calls += 1
                output += self.payload(machine, visit)
                created = self.allocations(machine, before)
                for entry in created:
                    strength = struct.unpack_from('<H', old, {23:33,26:28,27:29}[typ])[0]
                    output += self.initial_smoke(machine, entry, old, strength); owned.add(entry[0])
                allowed = {4:{22,25,28,29,32},17:{*range(4,14),22,25,26,27},18:{22,25,26,27},
                           23:{22,23,31,32},26:{22,28,29,30},27:{22,23,27,28,29,30}}[typ]
                updated = bytes(machine.mem_read(DGROUP + pointer, 55))
                if any(old[n] != updated[n] for n in range(55) if n not in allowed):
                    raise AssertionError('Original class changed an unrelated payload byte')
                if any(bytes(machine.mem_read(DGROUP + address, len(data))) != data for address,data in unrelated):
                    raise AssertionError('Original class changed another live/orphan payload')
            elif op == 9:
                pointer, _ = self.bindings(machine)[index]
                if pointer == 0: status = 1
                else: machine.mem_write(DGROUP + pointer, struct.pack('<H',kind)); owned.discard(pointer)
            else: raise AssertionError('C invalid-input fixtures have no original equivalent')
            output += line('result', [op,status,*visit,cursor,due,voice]) + self.shared_world(machine)
        return output

    def physical(self, machine):
        used = bytes(machine.mem_read(DGROUP + 0xe2f7,150)) + bytes(machine.mem_read(DGROUP+0xe38d,32))
        values=[]
        for slot,active in enumerate(used):
            if active:
                pointer = SHORT_BASE+slot*55 if slot<150 else LONG_BASE+(slot-150)*251
                typ=struct.unpack('<H',machine.mem_read(DGROUP+pointer,2))[0]
                values.append((pointer,typ))
        return values
