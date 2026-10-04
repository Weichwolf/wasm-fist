#!/usr/bin/env python3
"""Recover original CR3/CR0 transitions and all initially linked cache entries."""
import argparse
import copy
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
from capture_pit_events import digest, format_case, lines
from capture_pit_irq_frames import ROOT, source_paths
from sequence_format import validate, validate_endpoint

META = {'kind', 'label', 'arguments', 'context', 'group', 'architecture',
        'auto_determine', 'normal_core', 'cache', 'memory_file', 'return_value'}


def observer(repo):
    """Reuse the bootstrap owner, retaining its metadata and only CRX RAM files."""
    text = (repo/'tools/oracle/pit_irq_frame.gdb').read_text()
    for needle in ("(root/name).write_bytes(memory())",
                   "(root/q['memory_file']).write_bytes(m)",
                   "(root/q['memory_file']).write_bytes(memory())"):
        assert needle in text, ('shared observer RAM writer changed', needle)
        text = text.replace(needle, 'None # Canonical bootstrap owner retains these RAM boundaries.')
    needle = "trace=Fetch('fist_cpu_trace');trace.enabled=False"
    assert text.count(needle) == 1
    return text.replace(needle, (repo/'tools/oracle/paging_control.gdb.inc').read_text()+'\n'+needle)


def portable(row):
    """Keep actual cache slots/validity/types; raw host pointers stay in source."""
    q = copy.deepcopy(row)
    cache = q['cache']
    cache.pop('mem_base'); cache.pop('init_handler')
    for entry in cache['entries'].values():
        entry['read'] = bool(entry['read']); entry['write'] = bool(entry['write'])
        entry.pop('readhandler'); entry.pop('writehandler')
    return q


def verify(repo, evidence, reference=True):
    for path, h in json.loads((evidence/'producers.json').read_text()).items():
        assert digest(path) == h, path
    originals = json.loads((evidence/'original-hashes.json').read_text())
    for path, h in originals.items(): assert digest(repo/path) == h, path
    shared = json.loads((repo/'tools/oracle/pit_irq_frame_case.json').read_text())['original']
    bootstrap = json.loads((repo/'tools/oracle/resident_bootstrap_case.json').read_text())['original']
    for name in ('baseline', 'source'):
        folder = evidence/name
        assert (evidence/(name+'.exit')).read_text() == '0\n'
        assert 'Python Exception' not in (folder/'dosbox.log').read_text()
        assert validate_endpoint(folder/'sequence', 2000) == 2000
        assert validate(folder/'sequence.frames', 'F')['records'] == 137
        assert validate(folder/'sequence.pcm', 'A')['samples'] == 89258
        assert {k:digest(folder/('sequence.'+k)) for k in ('frames','pcm','end')} == shared['capture_sha256']
    for suffix in ('text', 'bda', 'vga'):
        assert (evidence/'baseline'/('start-state.'+suffix)).read_bytes() == (evidence/'source'/('start-state.'+suffix)).read_bytes()
    source = evidence/'source'
    assert lines(source/'irq-events.jsonl') == shared['events']
    assert lines(source/'irq-fetches.jsonl') == shared['fetches']
    assert lines(source/'system-events.jsonl') == bootstrap['system_events']
    boot = lines(source/'boot-fetches.jsonl')
    assert boot == bootstrap['bootstrap']['fetches']
    assert json.loads((source/'irq-completion.json').read_text()) == bootstrap['completion']
    rows = lines(source/'crx-events.jsonl')
    assert len(rows) == 8, 'incomplete actual CPU_WRITE_CRX boundaries'
    assert [q['label'] for q in rows] == [i for i in range(1,5) for _ in range(2)]
    assert {p.name for p in source.glob('*.memory')} == {q['memory_file'] for q in rows}, 'incomplete CRX RAM inventory'
    header = (repo/'third_party/dosbox-build/dosbox-0.74-3/include/paging.h').read_text()
    flags = {key:int(value,0) for key,value in re.findall(r'^#define\s+(PFLAG_INIT|PFLAG_NOCODE)\s+(0x[0-9a-f]+)',header,re.M)}
    assert len(flags) == 2
    init_flags = flags['PFLAG_INIT'] | flags['PFLAG_NOCODE']
    checks = []
    for a, b in zip(rows[::2], rows[1::2]):
        assert a['kind']=='before-crx' and b['kind']=='after-crx'
        for key in ('label','arguments','context','group','architecture','auto_determine','normal_core'):
            assert a[key] == b[key], key
        assert 'return_value' not in a and b['return_value'] == 0
        assert a['architecture']==255 and a['auto_determine']==0 and a['normal_core'] and a['cpu.cpl']==0
        expected = {k:copy.deepcopy(v) for k,v in a.items() if k not in META}
        cr = a['arguments']['cr']; value = a['arguments']['value']
        assert cr in (0,3)
        if cr == 3:
            expected['paging.cr3'] = value; invalidate = bool(a['paging.enabled'])
        else:
            expected['cpu.cr0'] = value; expected['cpu.pmode'] = int(bool(value&1))
            expected['paging.enabled'] = int(bool(value&1 and value&0x80000000))
            invalidate = expected['paging.enabled'] != a['paging.enabled']
        assert expected == {k:v for k,v in b.items() if k not in META}, 'complete CRX CPU/clock transition'
        before = (source/a['memory_file']).read_bytes(); after = (source/b['memory_file']).read_bytes()
        assert len(before)==len(after)==16777216 and before==after, 'complete CRX RAM transition'
        ca, cb = a['cache'], b['cache']
        assert ca['linked_pages'] and len(ca['linked_pages'])==len(set(ca['linked_pages']))
        assert cb['linked_pages'] == ([] if invalidate else ca['linked_pages']), 'complete linked-page transition'
        assert set(ca['entries'])==set(cb['entries'])=={str(page) for page in ca['linked_pages']}
        assert ca['mem_base']==cb['mem_base'] and ca['init_handler']==cb['init_handler']
        for page, entry in ca['entries'].items():
            want = copy.deepcopy(entry)
            if invalidate:
                want['read']=want['write']=0
                want['readhandler']=want['writehandler']=cb['init_handler']
                want['handler_types']={key:'InitPageHandler *' for key in ('readhandler','writehandler')}
                want['handler_flags']={key:init_flags for key in ('readhandler','writehandler')}
            assert cb['entries'][page] == want, 'complete observed cache entry transition'
            for key in ('read','write'):
                if entry[key]:
                    assert entry[key]==ca['mem_base']+((entry['phys_page']-int(page))<<12), 'direct cache mapping'
        checks.append(dict(label=a['label'],context=a['context'],group=a['group'],arguments=a['arguments'],
                           cache_invalidated=invalidate,linked_pages_before=len(ca['linked_pages']),
                           linked_pages_after=len(cb['linked_pages']),complete_CPU_and_clock_equal=True,
                           complete_RAM_unchanged=True,all_linked_cache_entries_equal=True,physical_slots_retained=True))
    assert [q['arguments']['cr'] for q in checks] == [3,0,3,0]
    assert [q['cache_invalidated'] for q in checks] == [False,True,False,True]
    original = dict(transitions=[portable(q) for q in rows],contracts=checks,
                    shared_bootstrap_case_sha256=digest(repo/'tools/oracle/resident_bootstrap_case.json'),
                    frames=137,mixed_samples=89258,endpoint_ms=2000,capture_sha256=shared['capture_sha256'])
    if reference:
        assert json.loads((repo/'tools/oracle/paging_control_case.json').read_text())['original'] == original, 'original CRX reference changed'
    proof = dict(scope='Original first/repeated actual CPU_WRITE_CRX entry/return. Four pairs preserve all CPU/system/time fields, eight whole16MiB boundaries and every initially linked cache entry/list. CR3 with paging off preserves links; PG enable invalidates read/write/handlers and retains physical slots. Shared IRQ/bootstrap metadata and complete matched original output unchanged. Other modes/permissions, faults/device execution, production CPU/RAM integration and full app output remain open.',
                 original=original,originals_unchanged=len(originals),producer_manifest_sha256=digest(evidence/'producers.json'),
                 memory_sha256={q['memory_file']:digest(source/q['memory_file']) for q in rows},
                 commit_parent=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
                 complete_original_acceptance=False,
                 reproduction=['python3 -B tools/oracle/capture_paging_control.py --repo . --output /tmp/wasm-fist-paging-control-replay',
                               'python3 -B tools/oracle/capture_paging_control.py --repo . --output /tmp/wasm-fist-paging-control-replay --verify-only'])
    (evidence/'proof.json').write_text(format_case(proof)+'\n')
    print('PASS: four original CRX pairs/eight full-RAM boundaries/all linked cache entries; complete137frame/89258PCM/end2000 unchanged')
    return proof


