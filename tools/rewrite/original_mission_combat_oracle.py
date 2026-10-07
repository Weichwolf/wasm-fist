"""Original consuming flight/damage/impact visits from a published-world boundary.

The fixture installs already-owned records and complete physical metadata, not
replacement methods. Flight, collision, damage, constructors, impact release and
supported effect/death/tree methods execute unchanged instructions. Device/UI
boundaries are those of the existing original damage/height oracles. Living
methods and aircraft death (which has a separately proved lifetime repair) are
outside this oracle. No original game tick or player UI run is claimed.
"""
import copy
import struct

from original_object_pool_oracle import LONG_BASE, REGISTRY, SHORT_BASE
from original_other_damage_oracle import OriginalOtherDamageOracle
from original_projectile_flight_oracle import OriginalProjectileFlightOracle
from original_unit_oracle import DGROUP
from test_object_pool import NONE
from test_projectile_flight import Pool

CENSUS = (0x799e, 0x799a, 0x79a2, 0x79a0, 0x799c)
METHODS = {4: 0xbab4, 17: 0x9b11, 18: 0x9bc6, 19: 0xc0ba,
           21: 0x9c4f, 23: 0xbc0c, 26: 0xbc46, 27: 0xb355}


def pointer(slot):
    if slot == NONE: return 0
    if not 0 <= slot < 182: raise ValueError('Physical slot is outside the original arenas')
    return SHORT_BASE + slot * 55 if slot < 150 else LONG_BASE + (slot - 150) * 251


