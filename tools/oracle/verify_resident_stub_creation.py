"""Verify original resident setup and every DWORD CALL-stub creation instruction."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

from resident_image import load, relocate
from sequence_format import validate, validate_endpoint


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(root, repo):
    for name in ('baseline', 'source'):
        assert (root / (name + '.exit')).read_text() == '0\n'
        prefix = root / name / 'sequence'
        assert validate_endpoint(prefix, 600) == 600
        assert validate(str(prefix) + '.frames', 'F')['records'] == 39
        assert validate(str(prefix) + '.pcm', 'A')['samples'] == 27518
        assert 'Python Exception' not in (root / name / 'dosbox.log').read_text()
    for suffix in ('frames', 'pcm', 'end'):
        assert (root / 'baseline' / ('sequence.' + suffix)).read_bytes() == (
            root / 'source' / ('sequence.' + suffix)).read_bytes(), suffix
    for path, value in json.loads((root / 'producers.json').read_text()).items():
        assert digest(path) == value, path
    originals = json.loads((root / 'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo / 'armoredfist').rglob('*')
            if p.is_file()} == set(originals)
    for path, value in originals.items():
        assert digest(repo / path) == value, path
    prior = json.loads((repo / 'tools/oracle/sound_vector_init_case.json').read_text())
    output_hashes = {s: digest(root / 'source' / ('sequence.' + s))
                     for s in ('frames', 'pcm', 'end')}
    assert output_hashes == prior['capture_sha256']
    folder = root / 'source'
    rows = [json.loads(line) for line in (folder / 'stub-fetches.jsonl').read_text().splitlines()]
    prefix_ips = [0x4242, 0x4247, 0x424b, 0x424e, 0x4251, 0x4253, 0x4255,
                  0x4257, 0x425b, 0x425f, 0x4263, 0x4267, 0x426c, 0x4270,
                  0x4277, 0x427b, 0x427e, 0x4283, 0x4288, 0x428a, 0x428c,
                  0x428f, 0x4293, 0x4296, 0x429a]
    assert [q['cpu_regs.ip.dword[0]'] for q in rows] == (
        prefix_ips + [0x429b, 0x429d, 0x42a3]*256 + [0x42a5])
    boundaries = {0x4242, 0x4257, 0x4267, 0x4288, 0x428f, 0x4293, 0x4296, 0x429a, 0x42a5}
    label = lambda q: '%04x-%03d' % (q['cpu_regs.ip.dword[0]'], q['stores'])
    expected_states = {label(q) for q in rows
                       if q['cpu_regs.ip.dword[0]'] in boundaries or
                       q['cpu_regs.ip.dword[0]'] == 0x429b and q['stores'] in (1, 256)}
    states = {p.stem: json.loads(p.read_text()) for p in folder.glob('*.json')}
    assert set(states) == expected_states and len(states) == 11
    ram = {name: (folder / (name + '.memory')).read_bytes() for name in states}
    assert all(len(m) == 16777216 for m in ram.values())
    resident = load(repo)
    code = relocate(resident, rows[0]['segments'][1]['base'])
    lengths = dict(zip(prefix_ips, [5, 4, 3, 3, 2, 2, 2, 4, 4, 4, 4, 5, 4,
                                    7, 4, 3, 5, 2, 2, 2, 3, 4, 3, 4, 1]))
    lengths.update({0x429b: 2, 0x429d: 6, 0x42a3: 2})
    count = 0
    for q in rows:
        ip = q['cpu_regs.ip.dword[0]']
        assert not q['paging.enabled'] and not q['cpu.pmode'] and not q['cpu.code.big']
        assert not q['cpu.stack.big'] and q['cpu.stack.mask'] == 65535
        assert q['cpu.stack.notmask'] == 0xffff0000
        assert q['segments'][1] == rows[0]['segments'][1]
        assert q['fetched_code_physical'] == q['segments'][1]['base'] + ip
        assert bytes.fromhex(q['fetched_code_hex']) == code[ip:ip+32], label(q)
        if ip == 0x429b:
            count += 1
        assert q['stores'] == count
        if label(q) in states:
            assert states[label(q)] == q, label(q)
            address = q['fetched_code_physical']
            assert ram[label(q)][address:address+32] == code[ip:ip+32], label(q)
    lazy_path = repo / 'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h'
    lazy = lazy_path.read_text().split('//Types of Flag changing instructions', 1)[1]
    names = re.findall(r'\bt_[A-Za-z0-9_]+\b', lazy.split('enum {', 1)[1].split('};', 1)[0])
    types = {name: index for index, name in enumerate(names)}
    memory = bytearray(ram[label(rows[0])])
    transitions, memory_transitions = [], []
    clock = lambda q: q['PIC_Ticks']*q['CPU_CycleMax'] + q['CPU_CycleMax'] - q['CPU_Cycles'] - q['CPU_CycleLeft']
    stores = []
    for q, after in zip(rows, rows[1:]):
        ip = q['cpu_regs.ip.dword[0]']
        ins = code[ip:ip+lengths[ip]]
        regs = q['registers'].copy()
        segments = [s.copy() for s in q['segments']]
        flags = {k: v for k, v in q.items() if k == 'cpu_regs.flags' or k.startswith('lflags.')}
        next_ip = ip + len(ins)
        def set_reg(index, value, bits):
            mask = (1 << bits) - 1
            regs[index] = (regs[index] & (0xffffffff ^ mask)) | (value & mask)
        def set_lazy_field(name, value, bits):
            key = 'lflags.' + name + '.dword[0]'
            mask = (1 << bits) - 1
            flags[key] = (flags[key] & (0xffffffff ^ mask)) | (value & mask)
        def alu(kind, bits, a, b, result):
            for field, value in (('var1', a), ('var2', b), ('res', result)):
                set_lazy_field(field, value, bits)
            flags['lflags.type'] = types['t_' + kind + {8: 'b', 16: 'w', 32: 'd'}[bits]]
        def flag(bit, value):
            flags['cpu_regs.flags'] = (flags['cpu_regs.flags'] & ~bit) | (bit if value else 0)
        def u16(offset):
            return struct.unpack_from('<H', ins, offset)[0]
        def u32(offset):
            return struct.unpack_from('<I', ins, offset)[0]
        def flat(offset, segment=3):
            return segments[segment]['base'] + offset
        if ip == 0x4242:
            regs[2] = struct.unpack_from('<I', memory, flat(u16(3)))[0]
        elif ip in (0x4247, 0x425f, 0x428f):
            index = 0 if ip == 0x428f else 2
            value, shift = regs[index], ins[3]
            result = value >> shift if ip == 0x4247 else (value << shift) & 0xffffffff
            set_lazy_field('var1', value, 32)
            set_lazy_field('var2', shift, 8)
            set_lazy_field('res', result, 32)
            flags['lflags.type'] = types['t_SHRd' if ip == 0x4247 else 't_SHLd']
            regs[index] = result
        elif ip in (0x424b, 0x4253, 0x425b, 0x4288):
            index = 0 if ip == 0x4288 else 2
            a = regs[index] & 65535
            b = ins[2] if ip == 0x424b else regs[3] & 65535 if ip == 0x4253 else u16(2) if ip == 0x425b else regs[2] & 65535
            subtract = ip in (0x425b, 0x4288)
            result = a-b if subtract else a+b
            alu('SUB' if subtract else 'ADD', 16, a, b, result)
            set_reg(index, result, 16)
        elif ip in (0x424e, 0x428a):
            index, bits = (3, 32) if ip == 0x424e else (7, 16)
            value = regs[index] & ((1 << bits)-1)
            alu('XOR', bits, value, value, 0)
            set_reg(index, 0, bits)
        elif ip == 0x4251:
            set_reg(3, segments[2]['value'], 16)
        elif ip == 0x4255:
            value = regs[2] & 65535
            segments[0] = dict(value=value, base=value << 4)
        elif ip in (0x4257, 0x4263, 0x426c):
            value = regs[0] if ip == 0x426c else regs[2]
            struct.pack_into('<H', memory, flat(u16(2), 1 if ip == 0x426c else 3), value & 65535)
        elif ip == 0x4267:
            regs[0] = (regs[2] + struct.unpack_from('<b', ins, 4)[0]) & 0xffffffff
        elif ip == 0x4270:
            alu('CMP', 32, regs[2], u32(3), regs[2]-u32(3))
        elif ip == 0x4277:
            assert flags['lflags.type'] == types['t_CMPd']
            if flags['lflags.var1.dword[0]'] > flags['lflags.var2.dword[0]']:
                next_ip += struct.unpack_from('<h', ins, 2)[0]
        elif ip in (0x427b, 0x428c, 0x4293):
            set_reg(1 if ip == 0x428c else 0, u16(1), 16)
        elif ip == 0x427e:
            value = memory[flat(u16(2))]
            alu('CMP', 8, value, ins[4], value-ins[4])
        elif ip == 0x4283:
            assert flags['lflags.type'] == types['t_CMPb']
            if flags['lflags.res.dword[0]'] & 255:
                next_ip += struct.unpack_from('<b', ins, 1)[0]
        elif ip == 0x4296:
            # Original RORD invokes FillFlagsNoCFOF before updating rotate CF/OF.
            assert flags['lflags.type'] == types['t_SHLd']
            result = flags['lflags.res.dword[0]']
            flag(0x40, result == 0)
            flag(0x80, result & 0x80000000)
            flag(0x04, (result & 255).bit_count() % 2 == 0)
            flag(0x10, flags['lflags.var2.dword[0]'] & 31)
            flags['lflags.type'] = types['t_UNKNOWN']
            value, shift = regs[0], ins[3]
            result = ((value >> shift) | (value << (32-shift))) & 0xffffffff
            set_lazy_field('var1', value, 32)
            set_lazy_field('var2', shift, 8)
            set_lazy_field('res', result, 32)
            regs[0] = result
            flag(1, result & 0x80000000)
            flag(0x800, (result ^ (result << 1)) & 0x80000000)
        elif ip == 0x429a:
            flag(0x400, False)
        elif ip == 0x429b:
            address = flat(regs[7] & 65535, 0)
            struct.pack_into('<I', memory, address, regs[0])
            stores.append(dict(address=address, eax=regs[0]))
            set_reg(7, regs[7] + (-4 if flags['cpu_regs.flags'] & 0x400 else 4), 16)
        elif ip == 0x429d:
            a, b = regs[0], u32(2)
            regs[0] = (a-b) & 0xffffffff
            alu('SUB', 32, a, b, regs[0])
        elif ip == 0x42a3:
            set_reg(1, regs[1]-1, 16)
            if regs[1] & 65535:
                next_ip += struct.unpack_from('<b', ins, 1)[0]
        else:
            raise AssertionError(hex(ip))
        assert after['cpu_regs.ip.dword[0]'] == next_ip, (label(q), 'IP')
        assert after['registers'] == regs, (label(q), 'all GP')
        assert after['segments'] == segments, (label(q), 'segments')
        for key, value in flags.items():
            assert after[key] == value, (label(q), key, hex(after[key]), hex(value))
        for key in q:
            if key.startswith(('cpu.', 'paging.')) or key in ('PIC_Ticks', 'CPU_CycleMax', 'CPU_CycleLeft'):
                assert after[key] == q[key], (label(q), key)
        assert clock(after)-clock(q) == 1, (label(q), 'instruction retirement')
        transitions.append([label(q), label(after)])
        if label(after) in ram:
            assert bytes(memory) == ram[label(after)], (label(q), label(after), 'whole RAM')
            memory_transitions.append(label(after))
    assert len(transitions) == 793 and len(stores) == 256 and len(memory_transitions) == 10
    first = rows[len(prefix_ips)]
    start = stores[0]['address']
    table_offset = start - first['segments'][1]['base']
    assert struct.unpack_from('<H', memory, rows[0]['segments'][3]['base']+0x206)[0] == table_offset
    assert struct.unpack_from('<H', memory, first['segments'][1]['base']+0x2c3a)[0] == table_offset+3
    targets = []
    for index, store in enumerate(stores):
        address = store['address']
        assert address == start + index*4
        stub = memory[address:address+4]
        assert stub[0] == 0xe8 and stub[3] == 0
        target = (table_offset + index*4 + 3 + struct.unpack_from('<h', stub, 1)[0]) & 65535
        targets.append(target)
    assert len(set(targets)) == 1 and targets[0] == 0x2c3c
    proof = dict(scope='Original resident4242 setup, nested-MZ relocated operand425d, '
                 'all793 GP/segment/raw-lazyflag/control/time instruction transitions '
                 'and ten complete16MiB RAM transitions through all256 DWORD CALL '
                 'stub stores. Real16-bit DI/CX preserve upper register words. '
                 'Other setup branches, vector installation, port CPU/IRQ/time '
                 'integration and complete frame/mixed PCM acceptance remain open.',
                 fetches=rows, states=states, transitions=transitions,
                 whole_RAM_transitions=memory_transitions, stores=stores,
                 table_start_physical=start, table_offset=table_offset,
                 table_bytes=1024, call_target=targets[0],
                 mz_header=resident['header'], mz_image_offset=resident['image_offset'],
                 resident_file_bias=resident['file_bias'], mz_initial_cs=resident['initial_cs'],
                 mz_relocations=resident['relocations'],
                 actual_mz_load_segment=(rows[0]['segments'][1]['base'] >> 4)-resident['initial_cs'],
                 memory_sha256={n: digest(folder / (n + '.memory')) for n in states},
                 capture_sha256=output_hashes, frames=39, mixed_samples=27518, endpoint_ms=600,
                 originals_unchanged=len(originals), producers=json.loads((root/'producers.json').read_text()),
                 verifier_sha256=digest(Path(__file__)), complete_original_acceptance=False)
    (root / 'proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    print('PASS: original794 fetches/793 complete instruction/10 whole-RAM transitions; '
          '256 DWORD stubs, upper register words and relocated operands; all39frames/27518PCM preserved')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    verify(args.output.resolve(strict=True), args.repo.resolve(strict=True))
