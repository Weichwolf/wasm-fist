#!/usr/bin/env python3
"""Observe complete physical providers and first-MB/A20 maps at original CRX boundaries."""
import argparse
import copy
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile
from capture_paging_control import observer as crx_observer, verify as verify_crx
from capture_pit_irq_frames import ROOT, source_paths
from capture_pit_events import digest, format_case, lines


def observer(repo):
    text=crx_observer(repo)
    needle='def crx_record(kind,label,args,watched=(),return_value=None):'
    assert text.count(needle)==1
    text=text.replace(needle,(repo/'tools/oracle/physical_provider.gdb.inc').read_text()+'\n'+needle)
    needle=" if return_value is not None:q['return_value']=int(return_value)"
    assert text.count(needle)==1
    return text.replace(needle," q['provider']=physical_provider_state()\n"+needle)


def portable(provider):
    """Lossless page ranges; host handler pointers remain only in raw observations."""
    p=copy.deepcopy(provider)
    slots=p.pop('slots');handlers=p.pop('handlers');ranges=[];start=0
    for end in range(1,len(slots)+1):
        if end==len(slots) or slots[end]!=slots[start]:
            ranges.append(dict(first_page=start,end_page=end,**handlers[str(slots[start])]))
            start=end
    p['ranges']=ranges
    for key in ('handler','mmiohandler'):p['lfb'][key]=bool(p['lfb'][key])
    return p


def crx_view(repo,evidence,rows):
    """Reverify the unchanged complete CRX owner, stripping only added provider metadata."""
    with tempfile.TemporaryDirectory(prefix='physical-provider-crx-',dir='/tmp') as temp:
        view=Path(temp)
        for path in evidence.iterdir():
            if path.name not in ('source','proof.json'):(view/path.name).symlink_to(path)
        (view/'source').mkdir()
        for path in (evidence/'source').iterdir():
            if path.name!='crx-events.jsonl':(view/'source'/path.name).symlink_to(path)
        base=copy.deepcopy(rows)
        for row in base:row.pop('provider')
        (view/'source/crx-events.jsonl').write_text(''.join(json.dumps(q)+'\n' for q in base))
        return verify_crx(repo,view)


def verify(repo,evidence,reference=True):
    for path,h in json.loads((evidence/'producers.json').read_text()).items():assert digest(path)==h,path
    rows=lines(evidence/'source/crx-events.jsonl')
    assert len(rows)==8,'incomplete physical-provider boundaries'
    header=(repo/'third_party/dosbox-build/dosbox-0.74-3/include/paging.h').read_text()
    flags={k:int(v,0) for k,v in re.findall(r'^#define\s+(PFLAG_[A-Z]+)\s+(0x[0-9a-f]+)',header,re.M)}
    types={'RAMPageHandler *':flags['PFLAG_READABLE']|flags['PFLAG_WRITEABLE'],
           'ROMPageHandler *':flags['PFLAG_READABLE']|flags['PFLAG_HASROM'],
           'VGA_Map_Handler *':flags['PFLAG_READABLE']|flags['PFLAG_WRITEABLE']|flags['PFLAG_NOCODE'],
           'VGA_ChainedVGA_Handler *':flags['PFLAG_NOCODE']}
    states=[]
    for row in rows:
        p=row['provider']
        assert p['pages']==len(p['slots'])==16777216//4096,'incomplete physical-provider slots'
        assert set(p['handlers'])=={str(v) for v in p['slots']},'incomplete physical-handler inventory'
        for h in p['handlers'].values():
            assert h['type'] in types and h['flags']==types[h['type']],'physical handler class/flags'
        assert len(p['firstmb'])==272 and all(type(v)==int and 0<=v<1048576 for v in p['firstmb']),'incomplete first-MB map'
        assert type(p['a20_enabled'])==bool and type(p['a20_controlport'])==int and 0<=p['a20_controlport']<256,'incomplete A20 state'
        assert p['firstmb'][256:272]==list(range(256 if p['a20_enabled'] else 0,272 if p['a20_enabled'] else 16)),'A20 first-MB mapping'
        lfb=p['lfb']
        assert lfb['end_page']==lfb['start_page']+lfb['pages'],'incomplete LFB range'
        assert set(lfb['handlers'])=={'handler','mmiohandler'},'incomplete LFB handler inventory'
        for key,h in lfb['handlers'].items():
            assert bool(lfb[key])==(h is not None),'LFB handler presence'
        cache=row['cache']
        for linear,entry in cache['entries'].items():
            if entry['readhandler']==entry['writehandler']==cache['init_handler']:continue
            physical=entry['phys_page']
            assert physical<p['pages'],'cached physical provider outside inventory'
            pointer=p['slots'][physical];h=p['handlers'][str(pointer)]
            for key in ('readhandler','writehandler'):
                assert entry[key]==pointer and entry['handler_types'][key]==h['type'] and entry['handler_flags'][key]==h['flags'],'cache/physical provider mismatch'
            for key,flag in (('read',flags['PFLAG_READABLE']),('write',flags['PFLAG_WRITEABLE'])):
                assert bool(entry[key])==bool(h['flags']&flag),'cache/physical direct capability mismatch'
            if not row['paging.enabled']:
                want=p['firstmb'][int(linear)] if int(linear)<272 else int(linear)
                assert physical==want,'cache/first-MB mapping mismatch'
        states.append(dict(label=row['label'],kind=row['kind'],context=row['context'],arguments=row['arguments'],provider=portable(p)))
    for a,b in zip(rows[::2],rows[1::2]):assert a['provider']==b['provider'],'CRX changed physical providers'
    base=crx_view(repo,evidence,rows)
    original=dict(states=states,shared_CRX_case_sha256=digest(repo/'tools/oracle/paging_control_case.json'),
                  frames=base['original']['frames'],mixed_samples=base['original']['mixed_samples'],
                  endpoint_ms=base['original']['endpoint_ms'],capture_sha256=base['original']['capture_sha256'])
    if reference:
        assert json.loads((repo/'tools/oracle/physical_provider_case.json').read_text())['original']==original,'original physical-provider reference changed'
    proof=dict(scope='Source observation only: all4096 physical slots, actual handler types/flags,272 first-MB mappings, separate A20 enabled/controlport and LFB state at eight actual CRX boundaries. Existing full CPU/system/time/RAM/cache/output contract reverified. Physical read/write execution, A20 transitions, production transport and complete original output remain open.',
               original=original,originals_unchanged=base['originals_unchanged'],
               producer_manifest_sha256=digest(evidence/'producers.json'),memory_sha256=base['memory_sha256'],
               shared_CRX_contracts=base['original']['contracts'],
               commit_parent=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
               complete_original_acceptance=False)
    (evidence/'proof.json').write_text(format_case(proof)+'\n')
    print('PASS: complete physical providers/first-MB/A20/LFB; unchanged full CRX/CPU/RAM/cache and137frames/89258PCM/end2000')
    return proof


