"""Complete original ground motion, manual turret and planar rotation returns."""
import struct

from original_unit_oracle import DGROUP, SERVICE_CS
from original_vehicle_start_oracle import OriginalVehicleStartOracle

MOTION = (0x7cd5, 0x88e4, 0x912d, 0x98c3)
TURRET = (0x7d0f, 0x8917, 0x90cd, 0x9911)


class OriginalVehicleMotionOracle(OriginalVehicleStartOracle):
    def machine(self, words=(0, 0, 0, 0), next_stream=0, link=0):
        machine = super().machine(words, next_stream, link)
        machine.mem_write(DGROUP + 0x70, struct.pack('<H', 0x2d74))
        machine.mem_write(DGROUP + 0x2040, b'\0')
        return machine

    def far_call(self, machine, entry):
        _, ds, sp, ss = self.machine_registers()
        machine.reg_write(sp, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        self.execute(machine, entry, 0xeff0, SERVICE_CS)
        if (machine.reg_read(sp), machine.reg_read(ds), machine.reg_read(ss)) != (
                0x9004, 0x1c00, 0x1c00):
            raise RuntimeError('Original complete far return/segment contract failed')

    def rotations(self, cases):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DX
        machine = self.machine()
        values = []
        for heading, magnitude, coarse in cases:
            machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
            machine.reg_write(UC_X86_REG_AX, heading)
            machine.reg_write(UC_X86_REG_DX, magnitude % 65536)
            self.call(machine, 0x3a9)
            values.append(struct.unpack('<hh', struct.pack('<HH',
                          machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX))))
        return values

    def motion(self, cases):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine = self.machine()
        observed = []
        for raw, steps in cases:
            kind, = struct.unpack_from('<H', raw)
            if struct.unpack_from('<H', raw, 0x97)[0] != 0:
                raise RuntimeError('Manual motion oracle requires an untargeted object')
            for _ in range(steps):
                machine.mem_write(DGROUP + 0x7000, raw)
                machine.reg_write(UC_X86_REG_DI, 0x7000)
                self.far_call(machine, 0x1a401)
                speed_event = machine.reg_read(UC_X86_REG_EFLAGS) & 1
                self.far_call(machine, 0x1a395)
                hull_event = 1 - (machine.reg_read(UC_X86_REG_EFLAGS) & 1)
                # Execute the complete actual class method from the unchanged
                # source too, including component writes and position integration.
                machine.mem_write(DGROUP + 0x7000, raw)
                machine.mem_write(DGROUP + 0x8e48, b'\0')
                self.call(machine, MOTION[kind])
                offset = bytes(machine.mem_read(DGROUP + 0x7089, 2))
                self.call(machine, TURRET[kind])
                turret_event = int(bytes(machine.mem_read(DGROUP + 0x7089, 2)) != offset)
                raw = bytes(machine.mem_read(DGROUP + 0x7000, 251))
                dirty = bytes(machine.mem_read(DGROUP + 0x8e48, 1))
                if dirty != bytes([3 if hull_event or turret_event else 0]):
                    raise RuntimeError('Original scene refresh and motion events disagree')
                observed.append((raw, (speed_event, hull_event, turret_event)))
                # Caller advances the active-object phase after the observed stage.
                raw = raw[:0x3d] + bytes([(raw[0x3d] + 2) % 256]) + raw[0x3e:]
        return observed
