"""Complete original a17e/08e8 returns and a reaching real target-selection scan."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle


class OriginalProximityOracle(OriginalVehicleStartOracle):
    def cases(self, cases):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DX, UC_X86_REG_DI,
                                      UC_X86_REG_SI, UC_X86_REG_SP, UC_X86_REG_EFLAGS)
        machine = self.machine((1, 2, 32768, 65535), 3)
        result = []
        for source, target, _ in cases:
            machine.mem_write(DGROUP + 0x7000, struct.pack('<2i', *source))
            machine.mem_write(DGROUP + 0x7010, struct.pack('<2i', *target))
            machine.reg_write(UC_X86_REG_SI, 0x7000)
            machine.reg_write(UC_X86_REG_DI, 0x7010)
            machine.reg_write(UC_X86_REG_SP, 0x9000)
            machine.reg_write(UC_X86_REG_EFLAGS, 3)
            machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
            before = bytes(machine.mem_read(DGROUP, 65536))
            self.execute(machine, 0xa17e, 0xeff0)
            value = machine.reg_read(UC_X86_REG_AX) | (machine.reg_read(UC_X86_REG_DX) << 16)
            if (machine.reg_read(UC_X86_REG_SP) != 0x9004 or
                    machine.reg_read(UC_X86_REG_SI) != 0x7000 or
                    machine.reg_read(UC_X86_REG_DI) != 0x7010 or
                    bytes(machine.mem_read(DGROUP + 0x458, 4)) != struct.pack('<I', value)):
                raise AssertionError('Complete original proximity return/input/scratch differs')
            flags = machine.reg_read(UC_X86_REG_EFLAGS)
            if flags & 1 or bool(flags & 64) != (value >> 16 == 0):
                raise AssertionError('Original proximity TEST DX flag contract differs')
            after = bytearray(machine.mem_read(DGROUP, 65536))
            for begin, end in ((0x458, 0x45c), (0x8ffe, 0x9000)):
                after[begin:end] = before[begin:end]
            if bytes(after) != before:
                raise AssertionError('Original proximity changed unrelated DGROUP/input/RNG')
            result.append(value)
        return result

    @staticmethod
    def reaching_selection():
        from unicorn.x86_const import (UC_X86_REG_DI, UC_X86_REG_EAX,
                                      UC_X86_REG_SP)
        from original_mission_ready_oracle import OriginalMissionReadyOracle
        from original_visibility_oracle import OriginalVisibilityOracle
        owner, visibility = OriginalMissionReadyOracle(), OriginalVisibilityOracle()
        records = []
        for index, (x, y, flags) in enumerate(((0, 0, 4), (-25600, -25600, 12), (-37120, 0, 12))):
            raw = bytearray(251)
            struct.pack_into('<3i', raw, 4, x, y, 4096)
            raw[22] = flags
            records.append((index, 1, bytes(raw)))
        machine, objects = owner.prepare_saved(records, (1, 2, 32768, 65535), 3, 0,
                                               (bytes(2144), bytes(176)))
        actor = objects[150][2]
        # Actual notification admission rejects playback when 6da2 != ffff.
        # This declares a notification gate input, not a patched audio return.
        machine.mem_write(DGROUP + 0x6da2, bytes(2))
        machine.mem_write(DGROUP + 0x6d34, struct.pack('<H', actor))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        kernel = visibility.prepare(512, bytes(512**2))
        transfers = []
        entry, segment = 0xb011, 0
        for expected in (objects[151][2], objects[152][2]):
            owner.execute(machine, entry, 0xe2a0, segment)
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != struct.pack('<H', 0x58):
                raise AssertionError('Actual target scan reached a different PM service')
            inbox = bytes(machine.mem_read(0x40000 + 0xd2, 24))
            source, target = struct.unpack('<3i', inbox[:12]), struct.unpack('<3i', inbox[12:])
            expected_pose = struct.unpack('<3i', machine.mem_read(DGROUP + expected + 4, 12))
            if target[:2] != expected_pose[:2] or source[:2] != (0, 0):
                raise AssertionError('Actual registry/aim wrapper transfer order differs')
            observed = visibility.visible(kernel, [(source, target)])[0]
            if not observed:
                raise AssertionError('Reaching selection fixture must have real visible candidates')
            # The actual DOS/PM boundary returns EAX; retain complete ffffffff.
            # All DOS aim preparation, nesting and post-transfer instructions run.
            machine.reg_write(UC_X86_REG_EAX, kernel.reg_read(UC_X86_REG_EAX))
            transfers.append((source, target, observed))
            entry, segment = 0xe2a3, 0
        owner.execute(machine, entry, 0xeff0)
        if machine.reg_read(UC_X86_REG_SP) != 0x9002:
            raise AssertionError('Complete original b011 near return did not complete')
        after = bytes(machine.mem_read(DGROUP, 65536))
        selected = struct.unpack_from('<H', after, actor + 0x9d)[0]
        seeds, cursor = owner.random_state(machine)
        if (selected != objects[152][2] or after[actor + 0x94] != 2 or
                struct.unpack_from('<H', after, 0x993e)[0] != 145 or
                (tuple(seeds), cursor) != ((1, 2, 32768, 65535), 3)):
            raise AssertionError('Original reached selection/range/count/RNG differs')
        normalized = bytearray(after)
        spans = ((0x458, 0x45c), (0x8fc0, 0x9000), (0x96ab, 0x96ac),
                 (0x9934, 0x9946), (0x9a0a, 0x9a0e), (0xea10, 0xea12),
                 (actor + 0x94, actor + 0x95), (actor + 0x9d, actor + 0x9f))
        for begin, end in spans:
            normalized[begin:end] = before[begin:end]
        if bytes(normalized) != before:
            changed = [hex(i) for i, (a, b) in enumerate(zip(normalized, before)) if a != b]
            raise AssertionError(f'Original scan changed undeclared state: {changed[:12]}')
        return {'selected_registry': 2, 'range': 145, 'winner_updates': 2,
                'transfers': transfers, 'complete_dgroup_bytes': 65536}
