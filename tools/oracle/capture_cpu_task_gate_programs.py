#!/usr/bin/env python3
"""Compare complete original and portable startup-bridge instruction programs."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import resource
from capture_cpu_byte_instructions import target_build
from capture_cpu_segment_push import run
from capture_cpu_shr_instructions import RECORD_SIZE, parse
from cpu_execute_probe import build as original_build
from cpu_task_gate_programs import MODES, programs, mutations


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def observed_sources(repo, root, mode):
    """Observe GP registers at actual RAM accesses; do not add accesses or charges."""
    support = (repo/'tools/oracle/rep_probe.cpp').read_text()
    marker = 'static unsigned ram_count,ram_accesses[65];'
    assert support.count(marker) == 1
    support = support.replace(marker, marker + '\nstatic unsigned ram_cpu[16*8];')
    marker = 'assert(ram_count<16);unsigned *r=ram_accesses+1+4*ram_count++;'
    assert support.count(marker) == 1
    support = support.replace(marker, 'assert(ram_count<16);for(unsigned i=0;i<8;i++)ram_cpu[ram_count*8+i]=reg_32(i);unsigned *r=ram_accesses+1+4*ram_count++;')
    (root/'rep_probe.cpp').write_text(support)
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    original = (repo/'tools/oracle/cpu_execute_probe.cpp').read_text()
    original = original.replace('#include "../../third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp"', '#include "' + str(tree/'src/cpu/flags.cpp') + '"')
    original = original.replace('ram_count=0;memset(ram_accesses,0,sizeof ram_accesses);', 'ram_count=0;memset(ram_accesses,0,sizeof ram_accesses);memset(ram_cpu,0,sizeof ram_cpu);')
    marker = 'assert(fwrite(ram_accesses,sizeof ram_accesses,1,stdout)==1);'
    assert original.count(marker) == 1
    original = original.replace(marker, marker + '\n  assert(fwrite(ram_cpu,sizeof ram_cpu,1,stdout)==1);')
    if mode == 'pop-ss':
        text = (tree/'src/cpu/cpu.cpp').read_text()
        start = text.index('bool CPU_SetSegGeneral(')
        # Only the complete original real-mode branch is reached by these inputs.
        # Protected and exception cases abort and are excluded from this scope.
        select = text[start:text.index('\n\t} else {', start)] + '\n\t}\n abort();\n}\n'
        pop = re.search(r'bool CPU_PopSeg\(SegNames seg,bool use32\) \{.*?\n\}', text, re.S)[0]
        marker = '#include "original_branch_support.h"'
        assert original.count(marker) == 1
        original = original.replace(marker, '#define mem_readw(a) load(a,2)\n' + select + '\n' + pop + '\n' + marker + '\n#undef RUNEXCEPTION\n#define RUNEXCEPTION() abort()')
    probe = root/'original-probe.cpp'
    probe.write_text(original)
    portable = '#include "fist_cpu.h"\n' + (repo/'tests/cpu_execute_controlled.c').read_text()
    # Install the observer before interrupt/stack helpers are defined, so PUSH/POP
    # accesses use the same observer as decoded operands.
    portable = portable.replace('#include "fist_interrupt.h"', '#include "fist_ram.h"')
    marker = 'static unsigned ram_count,ram_accesses[65];'
    assert portable.count(marker) == 1
    portable = portable.replace(marker, marker + '\nstatic unsigned ram_cpu[16*8];static const FistCpuState *cpu_observed;')
    marker = 'fist_cpu_require(ram_count<16);unsigned *r=ram_accesses+1+4*ram_count++;'
    assert portable.count(marker) == 1
    portable = portable.replace(marker, 'fist_cpu_require(ram_count<16 && cpu_observed!=NULL);memcpy(ram_cpu+ram_count*8,cpu_observed,8*sizeof(unsigned));unsigned *r=ram_accesses+1+4*ram_count++;')
    portable = portable.replace(' uint32_t value=fist_ram_resident_read(bus,segment,offset,width);', ' cpu_observed=bus->cpu;\n uint32_t value=fist_ram_resident_read(bus,segment,offset,width);')
    portable = portable.replace(' fist_ram_resident_write(bus,segment,offset,width,value);', ' cpu_observed=bus->cpu;\n fist_ram_resident_write(bus,segment,offset,width,value);')
    portable = portable.replace('ram_count=0;memset(ram_accesses,0,sizeof ram_accesses);', 'ram_count=0;memset(ram_accesses,0,sizeof ram_accesses);memset(ram_cpu,0,sizeof ram_cpu);')
    marker = 'fist_cpu_require(fwrite(ram_accesses,sizeof ram_accesses,1,stdout)==1);'
    assert portable.count(marker) == 1
    portable = portable.replace(marker, marker + '\n  fist_cpu_require(fwrite(ram_cpu,sizeof ram_cpu,1,stdout)==1);')
    (root/'portable-probe.c').write_text(portable)
    return probe, portable


def capture(repo, root, mode, *, headers=None):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True, exist_ok=False)
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    owner = headers or repo/'re_out'
    cpu = (owner/'fist_cpu.h').read_text()
    header = (owner/'fist_exec.h').read_text()
    stack = (repo/'re_out/fist_interrupt.h').read_text()
    values = programs(mode, tree)
    probe, portable = observed_sources(repo, root, mode)
    opcodes = {'push-byte': (0x6a,), 'push-ss': (0x16,), 'sub-word': (0x81, 0x83),
               'push-full': (0x68,), 'pop-ss': (0x17,), 'xchg': (0x87,), 'shl': (0xc1,)}[mode]
    options = dict(extra_opcodes=opcodes,
        byte_opcodes=(0x86,) if mode == 'xchg' else (0xc0,) if mode == 'shl' else (),
        trace=True, lazy_input=True, segment_input=True, group_shift=True, ram_trace=True)
    original = original_build(repo, root, source_file=probe, **options)
    silent = root/'without-GP-observer'
    silent.mkdir()
    # Keep the real-mode POP SS helper but remove only the GP observer.
    silent_source = probe.read_text().replace('memset(ram_cpu,0,sizeof ram_cpu);', '').replace('  assert(fwrite(ram_cpu,sizeof ram_cpu,1,stdout)==1);', '')
    (silent/'original-probe.cpp').write_text(silent_source)
    uninstrumented = original_build(repo, silent, source_file=silent/'original-probe.cpp', **options)
    def builds(folder, h=header, c=cpu, s=stack):
        folder.mkdir(parents=True, exist_ok=True)
        (folder/'fist_cpu.h').write_text(c)
        return target_build(repo, folder, h, segment_input=True, group_shift=True,
            ram_trace=True, controlled_source=portable, stack_header=s)
    commands = [('original', [original]), *builds(root/'targets')]
    size = RECORD_SIZE + 16*8*4
    hashes = {name: hashlib.sha256() for name, _ in commands}
    inputs = hashlib.sha256()
    silent_hash = hashlib.sha256()
    for start in range(0, len(values), 16):
        batch = values[start:start + 16]
        data = b''.join(batch)
        inputs.update(data)
        expected = None
        for target, command in commands:
            output = run(command, data)
            assert len(output) == len(batch)*size, (mode, target, start)
            if expected is None:
                expected = output
            else:
                assert output == expected, (mode, target, start, 'Complete instruction records differ')
            hashes[target].update(output)
        output = run([uninstrumented], data)
        assert output == b''.join(expected[i*size:i*size + RECORD_SIZE] for i in range(len(batch)))
        silent_hash.update(output)
        if start % 1024 == 0:
            print('Matched complete original', mode, start + len(batch), '/', len(values), flush=True)
    negatives = []
    for name, h, c, s, data in mutations(mode, header, cpu, stack, values):
        expected = run([original], data)
        a, sa, ma, fa, ra = parse(expected[:RECORD_SIZE])
        for target, command in builds(root/'negative'/name, h, c, s):
            actual = run(command, data)
            assert len(actual) == size and actual != expected, (mode, name, target)
            b, sb, mb, fb, rb = parse(actual[:RECORD_SIZE])
            if name == 'memory-before-register':
                assert actual[:RECORD_SIZE] == expected[:RECORD_SIZE]
            if name == 'operand-read-for-zero-count':
                assert (a, sa, ma, fa) == (b, sb, mb, fb) and ra != rb
            negatives.append(dict(fault=name, target=target,
                differing_CPU_words=[i for i, (x, y) in enumerate(zip(a, b)) if x != y],
                stack_metadata_equal=sa == sb, full_RAM_equal=ma == mb,
                code_fetch_equal=fa == fb, RAM_accesses_equal=ra == rb,
                intermediate_GP_equal=actual[RECORD_SIZE:] == expected[RECORD_SIZE:],
                input_sha256=hashlib.sha256(data).hexdigest(),
                original_sha256=hashlib.sha256(expected).hexdigest(),
                mutant_sha256=hashlib.sha256(actual).hexdigest()))
        print('PASS distinguish', mode, name, 'both targets', flush=True)
    paths = [Path(__file__), Path(__file__).with_name('cpu_task_gate_programs.py'), probe,
             root/'rep_probe.cpp', root/'portable-probe.c', owner/'fist_cpu.h', owner/'fist_exec.h',
             *[repo/'tools/oracle'/name for name in ('cpu_execute_probe.py', 'cpu_execute_probe.cpp',
                'rep_probe.cpp', 'capture_cpu_cmp.py', 'capture_cpu_segment_push.py',
                'capture_cpu_shr_instructions.py', 'capture_cpu_byte_instructions.py')],
             repo/'tests/cpu_execute_controlled.c', *sorted((repo/'re_out').glob('*.h')),
             *root.glob('*.h'), *silent.glob('*.h'), *tree.rglob('*.h'),
             *[tree/'src/cpu'/name for name in ('cpu.cpp', 'flags.cpp', 'core_normal.cpp')]]
    proof = dict(scope='Complete original-backed reached startup-bridge instruction programs. '
        'All31 CPU/cache/budget/lazy words,full-width stack metadata,whole2MiB RAM,ordered code/'
        'RAM accesses and all8 GP registers at every RAM access agree. An original without GP '
        'observation retains every pre-existing byte. POP SS controls cover real mode only; '
        'protected faults,actual runtime and complete original frame/audio acceptance remain open.',
        mode=mode, cases=len(values), record_bytes=size, input_sha256=inputs.hexdigest(),
        outputs_sha256={name: h.hexdigest() for name, h in hashes.items()},
        original_without_GP_sha256=silent_hash.hexdigest(), causal_results=negatives,
        inputs_sha256={str(p): digest(p) for p in paths}, complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mode', choices=MODES, required=True)
    args = parser.parse_args()
    resource.setrlimit(resource.RLIMIT_CORE, (0, resource.getrlimit(resource.RLIMIT_CORE)[1]))
    capture(args.repo.resolve(strict=True), args.output.resolve(), args.mode)
