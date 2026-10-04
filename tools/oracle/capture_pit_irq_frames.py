#!/usr/bin/env python3
"""Recover original system loads, IRQ0 frames and IRET from matched captures."""
import argparse
import ast
import copy
import json
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
from capture_pit_events import digest, format_case, lines
from sequence_format import validate, validate_endpoint

ROOT = Path(__file__).resolve().parents[2]
END_MS = 2000
META = {'kind', 'index', 'serial', 'memory_file', 'num', 'type', 'oldeip',
        'vector_hex', 'vector_physical', 'use32'}
SYSTEM_FIELDS = ('cpu.idt.table_base', 'cpu.idt.table_limit', 'cpu.gdt.table_base',
                 'cpu.gdt.table_limit', 'cpu.gdt.ldt_base', 'cpu.gdt.ldt_limit',
                 'cpu.gdt.ldt_value', 'cpu_tss.base', 'cpu_tss.limit', 'cpu_tss.selector',
                 'cpu_tss.is386', 'cpu_tss.valid', 'cpu.mpl', 'cpu.trap_skip',
                 'CPU_flag_id_toggle', 'cpu.direction', 'lastint',
                 'cpu_tss.desc.saved.fill[0]', 'cpu_tss.desc.saved.fill[1]',
                 'cpu.exception.which', 'cpu.exception.error')


def system_transition(repo, folder, before, after, physical, constants):
    """Verify reached original LGDT/LIDT/LTR and every CPU/RAM side effect."""
    meta = {'kind', 'label', 'operation', 'arguments', 'memory_file', 'return_value', 'context'}
    q = {k: copy.deepcopy(v) for k, v in before.items() if k not in meta}
    memory = bytearray((folder/before['memory_file']).read_bytes())
    assert len(memory) == 16777216
    reads, writes = [], []

    def read(address, width):
        addresses = [physical((address+i)&0xffffffff, memory, q) for i in range(width)]
        value = int.from_bytes(bytes(memory[p] for p in addresses), 'little')
        reads.append(dict(linear=address, physical=addresses, width=width, value=value))
        return value

    operation = before['operation']; args = before['arguments']
    assert before['kind'] == 'before-system' and after['kind'] == 'after-system'
    assert before['label'] == after['label']
    if operation in ('CPU_LGDT', 'CPU_LIDT'):
        table = 'gdt' if operation == 'CPU_LGDT' else 'idt'
        q['cpu.'+table+'.table_base'] = args['base']
        q['cpu.'+table+'.table_limit'] = args['limit']
        assert 'return_value' not in after
    else:
        assert operation == 'CPU_LTR' and after['return_value'] == 0
        selector = args['selector']; address = q['cpu.gdt.table_base']+(selector & ~7)
        assert selector & 0xfffc and not selector & 4
        assert selector & ~7 < q['cpu.gdt.table_limit']
        q['cpu.mpl'] = 0; raw = read(address, 8); q['cpu.mpl'] = 3
        assert (raw >> 40) & 31 == constants['DESC_386_TSS_A'] and raw & (1 << 47)
        # TaskStateSegment::SetSelector reloads the cache before making it busy.
        q['cpu_tss.valid'] = 0
        q['cpu.mpl'] = 0; assert read(address, 8) == raw; q['cpu.mpl'] = 3
        q['cpu_tss.selector'] = selector; q['cpu_tss.valid'] = 1
        q['cpu_tss.base'] = ((raw >> 16) & 0xffffff) | ((raw >> 32) & 0xff000000)
        limit = (raw & 0xffff) | ((raw >> 32) & 0xf0000)
        q['cpu_tss.limit'] = (limit << 12) | 0xfff if raw & (1 << 55) else limit
        q['cpu_tss.is386'] = ((raw >> 40) & 31) & 8
        raw |= 2 << 40
        q['cpu_tss.desc.saved.fill[0]'] = raw & 0xffffffff
        q['cpu_tss.desc.saved.fill[1]'] = raw >> 32
        q['cpu.mpl'] = 0
        for offset in (0, 4):
            addresses = [physical(address+offset+i, memory, q) for i in range(4)]
            value = (raw >> (offset*8)) & 0xffffffff
            for i, p in enumerate(addresses): memory[p] = (value >> (i*8)) & 255
            writes.append(dict(linear=address+offset, physical=addresses, width=4, value=value))
        q['cpu.mpl'] = 3
    expected = {k: v for k, v in after.items() if k not in meta}
    assert q == expected, ('complete system CPU transition', before['label'],
                           {k:(v, expected.get(k)) for k,v in q.items() if v != expected.get(k)})
    assert memory == (folder/after['memory_file']).read_bytes(), ('complete system RAM transition', before['label'])
    return dict(label=before['label'], operation=operation, reads=reads, writes=writes)


