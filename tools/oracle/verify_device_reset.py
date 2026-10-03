"""Verify the actual device tail dispatch and complete default DSP reset path."""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import re
import struct

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
    for suffix in ('text', 'bda', 'vga'):
        assert (root/'baseline'/('start-state.'+suffix)).read_bytes() == (
            root/'source'/('start-state.'+suffix)).read_bytes(), suffix
    producers = json.loads((root / 'producers.json').read_text())
    required = ('tools/oracle/capture_device_reset.py', 'tools/oracle/device_reset.gdb',
                'tools/oracle/verify_device_reset.py', 'tools/oracle/file_error.gdb',
                'tools/oracle/capture_sequence.sh', 'tools/oracle/sequence_format.py',
                'tools/oracle/sb_reset_clock_case.json', 'tools/oracle/sound_bank_startup_case.json',
                're_out/fist_image.bin', 'armoredfist/FIST.RUN', 'third_party/dosbox-fist')
    cpu_source = 'third_party/dosbox-build/dosbox-0.74-3/'
    source_paths = ('src/cpu/instructions.h', 'src/cpu/core_normal/prefix_66.h',
                    'include/cpu.h', 'src/cpu/lazyflags.h', 'src/cpu/core_normal.cpp',
                    'src/cpu/core_normal/prefix_none.h', 'src/cpu/core_normal/prefix_0f.h',
                    'src/cpu/flags.cpp', 'src/cpu/cpu.cpp', 'src/hardware/pic.cpp',
                    'src/hardware/iohandler.cpp', 'src/hardware/sblaster.cpp')
    assert set(producers) == {str(repo/p) for p in required + tuple(cpu_source+p for p in source_paths)}
    assert producers[str(Path(__file__).resolve())] == digest(__file__)
    for path, value in producers.items():
        assert digest(path) == value, path
    originals = json.loads((root / 'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo / 'armoredfist').rglob('*')
            if p.is_file()} == set(originals)
    for path, value in originals.items():
        assert digest(repo / path) == value, path
    bank = json.loads((repo / 'tools/oracle/sound_bank_startup_case.json').read_text())
    output_hashes = {s: digest(root / 'source' / ('sequence.' + s))
                     for s in ('frames', 'pcm', 'end')}
    assert output_hashes == bank['capture_sha256']
    shared = repo / 'tools/oracle/file_error.gdb'
    program = shared.read_text().split('\npython\n', 1)[1].rsplit('\nend\nrun', 1)[0]
    nodes = [n for n in ast.parse(program).body
             if isinstance(n, ast.FunctionDef) and n.name == 'physical']
    assert len(nodes) == 1
    namespace = {'struct': struct}
    exec(compile(ast.fix_missing_locations(ast.Module(body=nodes, type_ignores=[])),
                 str(shared), 'exec'), namespace)
    physical = namespace['physical']
    tree = repo / 'third_party/dosbox-build/dosbox-0.74-3'
    lazy = (tree / 'src/cpu/lazyflags.h').read_text().split(
        '//Types of Flag changing instructions', 1)[1]
    names = re.findall(r'\bt_[A-Za-z0-9_]+\b', lazy.split('enum {', 1)[1].split('};', 1)[0])
    types = {name: index for index, name in enumerate(names)}
    image = (repo / 're_out/fist_image.bin').read_bytes()
    case = json.loads((repo / 'tools/oracle/sb_reset_clock_case.json').read_text())
    assert digest(repo / 're_out/fist_image.bin') == case['image_sha256']
    assert digest(tree / 'src/hardware/sblaster.cpp') == case['device_source_sha256']
    assert digest(tree / 'src/hardware/pic.cpp') == case['pic_source_sha256']
    code = bytes.fromhex(case['code_bytes'])
    assert image[case['code_offset']:case['code_offset']+len(code)] == code
    gold = case['original_rows']
    clock_rows = [list(map(int, line.split()))
                  for line in case['original_clock_output'].splitlines()]
    assert len(gold) == len(clock_rows) == 218
    folder = root / 'source'
    rows = [json.loads(line) for line in (folder / 'reset-fetches.jsonl').read_text().splitlines()]
    assert len(rows) == 225
    states = {p.stem: json.loads(p.read_text()) for p in folder.glob('*.json')}
    return_ip = rows[0]['entry_return_ip']
    assert set(states) == {'%04x' % ip for ip in
                           (0x23c4, 0x133a, 0x133b, 0x1357, 0x137f, 0x1380, 0x138c, return_ip)}
    ram = {name: (folder / (name + '.memory')).read_bytes() for name in states}
    assert all(len(m) == 16777216 for m in ram.values())
    memory = ram['23c4']
    for name, q in states.items():
        assert [r for r in rows if r['cpu_regs.ip.dword[0]'] == int(name, 16)] == [q], name
        assert ram[name] == memory, (name, 'complete unchanged RAM')
    clock = lambda q: (q['PIC_Ticks']*q['CPU_CycleMax'] + q['CPU_CycleMax']
                       - q['CPU_Cycles'] - q['CPU_CycleLeft'])
    first = rows[0]
    assert first['cpu.code.big'] == first['cpu.stack.big'] == first['cpu.pmode'] == 1
    assert first['paging.enabled'] == 1 and first['cpu.cpl'] == 3
    assert first['cpu.stack.mask'] == 0xffffffff and first['cpu.stack.notmask'] == 0
    assert first['CPU_CycleMax'] == 30000
    configured = bank['states']['after-device-configuration']
    incoming = configured['registers'].copy()
    incoming[4] -= 4
    assert first['registers'] == incoming and first['segments'] == configured['segments']
    def address(q, segment, offset):
        return physical((q['segments'][segment]['base'] + offset) & 0xffffffff, memory, q)
    def read(q, segment, offset, width):
        p = address(q, segment, offset)
        return int.from_bytes(memory[p:p+width], 'little')
    stack_return = read(first, 2, first['registers'][4], 4)
    assert return_ip == stack_return == 0x77ff
    caller_ip = return_ip - 5
    assert image[caller_ip] == 0xe8
    assert return_ip + struct.unpack_from('<i', image, caller_ip+1)[0] == 0x23c4
    assert memory[address(first, 1, caller_ip):address(first, 1, caller_ip)+5] == image[caller_ip:return_ip]
    assert [q['cpu_regs.ip.dword[0]'] for q in rows[:6]] == [0x23c4, 0x23c6, 0x23c9, 0x23ce, 0x23d1, 0x23d8]
    assert [q['cpu_regs.ip.dword[0]'] for q in rows[6:-1]] == [r['ip'] for r in gold]
    assert rows[-1] == states['77ff']
    invariant_keys = [k for k in first if k.startswith(('cpu.', 'paging.'))]
    for q in rows:
        assert set(q) == set(first)
        assert q['entry_return_ip'] == return_ip
        assert q['segments'] == first['segments']
        for key in invariant_keys + ['PIC_Ticks', 'CPU_CycleMax']:
            assert q[key] == first[key], (hex(q['cpu_regs.ip.dword[0]']), key)
        ip = q['cpu_regs.ip.dword[0]']
        p = address(q, 1, ip)
        assert q['fetched_code_physical'] == p
        assert bytes.fromhex(q['fetched_code_hex']) == memory[p:p+32] == image[ip:ip+32], hex(ip)
    # Consume the existing actual-original PIC/SB/I/O clock owner. No second
    # event scheduler, fixed poll count or fitted elapsed delay is used.
    for q, event, timing in zip(rows[6:-1], gold, clock_rows):
        assert clock(q) == event['cycle'] == timing[0]
        operation = 'r' if image[event['ip']] == 0xec else 'w' if image[event['ip']] == 0xee else 'n'
        assert event['op'] == operation
        if operation != 'n':
            assert q['registers'][2] & 65535 == event['port']
        if operation == 'w':
            assert q['registers'][0] & 255 == event['value']
    assert clock(rows[6])-1 == case['start_cycle_before_fetch']
    lengths = {0x23c4: 2, 0x23c6: 3, 0x23c9: 2, 0x23ce: 3, 0x23d1: 7, 0x23d8: 2,
               0x133a: 1, 0x133b: 7, 0x1342: 4, 0x1346: 2, 0x1348: 1, 0x1349: 2,
               0x134b: 5, 0x1350: 1, 0x1351: 2, 0x1353: 2, 0x1355: 2, 0x1357: 1,
               0x1358: 2, 0x135a: 5, 0x135f: 7, 0x1366: 4, 0x136a: 1, 0x136b: 2,
               0x136d: 2, 0x136f: 2, 0x1371: 7, 0x1378: 4, 0x137c: 1, 0x137d: 2,
               0x137f: 1, 0x1380: 2, 0x1382: 2, 0x1387: 5, 0x138c: 1}
    transitions, exits, io = [], [], []
    reset_high = False
    for index, (q, after) in enumerate(zip(rows, rows[1:])):
        ip = q['cpu_regs.ip.dword[0]']
        ins = image[ip:ip+lengths[ip]]
        next_ip = ip + len(ins)
        regs = q['registers'].copy()
        flags = {k: v for k, v in q.items() if k == 'cpu_regs.flags' or k.startswith('lflags.')}
        def set_reg(reg, value, bits):
            mask = (1 << bits)-1
            regs[reg] = (regs[reg] & (0xffffffff ^ mask)) | (value & mask)
        def alu(kind, bits, a, b, result):
            mask = (1 << bits)-1
            for field, value in (('var1', a), ('var2', b), ('res', result)):
                key = 'lflags.'+field+'.dword[0]'
                flags[key] = (flags[key] & (0xffffffff ^ mask)) | (value & mask)
            flags['lflags.type'] = types['t_'+kind+{8: 'b', 16: 'w'}[bits]]
        def zf():
            if flags['lflags.type'] == types['t_UNKNOWN']:
                return bool(flags['cpu_regs.flags'] & 0x40)
            assert flags['lflags.type'] in (types['t_CMPb'], types['t_TESTb'])
            return flags['lflags.res.dword[0]'] & 255 == 0
        def fill_flags():
            assert flags['lflags.type'] in (types['t_XORb'], types['t_TESTb'])
            result = flags['lflags.res.dword[0]'] & 255
            for bit, value in ((1, False), (0x10, False), (0x800, False),
                               (0x40, result == 0), (0x80, result & 0x80),
                               (4, result.bit_count() % 2 == 0)):
                flags['cpu_regs.flags'] = (flags['cpu_regs.flags'] & ~bit) | (bit if value else 0)
            flags['lflags.type'] = types['t_UNKNOWN']
        if ip == 0x23c4:
            assert ins == bytes.fromhex('8ad8')
            set_reg(3, regs[0], 8)
        elif ip == 0x23c6:
            assert ins[:2] == bytes.fromhex('80fb')
            value = regs[3] & 255
            alu('CMP', 8, value, ins[2], value-ins[2])
        elif ip == 0x23c9:
            assert ins[0] == 0x76 and flags['lflags.type'] == types['t_CMPb']
            carry = (flags['lflags.var1.dword[0]'] & 255) < (flags['lflags.var2.dword[0]'] & 255)
            if carry or zf():
                next_ip += struct.unpack_from('<b', ins, 1)[0]
        elif ip == 0x23ce:
            assert ins == bytes.fromhex('0fb6db')
            regs[3] &= 255
        elif ip == 0x23d1:
            assert ins[:3] == bytes.fromhex('8b049d')
            offset = struct.unpack_from('<I', ins, 3)[0] + regs[3]*4
            regs[0] = read(q, 3, offset, 4)
        elif ip == 0x23d8:
            assert ins == bytes.fromhex('ffe0')
            next_ip = regs[0]
        elif ins in (b'\xfa', b'\xfb'):
            assert not flags['cpu_regs.flags'] & 0x20000
            assert ((flags['cpu_regs.flags'] >> 12) & 3) >= q['cpu.cpl']
            flags['cpu_regs.flags'] = (flags['cpu_regs.flags'] & ~0x200) | (0x200 if ins == b'\xfb' else 0)
        elif ins[:3] == bytes.fromhex('668b15'):
            set_reg(2, read(q, 3, struct.unpack_from('<I', ins, 3)[0], 2), 16)
        elif ins[:3] == bytes.fromhex('6683c2'):
            a, b = regs[2] & 65535, struct.unpack_from('<b', ins, 3)[0]
            alu('ADD', 16, a, b, a+b)
            set_reg(2, a+b, 16)
        elif ins[0] == 0xb0:
            set_reg(0, ins[1], 8)
        elif ins[0] in (0xb8, 0xb9):
            regs[ins[0]-0xb8] = struct.unpack_from('<I', ins, 1)[0]
        elif ins[0] == 0xeb:
            next_ip += struct.unpack_from('<b', ins, 1)[0]
        elif ins[0] in (0xe1, 0xe2):
            regs[1] = (regs[1]-1) & 0xffffffff
            if regs[1] and (ins[0] == 0xe2 or zf()):
                next_ip += struct.unpack_from('<b', ins, 1)[0]
        elif ins == bytes.fromhex('32c0'):
            value = regs[0] & 255
            alu('XOR', 8, value, value, 0)
            set_reg(0, 0, 8)
        elif ins[0] in (0xa8, 0x3c):
            value = regs[0] & 255
            test = ins[0] == 0xa8
            alu('TEST' if test else 'CMP', 8, value, ins[1], value & ins[1] if test else value-ins[1])
        elif ins[0] == 0x74:
            if zf():
                next_ip += struct.unpack_from('<b', ins, 1)[0]
        elif ins == b'\xec':
            set_reg(0, gold[index-6]['value'], 8)
        elif ins == b'\xee':
            pass
        elif ins == b'\xc3':
            next_ip = read(q, 2, regs[4] & q['cpu.stack.mask'], 4)
            regs[4] = (regs[4] & q['cpu.stack.notmask']) | ((regs[4]+4) & q['cpu.stack.mask'])
        else:
            raise AssertionError((hex(ip), ins.hex()))
        delay = 0
        if ins in (b'\xec', b'\xee'):
            divisor = 1024 if ins == b'\xec' else int(1024/.75)
            delay = q['CPU_CycleMax']//divisor
            if q['CPU_Cycles'] < 3*delay:
                delay = 0
            io.append(dict(index=index, ip=ip, operation=gold[index-6]['op'],
                           port=gold[index-6]['port'], value=gold[index-6]['value'], delay=delay))
        post_budget = q['CPU_Cycles']-delay
        post_left = q['CPU_CycleLeft']
        if ins == b'\xee':
            # The observed high->low write queues original DSP_FinishReset.
            value = q['registers'][0] & 255
            if value & 1:
                reset_high = True
            elif reset_high:
                assert post_budget > q['CPU_CycleMax']*20/1000
                post_left += post_budget
                post_budget = 0
                reset_high = False
        if index >= 6:
            fetched, after_io, budget = clock_rows[index-6]
            assert fetched == clock(q) and after_io == clock(q)+delay
            assert post_budget == budget, (index, 'original PIC post-instruction budget')
        if post_budget <= 0:
            # Normal-core while(CPU_Cycles-->0) exits and FillFlags runs before
            # PIC_RunQueue. The original clock owner supplies the next slice.
            lazy_before_fill = flags['lflags.type']
            fill_flags()
            next_gold = clock_rows[index+1-6]
            next_op = gold[index+1-6]['op']
            next_delay = q['CPU_CycleMax']//(1024 if next_op == 'r' else int(1024/.75)) if next_op != 'n' else 0
            next_budget = next_gold[2]+next_delay
            next_left = post_left + post_budget-1-(next_budget+1)
            assert clock(after) == next_gold[0]
            exits.append(dict(index=index, ip=ip, lazy_type_before_fill=lazy_before_fill,
                              post_instruction_budget=post_budget, queued_cycles=post_left,
                              next_budget=next_budget, next_left=next_left))
        else:
            next_budget, next_left = post_budget-1, post_left
            assert clock(after)-clock(q) == delay+1, (index, 'instruction and I/O retirement')
        assert after['CPU_Cycles'] == next_budget and after['CPU_CycleLeft'] == next_left, (index, 'complete CPU budget')
        assert after['cpu_regs.ip.dword[0]'] == next_ip, (index, 'IP')
        assert after['registers'] == regs, (index, hex(ip), 'all GP', after['registers'], regs)
        assert after['segments'] == q['segments'], (index, 'segments')
        for key, value in flags.items():
            assert after[key] == value, (index, hex(ip), key, hex(after[key]), hex(value))
        transitions.append([ip, next_ip])
    assert len(transitions) == 224 and len(exits) == 2
    assert len(io) == 57
    assert rows[-1]['registers'][4] == first['registers'][4]+4
    proof = dict(scope='Original default device mode1: actual23c4 tail JMP through dispatch table, '
                 'all224 complete GP/segment/raw-lazyflag/control/CPU-budget instruction transitions, '
                 'eight unchanged whole16MiB boundaries, CLI/STI, original PIC/SB/I/O-clock reuse '
                 'and both normal-core FillFlags exits. The actual near RET consumes the existing '
                 'caller frame. Other modes, failed-AA branch, port implementation, production '
                 'bank/device/IRQ/CPU/time, first817 and final mixed PCM remain open.',
                 fetches=rows, states=states, transitions=transitions, core_exits=exits, io=io,
                 memory_sha256={n: digest(folder/(n+'.memory')) for n in states},
                 caller_ip=caller_ip, caller_bytes=image[caller_ip:return_ip].hex(),
                 entry_return_ip=return_ip, dispatch_table_offset=0x159f,
                 reset_code_offset=case['code_offset'], reset_code_bytes=case['code_bytes'],
                 clock_case_sha256=digest(repo/'tools/oracle/sb_reset_clock_case.json'),
                 cycle_interval=clock(rows[-1])-clock(first),
                 capture_sha256=output_hashes, frames=39, mixed_samples=27518, endpoint_ms=600,
                 originals_unchanged=len(originals), producers=producers,
                 verifier_sha256=digest(__file__), complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: original225 fetches/224 full instruction transitions/eight whole-RAM boundaries; '
          'tail JMP, DSP reset, IF, CPU budgets and two flag-materializing exits; '
          'all39frames/27518PCM/end600 preserved')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    verify(args.output.resolve(strict=True), args.repo.resolve(strict=True))
