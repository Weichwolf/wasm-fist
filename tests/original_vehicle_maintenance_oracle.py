"""Actual per-class phase dispatch and complete movement maintenance returns."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_history_oracle import OriginalVehicleHistoryOracle
from original_weapon_control_oracle import DISPATCH

WRAPPER = (0x7cbf, 0x88ce, 0x9160, 0x98fb)


class OriginalVehicleMaintenanceOracle(OriginalVehicleHistoryOracle):
    def maintenance_phase(self, machine, kind):
        from unicorn.x86_const import UC_X86_REG_BX
        _, index, call, table = DISPATCH[kind]
        self.execute(machine, index, call, 0)
        entry, = struct.unpack_from('<H', self.image, table + machine.reg_read(UC_X86_REG_BX))
        if entry == WRAPPER[kind]:
            if self.image[entry:entry + 5] != bytes.fromhex('9a6ca9690f'):
                raise RuntimeError('Original complete maintenance wrapper differs from its pin')
            self.call(machine, entry)

    def maintenance_cases(self, cases):
        from unicorn.x86_const import UC_X86_REG_DI
        machine = self.machine()
        observed = []
        for original, steps in cases:
            kind, = struct.unpack_from('<H', original)
            machine.mem_write(DGROUP + 0x7000, original)
            for _ in range(steps):
                machine.reg_write(UC_X86_REG_DI, 0x7000)
                self.maintenance_phase(machine, kind)
                observed.append(bytes(machine.mem_read(DGROUP + 0x7000, len(original))))
        return observed
