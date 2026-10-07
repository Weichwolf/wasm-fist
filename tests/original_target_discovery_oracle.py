"""Complete original b011 scan with actual aim/visibility and request admission.

Original allocation, registry traversal, preference, secondary bearing and tail
instructions run unchanged. Height visibility executes in the actual PM kernel.
An admitted op-64 request is observed at its device boundary; PCM playback is
outside this observer. No patched instructions or instruction hooks are used.
"""
import struct

from original_mission_ready_oracle import OriginalMissionReadyOracle
from original_object_pool_oracle import REGISTRY
from original_unit_oracle import DGROUP, SERVICE_CS
from original_visibility_oracle import OriginalVisibilityOracle


def signed(value):
    return (value + 2**31) % 2**32 - 2**31


class OriginalTargetDiscoveryOracle(OriginalMissionReadyOracle):
    def __init__(self):
        super().__init__()
        self.visibility = OriginalVisibilityOracle()
        self.preferences = (
            (0, 0, 0, 0, 99, 1, 1, *([99] * 19), 2, 2),
            (1, 1, 1, 1, 99, 0, 0, *([99] * 19), 2, 2))
        self.target_heights = (1792, 2048, 1792, 1536, 0, 256, 256, *([0] * 20), 2048)
        self.source_heights = (2048, 2560, 2048, 1920, *([0] * 22), 1536, 0)
        self.variant_heights = (3840, 4352, 2560, 3072)
        self.ranges = ((1000, 1000, 1000, 1000), (150, 150, 150, 150),
                       (625, 625, 450, 450))
        if (struct.unpack_from('<4H', self.image, DGROUP + 0x989c) !=
                (0x98a4, 0x98c0, 0x98a4, 0x98c0)):
            raise AssertionError('Original ground preference selection differs')
        for address, expected, format_code in (
                (0x98a4, self.preferences[0], 'B'), (0x98c0, self.preferences[1], 'B'),
                (0xe588, self.target_heights, 'H'), (0xe5c0, self.source_heights, 'H'),
                (0x9eaf, self.variant_heights, 'H'),
                (0x972e, sum(self.ranges, ()), 'H')):
            if struct.unpack_from('<' + str(len(expected)) + format_code,
                                  self.image, DGROUP + address) != expected:
                raise AssertionError(f'Original discovery table {address:#x} differs')

    @staticmethod
    def raw(machine, pointer):
        kind = struct.unpack('<H', machine.mem_read(DGROUP + pointer, 2))[0]
        return bytes(machine.mem_read(DGROUP + pointer, 251 if kind in (0, 1, 2, 3, 19) else 55))

    def scan(self, machine, actor, kernel, *, coarse=0, gate=0, selected=None,
             clock=0, last_voice=0):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_DS,
                                      UC_X86_REG_DX, UC_X86_REG_EAX, UC_X86_REG_ECX,
                                      UC_X86_REG_SI, UC_X86_REG_SP, UC_X86_REG_SS)
        actor_raw = self.raw(machine, actor)
        kind = int.from_bytes(actor_raw[:2], 'little')
        if kind >= 4:
            raise ValueError('Ground discovery requires an actual ground actor')
        entries = struct.unpack('<364H', machine.mem_read(DGROUP + REGISTRY, 728))
        candidates = [(index, pointer) for index, pointer in enumerate(entries[::2])
                      if pointer and pointer != actor and
                      self.raw(machine, pointer)[22] & 4 and
                      (self.raw(machine, pointer)[22] ^ actor_raw[22]) & 8]
        for address, value in ((0x6da2, gate), (0x6d34, actor if selected is None else selected),
                               (0x452, clock), (0x9fca, last_voice)):
            machine.mem_write(DGROUP + address, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, 0x4000))
        machine.reg_write(UC_X86_REG_DI, actor)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        code_before = bytes(machine.mem_read(0, DGROUP))
        mailbox_before = bytes(machine.mem_read(0x40000, 4096))
        random_before = self.random_state(machine)
        transfers = []
        entry, segment = 0xb011, 0
        for index, pointer in candidates:
            self.execute(machine, entry, 0xe2a0, segment)
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != struct.pack('<H', 0x58):
                raise AssertionError('Actual scan registry/visibility dispatch differs')
            inbox = bytes(machine.mem_read(0x40000 + 0xd2, 24))
            source, target = struct.unpack('<3i', inbox[:12]), struct.unpack('<3i', inbox[12:])
            candidate_raw = self.raw(machine, pointer)
            candidate_kind = int.from_bytes(candidate_raw[:2], 'little')
            expected_source = list(struct.unpack_from('<3i', actor_raw, 4))
            expected_source[2] = signed(expected_source[2] + self.source_heights[kind])
            expected_target = list(struct.unpack_from('<3i', candidate_raw, 4))
            height = (self.variant_heights[candidate_raw[25] & 3] if candidate_kind == 26
                      else self.target_heights[candidate_kind])
            expected_target[2] = signed(expected_target[2] + height)
            if source != tuple(expected_source) or target != tuple(expected_target):
                raise AssertionError('Complete actual aim XYZ differs from class/variant tables')
            visible = self.visibility.visible(kernel, [(source, target)])[0]
            machine.reg_write(UC_X86_REG_EAX, kernel.reg_read(UC_X86_REG_EAX))
            self.execute(machine, 0xe2a3, 0x1aafd)
            if machine.reg_read(UC_X86_REG_SI) != pointer:
                raise AssertionError('Complete primary scan changed the candidate')
            transfers.append({'registry': index, 'pointer': pointer, 'source': source, 'target': target,
                              'visible': visible, 'secondary_operand': machine.reg_read(UC_X86_REG_AX),
                              'primary': struct.unpack('<H', machine.mem_read(DGROUP + 0x993c, 2))[0],
                              'priority': machine.mem_read(DGROUP + 0x9935, 1)[0]})
            entry, segment = 0x1aafd, SERVICE_CS
        self.execute(machine, entry, 0x1ab34, segment)
        count = machine.mem_read(DGROUP + 0x9934, 1)[0]
        admitted = (actor_raw[0x94] == 0 and count != 0 and gate == 65535 and
                    (selected is None or selected == actor) and actor_raw[22] & 8 == 0 and
                    (clock - last_voice) % 65536 >= 30)
        requests = []
        if admitted:
            self.execute(machine, 0x1ab34, 0xe2db, SERVICE_CS)
            request = (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX),
                       machine.reg_read(UC_X86_REG_ECX))
            if (request != (0x028e, clock & 0xff00, 0) or
                    bytes(machine.mem_read(DGROUP + 0xea10, 2)) != struct.pack('<H', 0x64)):
                raise AssertionError('Actual admitted target-acquired device request differs')
            requests.append(request)
            # Explicit unconsumed audio-device boundary. Observe the actual
            # request; resume its caller, whose tail does not use a device result.
            self.execute(machine, 0xe2de, 0x1ab4e)
        else:
            self.execute(machine, 0x1ab34, 0x1ab4e, SERVICE_CS)
        self.execute(machine, 0x1ab4e, 0xeff0, SERVICE_CS)
        if (machine.reg_read(UC_X86_REG_SP), machine.reg_read(UC_X86_REG_DS),
                machine.reg_read(UC_X86_REG_SS), machine.reg_read(UC_X86_REG_DI)) != (
                    0x9002, 0x1c00, 0x1c00, actor):
            raise AssertionError('Complete b011 return/segments/actor identity differs')
        if self.random_state(machine) != random_before:
            raise AssertionError('Discovery or request admission changed canonical RNG')
        after = bytes(machine.mem_read(DGROUP, 65536))
        normalized = bytearray(after)
        # Original scratch, bounded call stack and exact actor destinations.
        spans = ((0x458, 0x45c), (0x8fc0, 0x9000), (0x96ab, 0x96ac),
                 (0x9934, 0x9946), (0x9a0a, 0x9a0e), (0xea10, 0xea12),
                 (actor + 0x19, actor + 0x1a), (actor + 0x8e, actor + 0x90),
                 (actor + 0x94, actor + 0x95), (actor + 0x97, actor + 0x99),
                 (actor + 0x9d, actor + 0x9f))
        if admitted:
            spans += ((0x9fca, 0x9fcc),)
        for begin, end in spans:
            normalized[begin:end] = before[begin:end]
        mailbox_after = bytearray(machine.mem_read(0x40000, 4096))
        for begin, end in ((0xd2, 0xea), (0x3f2, 0x3f6)):
            mailbox_after[begin:end] = mailbox_before[begin:end]
        if (bytes(normalized) != before or bytes(machine.mem_read(0, DGROUP)) != code_before or
                bytes(mailbox_after) != mailbox_before):
            changed = [hex(i) for i, (a, b) in enumerate(zip(normalized, before)) if a != b]
            raise AssertionError(f'Original discovery changed undeclared memory: {changed[:12]}')
        return {'actor': self.raw(machine, actor), 'transfers': transfers, 'requests': requests,
                'primary': struct.unpack_from('<H', after, 0x993c)[0],
                'secondary': struct.unpack_from('<H', after, 0x9940)[0],
                'primary_range': struct.unpack_from('<H', after, 0x993e)[0],
                'secondary_operand': struct.unpack_from('<H', after, 0x9942)[0],
                'priority': after[0x9935], 'count': count,
                'limit': struct.unpack_from('<H', after, 0x993a)[0],
                'last_voice': struct.unpack_from('<H', after, 0x9fca)[0],
                'complete_dgroup_bytes': 65536}
