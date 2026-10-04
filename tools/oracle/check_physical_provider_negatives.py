#!/usr/bin/env python3
"""Reject missing provider state, invented identity maps and writable ROM evidence."""
import argparse
import copy
import json
from pathlib import Path
import tempfile
from capture_physical_provider import ROOT, verify
from capture_pit_events import digest, format_case, lines


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,default=ROOT);p.add_argument('--source',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    repo=a.repo.resolve(strict=True);source=a.source.resolve(strict=True);output=a.output.resolve()
    assert output.is_relative_to(Path('/tmp'));output.mkdir(parents=True,exist_ok=False)
    verify(repo,source);results=[]
    cases=(('missing-boundary','incomplete physical-provider boundaries'),
           ('missing-slot','incomplete physical-provider slots'),
           ('missing-first-MB-page','incomplete first-MB map'),
           ('writable-ROM','physical handler class/flags'),
           ('wrong-cache-provider','cache/physical provider mismatch'),
           ('A20-derived-from-controlport','A20 first-MB mapping'),
           ('invented-identity-first-MB','original physical-provider reference changed'))
    for name,reason in cases:
        with tempfile.TemporaryDirectory(prefix='case-',dir=output) as temp:
            root=Path(temp)
            for path in source.iterdir():
                if path.name not in ('source','proof.json'):(root/path.name).symlink_to(path)
            (root/'source').mkdir()
            for path in (source/'source').iterdir():
                if path.name!='crx-events.jsonl':(root/'source'/path.name).symlink_to(path)
            rows=copy.deepcopy(lines(source/'source/crx-events.jsonl'))
            provider=rows[0]['provider']
            if name=='missing-boundary':rows.pop()
            elif name=='missing-slot':provider['slots'].pop()
            elif name=='missing-first-MB-page':provider['firstmb'].pop()
            elif name=='writable-ROM':
                next(h for h in provider['handlers'].values() if h['type']=='ROMPageHandler *')['flags']|=2
            elif name=='wrong-cache-provider':
                ram=next(int(k) for k,h in provider['handlers'].items() if h['type']=='RAMPageHandler *')
                entry=next(v for v in rows[0]['cache']['entries'].values() if v['handler_types']['writehandler']=='ROMPageHandler *')
                provider['slots'][entry['phys_page']]=ram
            elif name=='A20-derived-from-controlport':provider['a20_enabled']=bool(provider['a20_controlport']&2)
            else:
                for row in rows:row['provider']['firstmb']=list(range(272))
            (root/'source/crx-events.jsonl').write_text(''.join(json.dumps(q)+'\n' for q in rows))
            try:verify(repo,root)
            except AssertionError as error:
                assert str(error)==reason,(name,str(error),reason)
                results.append(dict(name=name,rejected=True,reason=reason))
            else:raise AssertionError(('invalid original provider evidence accepted',name))
    proof=dict(scope='Physical-provider observation completeness and evidence negatives only; physical memory execution and full original output remain open.',
               cases=results,checker_sha256=digest(Path(__file__).resolve()),
               source_case_sha256=digest(repo/'tools/oracle/physical_provider_case.json'),
               source_manifest_sha256=digest(source/'producers.json'),complete_original_acceptance=False)
    (output/'proof.json').write_text(format_case(proof)+'\n')
    print('PASS: seven incomplete or incorrect provider observations rejected')


if __name__=='__main__':main()
