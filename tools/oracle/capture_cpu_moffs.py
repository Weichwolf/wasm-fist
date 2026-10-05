#!/usr/bin/env python3
"""Compare actual original A0/A1 loads and every ordered code/RAM access."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import struct
import subprocess
from cpu_execute_probe import build as original_build
from capture_cpu_byte_instructions import target_build, digest
from capture_cpu_shr_instructions import RECORD_SIZE, parse


def packet(code, big=0, stack=0, ip=0x3b43, lazy=23):
    words = [big, stack, ip, 0xfedcba98, 0xcafea57e, 0x9abc1234,
             0xabcd5678, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000,
             0xffffffff, 1796, lazy, len(code)]
    segments = [v for selector in (0x10, 0x2082, 0x26e, 0x2d19, 0x4000, 0x6000)
                for v in (selector, selector << 4)]
    return struct.pack('<28I', *words, *segments) + code


def cases():
    corpus = []
    for big, width, address, stack, segment, offset in itertools.product(
            (0, 1), (1, 2, 4), (2, 4), (0, 1),
            (None, 0x26, 0x2e, 0x36, 0x3e, 0x64, 0x65),
            (0, 0x738, 0x73b, 0xfffe, 0xffff)):
        default = 4 if big else 2
        prefix = (b'\x66' if width != 1 and width != default else b'')
        prefix += b'\x67' if address != default else b''
        prefix += bytes([segment]) if segment is not None else b''
        code = prefix + bytes([0xa0 if width == 1 else 0xa1])
        code += offset.to_bytes(address, 'little')
        corpus.append(packet(code, big, stack,
                             ip=(0xffff, 0x3b43, 0x1fffe)[len(corpus) % 3],
                             lazy=len(corpus) % 65))
        if width == 1:
            corpus.append(packet(b'\x66' + code, big, stack, lazy=len(corpus) % 65))
    assert len(corpus) == 1120
    return corpus


def negative(repo, root, original, commands):
    header = (repo / 're_out/fist_exec.h').read_text()
    mutations = [
        ('widened-byte-load', header.replace('bytes=op==0xa0?1:width', 'bytes=width'),
         packet(b'\xa0\x3b\x07')),
        ('operand-sized-address', header.replace(
            'op==0xa0||op==0xa1){unsigned offset=fist_exec_fetch_code(e,&ip,address)',
            'op==0xa0||op==0xa1){unsigned offset=fist_exec_fetch_code(e,&ip,width)'),
         packet(b'\x67\xa0\x3b\x07\x01\x00')),
        ('ignored-cs-override', header.replace(
            'fist_ram_resident_read(e->bus,seg<6?seg:3,offset,bytes)',
            'fist_ram_resident_read(e->bus,3,offset,bytes)'),
         packet(b'\x2e\xa0\x3b\x07')),
        ('materialized-flags', header.replace('else if(op==0xa0||op==0xa1){',
            'else if(op==0xa0||op==0xa1){fist_cpu_fill_flags(e->bus->cpu);'),
         packet(b'\xa0\x3b\x07')),
    ]

    def run(command, data):
        result = subprocess.run(command, input=data, capture_output=True, timeout=30)
        assert result.returncode == 0, result.stderr.decode()
        assert len(result.stdout) == RECORD_SIZE
        return result.stdout

    results = []
    for name, mutant, data in mutations:
        assert mutant != header, name
        expected = run([original], data)
        a, sa, ma, fa, ra = parse(expected)
        for target, command in commands:
            assert run(command, data) == expected, (name, target, 'unmodified positive')
        for target, command in target_build(repo, root / name, mutant,
                segment_input=True, group_shift=True, ram_trace=True):
            actual = run(command, data)
            b, sb, mb, fb, rb = parse(actual)
            assert actual != expected, (name, target)
            assert ma == mb and sa == sb, (name, target, 'load changed RAM/stack metadata')
            fields = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
            assert fields, (name, target)
            if name == 'materialized-flags':
                assert fields == [9, 13] and fa == fb and ra == rb
            results.append(dict(fault=name, target=target, differing_CPU_words=fields,
                all2MiB_RAM_equal=True, full_stack_metadata_equal=True,
                code_fetch_equal=fa == fb, all_RAM_accesses_equal=ra == rb,
                unmodified_positive_source_bytes_equal=True,
                input_sha256=hashlib.sha256(data).hexdigest(),
                original_sha256=hashlib.sha256(expected).hexdigest(),
                mutant_sha256=hashlib.sha256(actual).hexdigest()))
        print('PASS distinguish moffs load', name, 'on both targets', flush=True)
    return results


def capture(repo, root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True, exist_ok=False)
    source = root / 'source'
    source.mkdir()
    original = original_build(repo, source, extra_opcodes=(0xa1,), byte_opcodes=(0xa0,),
        trace=True, lazy_input=True, segment_input=True, group_shift=True, ram_trace=True)
    commands = target_build(repo, root / 'targets',
        segment_input=True, group_shift=True, ram_trace=True)
    runs = [('original', [original]), *commands]
    corpus = cases()
    hashes = {target: hashlib.sha256() for target, _ in runs}
    for start in range(0, len(corpus), 16):
        batch = corpus[start:start + 16]
        data = b''.join(batch)
        expected = None
        for target, command in runs:
            result = subprocess.run(command, input=data, capture_output=True, timeout=45)
            assert result.returncode == 0, (target, start, result.stderr.decode())
            assert len(result.stdout) == len(batch) * RECORD_SIZE, (target, start, 'incomplete output')
            if expected is None:
                expected = result.stdout
            else:
                assert result.stdout == expected, (target, start,
                    next(i for i, (a, b) in enumerate(zip(result.stdout, expected)) if a != b))
            hashes[target].update(result.stdout)
        if start % 256 == 0:
            print('Matched original moffs loads', min(start + 16, len(corpus)), '/', len(corpus), flush=True)
    negatives = negative(repo, root / 'negative', original, commands)
    tree = repo / 'third_party/dosbox-build/dosbox-0.74-3'
    paths = [Path(__file__), repo / 'tools/oracle/capture_cpu_byte_instructions.py',
        repo / 'tools/oracle/capture_cpu_shr_instructions.py', repo / 'tests/cpu_execute_controlled.c',
        repo / 'tools/oracle/cpu_execute_probe.py', repo / 'tools/oracle/cpu_execute_probe.cpp',
        repo / 'tools/oracle/rep_probe.cpp', *source.glob('*.h'),
        *sorted((repo / 're_out').glob('*.h')), *tree.rglob('*.h'),
        tree / 'src/cpu/flags.cpp', tree / 'src/cpu/cpu.cpp',
        tree / 'src/cpu/modrm.cpp', tree / 'src/cpu/core_normal.cpp']
    proof = dict(scope='Actual original A0/A1 CASE_B/W/D and GetEADirect loads: '
        'complete31 CPU/cache words, three64-bit stack words, all2MiB RAM, every code '
        'fetch and every ordered physical RAM read/address/width/value match both release targets. '
        'Both code/address/stack sizes, all segments, unaligned/16-bit edge offsets, dirty upper '
        'EAX, byte66 independence and all65 incoming lazy tags are covered. Four causal faults '
        'fail both targets after their unmodified positives match the original. Protected/paging/'
        'provider faults, whole startup/handler and complete frame/audio acceptance remain separate.',
        cases=len(corpus), record_size=RECORD_SIZE,
        corpus_sha256=hashlib.sha256(b''.join(corpus)).hexdigest(),
        outputs_sha256={name: h.hexdigest() for name, h in hashes.items()},
        causal_negatives=negatives, inputs_sha256={str(p): digest(p) for p in paths},
        complete_original_acceptance=False)
    (root / 'proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    print('PASS original1120 moffs programs/8 causal negatives', flush=True)
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    capture(args.repo.resolve(), args.output.resolve())
