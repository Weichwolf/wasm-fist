"""Execute original manual, gun/recoil, reload/history, selection and contact stages.

This verifies the declared driving subset, not a full original class/mission tick.
Actual class-entry altitude MOVs and phase ADD/index instructions execute at
their declared boundaries. Selection also runs the complete control refresh.
"""
import struct

from original_vehicle_motion_oracle import MOTION, TURRET
from original_ground_oracle import OriginalGroundOracle
from original_unit_oracle import DGROUP
from original_vehicle_history_oracle import OriginalVehicleHistoryOracle

CLASS_ENTRY = (0x7c1d, 0x87df, 0x902c, 0x97d5)


class OriginalDriverOracle:
    def __init__(self, side, pixels):
        self.motion = OriginalVehicleHistoryOracle()
        self.machine = self.motion.machine()
        self.ground = OriginalGroundOracle()
        self.field = self.ground.prepare(side, pixels)

    def take_control(self, machine):
        from unicorn.x86_const import UC_X86_REG_SP, UC_X86_REG_DS, UC_X86_REG_SS
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        # aae8 belongs to segment zero; the movement services use f69 instead.
        self.motion.execute(machine, 0xaae8, 0xeff0, 0)
        if (machine.reg_read(UC_X86_REG_SP), machine.reg_read(UC_X86_REG_DS),
                machine.reg_read(UC_X86_REG_SS)) != (0x9004, 0x1c00, 0x1c00):
            raise RuntimeError('Original complete control-prefix return failed')

    def control(self, record):
        from unicorn.x86_const import UC_X86_REG_DI
        identity, generation, raw = record
        self.machine.mem_write(DGROUP + 0x7000, raw)
        self.machine.reg_write(UC_X86_REG_DI, 0x7000)
        self.take_control(self.machine)
        return identity, generation, bytes(self.machine.mem_read(DGROUP + 0x7000, len(raw)))

    def step(self, record, keys):
        from unicorn.x86_const import UC_X86_REG_DI
        identity, generation, raw = record
        kind, = struct.unpack_from('<H', raw)
        if struct.unpack_from('<H', raw, 0x97)[0] != 0:
            raise ValueError('Manual driver boundary requires an untargeted actor')
        machine = self.machine
        machine.mem_write(DGROUP + 0x7000, raw)
        machine.reg_write(UC_X86_REG_DI, 0x7000)
        throttle = bool(keys & 1) - bool(keys & 2)
        steering = bool(keys & 8) - bool(keys & 4)
        turret = bool(keys & 32) - bool(keys & 16)
        if throttle or steering or turret:
            self.take_control(machine)
        if throttle:
            self.motion.call(machine, 0xa410 if throttle > 0 else 0xa427)
        if keys & 64:
            self.motion.call(machine, 0xa45c)
        if steering:
            self.motion.call(machine, 0xa3ec if steering > 0 else 0xa3e2)
        if turret:
            machine.mem_write(DGROUP + 0x9746, struct.pack('<H', 256 if turret > 0 else 64))
            self.motion.call(machine, 0xa3a8 if turret > 0 else 0xa376)
        # Execute both actual MOV instructions, stopping before other class
        # methods. This does not claim a complete class tick.
        entry = CLASS_ENTRY[kind]
        transfer = bytes.fromhex('8a451d88450d')
        if self.motion.image[entry:entry + len(transfer)] != transfer:
            raise RuntimeError('Original class altitude transfer differs from its pin')
        self.motion.execute(machine, entry, entry + len(transfer), 0)
        self.motion.begin(machine, kind)
        self.motion.call(machine, MOTION[kind])
        self.motion.call(machine, TURRET[kind])
        self.motion.reload_phase(machine, kind, advance=True)
        self.motion.history_phase(machine, kind)
        current = bytearray(machine.mem_read(DGROUP + 0x7000, 251))
        return self.ground.contact(self.field, [(identity, generation, bytes(current))])[0]

    def select(self, record, operation, argument):
        from unicorn.x86_const import UC_X86_REG_DI
        identity, generation, raw = record
        selected = self.motion.transitions([(raw, 1, operation, argument)])[0][0]
        self.machine.mem_write(DGROUP + 0x7000, selected)
        self.machine.reg_write(UC_X86_REG_DI, 0x7000)
        self.take_control(self.machine)
        return identity, generation, bytes(self.machine.mem_read(DGROUP + 0x7000, 251))
