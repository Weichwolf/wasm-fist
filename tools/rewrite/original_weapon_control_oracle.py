"""Actual original ground weapon setters, requests and bounded reload/pose stages.

Complete near/far method returns are observed without instruction hooks or
replacement calls. The class-entry pose/recoil fragment stops before motion.
No firing, rack replenishment, damage or audio playback is substituted here.
"""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_motion_oracle import OriginalVehicleMotionOracle

SELECT = (0x7963, 0x17fbc, 0x18b7c, 0x1964e)
CYCLE = (0x7e87, 0x89fe, 0x921a, 0x9a04)
RELOAD = (0x7d69, 0x8971, 0x917d, 0x997d)
ENTRY = (0x7c1d, 0x87df, 0x902c, 0x97d5)
DISPATCH = ((0x7c47, 0x7c4b, 0x7c51, 0x7c91), (0x880e, 0x8812, 0x8818, 0x8858),
            (0x9056, 0x905a, 0x9060, 0x90a0), (0x9804, 0x9808, 0x980f, 0x984e))


class OriginalWeaponControlOracle(OriginalVehicleMotionOracle):
    def tables(self):
        addresses = ((0x8f54, 0x8f0e, 0x8f0a, 4), (0x90f6, 0x90ee, None, 4),
                     (0x9186, 0x9196, 0x919b, 5), (0x9282, 0x9242, None, 4))
        return [tuple(tuple(self.image[DGROUP + address:DGROUP + address + count])
                      if address is not None else () for address in fields)
                for *fields, count in addresses]

    def far_zero(self, machine, entry):
        _, ds, sp, ss = self.machine_registers()
        machine.reg_write(sp, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        self.execute(machine, entry, 0xeff0, 0)
        if (machine.reg_read(sp), machine.reg_read(ds), machine.reg_read(ss)) != (
                0x9004, 0x1c00, 0x1c00):
            raise RuntimeError('Original complete zero-segment far return failed')

    def begin(self, machine, kind):
        start = ENTRY[kind] + 6
        end = ENTRY[kind] + 25
        expected = bytes.fromhex('8b4538c1f8088885a700807d3c007403fe4d3c')
        if self.image[start:end] != expected:
            raise RuntimeError('Original bounded elevation/recoil prefix differs from its pin')
        self.execute(machine, start, end, 0)

    def reload_phase(self, machine, kind, *, advance):
        from unicorn.x86_const import UC_X86_REG_BX
        add, index, call, table = DISPATCH[kind]
        if self.image[add:index] != bytes.fromhex('80453d02') or (
                self.image[call:call + 5] != b'\x2e\xff\x97' + struct.pack('<H', table)) or (
                struct.unpack_from('<H', self.image, table)[0] != RELOAD[kind]):
            raise RuntimeError('Original phase/reload dispatch differs from its pin')
        # Execute the actual ADD (when requested) and complete phase-index
        # instructions for every byte. Other phase callbacks remain outside
        # this bounded weapon stage; the selected reload callback runs fully.
        self.execute(machine, add if advance else index, call, 0)
        if machine.reg_read(UC_X86_REG_BX) == 0:
            self.call(machine, RELOAD[kind])

    def transitions(self, cases):
        from unicorn.x86_const import UC_X86_REG_BX, UC_X86_REG_DI
        machine = self.machine()
        # Suppress actual playback at the original mute gate, while allowing
        # the real selected-player notice and global HUD writes to execute.
        machine.mem_write(DGROUP + 0x6da2, b'\0\0')
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', 0x7000))
        machine.mem_write(DGROUP + 0x6ce6, b'\0')
        machine.mem_write(DGROUP + 0x6cde, b'\0\0')
        observed = []
        for original, steps, operation, argument in cases:
            kind, = struct.unpack_from('<H', original)
            machine.mem_write(DGROUP + 0x7000, original)
            for _ in range(steps):
                machine.reg_write(UC_X86_REG_DI, 0x7000)
                machine.mem_write(DGROUP + 0x8e60, bytes(16))
                machine.mem_write(DGROUP + 0x9fd6, b'\xff\0\0')
                if operation == 0:
                    machine.reg_write(UC_X86_REG_BX, argument)
                    if kind == 0:
                        self.far_zero(machine, SELECT[kind])
                    else:
                        self.far_call(machine, SELECT[kind])
                elif operation == 1:
                    self.call(machine, CYCLE[kind])
                elif operation == 2:
                    self.call(machine, 0xa286)
                elif operation == 3:
                    self.begin(machine, kind)
                elif operation in (4, 5):
                    if operation == 5:
                        self.begin(machine, kind)
                    self.reload_phase(machine, kind, advance=operation == 5)
                else:
                    raise RuntimeError('Unknown weapon-control oracle operation')
                raw = bytes(machine.mem_read(DGROUP + 0x7000, 251))
                hud = bytes(machine.mem_read(DGROUP + 0x8e60, 16))
                notice = struct.unpack('<BH', machine.mem_read(DGROUP + 0x9fd6, 3))
                observed.append((raw, hud, notice))
        return observed
