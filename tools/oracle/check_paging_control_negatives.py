#!/usr/bin/env python3
"""Reject incomplete CRX observations and incorrect cache transitions."""
import argparse
import copy
import json
from pathlib import Path
import shutil
import tempfile
from capture_paging_control import ROOT, verify
from capture_pit_events import digest, format_case, lines


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    repo=args.repo.resolve(strict=True);source=args.source.resolve(strict=True);output=args.output.resolve()
    assert output.is_relative_to(Path('/tmp'));output.mkdir(parents=True,exist_ok=False)
    verify(repo,source)
    results=[]
    for name,reason in (('missing-boundary','incomplete actual CPU_WRITE_CRX boundaries'),
                        ('missing-full-RAM','incomplete CRX RAM inventory'),
                        ('omitted-invalidation','complete observed cache entry transition'),
                        ('lost-physical-slot','complete observed cache entry transition'),
                        ('premature-CR3-invalidation','complete linked-page transition')):
        with tempfile.TemporaryDirectory(prefix='case-',dir=output) as temp:
            root=Path(temp)/'original'
            shutil.copytree(source,root,ignore=shutil.ignore_patterns('game'))
            folder=root/'source';rows=lines(folder/'crx-events.jsonl')
            if name=='missing-boundary':rows.pop()
            elif name=='missing-full-RAM':(folder/rows[-1]['memory_file']).unlink()
            elif name=='omitted-invalidation':
                before,after=rows[2:4]
                page=next(k for k,v in before['cache']['entries'].items() if v['read'])
                after['cache']['entries'][page]=copy.deepcopy(before['cache']['entries'][page])
            elif name=='lost-physical-slot':
                entry=next(v for v in rows[3]['cache']['entries'].values() if v['phys_page'])
                entry['phys_page']=0
            else:rows[1]['cache']['linked_pages']=[]
            (folder/'crx-events.jsonl').write_text(''.join(json.dumps(q)+'\n' for q in rows))
            try:verify(repo,root)
            except AssertionError as error:
                assert str(error)==reason,(name,str(error),reason)
                results.append(dict(name=name,rejected=True,reason=reason))
            else:raise AssertionError(('invalid original evidence accepted',name))
    proof=dict(scope='Original CRX completeness and observed cache-transition negatives only; no production CPU/RAM or full original-output acceptance.',
               cases=results,checker_sha256=digest(Path(__file__).resolve()),
               source_case_sha256=digest(repo/'tools/oracle/paging_control_case.json'),
               source_manifest_sha256=digest(source/'producers.json'),complete_original_acceptance=False)
    (output/'proof.json').write_text(format_case(proof)+'\n')
    print('PASS: missing CRX/RAM, omitted invalidation, lost physical slot and premature CR3 invalidation rejected')


if __name__=='__main__':main()