def capture(repo,evidence):
    assert evidence.is_relative_to(Path('/tmp'))
    evidence.mkdir(parents=True,exist_ok=False)
    probe=evidence/'physical-provider.gdb';probe.write_text(observer(repo))
    names=('capture_paging_control.py','paging_control.gdb.inc','resident_bootstrap_case.json',
           'pit_irq_frame_case.json','paging_control_case.json','capture_physical_provider.py','physical_provider.gdb.inc')
    paths=[*source_paths(repo),*(repo/'tools/oracle'/n for n in names),probe,
           *(Path(subprocess.check_output(['which',n],text=True).strip()).resolve() for n in ('python3','gdb'))]
    (evidence/'producers.json').write_text(format_case({str(p):digest(p) for p in paths})+'\n')
    (evidence/'original-hashes.json').write_text(format_case({str(p.relative_to(repo)):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()})+'\n')
    for name in ('baseline','source'):
        folder=evidence/name;folder.mkdir();wrapper=evidence/('dosbox-'+name)
        command=(['gdb','-q','-batch','-x',str(probe),'--args'] if name=='source' else [])+[str(repo/'third_party/dosbox-fist')]
        wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec '+shlex.join(command)+' "$@"\n');wrapper.chmod(0o755)
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
        env.update(FIST_SEQUENCE_END_MS='2000',FIST_ORACLE_WALL_SECONDS='120',DOSBOX=str(wrapper),
                   FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder),FIST_ORACLE_BOOTSTRAP='1')
        with (evidence/(name+'.log')).open('w') as log:
            result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','2',str(folder)],cwd=repo,env=env,
                                  stdout=log,stderr=subprocess.STDOUT,timeout=130)
        (evidence/(name+'.exit')).write_text(str(result.returncode)+'\n');assert result.returncode==0,(name,result.returncode)
    return verify(repo,evidence,reference=(repo/'tools/oracle/physical_provider_case.json').exists())


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,default=ROOT);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--verify-only',action='store_true');a=p.parse_args()
    repo=a.repo.resolve(strict=True);evidence=a.output.resolve()
    if a.verify_only:verify(repo,evidence)
    else:capture(repo,evidence)


if __name__=='__main__':main()
