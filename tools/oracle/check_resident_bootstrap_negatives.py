#!/usr/bin/env python3
"""Reject incomplete original bootstrap captures and missing reached busy clears."""
import argparse
import json
from pathlib import Path
import shutil
import tempfile
import traceback
from capture_pit_events import digest, lines
from capture_pit_irq_frames import verify, shared_physical


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();repo=args.repo.resolve(strict=True);source=args.source.resolve(strict=True);root=args.output.resolve()
    assert root.is_relative_to(Path('/tmp')) and not root.is_relative_to(source)
    root.mkdir(parents=True,exist_ok=False);verify(repo,source,bootstrap=True);cases=[]
    for name,expected in [('missing-fetch','incomplete bootstrap fetch output'),
                          ('missing-full-RAM','memory_file'),
                          ('omitted-busy-clear','complete bootstrap AND RAM transition'),
                          ('wrong-AND-lazy-tag','complete bootstrap AND CPU transition'),
                          ('omitted-MOV-SS-credit','stack_before')]:
        with tempfile.TemporaryDirectory(dir=root,prefix=name+'-') as temporary:
            clone=Path(temporary)/'evidence';shutil.copytree(source,clone)
            folder=clone/'source';rows=lines(folder/'boot-fetches.jsonl')
            if name=='missing-fetch':
                missing=rows.pop();(folder/missing['memory_file']).unlink()
            elif name=='missing-full-RAM':(folder/rows[-1]['memory_file']).unlink()
            elif name=='omitted-busy-clear':
                b=next(q for q in rows if q['context'] and q['ordinal']==9)
                a=next(q for q in rows if q['group']==b['group'] and q['ordinal']==8)
                memory=bytearray((folder/b['memory_file']).read_bytes());old=(folder/a['memory_file']).read_bytes()
                opcode=bytes.fromhex(a['fetched_code_hex']);offset=int.from_bytes(opcode[3:5],'little')
                address=shared_physical(repo)(a['segments'][2]['base']+offset,memory,a)
                assert old[address]!=memory[address];memory[address]=old[address]
                (folder/b['memory_file']).write_bytes(memory)
            elif name=='wrong-AND-lazy-tag':
                b=next(q for q in rows if q['context']==0 and q['ordinal']==9);b['lflags.type']=0
            else:
                for q in rows:
                    if q['context']==0 and q['ordinal']>=8:q['CPU_Cycles']-=1
            (folder/'boot-fetches.jsonl').write_text(''.join(json.dumps(q)+'\n' for q in rows))
            try:verify(repo,clone,bootstrap=True)
            except (AssertionError,FileNotFoundError) as error:
                frames=traceback.extract_tb(error.__traceback__);last=frames[-1]
                if name in ('missing-full-RAM','omitted-MOV-SS-credit'):
                    assert isinstance(error,AssertionError) and expected in last.line,(name,str(error),last.line)
                else:assert expected in str(error),(name,str(error))
                cases.append(dict(name=name,rejected=True,reason=str(error),check=last.line))
            else:raise AssertionError('negative accepted: '+name)
    proof=dict(scope='Original bootstrap completeness and reached AND/MOV-SS verifier negatives; no port instruction/IRQ integration or full original output acceptance.',cases=cases,checker_sha256=digest(__file__),source_fixture_sha256=digest(repo/'tools/oracle/resident_bootstrap_case.json'),source_manifest_sha256=digest(source/'producers.json'),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: all five original bootstrap negatives reject; full-RAM clones retired')


if __name__=='__main__':main()
