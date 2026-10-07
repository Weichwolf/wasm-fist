"""Unchanged ae32/a6e3, actual aim visibility, display producer and bf3c admission.

Only the DOS/PM register transfer and admitted unconsumed PCM-device request are
explicit boundaries, as in discovery. All near/far return instructions execute.
"""
import struct

from original_target_discovery_oracle import OriginalTargetDiscoveryOracle
from original_unit_oracle import DGROUP, OriginalUnitOracle
from target_acquisition_contract import ACQUISITION_THRESHOLDS, TARGET_VOICES, acquisition, aim_positions

TEXT_BASE = 0x2d740
MAILBOX = 0x40000


class OriginalTargetAcquisitionOracle(OriginalTargetDiscoveryOracle):
    @staticmethod
    def execute(machine, start, stop, code_segment=0):
        # Unicorn shares translated blocks across observation boundaries. A new
        # entry/stop/segment requires eviction; an unchanged boundary and immutable
        # instructions can reuse its blocks. Independent replay tests compare
        # this policy with the existing unconditional-eviction observer.
        boundary = (start, stop, code_segment)
        if getattr(machine, '_fist_acquisition_boundary', None) != boundary:
            machine.ctl_remove_cache(0, 0x60000)
            machine._fist_acquisition_boundary = boundary
        OriginalUnitOracle.execute(machine, start, stop, code_segment)

    def __init__(self):
        super().__init__()
        self.enemy_text = struct.unpack_from('<28H', self.image, TEXT_BASE + 0x2daf)
        self.friendly_text = struct.unpack_from('<28H', self.image, TEXT_BASE + 0x2de7)
        self.variant_text = struct.unpack_from('<256H', self.image, TEXT_BASE + 0x2e1f)
        if self.image[DGROUP + 0x994a:DGROUP + 0x994e] != bytes(ACQUISITION_THRESHOLDS):
            raise AssertionError('Original acquisition probabilities differ')
        if self.image[DGROUP + 0x9702:DGROUP + 0x971e] != bytes(TARGET_VOICES):
            raise AssertionError('Original acquisition voice table differs')
        if self.image[0x1a4a2:0x1a4b2] != bytes.fromhex('3b3e346d7509a3a096c7069e967800cb'):
            raise AssertionError('Complete selected-target display producer differs')
        for enemy, table in ((True, self.enemy_text), (False, self.friendly_text)):
            for kind, handle in enumerate(table):
                expected = ('ENEMY' if enemy else 'FRIENDLY') + ' TARGET:'
                if kind < 4:
                    expected += ('M1', 'M3', 'T80', 'BMP')[kind]
                elif kind in (5, 6):
                    expected = 'TARGET:' + ('APACHE' if kind == 5 else 'HIND')
                elif kind == 26:
                    if handle != 65535:
                        raise AssertionError('Original type-26 variant sentinel differs')
                    continue
                elif kind == 27:
                    expected = ('ENEMY' if enemy else 'FRIENDLY') + ' TARGET LOCK: ARTILLERY'
                else:
                    expected = 'TARGET LOCK'
                if self.text(handle) != expected:
                    raise AssertionError('Original selected target message differs')
        for variant, label in enumerate(('FUEL TANK', 'SATELLITE DISH', 'PROPANE TANK', 'BUNKER')):
            if self.text(self.variant_text[variant]) != 'ENEMY TARGET:' + label:
                raise AssertionError('Original target-variant message differs')

    def text(self, handle):
        return self.image[TEXT_BASE + handle:TEXT_BASE + handle + 80].split(b'\0')[0].decode('ascii')

    @staticmethod
    def unchanged(before, after, spans, description):
        normalized = bytearray(after)
        for begin, end in spans:
            normalized[begin:end] = before[begin:end]
        if bytes(normalized) != before:
            changes = [hex(i) for i, (a, b) in enumerate(zip(before, normalized)) if a != b]
            raise AssertionError(description + ': ' + str(changes[:16]))

    def acquire(self, machine, pointer, kernel, *, automatic=True, candidate=0,
                selected=0, gate=0, clock=0, last_voice=0, side=512, pixels=None):
        from unicorn.x86_const import (UC_X86_REG_AX, UC_X86_REG_DI, UC_X86_REG_DX,
                                      UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_SP)
        actor = self.raw(machine, pointer)
        platoon = actor[27]
        descriptor = 0x85b6 + 22 * platoon
        behavior = struct.unpack('<H', machine.mem_read(DGROUP + descriptor, 2))[0]
        if automatic:
            candidate = struct.unpack_from('<H', actor, 0x9d)[0]
        old = struct.unpack_from('<H', actor, 0x97)[0]
        objects = {p: self.raw(machine, p) for p in (candidate, old) if p}
        for offset, value in ((0x9796, descriptor), (0x6d34, selected), (0x6da2, gate),
                              (0x452, clock), (0x9fca, last_voice)):
            machine.mem_write(DGROUP + offset, struct.pack('<H', value))
        machine.mem_write(DGROUP + 0xea2c, struct.pack('<HH', 0, MAILBOX // 16))
        machine.reg_write(UC_X86_REG_DI, pointer)
        machine.reg_write(UC_X86_REG_AX, candidate)
        machine.reg_write(UC_X86_REG_SP, 0x9000)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<H', 0xeff0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        code = bytes(machine.mem_read(0, DGROUP))
        text_data = bytes(machine.mem_read(TEXT_BASE, 65536))
        mailbox = bytes(machine.mem_read(MAILBOX, 4096))
        predicted = acquisition(self, pointer, actor, objects, self.random_state(machine), behavior,
                                side, pixels, automatic=automatic, candidate=candidate,
                                selected=selected, gate=gate, clock=clock, last_voice=last_voice,
                                display=struct.unpack_from('<HH', before, 0x969e))
        entry = 0xae32 if automatic else 0xa6e3
        transfer = None
        if predicted['transfer'] is not None:
            self.execute(machine, entry, 0xe2a0)
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x58\0':
                raise AssertionError('Actual acquisition reached another PM operation')
            source, target = struct.unpack('<3i', machine.mem_read(MAILBOX + 0xd2, 12)), struct.unpack(
                '<3i', machine.mem_read(MAILBOX + 0xde, 12))
            if (source, target) != aim_positions(self, actor, objects[candidate]):
                raise AssertionError('Actual acquisition aim operands differ')
            visible = self.visibility.visible(kernel, [(source, target)])[0]
            transfer = {'source': source, 'target': target, 'visible': visible}
            machine.reg_write(UC_X86_REG_EAX, kernel.reg_read(UC_X86_REG_EAX))
            entry = 0xe2a3
        requests = []
        if predicted['requests']:
            self.execute(machine, entry, 0xe2db)
            request = (machine.reg_read(UC_X86_REG_AX), machine.reg_read(UC_X86_REG_DX),
                       machine.reg_read(UC_X86_REG_ECX))
            if bytes(machine.mem_read(DGROUP + 0xea10, 2)) != b'\x64\0':
                raise AssertionError('Actual acquisition voice selected another operation')
            requests.append(request)
            # The actual bf3c tail consumes no device result. PCM stays open.
            entry = 0xe2de
        self.execute(machine, entry, 0xeff0)
        di, ds, sp, ss = self.machine_registers()
        if tuple(machine.reg_read(reg) for reg in (di, ds, sp, ss)) != (pointer, 0x1c00, 0x9002, 0x1c00):
            raise AssertionError('Complete acquisition near return/actor/segments differ')
        after = bytes(machine.mem_read(DGROUP, 65536))
        spans = ((pointer + 0x40, pointer + 0x42), (pointer + 0x97, pointer + 0x99),
                 (0x0342, 0x0344), (0x1f82, 0x1f8c), (0x8fc0, 0x9000),
                 (0x969e, 0x96a2), (0xea10, 0xea12), (0x9fca, 0x9fcc))
        self.unchanged(before, after, spans, 'Acquisition changed unrelated DGROUP/world')
        self.unchanged(mailbox, bytes(machine.mem_read(MAILBOX, 4096)),
                       ((0xd2, 0xea), (0x3f2, 0x3f6)), 'Acquisition changed unrelated mailbox')
        if bytes(machine.mem_read(0, DGROUP)) != code or bytes(machine.mem_read(TEXT_BASE, 65536)) != text_data:
            raise AssertionError('Acquisition changed code or read-only GS tables/text')
        display = struct.unpack_from('<HH', after, 0x969e)
        actual = {'actor': self.raw(machine, pointer), 'random': self.random_state(machine),
                  'draw': struct.unpack_from('<H', after, 0x0342)[0] if predicted['draw'] is not None else None,
                  'transfer': transfer, 'display': display,
                  'messages': [display[1]] if predicted['messages'] else [], 'requests': requests,
                  'last_voice': struct.unpack_from('<H', after, 0x9fca)[0], 'complete_dgroup_bytes': 65536}
        return actual, predicted

    def aim(self, machine, pointer, coarse):
        from unicorn.x86_const import UC_X86_REG_DI, UC_X86_REG_SI
        machine.mem_write(DGROUP + 0x2040, bytes([coarse]))
        machine.reg_write(UC_X86_REG_DI, pointer)
        machine.reg_write(UC_X86_REG_SI, 0x1234)
        machine.mem_write(DGROUP + 0x9000, struct.pack('<HH', 0xeff0, 0))
        before = bytes(machine.mem_read(DGROUP, 65536))
        code = bytes(machine.mem_read(0, DGROUP))
        random = self.random_state(machine)
        self.far_call(machine, 0x1a265)
        after = bytes(machine.mem_read(DGROUP, 65536))
        if machine.reg_read(UC_X86_REG_DI) != pointer:
            raise AssertionError('Complete target geometry changed actor')
        target = struct.unpack_from('<H', before, pointer + 0x97)[0]
        if target and machine.reg_read(UC_X86_REG_SI) != target:
            raise AssertionError('Complete target geometry changed loaded target identity')
        self.unchanged(before, after, ((pointer + 0x38, pointer + 0x3a),
                                     (pointer + 0x99, pointer + 0x9d),
                                     (0x2034, 0x203e), (0x9684, 0x969c), (0x8fc0, 0x9000)),
                       'Target geometry changed unrelated world')
        if self.random_state(machine) != random or bytes(machine.mem_read(0, DGROUP)) != code:
            raise AssertionError('Target geometry changed RNG or instructions')
        return self.raw(machine, pointer)
