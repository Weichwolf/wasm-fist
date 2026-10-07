"""One original saved-world/control/contact owner for the reached M1 class prefix.

Execute only 7c1d..7c7e, before the engine PCM device call. Reserved address-0/1
return scaffolding is an error, never a substitute for a missing PM device.
"""
import struct

from original_driver_oracle import OriginalDriverOracle
from original_mission_world_oracle import OriginalMissionWorldOracle
from original_unit_oracle import DGROUP


class OriginalControlledM1Oracle:
    def __init__(self, records, side, pixels, *, patches=()):
        from unicorn import UC_HOOK_CODE
        from unicorn.x86_const import UC_X86_REG_DI
        self.owner = OriginalMissionWorldOracle()
        self.machine, self.objects = self.owner.prepare(records, (0, 0, 0, 0), 0, 0)
        self.pointer = int.from_bytes(self.machine.mem_read(DGROUP + 0x6d3c, 2), 'little')
        self.selected = self.owner.slot(self.pointer)
        self.allocation, _, self.size = self.objects[self.selected]
        if self.allocation[0] != 0:
            raise ValueError('The recovered complete prefix belongs to M1 only')
        self.driver = OriginalDriverOracle(side, pixels)
        self.machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', self.pointer))
        self.machine.mem_write(DGROUP + 0x6da2, bytes(2))  # Declared voice mute gate.
        self.machine.mem_write(DGROUP + 0x8b43, bytes(2))  # Declared no-joystick selection.
        self.machine.reg_write(UC_X86_REG_DI, self.pointer)
        self.driver.take_control(self.machine)
        for offset, data in patches:
            if offset < 4 or offset + len(data) > self.size:
                raise ValueError('Constructed actor patch crosses its payload boundary')
            self.machine.mem_write(DGROUP + self.pointer + offset, data)
        self.state = self.contact()
        self.machine.mem_write(DGROUP + self.pointer, self.state[2])
        self.initial_random = self.owner.random_state(self.machine)
        self.initial_others = [(p, bytes(self.machine.mem_read(DGROUP + p, n)))
                               for slot, (_, p, n) in self.objects.items() if slot != self.selected]
        self.calls = []
        self.last_calls = []
        def observe(uc, address, length, context):
            if address in (0, 1, 0x7c1d, 0xaa37, 0x1a6c8, 0x7cbf, 0x19ffc, 0xab03, 0x0291):
                self.calls.append(address)
                self.last_calls.append(address)
            if address in (0, 1):
                uc.emu_stop()
        self.handle = self.machine.hook_add(UC_HOOK_CODE, observe)
        # Constructors already translated shared routines such as RNG. Flush
        # those blocks so observational hooks cover reused code as well.
        self.machine.ctl_flush_tb()

    def __enter__(self):
        return self

    def __exit__(self, kind, value, traceback):
        self.machine.hook_del(self.handle)

    def contact(self):
        raw = bytes(self.machine.mem_read(DGROUP + self.pointer, self.size))
        return self.driver.ground.contact(self.driver.field, [(self.allocation[2], self.allocation[3], raw)])[0]

    def step(self):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SP
        self.last_calls = []
        self.machine.reg_write(UC_X86_REG_DI, self.pointer)
        self.machine.reg_write(UC_X86_REG_SP, 0x9000)
        self.machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        self.owner.execute(self.machine, 0x7c1d, 0x7c7e, 0)
        if self.machine.reg_read(UC_X86_REG_SP) != 0x9000 or any(address in (0, 1) for address in self.last_calls):
            raise RuntimeError('Incomplete original prefix or unconfigured PM driver return')
        self.state = self.contact()
        self.machine.mem_write(DGROUP + self.pointer, self.state[2])
        return self.state

    def unchanged_others(self):
        return all(bytes(self.machine.mem_read(DGROUP + address, len(before))) == before
                   for address, before in self.initial_others)

    def random_state(self):
        return self.owner.random_state(self.machine)
