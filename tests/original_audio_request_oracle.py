"""Complete unchanged original op-64 returns with original read-only sound banks.

Only valid caller bank/device/queue state is supplied. Every kernel byte,
general register, bounded stack return and unrelated mapped buffer is checked.
PCM mixing/device initialization is outside this original queue contract.
"""
import pathlib
import struct

from audio_request_contract import BANK_PINS, SILENCE, bank_records, request
from original_ground_oracle import BUFFER, DGROUP, RETURN, ROSTER, STACK, OriginalGroundOracle

ROOT = pathlib.Path(__file__).resolve().parents[1]
BANK_BASES = {'DSOUNDS.BIN': 0x4000000, 'WVSOUNDS.BIN': 0x5000000, 'EVSOUNDS.BIN': 0x6000000}


class OriginalAudioRequestOracle:
    def __init__(self):
        from unicorn import UC_PROT_READ
        from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                      UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                                      UC_X86_REG_EBP)
        self.owner = OriginalGroundOracle()
        self.machine = self.owner.prepare(2, bytes(4))
        self.baseline = bytes(self.machine.mem_read(0, len(self.owner.image)))
        if struct.unpack_from('<I', self.baseline, 0xcb3 + 0x64)[0] != 0x786a:
            raise AssertionError('Original op-64 dispatch entry differs')
        self.registers = dict(zip(('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp'),
                                 (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                                  UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)))
        self.assets = {}
        self.placements = {}
        self.banks = {}
        for name in BANK_PINS:
            path = ROOT / 'armoredfist/FISTDATA' / name
            if path.stat().st_mode & 0o222:
                raise AssertionError('Provisioned original sound bank must be read-only')
            data = path.read_bytes()
            records = bank_records(name, data)
            size = (len(data) + 255 + 4095) & ~4095
            base = BANK_BASES[name]
            self.machine.mem_map(base, size)
            self.machine.mem_write(base, data)
            self.machine.mem_protect(base, size, UC_PROT_READ)
            self.assets[name] = (data, records, size)
            self.placements[name] = 0
            self.banks[base] = (data, records)
        self.guards = {address: bytes(self.machine.mem_read(address, size))
                       for address, size in ((BUFFER, 4096), (DGROUP, 65536), (ROSTER, 4096),
                                             (RETURN, 4096), (len(self.baseline), (-len(self.baseline)) % 4096))}
        self.stack = bytes(self.machine.mem_read(STACK, 4096))
        self.fixture_count = 0

    def fresh_machine(self):
        """Bound translator storage between independent caller fixtures.

        Unchanged instruction execution and all per-case guards remain required.
        Sequential requests do not reset or recycle their retained state.
        """
        from unicorn import UC_PROT_READ
        machine = self.owner.prepare(2, bytes(4))
        if bytes(machine.mem_read(0, len(self.baseline))) != self.baseline:
            raise AssertionError('Fresh original kernel fixture differs')
        for name, (data, _, size) in self.assets.items():
            base, offset = BANK_BASES[name], self.placements[name]
            machine.mem_map(base, size)
            machine.mem_write(base + offset, data)
            machine.mem_protect(base, size, UC_PROT_READ)
        self.machine = machine
        self.fixture_count = 0

    def place(self, name, offset):
        from unicorn import UC_PROT_ALL, UC_PROT_READ
        if not 0 <= offset < 256:
            raise ValueError('Bank placement fixture is outside a full low-byte turn')
        if self.placements[name] == offset:
            return
        data, records, size = self.assets[name]
        base = BANK_BASES[name]
        del self.banks[base + self.placements[name]]
        mapped = bytearray(size)
        mapped[offset:offset + len(data)] = data
        self.machine.mem_protect(base, size, UC_PROT_ALL)
        self.machine.mem_write(base, bytes(mapped))
        self.machine.mem_protect(base, size, UC_PROT_READ)
        self.placements[name] = offset
        self.banks[base + offset] = (data, records)

    def fixture(self, *, packet=11, pitch=0, attenuation=0, enabled=1,
                voice='WVSOUNDS.BIN', missing_effects=False, missing_voice=False, busy=False):
        if self.fixture_count == 256:
            self.fresh_machine()
        self.fixture_count += 1
        data = bytearray(self.baseline)
        data[0x2293] = enabled
        struct.pack_into('<I', data, 0x85b0, 0 if missing_effects else BANK_BASES['DSOUNDS.BIN'] + self.placements['DSOUNDS.BIN'])
        struct.pack_into('<I', data, 0x85b4, 0 if missing_voice else BANK_BASES[voice] + self.placements[voice])
        if busy:
            for channel in range(3):
                struct.pack_into('<I', data, 0x15d7 + 4 * channel,
                                 BANK_BASES['DSOUNDS.BIN'] + self.placements['DSOUNDS.BIN'] + 2)
        else:
            for channel in range(3):
                struct.pack_into('<I', data, 0x15d7 + 4 * channel, SILENCE)
        registers = {'eax': packet, 'ebx': 39, 'ecx': pitch, 'edx': 0xa500 | attenuation,
                     'esi': 0x11223344, 'edi': 0xaaaa1111, 'ebp': 0x55667788}
        self.machine.mem_write(0, bytes(data))
        self.machine.mem_write(STACK, self.stack)
        for name, value in registers.items():
            self.machine.reg_write(self.registers[name], value)
        return bytes(data), registers

    def observe(self, before, registers):
        expected, predicted_registers, effect = request(before, registers, self.banks)
        self.owner.call(self.machine, 0x786a)
        actual = bytes(self.machine.mem_read(0, len(self.baseline)))
        if actual != expected:
            differences = [hex(i) for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
            raise AssertionError('Original queue/kernel writes differ: ' + str(differences[:16]))
        actual_registers = {name: self.machine.reg_read(register) for name, register in self.registers.items()}
        if actual_registers != predicted_registers:
            raise AssertionError(f'Original queue registers differ: {actual_registers!r} != {predicted_registers!r}')
        stack = bytes(self.machine.mem_read(STACK, 4096))
        # Original near call, ECX/ESI saves and return slots only. Deepest path
        # uses twelve bytes below entry ESP; guard the remainder independently.
        if stack[:0xfe4] != self.stack[:0xfe4] or stack[0xff4:] != self.stack[0xff4:]:
            raise AssertionError('Original queue escaped bounded stack scratch')
        for address, guard in self.guards.items():
            if bytes(self.machine.mem_read(address, len(guard))) != guard:
                raise AssertionError('Original queue changed an unrelated mapped buffer')
        return actual, actual_registers, effect

    def verify_assets(self):
        from unicorn import UC_PROT_READ
        regions = {begin: (end, permissions) for begin, end, permissions in self.machine.mem_regions()}
        for name, (data, _, size) in self.assets.items():
            offset = self.placements[name]
            expected = bytes(offset) + data + bytes(size - len(data) - offset)
            if bytes(self.machine.mem_read(BANK_BASES[name], size)) != expected:
                raise AssertionError('Original queue changed sound bank bytes or guards')
            if regions[BANK_BASES[name]] != (BANK_BASES[name] + size - 1, UC_PROT_READ):
                raise AssertionError('Original sound bank is not mapped read-only')

    def effects_allocation(self, heap_low_byte):
        """Prove legitimate placements with the actual loader's full allocator.

        The XMS heap frontier is an explicit initialized caller input. DOS/XMS
        initialization and file I/O remain outside this request contract.
        """
        if not 0 <= heap_low_byte < 256:
            raise ValueError('Heap fixture must use one complete low-byte turn')
        before, registers = self.fixture(packet=3, pitch=len(self.assets['DSOUNDS.BIN'][0]))
        data = bytearray(before)
        frontier = BANK_BASES['DSOUNDS.BIN'] + heap_low_byte
        length = registers['ecx']
        struct.pack_into('<I', data, 0x2f50, frontier)
        struct.pack_into('<I', data, 0x2f54, 0)
        struct.pack_into('<I', data, 0x90f, BANK_BASES['DSOUNDS.BIN'] + self.assets['DSOUNDS.BIN'][2])
        expected = bytearray(data)
        aligned = (frontier + 3) & ~3
        end = aligned + length
        writes = {0x2f5c: RETURN, 0x28ac: 0x85b0, 0x2bcc: length, 0x2d5c: 4,
                  0x2f50: end, 0x85b0: aligned, 0x2a3c: aligned, 0x2f54: 1,
                  0x2f68: max(struct.unpack_from('<I', data, 0x2f68)[0], 1),
                  0x2f6c: max(struct.unpack_from('<I', data, 0x2f6c)[0], end)}
        for offset, value in writes.items():
            struct.pack_into('<I', expected, offset, value)
        expected[0x2eec] = 3
        registers.update(ebx=4, edx=0x85b0)
        self.machine.mem_write(0, bytes(data))
        for name, value in registers.items():
            self.machine.reg_write(self.registers[name], value)
        self.owner.call(self.machine, 0x36bf)
        if bytes(self.machine.mem_read(0, len(expected))) != expected:
            raise AssertionError('Actual complete sound-bank allocation writes differ')
        predicted = dict(registers, eax=aligned, ebx=0x85b0, edx=3, esi=0, ebp=3)
        actual = {name: self.machine.reg_read(register) for name, register in self.registers.items()}
        if actual != predicted:
            raise AssertionError(f'Actual complete sound-bank allocation registers differ: {actual!r}')
        stack = bytes(self.machine.mem_read(STACK, 4096))
        if stack[:0xfe4] != self.stack[:0xfe4] or stack[0xff4:] != self.stack[0xff4:]:
            raise AssertionError('Actual sound allocation escaped bounded stack scratch')
        for address, guard in self.guards.items():
            if bytes(self.machine.mem_read(address, len(guard))) != guard:
                raise AssertionError('Actual sound allocation changed an unrelated buffer')
        return aligned
