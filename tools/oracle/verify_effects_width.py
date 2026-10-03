"""Bind the shared initial-mixer fixture to actual effects-call RAM boundaries."""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import struct

from verify_effects_mode import verify


def bind(root, repo):
    source = verify(root, repo)
    fixture_path = repo / 'tools/oracle/initial_mixer_call_case.json'
    fixture = json.loads(fixture_path.read_text())
    program = (repo / 'tools/oracle/file_error.gdb').read_text().split(
        '\npython\n', 1)[1].rsplit('\nend\nrun', 1)[0]
    nodes = [n for n in ast.parse(program).body
             if isinstance(n, ast.FunctionDef) and n.name == 'physical']
    assert len(nodes) == 1
    namespace = {'struct': struct}
    exec(compile(ast.fix_missing_locations(ast.Module(body=nodes, type_ignores=[])),
                 'file_error.gdb:physical', 'exec'), namespace)
    physical = namespace['physical']
    checks = []
    for name, fields in [('02-76fd', 'before'), ('02-138d', 'after')]:
        q = source['states'][name]
        memory = (root / 'source' / (name + '.memory')).read_bytes()
        assert len(memory) == 16777216
        assert hashlib.sha256(memory).hexdigest() == source['memory_sha256'][name]
        def read(linear, size):
            return bytes(memory[physical((linear + i) & 0xffffffff, memory, q)]
                         for i in range(size))
        for field in fixture[fields]:
            expected = bytes.fromhex(field['hex'])
            actual = read(q['segments'][3]['base'] + field['offset'], len(expected))
            assert actual == expected, (name, hex(field['offset']))
            checks.append(dict(state=name, offset=field['offset'], hex=actual.hex()))
        assert read(q['segments'][3]['base'] + 0x77e0, 6).hex() == '0102c605e077'
        ring = read(fixture['ring_physical'], fixture['ring_length'])
        assert hashlib.sha256(ring).hexdigest() == fixture['ring_sha256'], name
    proof = dict(
        scope='Actual ready1/mode2 effects76fd through138d: all20 shared module '
              'fixture fields and both complete2048-byte DMA boundaries match '
              'initial_mixer_call_case.json. BYTE ready/mode neighbors retain '
              '0102c605e077. No complete port GP/stack/device/time/PCM acceptance.',
        module_fields=checks, dma_boundaries=2, dma_bytes=fixture['ring_length'],
        source_memory_sha256={n: source['memory_sha256'][n]
                              for n in ('02-76fd', '02-138d')},
        fixture_sha256=hashlib.sha256(fixture_path.read_bytes()).hexdigest(),
        effects_case_sha256=hashlib.sha256(
            (repo / 'tools/oracle/effects_mode_case.json').read_bytes()).hexdigest(),
        complete_original_acceptance=False)
    print('PASS: actual effects before/after matches all20 shared mixer fields '
          'and both complete DMA boundaries; BYTE neighbors unchanged')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    proof = bind(args.source.resolve(strict=True), args.repo.resolve(strict=True))
    args.output.write_text(json.dumps(proof, indent=2) + '\n')
