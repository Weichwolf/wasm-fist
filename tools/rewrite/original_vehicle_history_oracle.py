"""Actual class phase indices and complete original position-history returns."""
import struct

from original_unit_oracle import DGROUP
from original_weapon_control_oracle import DISPATCH, OriginalWeaponControlOracle


class OriginalVehicleHistoryOracle(OriginalWeaponControlOracle):
    def history_phase(self, machine, kind):
        from unicorn.x86_const import UC_X86_REG_BX
        _, index, call, table = DISPATCH[kind]
        if struct.unpack_from('<H', self.image, table + 6)[0] != 0xaa37:
            raise RuntimeError('Original history dispatch differs from its pin')
        # The real class-specific index instructions decide admission. The
        # caller owns phase advancement; other callbacks are not substituted.
        self.execute(machine, index, call, 0)
        if machine.reg_read(UC_X86_REG_BX) == 6:
            self.call(machine, 0xaa37)

    def histories(self, cases):
        from unicorn.x86_const import UC_X86_REG_DI
        machine = self.machine()
        observed = []
        for original, steps in cases:
            kind, = struct.unpack_from('<H', original)
            machine.mem_write(DGROUP + 0x7000, original)
            for _ in range(steps):
                machine.reg_write(UC_X86_REG_DI, 0x7000)
                self.history_phase(machine, kind)
                observed.append(bytes(machine.mem_read(DGROUP + 0x7000, len(original))))
        return observed
