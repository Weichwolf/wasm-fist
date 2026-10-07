"""Complete unchanged ad08/f69:b57a returns and reaching route-boundary evidence."""
import struct

from original_unit_oracle import DGROUP
from original_vehicle_start_oracle import OriginalVehicleStartOracle


class OriginalGroundRouteOracle(OriginalVehicleStartOracle):
    def __init__(self):
        super().__init__()
        self.dispatch = struct.unpack_from('<8H', self.image, DGROUP + 0x9820)
        if self.dispatch != (0xad11, 0xad28, 0xad2a, 0xad29, 0xad2b, 0xad2c, 0xad2d, 0xad2e):
            raise RuntimeError('Original complete route dispatch differs')
        if self.image[0xad28:0xad2f] != b'\xc3' * 7:
            raise RuntimeError('Original seven route-mode returns differ')
        if struct.unpack_from('<4H', self.image, 0x1ac31) != (0xb5a9, 0xb5cd, 0xb5f1, 0xb615):
            raise RuntimeError('Original complete waypoint service bank differs')
        if struct.unpack_from('<8H', self.image, DGROUP + 0x7d2a) != tuple(0x7d40 + p * 268 for p in range(8)):
            raise RuntimeError('Original route pointer owner differs')

    def advance(self, machine, pointer, platoon):
        from unicorn.x86_const import UC_X86_REG_DI
        route = 0x7d40 + platoon * 268
        machine.mem_write(DGROUP + 0x9796, struct.pack('<HH', 0x85b6 + platoon * 22, route))
        before = bytes(machine.mem_read(DGROUP, 65536))
        random = self.random_state(machine)
        machine.reg_write(UC_X86_REG_DI, pointer)
        self.call(machine, 0xad08)
        if machine.reg_read(UC_X86_REG_DI) != pointer or self.random_state(machine) != random:
            raise AssertionError('Original complete route return changed identity or RNG')
        after = bytes(machine.mem_read(DGROUP, 65536))
        ranges = sorted(((pointer, pointer + 251), (route, route + 268), (0x8fc0, 0x9002)))
        offset = 0
        for begin, end in (*ranges, (65536, 65536)):
            if before[offset:begin] != after[offset:begin]:
                raise AssertionError('Original route changed unrelated DGROUP/orders/payloads')
            offset = end
        return bytes(machine.mem_read(DGROUP + pointer, 251)), bytes(machine.mem_read(DGROUP + route, 268))

    def cases(self, cases):
        machine = self.machine((1, 2, 32768, 65535), 3)
        results = []
        for raw, _, _, descriptor, route, seeds, cursor in cases:
            platoon = raw[0x1b]
            machine.mem_write(DGROUP + 0x7000, raw)
            machine.mem_write(DGROUP + 0x85b6 + platoon * 22, struct.pack('<11H', *descriptor))
            machine.mem_write(DGROUP + 0x7d40 + platoon * 268, route)
            machine.mem_write(DGROUP + 0x1f82, struct.pack('<5H',
                              0x1f84 + ((cursor + 3) % 4) * 2, *seeds))
            results.append(self.advance(machine, 0x7000, platoon))
        return results

    def full_capacity_proof(self):
        """Observe actual operand addresses without replacing any instruction/read.

        Complete return/output dependency independently proves the pop defect.
        A code hook only observes SI at the original DWORD load instructions;
        it does not implement a memory read or change registers/guest memory.
        """
        from unicorn import UC_HOOK_CODE
        from unicorn.x86_const import UC_X86_REG_SI
        from orders_contract import constructed_blocks
        from test_vehicle_motion import start
        reads = {0x1ac46: 0, 0x1ac4c: 4, 0x1ac6a: 0, 0x1ac70: 4,
                 0x1ac8e: 0, 0x1ac94: 4, 0x1acbb: 0, 0x1acc1: 4}
        pop_loads = ((0x1ac46, 0x1ac4c), (0x1ac6a, 0x1ac70), (0x1ac8e, 0x1ac94))
        observations = []
        for platoon in range(8):
            for count in (31, 32):
                for mode in (0, 1, 2, 3, 4, 65535):
                    for neighbor in (bytes.fromhex('1032547698badcfe'), bytes.fromhex('efcdab8967452301')):
                        machine = self.machine((1, 2, 32768, 65535), 3)
                        raw = bytearray(start(platoon % 4))
                        raw[0x1b], raw[0x43] = platoon, 0
                        struct.pack_into('<H', raw, 0x40, 65535)
                        struct.pack_into('<H', raw, 0x53, 48)
                        paths, info = constructed_blocks(count, salt=platoon)
                        descriptor = bytearray(info)
                        struct.pack_into('<H', descriptor, platoon * 22 + 2, mode)
                        machine.mem_write(DGROUP + 0x7d40, paths)
                        machine.mem_write(DGROUP + 0x85b6, bytes(descriptor))
                        route = DGROUP + 0x7d40 + platoon * 268
                        machine.mem_write(route + 268, neighbor)
                        machine.mem_write(DGROUP + 0x7000, bytes(raw))
                        unobserved = self.machine((1, 2, 32768, 65535), 3)
                        unobserved.mem_write(DGROUP, bytes(machine.mem_read(DGROUP, 65536)))
                        addresses = []

                        def observe(uc, address, size, _):
                            if address in reads:
                                addresses.append((address, DGROUP + uc.reg_read(UC_X86_REG_SI) + reads[address], 4))

                        hook = machine.hook_add(UC_HOOK_CODE, observe, begin=0x1ac39, end=0x1acdf)
                        actual, changed = self.advance(machine, 0x7000, platoon)
                        machine.hook_del(hook)
                        if self.advance(unobserved, 0x7000, platoon) != (actual, changed) or bytes(
                                unobserved.mem_read(DGROUP, 65536)) != bytes(machine.mem_read(DGROUP, 65536)):
                            raise AssertionError('Operand observation changed the complete original return')
                        outside = [value for value in addresses if value[1] >= route + 268]
                        loads = (0x1acbb, 0x1acc1) if mode == 3 else pop_loads[mode if mode < 3 else 0]
                        expected = [] if count == 31 else [(loads[0], route + 268, 4), (loads[1], route + 272, 4)]
                        if outside != expected:
                            raise AssertionError('Original complete reaching overread differs')
                        if int.from_bytes(actual[0x40:0x42], 'little') != 65533:
                            raise AssertionError('Original admitted progress did not clear only goal validity')
                        if mode == 3:
                            if changed[0] != count or changed[12 + (count - 1) * 8:20 + (count - 1) * 8] != paths[platoon * 268 + 12:platoon * 268 + 20]:
                                raise AssertionError('Original complete cyclic output differs')
                        elif changed[0] != count - 1 or (count == 32 and changed[-8:] != neighbor):
                            raise AssertionError('Original complete pop did not consume the neighbor bytes')
                        observations.append({'platoon': platoon, 'count': count, 'mode': mode,
                                             'neighbor': neighbor.hex(), 'tail': changed[-8:].hex(),
                                             'outside_loads': outside})
        return observations
