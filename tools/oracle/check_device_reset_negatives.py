#!/usr/bin/env python3
"""Reject missing and coherently corrupted original DSP-reset evidence."""
import argparse
import copy
import json
import os
from pathlib import Path
import shutil
import tempfile

from verify_device_reset import digest, verify


def check(root, repo, output):
    verify(root, repo)
    assert output.resolve().is_relative_to(Path('/tmp'))
    output.mkdir(parents=True, exist_ok=False)
    rows = [json.loads(line) for line in (root/'source/reset-fetches.jsonl').read_text().splitlines()]
    cases = ('missing-return-RAM', 'MOV-BL-clears-upper-EBX', 'extra-dispatch-return-frame',
             'omitted-CLI', 'omitted-reset-core-FillFlags', 'omitted-budget-core-FillFlags',
             'wrong-DSP-ready-byte')
    results = []
    for name in cases:
        with tempfile.TemporaryDirectory(prefix=name+'-', dir=output) as temporary:
            clone = Path(temporary)
            for phase in ('baseline', 'source'):
                shutil.copytree(root/phase, clone/phase, copy_function=os.link,
                                ignore=shutil.ignore_patterns('game'))
            for file in ('baseline.exit', 'source.exit', 'producers.json', 'original-hashes.json'):
                shutil.copy2(root/file, clone/file)
            altered = copy.deepcopy(rows)
            if name == 'missing-return-RAM':
                (clone/'source/77ff.memory').unlink()
            elif name == 'MOV-BL-clears-upper-EBX':
                for q in altered[1:4]:
                    q['registers'][3] &= 255
            elif name == 'extra-dispatch-return-frame':
                for q in altered[6:]:
                    q['registers'][4] -= 4
            elif name == 'omitted-CLI':
                for q in altered[7:220]:
                    q['cpu_regs.flags'] |= 0x200
            elif name == 'omitted-reset-core-FillFlags':
                for q in altered[75:210]:
                    q['cpu_regs.flags'] &= ~0x40
                for q in altered[75:79]:
                    q['lflags.type'] = rows[74]['lflags.type']
            elif name == 'omitted-budget-core-FillFlags':
                for q in altered[210:214]:
                    q['lflags.type'] = rows[209]['lflags.type']
            elif name == 'wrong-DSP-ready-byte':
                altered[218]['registers'][0] ^= 1
            else:
                raise AssertionError(name)
            # Rewrite all paired boundary JSON as well as fetch rows. A rejection
            # must prove behavior, rather than stale JSON/fetch disagreement.
            fetched = clone/'source/reset-fetches.jsonl'
            fetched.unlink()
            fetched.write_text(''.join(json.dumps(q)+'\n' for q in altered))
            for path in (clone/'source').glob('*.json'):
                matches = [q for q in altered if q['cpu_regs.ip.dword[0]'] == int(path.stem, 16)]
                assert len(matches) == 1
                path.unlink()
                path.write_text(json.dumps(matches[0], indent=2)+'\n')
            try:
                verify(clone, repo)
            except (AssertionError, FileNotFoundError) as error:
                results.append(dict(name=name, rejected=True, error_type=type(error).__name__,
                                    reason=str(error)))
            else:
                raise AssertionError('Verifier accepted '+name)
    proof = dict(scope='Seven coherent negative cases for actual original DSP reset evidence.',
                 verifier_sha256=digest(repo/'tools/oracle/verify_device_reset.py'),
                 checker_sha256=digest(__file__), source_proof_sha256=digest(root/'proof.json'),
                 cases=results, retired_clone_directories=True)
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: all seven missing/coherent-corruption cases rejected; disposable clones removed')
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--capture', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    check(args.capture.resolve(strict=True), args.repo.resolve(strict=True), args.output)