class OriginalMissionCombatOracle(OriginalProjectileFlightOracle, OriginalOtherDamageOracle):
    def __init__(self):
        super().__init__()
        updates=struct.unpack_from('<28H',self.image,DGROUP+0xe454)
        if any(updates[kind]!=method for kind,method in METHODS.items()):
            raise AssertionError('Original delivered class dispatch table changed')
        self.class_calls=self.living_boundaries=self.continuations=0

    def prepare_world(self, world):
        machine = self.machine(world.words, world.cursor)
        self.far_call(machine, 0x1b176)
        for slot, (_, raw) in world.objects.items():
            machine.mem_write(DGROUP + pointer(slot), bytes(raw))
        machine.mem_write(DGROUP + 0xe2f7, bytes(used for used, _ in world.pool.slots[:150]))
        machine.mem_write(DGROUP + 0xe38d, bytes(used for used, _ in world.pool.slots[150:]))
        counts = [sum(used for used, _ in arena) for arena in (world.pool.slots[:150], world.pool.slots[150:])]
        machine.mem_write(DGROUP + 0xe294, struct.pack('<2H', *counts))
        machine.mem_write(DGROUP + REGISTRY, b''.join(struct.pack('<HH', pointer(slot), value) for slot, value in world.pool.registry))
        machine.mem_write(DGROUP + 0xe3ae, struct.pack('<2H', *world.scales))
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', pointer(world.selected)))
        machine.mem_write(DGROUP + 0x6d3c, struct.pack('<32H', *(pointer(slot) for slot in world.roster)))
        machine.mem_write(DGROUP + 0x7a14, struct.pack('<4H', *world.sizes))
        machine.mem_write(DGROUP + 0x73e, bytes([world.flash]))
        for address, value in zip(CENSUS, world.counters):
            machine.mem_write(DGROUP + address, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0x9fdf, bytes(2))
        machine.mem_write(DGROUP + 0x6da2, bytes(2))
        machine.mem_write(DGROUP + 0x9fea, b'\xff')
        machine.mem_write(DGROUP + 0x6ce6, b'\x02')
        machine.mem_write(DGROUP + 0xea2e, struct.pack('<H', 0x4000))
        machine.mem_write(DGROUP + 0xea2c, bytes(2))
        machine.mem_write(DGROUP + 0x8b4f, bytes([world.enabled]))
        machine.mem_write(DGROUP + 0x92f2, struct.pack('<2i', *world.wind))
        machine.mem_write(DGROUP + 0x930d, bytes([world.trees[0]]))
        machine.mem_write(DGROUP + 0x930c, bytes([world.trees[1]]))
        return machine

    def observe_world(self, machine, world):
        observed = copy.copy(world)
        observed.pool = Pool([])
        used = bytes(machine.mem_read(DGROUP + 0xe2f7, 150)) + bytes(machine.mem_read(DGROUP + 0xe38d, 32))
        observed.pool.slots = [(active, struct.unpack('<H', machine.mem_read(DGROUP + pointer(slot), 2))[0] if active else 0)
                               for slot, active in enumerate(used)]
        observed.pool.registry = [(self.slot(address) if address else NONE, value) for address, value in self.bindings(machine)]
        if self.state(machine)!=observed.pool.state():
            raise AssertionError('Original complete counts, used maps, types and registry disagree')
        observed.objects = {}
        for slot, (active, kind) in enumerate(observed.pool.slots):
            if not active: continue
            bindings = [(index, value) for index, (physical, value) in enumerate(observed.pool.registry) if physical == slot]
            if bindings:
                index, value = bindings[0]
            else:
                # Orphans retain fixture construction identity; original records
                # themselves carry only the physical arena index.
                _, _, index, value = world.objects[slot][0]
            observed.objects[slot] = ((kind, slot, index, value), bytearray(machine.mem_read(DGROUP + pointer(slot), 55 if slot < 150 else 251)))
        observed.words, observed.cursor = self.random_state(machine)
        observed.scales = struct.unpack('<2H', machine.mem_read(DGROUP + 0xe3ae, 4))
        selected = struct.unpack('<H', machine.mem_read(DGROUP + 0x6d34, 2))[0]
        observed.selected = self.slot(selected) if selected else NONE
        observed.roster = [self.slot(address) if address else NONE for address in struct.unpack('<32H', machine.mem_read(DGROUP + 0x6d3c, 64))]
        observed.sizes = list(struct.unpack('<4H', machine.mem_read(DGROUP + 0x7a14, 8)))
        observed.counters = [struct.unpack('<H', machine.mem_read(DGROUP + address, 2))[0] for address in CENSUS]
        observed.flash = machine.mem_read(DGROUP + 0x73e, 1)[0]
        return observed

    def assert_world(self, machine, world):
        actual = self.observe_world(machine, world)
        expected = world.state().splitlines()
        observed = actual.state().splitlines()
        if expected != observed:
            for row, (wanted, result) in enumerate(zip(expected, observed), 1):
                if wanted != result: raise AssertionError(f'Original complete world difference at row {row}: actual={result!r}, expected={wanted!r}')
            raise AssertionError(f'Original complete world lengths differ: {len(observed)}/{len(expected)}')
        # Complete original raw records additionally prove that fields outside
        # typed C ownership survive; the constructor's arena index is included.
        for slot, (_, raw) in actual.objects.items():
            wanted = world.objects[slot][1]
            if raw != wanted:
                offset = next(i for i, (a,b) in enumerate(zip(raw,wanted)) if a != b)
                raise AssertionError(f'Original complete raw payload difference: slot {slot}, +{offset:#x}, actual {raw[offset]}, expected {wanted[offset]}')
        return len(actual.objects)

    def visit_world(self, machine, world, index, tick):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_DI
        slot, _ = world.pool.registry[index]
        kind = world.pool.slots[slot][1]
        raw = world.objects[slot][1]
        if world.pending != NONE: raise ValueError('Player-loss consumer acknowledgement is required')
        if kind < 4 or kind in (5,6) or kind == 26 and not raw[25] & 4:
            # This is an explicit class-call boundary, not an executed no-op.
            if kind in (5,6) and raw[37] == 12:
                raise ValueError('Aircraft death repair belongs to its separate original gate')
            text, cursor = world.visit(index, tick)
            if not text.startswith(f'visit {index} {slot} {kind} 2\n'):
                raise AssertionError('Missing explicit living-method rejection')
            self.living_boundaries+=1
            return self.assert_world(machine, world), cursor
        address = pointer(slot)
        self.class_calls+=1
        if kind == 8:
            phase, hit = self.flight(machine, address, {'heights': world.heights})
            if phase == 2:
                actual_parameter=(machine.reg_read(UC_X86_REG_AX),machine.reg_read(UC_X86_REG_BX))
                if actual_parameter!=(raw[42],raw[41]):
                    raise AssertionError(f'Original reached damage parameter/profile differs: {actual_parameter}/{(raw[42],raw[41])}')
                target_kind = world.pool.slots[hit[0]][1]
                _, _, _, selected_loss = self.dispatch(machine, address, voice_entries=(0xbf3c,) if target_kind < 4 else (0xbefb,))
                if selected_loss:
                    # Actual damage after the named UI block, before impact.
                    # The block itself remains an explicit consumer boundary.
                    text, cursor = world.visit(index, tick)
                    self.assert_flight(text,phase,hit)
                    if world.pending != slot: raise AssertionError('Original selected loss must suspend the canonical shell')
                    return self.assert_world(machine, world), cursor
            if phase in (1,2): self.finish(machine, address, phase)
        else:
            machine.reg_write(UC_X86_REG_DI, address)
            self.call(machine, METHODS[kind])
        text, cursor = world.visit(index, tick)
        if kind==8: self.assert_flight(text,phase,hit)
        return self.assert_world(machine, world), cursor

    @staticmethod
    def assert_flight(text,phase,hit):
        event=next(row for row in text.splitlines() if row.startswith('event '))
        actual=tuple(map(int,event.split()[1:6]))
        if actual!=(phase,*hit):
            raise AssertionError(f'Original complete flight event differs: original={(phase,*hit)}, canonical={actual}')

    def resume_world(self, machine, world):
        slot = world.pending
        if slot == NONE: raise ValueError('No selected loss awaiting acknowledgement')
        self.finish(machine, pointer(slot), 2)
        self.continuations+=1
        world.impact(slot, 2)
        world.pending = NONE
        return self.assert_world(machine, world)
