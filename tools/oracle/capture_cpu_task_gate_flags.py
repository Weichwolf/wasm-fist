#!/usr/bin/env python3
"""Compare original SUBW/SHL raw/lazy flags, all queries, materialization and INC."""
import argparse
import hashlib
import json
from pathlib import Path
import resource
import struct
import subprocess
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2]/'tests'))
from test_port_io import tool


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def mutants(mode, cpu, header):
    if mode == 'sub-word':
        cf = 'case FIST_LAZY_SUBW: case FIST_LAZY_CMPW: return (uint16_t)f->var1 < (uint16_t)f->var2;'
        begin = cpu.index('static inline int fist_cpu_overflow(')
        end = cpu.index('static inline int fist_cpu_of(', begin)
        return [
            ('wide-carry', cpu.replace(cf, 'case FIST_LAZY_SUBW: return f->var1 < f->var2; case FIST_LAZY_CMPW: return (uint16_t)f->var1 < (uint16_t)f->var2;'), header),
            ('missing-overflow', cpu[:begin] + cpu[begin:end].replace('f->type==FIST_LAZY_SUBW || ', '') + cpu[end:], header),
            ('missing-materialized-AF', cpu.replace('arithmetic && ((a^b^result) & 0x10)', 'arithmetic && f->type!=FIST_LAZY_SUBW && ((a^b^result) & 0x10)'), header),
            ('wrong-lazy-tag', cpu, header.replace('width==2?FIST_LAZY_SUBW:FIST_LAZY_SUBD', 'width==2?FIST_LAZY_CMPW:FIST_LAZY_SUBD')),
            ('eager-flags', cpu, header.replace('case 5:result=a-b;type=width==1?FIST_LAZY_SUBB:width==2?FIST_LAZY_SUBW:FIST_LAZY_SUBD;break;', 'case 5:result=a-b;type=width==1?FIST_LAZY_SUBB:width==2?FIST_LAZY_SUBW:FIST_LAZY_SUBD;fist_cpu_alu(e->bus->cpu,type,width*8,a,b,result);fist_cpu_fill_flags(e->bus->cpu);return fist_cpu_low(0,result,width*8);')),
            ('lost-dirty-upper', cpu.replace('f->var1=fist_cpu_low(f->var1,a,bits);', 'f->var1=fist_cpu_low(0,a,bits);').replace('f->res=fist_cpu_low(f->res,result,bits);', 'f->res=fist_cpu_low(0,result,bits);'), header)]
    begin = cpu.index('static inline uint32_t fist_cpu_shl(')
    end = cpu.index('static inline uint32_t fist_cpu_shr(', begin)
    body = cpu[begin:end]
    return [
        ('next-carry-bit', cpu.replace('>>(bits-count)) & 1', '>>(bits-count-1)) & 1'), header),
        ('DWORD-only-OF-count1', cpu.replace('return !!((result^a) & sign);', 'return !!((result^a) & sign) && (f->type!=FIST_LAZY_SHLD || (f->var2&31)==1);'), header),
        ('lost-upper-count', cpu[:begin] + body.replace('fist_cpu_low(f->var2,count,8)', 'count') + cpu[end:], header),
        ('wrong-DWORD-tag', cpu[:begin] + body.replace('bits==16 ? FIST_LAZY_SHLW : FIST_LAZY_SHLD', 'bits==16 ? FIST_LAZY_SHLW : FIST_LAZY_SHLW') + cpu[end:], header),
        ('zero-count-writes-lazy', cpu[:begin] + body.replace('if (!count) return fist_cpu_low(0,a,bits);', '') + cpu[end:], header)]