def shared_physical(repo):
    path = repo/'tools/oracle/file_error.gdb'
    program = path.read_text().split('\npython\n', 1)[1].rsplit('\nend\nrun', 1)[0]
    nodes = [n for n in ast.parse(program).body if isinstance(n, ast.FunctionDef)
             and n.name == 'physical']
    assert len(nodes) == 1
    ns = {'struct': struct}
    exec(compile(ast.fix_missing_locations(ast.Module(body=nodes, type_ignores=[])),
                 str(path), 'exec'), ns)
    return ns['physical']


def source_constants(repo):
    """Read original flag/type values and TSS field offsets, rather than fitted data."""
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    constants = {}
    for name in ('regs.h', 'cpu.h'):
        for key, value in re.findall(r'^#define\s+(FLAG_\w+|FMASK_\w+|DESC_\w+)\s+([^\n]+)',
                                     (tree/'include'/name).read_text(), re.M):
            value = value.split('//', 1)[0].strip()
            if re.fullmatch(r'0x[0-9a-fA-F]+', value):
                constants[key] = int(value, 16)
            else:
                parts = value.strip('() ').split('|')
                assert all(p.strip() in constants for p in parts), (key, value)
                constants[key] = 0
                for p in parts: constants[key] |= constants[p.strip()]
    offsets = {}
    header = (tree/'include/cpu.h').read_text()
    for bits in (16, 32):
        body = header.split('struct TSS_%d {' % bits, 1)[1].split('}', 1)[0]
        fields = re.findall(r'Bit%d[u]?\s+([^;]+);' % bits, body)
        names = [n.strip() for f in fields for n in f.split(',')]
        offsets[bits] = {n: i*(bits//8) for i, n in enumerate(names)}
    return constants, offsets


def transition(repo, folder, before, after, physical, constants, offsets):
    """Check complete reached CPU_Interrupt/CPU_IRET effects against original RAM."""
    q = {k: copy.deepcopy(v) for k, v in before.items() if k not in META}
    memory = bytearray((folder/before['memory_file']).read_bytes())
    assert len(memory) == 16777216
    reads, writes = [], []

    def read(linear, width):
        addresses = [physical((linear+i)&0xffffffff, memory, q) for i in range(width)]
        value = int.from_bytes(bytes(memory[p] for p in addresses), 'little')
        reads.append(dict(linear=linear, physical=addresses, width=width, value=value))
        return value

    def write(linear, width, value):
        addresses = [physical((linear+i)&0xffffffff, memory, q) for i in range(width)]
        for i, p in enumerate(addresses): memory[p] = (value >> (i*8)) & 255
        writes.append(dict(linear=linear, physical=addresses, width=width, value=value))

    def advance(delta):
        q['registers'][4] = ((q['registers'][4] & q['cpu.stack.notmask']) |
                            ((q['registers'][4]+delta) & q['cpu.stack.mask']))

    def push(value, width):
        advance(-width)
        write(q['segments'][2]['base']+(q['registers'][4] & q['cpu.stack.mask']), width, value)

    def pop(width):
        value = read(q['segments'][2]['base']+(q['registers'][4] & q['cpu.stack.mask']), width)
        advance(width)
        return value

    def descriptor(selector):
        table = 'ldt' if selector & 4 else 'table'
        assert (selector & ~7) < q['cpu.gdt.'+table+'_limit']
        raw = read(q['cpu.gdt.'+table+'_base']+(selector & ~7), 8)
        return dict(base=((raw >> 16) & 0xffffff) | ((raw >> 32) & 0xff000000),
                    type=(raw >> 40) & 31, dpl=(raw >> 45) & 3,
                    present=(raw >> 47) & 1, big=(raw >> 54) & 1)

    def set_stack(selector, esp, desc):
        assert desc['present'] and desc['type'] in [constants[n] for n in
               ('DESC_DATA_EU_RW_NA', 'DESC_DATA_EU_RW_A', 'DESC_DATA_ED_RW_NA', 'DESC_DATA_ED_RW_A')]
        q['segments'][2] = dict(value=selector, base=desc['base'])
        q['cpu.stack.big'] = desc['big']
        q['cpu.stack.mask'] = 0xffffffff if desc['big'] else 0xffff
        q['cpu.stack.notmask'] = 0 if desc['big'] else 0xffff0000
        q['registers'][4] = esp if desc['big'] else (q['registers'][4] & 0xffff0000) | (esp & 0xffff)

    def set_flags(value, mask):
        mask |= q['CPU_flag_id_toggle']
        q['cpu_regs.flags'] = (q['cpu_regs.flags'] & ~mask) | (value & mask) | 2
        q['cpu.direction'] = 1-((q['cpu_regs.flags'] & constants['FLAG_DF']) >> 9)
        q['lflags.type'] = 0  # DestroyConditionFlags, retaining operands/oldcf/prev_type.

    if before['kind'] == 'before-hardware':
        # All three actually reached hardware entries exit the normal core with materialized flags.
        assert q['lflags.type'] == 0 and before['type'] == 0
        num = before['num']; q['lastint'] = num
        assert before['oldeip'] == q['cpu_regs.ip.dword[0]']
        width = 2
        if q['cpu.pmode']:
            assert not q['cpu_regs.flags'] & constants['FLAG_VM']
            assert num*8 < q['cpu.idt.table_limit']
            gate = read(q['cpu.idt.table_base']+num*8, 8)
            assert gate.to_bytes(8, 'little').hex() == before['vector_hex']
            gate_type = (gate >> 40) & 31
            assert gate_type == constants['DESC_386_INT_GATE'] and gate & (1 << 47)
            selector = (gate >> 16) & 0xffff
            desc = descriptor(selector)
            assert desc['present'] and desc['type'] in [constants[n] for n in
                   ('DESC_CODE_N_NC_A', 'DESC_CODE_N_NC_NA', 'DESC_CODE_R_NC_A', 'DESC_CODE_R_NC_NA')]
            assert desc['dpl'] < q['cpu.cpl']  # The original route is an inward privilege switch.
            old_ss, old_esp = q['segments'][2]['value'], q['registers'][4]
            assert q['cpu_tss.valid']
            bits = 32 if q['cpu_tss.is386'] else 16
            name = 'esp0' if bits == 32 else 'sp0'
            tss = q['cpu_tss.base']+offsets[bits][name]+desc['dpl']*(bits//4)
            esp = read(tss, bits//8); ss = read(tss+bits//8, 2)
            sd = descriptor(ss)
            assert (ss & 3) == sd['dpl'] == desc['dpl']
            set_stack(ss, esp, sd); q['cpu.cpl'] = desc['dpl']; q['cpu.mpl'] = 3
            width = 4; push(old_ss, width); push(old_esp, width)
            destination = ((gate >> 48) << 16) | (gate & 0xffff)
            new_cs = dict(value=(selector & 0xfffc) | q['cpu.cpl'], base=desc['base'])
            q['cpu.code.big'] = desc['big']
        else:
            destination = read(q['cpu.idt.table_base']+num*4, 2)
            cs = read(q['cpu.idt.table_base']+num*4+2, 2)
            assert (destination.to_bytes(2, 'little')+cs.to_bytes(2, 'little')).hex() == before['vector_hex']
            new_cs = dict(value=cs, base=cs << 4); q['cpu.code.big'] = 0
        push(q['cpu_regs.flags'] & ((1 << (width*8))-1), width)
        push(q['segments'][1]['value'], width); push(before['oldeip'] & ((1 << (width*8))-1), width)
        q['cpu_regs.flags'] &= ~(constants['FLAG_IF'] | constants['FLAG_TF'])
        if q['cpu.pmode']: q['cpu_regs.flags'] &= ~(constants['FLAG_NT'] | constants['FLAG_VM'])
        q['segments'][1] = new_cs; q['cpu_regs.ip.dword[0]'] = destination
    else:
        assert before['kind'] == 'before-iret' and after['kind'] == 'after-iret'
        width = 4 if before['use32'] else 2
        ip, cs, flags = pop(width), pop(width) & 0xffff, pop(width)
        if not q['cpu.pmode']:
            q['segments'][1] = dict(value=cs, base=cs << 4)
            q['cpu.code.big'] = 0
            set_flags(flags, constants['FMASK_ALL'] & ((1 << (width*8))-1))
        else:
            assert before['use32'] and not q['cpu_regs.flags'] & (constants['FLAG_VM'] | constants['FLAG_NT'])
            assert not flags & constants['FLAG_VM']
            desc = descriptor(cs)
            assert desc['present'] and (cs & 3) == desc['dpl'] > q['cpu.cpl']
            assert desc['type'] in [constants[n] for n in ('DESC_CODE_N_NC_A', 'DESC_CODE_N_NC_NA', 'DESC_CODE_R_NC_A', 'DESC_CODE_R_NC_NA')]
            esp, ss = pop(width), pop(width) & 0xffff
            sd = descriptor(ss); assert (ss & 3) == sd['dpl'] == (cs & 3)
            mask = constants['FMASK_ALL'] if q['cpu.cpl'] == 0 else constants['FMASK_NORMAL'] | constants['FLAG_NT']
            if (q['cpu_regs.flags'] >> 12) & 3 < q['cpu.cpl']: mask &= ~constants['FLAG_IF']
            set_flags(flags, mask); q['cpu.cpl'] = cs & 3
            q['segments'][1] = dict(value=cs, base=desc['base']); q['cpu.code.big'] = desc['big']
            set_stack(ss, esp, sd)
            # Actual outer return checks all four data segments, including null selectors.
            for segment in (0, 3, 4, 5):
                d = descriptor(q['segments'][segment]['value'])
                types = [constants[n] for n in ('DESC_DATA_EU_RO_NA', 'DESC_DATA_EU_RO_A',
                         'DESC_DATA_EU_RW_NA', 'DESC_DATA_EU_RW_A', 'DESC_DATA_ED_RO_NA',
                         'DESC_DATA_ED_RO_A', 'DESC_DATA_ED_RW_NA', 'DESC_DATA_ED_RW_A',
                         'DESC_CODE_N_NC_A', 'DESC_CODE_N_NC_NA', 'DESC_CODE_R_NC_A', 'DESC_CODE_R_NC_NA')]
                assert d['type'] not in types or q['cpu.cpl'] <= d['dpl']
        q['cpu_regs.ip.dword[0]'] = ip
    expected = {k: v for k, v in after.items() if k not in META}
    assert q == expected, ('complete CPU transition', before['serial'], {k:(v, expected.get(k)) for k,v in q.items() if v != expected.get(k)})
    assert memory == (folder/after['memory_file']).read_bytes(), ('complete IRQ RAM transition', before['serial'])
    return dict(before=before['serial'], after=after['serial'], reads=reads, writes=writes)


def source_paths(repo):
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    names = ('capture_pit_irq_frames.py', 'pit_irq_frame.gdb', 'file_error.gdb', 'capture_pit_events.py',
             'capture_sequence.sh', 'sequence_format.py', 'start_state.text.gz.b64', 'start_state.bda.gz.b64', 'start_state.vga')
    return sorted({*(repo/'tools/oracle'/n for n in names), repo/'tools/work_dir.sh', repo/'third_party/dosbox-fist',
                   *tree.rglob('*.h'), *(tree/'src'/n for n in ('cpu/cpu.cpp', 'cpu/paging.cpp', 'cpu/flags.cpp',
                   'cpu/core_normal.cpp', 'hardware/pic.cpp', 'hardware/timer.cpp', 'hardware/iohandler.cpp',
                   'hardware/memory.cpp', 'shell/shell.cpp'))})


def verify(repo, root, reference=True, bootstrap=False):
    producers = json.loads((root/'producers.json').read_text())
    for p, h in producers.items(): assert digest(p) == h, p
    originals = json.loads((root/'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()} == set(originals)
    for p, h in originals.items(): assert digest(repo/p) == h, p
    captures = {}
    for name in ('baseline', 'source'):
        assert (root/(name+'.exit')).read_text() == '0\n'
        folder = root/name
        assert 'Python Exception' not in (folder/'dosbox.log').read_text()
        assert validate_endpoint(folder/'sequence', END_MS) == END_MS
        captures[name] = {suffix: digest(folder/('sequence.'+suffix)) for suffix in ('frames', 'pcm', 'end')}
        assert validate(folder/'sequence.frames', 'F')['records'] == 137
        assert validate(folder/'sequence.pcm', 'A')['samples'] == 89258
    assert captures['baseline'] == captures['source'], 'original observer changed complete output'
    for suffix in ('text', 'bda', 'vga'):
        assert (root/'baseline'/('start-state.'+suffix)).read_bytes() == (root/'source'/('start-state.'+suffix)).read_bytes()
    folder = root/'source'; events = lines(folder/'irq-events.jsonl'); fetches = lines(folder/'irq-fetches.jsonl')
    completion = json.loads((folder/'irq-completion.json').read_text())
    expected_completion = dict(exit_code=0, hardware_events=128, selected=[1, 7, 56], active=[], serial=146, system_events=4, fetches=2196)
    if bootstrap: expected_completion.update(system_events=8, boot_fetches=9, boot_finished=[0,56], boot_groups=2)
    assert completion == expected_completion, 'incomplete IRQ observer'
    assert len(events) == completion['serial'] and [q['serial'] for q in events] == list(range(1, len(events)+1)), 'incomplete IRQ events'
    hardware = [q for q in events if q['kind'] == 'hardware']
    boundaries = [q for q in events if 'memory_file' in q]
    system = lines(folder/'system-events.jsonl')
    count=completion['system_events']
    assert len(system) == count*2 and [q['label'] for q in system] == [i for i in range(1,count+1) for _ in range(2)], 'incomplete system load output'
    boot = lines(folder/'boot-fetches.jsonl') if bootstrap else []
    assert len(hardware) == 128 and [q['index'] for q in hardware] == list(range(1, 129))
    assert len(boundaries) == 18 and len(fetches) == completion['fetches'], 'incomplete IRQ frame/fetch output'
    assert {p.name for p in folder.glob('*.memory')} == {q['memory_file'] for q in boundaries+system+boot}
    assert all(q['kind'] == 'fetch' and q['index'] in completion['selected'] and len(bytes.fromhex(q['fetched_code_hex'])) == 32 for q in fetches)
    tail = json.loads((folder/'shell-tail-copy.json').read_text())
    assert tail['caller'] == 'SHELL_Init' and tail['physical'] == tail['psp_segment']*16+128
    assert tail['size'] == 128 and tail['initialized_bytes'] == tail['count']+2
    assert tail['source_hex'] == tail['after_hex'] and len(bytes.fromhex(tail['source_hex'])) == tail['size']
    assert bytes.fromhex(tail['source_hex'])[:tail['initialized_bytes']].hex() == tail['defined_prefix_hex']
    for q in boundaries+system+boot:
        memory = (folder/q['memory_file']).read_bytes()
        assert memory[tail['physical']:tail['physical']+tail['size']].hex() == tail['source_hex'], 'captured boot tail input changed'
    physical = shared_physical(repo); constants, offsets = source_constants(repo)
    transitions = [transition(repo, folder, a, b, physical, constants, offsets)
                   for a, b in zip(boundaries[::2], boundaries[1::2])]
    system_transitions = [system_transition(repo, folder, a, b, physical, constants)
                          for a,b in zip(system[::2], system[1::2])]
    for index in completion['selected']:
        selected = [q for q in boundaries if q['index'] == index]
        observed = [q for q in fetches if q['index'] == index]
        assert observed and selected[0]['kind'] == 'before-hardware' and selected[-1]['kind'] == 'after-iret'
        assert all(selected[0][k] == selected[-1][k] for k in ('registers', 'segments', 'cpu_regs.ip.dword[0]', 'cpu_regs.flags', 'cpu.cpl', 'cpu.pmode'))
        for q in selected:
            memory = (folder/q['memory_file']).read_bytes()
            if q['kind'] == 'before-hardware':
                assert physical(q['cpu.idt.table_base']+q['num']*(8 if q['cpu.pmode'] else 4), memory, q) == q['vector_physical']
            if q['kind'] in ('before-iret', 'after-hardware'):
                match = [r for r in observed if r['segments'][1] == q['segments'][1] and r['cpu_regs.ip.dword[0]'] == q['cpu_regs.ip.dword[0]']]
                assert match, ('missing reached fetch', q['serial'])
                address = physical(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'], memory, q)
                assert any(r['fetched_code_physical'] == address and bytes.fromhex(r['fetched_code_hex']) == memory[address:address+32] for r in match)
    original = dict(events=events, fetches=fetches, completion=completion, transitions=transitions,
                    system_events=system, system_transitions=system_transitions,
                    boot_tail_contract={k:v for k,v in tail.items() if k not in ('source_hex', 'after_hex')},
                    capture_sha256=captures['source'], frames=137, mixed_samples=89258, endpoint_ms=END_MS)
    if bootstrap:
        from capture_resident_bootstrap import bootstrap_contract
        original['bootstrap'] = bootstrap_contract(repo, folder, boot, system, events, fetches, physical)
    if bootstrap:
        shared=json.loads((repo/'tools/oracle/pit_irq_frame_case.json').read_text())['original']
        for key in ('events','fetches','transitions','boot_tail_contract','capture_sha256','frames','mixed_samples','endpoint_ms'):
            assert original[key]==shared[key], ('shared original IRQ reference changed',key)
        assert [{k:v for k,v in q.items() if k!='context'} for q in system if q['context']==0]==shared['system_events']
        bootstrap_original={k:original[k] for k in ('completion','system_events','system_transitions','bootstrap')}
        bootstrap_original['shared_irq_case_sha256']=digest(repo/'tools/oracle/pit_irq_frame_case.json')
    if reference:
        filename = 'resident_bootstrap_case.json' if bootstrap else 'pit_irq_frame_case.json'
        case = json.loads((repo/'tools/oracle'/filename).read_text())
        assert case['original'] == (bootstrap_original if bootstrap else original), 'complete original IRQ reference changed'
    proof = dict(scope='Read-only original IRQ0 at first distinct BIOS, loader and protected IDT destinations. '
                 'Complete matched 2000ms output, 128 hardware entries, 2196 handler fetches, 18 full16MiB boundaries '
                 'and nine CPU_Interrupt/IRET transitions. Four reached GDT/IDT/TSS loads preserve another eight '
                 'full16MiB boundaries, cached descriptors and exception fields. No LLDT call is reached. '
                 'Protected entry switches CPL3 to0 through the actual TSS; '
                 'return restores CPL3 with a32-bit frame and16-bit stack. Handler instructions are traced, not fully '
                 'interpreted. SHELL_Init copies uninitialized CommandTail suffix bytes from the host stack: '
                 'the raw per-run boot input and every full-RAM hash are retained separately from the stable '
                 'CPU/event/output property reference. Every API transition still compares all16MiB without '
                 'normalization. No identical whole boot RAM across runs, port IRQ/IF/IRET/device-time or '
                 'complete original sequence acceptance.',
                 commit_parent=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip(),
                 original=original, original_source_inputs={str(p.relative_to(repo)): digest(p) for p in source_paths(repo)},
                 producer_manifest_sha256=digest(root/'producers.json'), original_files=len(originals), originals_unchanged=True,
                 memory_sha256={q['memory_file']:digest(folder/q['memory_file']) for q in boundaries+system+boot},
                 boot_command_tail_copy=tail,
                 complete_original_acceptance=False,
                 reproduction=['python3 -B tools/oracle/capture_pit_irq_frames.py --repo . --output /tmp/wasm-fist-pit-irq-replay',
                               'python3 -B tools/oracle/capture_pit_irq_frames.py --repo . --output /tmp/wasm-fist-pit-irq-replay --verify-only'])
    if bootstrap:proof['bootstrap_original']=bootstrap_original
    (root/'proof.json').write_text(format_case(proof)+'\n')
    print('PASS: original128 hardware entries/2196 fetches/%d full-RAM boundaries/%d IRQ-IRET-system transitions; complete137frame/89258PCM/end2000 retained' % (len(boundaries)+len(system)+len(boot),len(transitions)+len(system_transitions)))
    return proof


def capture(repo, root, bootstrap=False):
    assert root.is_relative_to(Path('/tmp')); root.mkdir(parents=True, exist_ok=False)
    originals = {str(p.relative_to(repo)): digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()}
    (root/'original-hashes.json').write_text(json.dumps(originals, indent=2)+'\n')
    paths = source_paths(repo)
    if bootstrap:
        paths += [repo/'tools/oracle'/name for name in ('capture_resident_bootstrap.py','resident_image.py','sb_irq_frame_case.json','pit_irq_frame_case.json')]
        paths += [repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu'/name for name in ('lazyflags.h','core_normal/prefix_none.h')]
    paths += [Path(subprocess.check_output(['which', n], text=True).strip()).resolve() for n in ('gdb', 'python3')]
    (root/'producers.json').write_text(json.dumps({str(p): digest(p) for p in paths}, indent=2)+'\n')
    for name in ('baseline', 'source'):
        folder = root/name; folder.mkdir(); wrapper = root/('dosbox-'+name)
        command = (['gdb', '-q', '-batch', '-x', str(repo/'tools/oracle/pit_irq_frame.gdb'), '--args'] if name == 'source' else [])+[str(repo/'third_party/dosbox-fist')]
        wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec '+shlex.join(command)+' "$@"\n'); wrapper.chmod(0o755)
        env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
        env.update(FIST_SEQUENCE_END_MS=str(END_MS), FIST_ORACLE_WALL_SECONDS='80', DOSBOX=str(wrapper), FIST_DETAIL_REPO=str(repo), FIST_DETAIL_OPERANDS_DIR=str(folder))
        if bootstrap: env['FIST_ORACLE_BOOTSTRAP']='1'
        with (root/(name+'.log')).open('w') as log:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '2', str(folder)], cwd=repo,
                                    env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n'); assert result.returncode == 0, (name, result.returncode)
    return verify(repo, root, reference=False, bootstrap=bootstrap)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=ROOT); parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true'); parser.add_argument('--record-case', action='store_true')
    args = parser.parse_args(); repo = args.repo.resolve(strict=True); root = args.output.resolve()
    case = repo/'tools/oracle/pit_irq_frame_case.json'
    if args.record_case: assert not case.exists() and not args.verify_only
    proof = verify(repo, root) if args.verify_only else capture(repo, root)
    if args.record_case: case.write_text(format_case(proof)+'\n')
    elif not args.verify_only: verify(repo, root)


if __name__ == '__main__': main()
