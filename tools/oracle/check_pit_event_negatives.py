#!/usr/bin/env python3
"""Reject missing output and coherent changes to complete original PIT records."""
import argparse
import copy
import json
from pathlib import Path
import shutil
import tempfile
from capture_pit_events import digest,lines,verify

def write(path,rows):
    Path(path).write_text(''.join(json.dumps(q)+'\n' for q in rows))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();repo=args.repo.resolve(strict=True);source=args.source.resolve(strict=True);root=args.output.resolve()
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    verify(repo,source)
    cases=[]
    for name,expected in (('missing-original-row','incomplete original budget output'),
                          ('coherent-one-cycle-budget-change','complete source reference changed'),
                          ('coherent-deadline-change','queue float deadline does not match its original index'),
                          ('changed-IRQ-request','complete source reference changed'),
                          ('missing-port-completion','incomplete port clock output')):
        with tempfile.TemporaryDirectory(dir=root,prefix=name+'-') as temporary:
            clone=Path(temporary)/'evidence';shutil.copytree(source,clone)
            baseline=lines(clone/'baseline-budget.stdout');trace=lines(clone/'trace-budget.stdout')
            if name=='missing-original-row':
                baseline.pop();trace=[q for q in trace if not (q['kind']=='budget' and q['label']==735)]
            elif name=='coherent-one-cycle-budget-change':
                baseline[107]['cycles']+=1
                trace[next(i for i,q in enumerate(trace) if q['kind']=='budget' and q['label']==107)]=copy.deepcopy(baseline[107])
            elif name=='coherent-deadline-change':
                baseline[107]['queue'][0][1]+=1
                trace[next(i for i,q in enumerate(trace) if q['kind']=='budget' and q['label']==107)]=copy.deepcopy(baseline[107])
            elif name=='changed-IRQ-request':
                q=next(q for q in trace if q['kind']=='irq0-activate');q['irq0'][0]=0;q['IRQCheck']=0
            else:
                path=clone/'wasm-clock.stdout';path.write_text('\n'.join(path.read_text().splitlines()[:-1])+'\n')
            write(clone/'baseline-budget.stdout',baseline);write(clone/'trace-budget.stdout',trace)
            try:verify(repo,clone)
            except AssertionError as error:
                assert str(error)==expected,(name,str(error))
                cases.append(dict(name=name,rejected=True,reason=str(error)))
            else:raise AssertionError('negative accepted: '+name)
    proof=dict(scope='Complete source verifier rejects missing original/port output, a coherent one-cycle budget adjustment, a coherent queue deadline change and changed real IRQ request state. Temporary clones retired; no runtime clock/full-output acceptance.',
               cases=cases,checker_sha256=digest(Path(__file__)),verifier_sha256=digest(repo/'tools/oracle/capture_pit_events.py'),
               source_fixture_sha256=digest(repo/'tools/oracle/pit_event_case.json'),
               original_source_output_sha256=digest(source/'baseline-budget.stdout'),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: all five PIT source verifier negatives reject')

if __name__=='__main__':main()
