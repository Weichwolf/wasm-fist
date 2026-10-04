#!/usr/bin/env python3
"""Reject missing RAM, corrupted caller frames and coherent startup-prefix CPU defects."""
import argparse
import copy
import json
import os
from pathlib import Path
import shutil
import tempfile
from verify_device_start_prefix import digest,verify


def check(root,repo,output):
    source=verify(root,repo)
    assert output.resolve().is_relative_to(Path('/tmp'))
    output.mkdir(parents=True,exist_ok=False)
    rows=[json.loads(s) for s in (root/'source/prefix-fetches.jsonl').read_text().splitlines()]
    cases=('missing-terminal-memory','wrong-caller-CALL-frame','corrupted-upper-EAX',
           'lost-ORb-tag','narrowed-logical-TCB','wrong-RET-stack-width')
    results=[]
    for kind in cases:
        with tempfile.TemporaryDirectory(dir=output,prefix=kind+'-') as temporary:
            clone=Path(temporary)
            for phase in ('baseline','source'):
                shutil.copytree(root/phase,clone/phase,copy_function=os.link,
                                ignore=shutil.ignore_patterns('game'))
            for name in ('baseline.exit','source.exit','producers.json','original-hashes.json'):
                shutil.copy2(root/name,clone/name)
            altered=copy.deepcopy(rows)
            if kind=='missing-terminal-memory':
                (clone/'source/7809.memory').unlink()
            elif kind=='wrong-caller-CALL-frame':
                frame=source['calls'][0]['stack_physical']
                for label in ('1280','12ab','77ee'):
                    p=clone/'source'/(label+'.memory')
                    ram=bytearray(p.read_bytes());ram[frame]^=1
                    p.unlink();p.write_bytes(ram)
            elif kind=='corrupted-upper-EAX':
                altered[4]['registers'][0]^=0x10000
            elif kind=='lost-ORb-tag':
                for q in altered[239:]:q['lflags.type']=0
            elif kind=='narrowed-logical-TCB':
                for q in altered[3:14]:q['registers'][3]&=0xffff
            elif kind=='wrong-RET-stack-width':
                for q in altered[10:14]:q['registers'][4]-=2
            else:raise AssertionError(kind)
            if kind not in ('missing-terminal-memory','wrong-caller-CALL-frame'):
                p=clone/'source/prefix-fetches.jsonl';p.unlink()
                p.write_text(''.join(json.dumps(q)+'\n' for q in altered))
                for p in (clone/'source').glob('*.json'):
                    candidates=[q for q in altered if q['cpu_regs.ip.dword[0]']==int(p.stem,16)]
                    assert len(candidates)==1
                    p.unlink();p.write_text(json.dumps(candidates[0],indent=2)+'\n')
            try:verify(clone,repo)
            except (AssertionError,FileNotFoundError) as error:
                results.append(dict(negative=kind,rejected=True,error_type=type(error).__name__,reason=str(error)))
            else:raise AssertionError('Verifier accepted '+kind)
    proof=dict(scope='Six missing/coherent-corruption cases for the complete original77e2 prefix evidence.',
               source_proof_sha256=digest(root/'proof.json'),verifier_sha256=digest(repo/'tools/oracle/verify_device_start_prefix.py'),
               checker_sha256=digest(__file__),cases=results,retired_clone_directories=True)
    (output/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: all six original prefix verifier negatives rejected; temporary clones removed')
    return proof


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    return check(args.source.resolve(strict=True),args.repo.resolve(strict=True),args.output.resolve())


if __name__=='__main__':main()