def capture(repo, root, mode, *, headers=None, sources=None):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True, exist_ok=False)
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    owner = headers or repo/'re_out'
    fixtures = sources or repo
    cpu = (owner/'fist_cpu.h').read_text()
    header = (owner/'fist_exec.h').read_text()
    original = root/'original'
    p = subprocess.run(['g++', '-O2', '-std=gnu++11', '-ffunction-sections', '-fdata-sections',
        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
        '-I'+str(tree/'include'), '-I'+str(tree), '-I'+str(tree/'src/cpu'),
        '-I'+str(fixtures/'tests'), str(fixtures/'tools/oracle/cpu_task_gate_flags.cpp'),
        '-Wl,--gc-sections', '-o', str(original)], capture_output=True, text=True, timeout=120)
    (root/'original-build.log').write_text(p.stdout+p.stderr)
    assert p.returncode == 0, p.stderr
    def build(folder, c=cpu, h=header):
        folder.mkdir(parents=True, exist_ok=True)
        (folder/'fist_cpu.h').write_text(c)
        (folder/'fist_exec.h').write_text(h)
        result = []
        for target, compiler, options, output, runner in (
            ('native', ['gcc', '-m32'], ['-Wl,--gc-sections'], folder/'native', []),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc')],
             ['-sNODERAWFS=1', '-sEXIT_RUNTIME=1', '-sALLOW_MEMORY_GROWTH=1'],
             folder/'wasm.js', [tool('node', 'Git/emsdk/node/*/bin/node')])):
            p = subprocess.run([*compiler, '-O2', '-DNDEBUG', '-ffunction-sections',
                '-fdata-sections', '-I'+str(folder), '-I'+str(fixtures/'tests'),
                '-I'+str(repo/'tests'), '-I'+str(repo/'re_out'),
                str(fixtures/'tests/cpu_task_gate_flags.c'), *options, '-o', str(output)],
                capture_output=True, text=True, timeout=120)
            (folder/(target+'-build.log')).write_text(p.stdout+p.stderr)
            assert p.returncode == 0, p.stderr
            result.append((target, [*runner, str(output)]))
        return result
    counts = {'sub-word': (131360, 288), 'shl': (410816, 1356)}
    def run(command, edge=False):
        output = root/'output.raw'
        p = subprocess.run([*command, str(output), mode, *(['edge'] if edge else [])],
                           capture_output=True, text=True, timeout=60)
        assert p.returncode == 0, p.stderr
        data = output.read_bytes()
        assert len(data) == counts[mode][edge]*4*72
        output.unlink()
        return data
    expected = None
    results = []
    for target, command in [('original', [str(original)]), *build(root/'positive')]:
        data = run(command)
        if expected is None:
            expected = data
        else:
            assert data == expected, (mode, target, 'Complete original flag records differ')
        results.append(dict(target=target, programs=counts[mode][0], records=counts[mode][0]*4,
                            sha256=hashlib.sha256(data).hexdigest()))
    print('PASS complete original', mode, counts[mode][0], 'flag programs on both targets', flush=True)
    expected = run([str(original)], True)
    negatives = []
    for name, c, h in mutants(mode, cpu, header):
        assert (c, h) != (cpu, header), name
        for target, command in build(root/'negative'/name, c, h):
            data = run(command, True)
            assert len(data) == len(expected) and data != expected, (mode, name, target)
            first = next(i for i, (a, b) in enumerate(zip(data, expected)) if a != b)//72
            a = struct.unpack_from('<10I4Q', expected, first*72)
            b = struct.unpack_from('<10I4Q', data, first*72)
            negatives.append(dict(fault=name, target=target, first_record=first,
                differing_fields=[i for i, (x, y) in enumerate(zip(a, b)) if x != y],
                original=list(a), mutant=list(b)))
        print('PASS distinguish original', mode, name, 'both targets', flush=True)
    paths = [Path(__file__), owner/'fist_cpu.h', owner/'fist_exec.h',
        fixtures/'tests/cpu_task_gate_flags.c', fixtures/'tests/cpu_task_gate_flags_cases.inc',
        fixtures/'tools/oracle/cpu_task_gate_flags.cpp', *sorted((repo/'re_out').glob('*.h')),
        *tree.rglob('*.h'), tree/'src/cpu/flags.cpp', tree/'src/cpu/instructions.h']
    proof = dict(scope='Complete original SUBW or SHLB/W/D operations,all six queries,FillFlags '
        'and carry-preserving INC from shared explicit inputs. Dirty raw/lazy upper fields are '
        'compared; portable PF/AF use existing FillFlags. This is a flag contract only; '
        'instruction execution,protected faults,runtime and whole-game parity require other proofs.',
        mode=mode, results=results, causal_results=negatives,
        inputs_sha256={str(p): digest(p) for p in paths}, complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mode', choices=('sub-word', 'shl'), required=True)
    args = parser.parse_args()
    resource.setrlimit(resource.RLIMIT_CORE, (0, resource.getrlimit(resource.RLIMIT_CORE)[1]))
    capture(args.repo.resolve(strict=True), args.output.resolve(), args.mode)