def capture(repo, evidence):
    assert evidence.is_relative_to(Path('/tmp'))
    evidence.mkdir(parents=True,exist_ok=False)
    gdb_script=evidence/'paging-control.gdb'; gdb_script.write_text(observer(repo))
    paths=[*source_paths(repo),*(repo/'tools/oracle'/name for name in (
        'capture_paging_control.py','paging_control.gdb.inc','resident_bootstrap_case.json','pit_irq_frame_case.json')),
        gdb_script,*(Path(subprocess.check_output(['which',name],text=True).strip()).resolve() for name in ('python3','gdb'))]
    (evidence/'producers.json').write_text(format_case({str(p):digest(p) for p in paths})+'\n')
    (evidence/'original-hashes.json').write_text(format_case({str(p.relative_to(repo)):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()})+'\n')
    for name in ('baseline','source'):
        folder=evidence/name; folder.mkdir(); wrapper=evidence/('dosbox-'+name)
        command=(['gdb','-q','-batch','-x',str(gdb_script),'--args'] if name=='source' else [])+[str(repo/'third_party/dosbox-fist')]
        wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec '+shlex.join(command)+' "$@"\n');wrapper.chmod(0o755)
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
        env.update(FIST_SEQUENCE_END_MS='2000',FIST_ORACLE_WALL_SECONDS='120',DOSBOX=str(wrapper),
                   FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder),FIST_ORACLE_BOOTSTRAP='1')
        with (evidence/(name+'.log')).open('w') as log:
            result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','2',str(folder)],cwd=repo,env=env,
                                  stdout=log,stderr=subprocess.STDOUT,timeout=130)
        (evidence/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode==0,(name,result.returncode)
    return verify(repo,evidence,reference=(repo/'tools/oracle/paging_control_case.json').exists())


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true');args=parser.parse_args()
    repo=args.repo.resolve(strict=True);evidence=args.output.resolve()
    if args.verify_only:verify(repo,evidence)
    else:capture(repo,evidence)


if __name__=='__main__':main()
