#!/usr/bin/env python3
"""Reject incomplete RAM and coherent corruption of the controlled configuration capture."""
import argparse
import copy
import json
import os
from pathlib import Path
import shutil
import tempfile
from capture_device_config_cpu import verify
from verify_device_start_prefix import digest


def check(root, repo, baseline, output):
    source = verify(root, repo, baseline)
    assert output.resolve().is_relative_to(Path('/tmp'))
    output.mkdir(parents=True, exist_ok=False)
    rows = source['fetches']
    results = []
    for kind in ('missing-return-memory', 'short-return-memory', 'corrupted-upper-AX',
                 'narrowed-logical-TCB', 'wrong-RET-stack-width', 'changed-code-lookahead'):
        with tempfile.TemporaryDirectory(dir=output, prefix=kind+'-') as temporary:
            clone = Path(temporary)
            shutil.copytree(root/'source', clone/'source', copy_function=os.link,
                            ignore=shutil.ignore_patterns('game'))
            for name in ('source.exit', 'producers.json'):
                shutil.copy2(root/name, clone/name)
            altered = copy.deepcopy(rows)
            if kind == 'missing-return-memory':
                (clone/'source/77ee.memory').unlink()
            elif kind == 'short-return-memory':
                p = clone/'source/77ee.memory'
                data = p.read_bytes(); p.unlink(); p.write_bytes(data[:-1])
            elif kind == 'corrupted-upper-AX':
                for q in altered[1:4]: q['registers'][0] &= 0xffff
            elif kind == 'narrowed-logical-TCB':
                for q in altered[1:]: q['registers'][3] &= 0xffff
            elif kind == 'wrong-RET-stack-width':
                altered[-1]['registers'][4] -= 2
            elif kind == 'changed-code-lookahead':
                q = altered[6]
                code = bytearray.fromhex(q['fetched_code_hex']); code[-1] ^= 1
                q['fetched_code_hex'] = code.hex()
            else:
                raise AssertionError(kind)
            if altered != rows:
                p = clone/'source/reset-fetches.jsonl'
                p.unlink(); p.write_text(''.join(json.dumps(q)+'\n' for q in altered))
            try:
                verify(clone, repo, baseline)
            except (AssertionError, FileNotFoundError) as error:
                results.append(dict(negative=kind, rejected=True,
                                    error_type=type(error).__name__, reason=str(error)))
            else:
                raise AssertionError('Verifier accepted '+kind)
    proof = dict(scope='Six missing/coherent-corruption cases for the controlled full CPU/RAM configuration evidence.',
                 source_proof_sha256=digest(root/'proof.json'),
                 verifier_sha256=digest(repo/'tools/oracle/capture_device_config_cpu.py'),
                 checker_sha256=digest(__file__), cases=results, retired_clone_directories=True)
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: six controlled configuration negatives rejected; temporary clones removed')
    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    return check(args.source.resolve(strict=True), args.repo.resolve(strict=True),
                 args.baseline.resolve(strict=True), args.output.resolve())


if __name__ == '__main__':
    main()
