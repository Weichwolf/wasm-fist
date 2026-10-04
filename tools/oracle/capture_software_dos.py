#!/usr/bin/env python3
"""Actual first DOS software interrupt and outer RETF in the original startup."""
import argparse
import copy
import json
from pathlib import Path
import tempfile

from capture_device_start_prefix import capture as capture_prefix
from verify_device_start_prefix import verify as verify_prefix, producer_paths, digest
from capture_pit_irq_frames import transition, source_constants, shared_physical
from memory_context import observer, legacy_view, portable, validate_context

EVENTS = ('before-software','after-software','before-software-ret',
          'after-software-ret','software-caller')
PREFIX_BOUNDARIES = {'77e2','77e9','1280','12ab','77ee','23c4','133a','77ff',
                     '7809','3322','780e','6032','603f','5cc2','5cdd'}


def read_lines(path):
    return [json.loads(line) for line in path.read_text().splitlines()]


def properties(prefix,events,fetches):
    def row(q):
        q=copy.deepcopy(q)
        if 'memory_context' in q:q['memory_context']=portable(q['memory_context'])
        return q
    return dict(scope='Original380 complete startup fetch states, actual first INT21/outer32-bit RETF pairs and158 complete handler/caller instruction observations. Every CPU/system/clock/code field and complete portable provider/cache/VGA state remains in this property reference. Raw per-run boot RAM is retained and compared within each API pair, never normalized. Handler instructions are observed, not completely interpreted; no application transport or full original parity acceptance.',
                prefix=[row(q) for q in prefix],events=[row(q) for q in events],fetches=[row(q) for q in fetches])


def verify(repo,root,reference=True):
    for p,h in json.loads((root/'producers.json').read_text()).items():assert digest(p)==h,p
    with legacy_view(root,'device',repo) as view:
        folder=view/'source'
        kept={'77e2','77e9','1280','12ab','77ee','23c4','133a','77ff','7809'}
        for p in list(folder.iterdir()):
            if p.suffix in ('.memory','.json') and p.stem not in kept:p.unlink()
        rows=read_lines(folder/'prefix-fetches.jsonl')
        (folder/'prefix-fetches.jsonl').write_text(''.join(json.dumps(q)+'\n' for q in rows[:242]))
        legacy=verify_prefix(view,repo)
    source=root/'source'
    prefix=read_lines(source/'prefix-fetches.jsonl')
    events=read_lines(source/'software-events.jsonl')
    fetches=read_lines(source/'software-fetches.jsonl')
    assert len(prefix)==380 and prefix[-1]['cpu_regs.ip.dword[0]']==0x5cdd
    assert tuple(q['kind'] for q in events)==EVENTS and [q['serial'] for q in events]==list(range(1,6))
    assert len(fetches)==158 and fetches[-1]['cpu_regs.ip.dword[0]']==0x5cdf
    assert fetches[-2]['segments'][1]['value']==8 and fetches[-2]['fetched_code_hex'].startswith('66cb')
    contexts=[q for q in prefix+events if 'memory_context' in q]
    assert len(contexts)==20
    assert {p.stem for p in source.glob('*.json')}==PREFIX_BOUNDARIES
    memory_files={q.get('memory_file','%04x.memory'%q['cpu_regs.ip.dword[0]']) for q in contexts}
    assert {p.name for p in source.glob('*.memory')}==memory_files
    vga_files=set()
    for q in contexts:
        memory=q.get('memory_file','%04x.memory'%q['cpu_regs.ip.dword[0]'])
        assert (source/memory).stat().st_size==16777216
        validate_context(q['memory_context'],source,repo)
        vga_files.update(q['memory_context']['vga'][k]['file'] for k in ('linear','fastmem'))
    assert {p.name for p in source.glob('*.memory.*')}==vga_files
    physical=shared_physical(repo);constants,offsets=source_constants(repo)
    transitions=[transition(repo,source,a,b,physical,constants,offsets)
                 for a,b in ((events[0],events[1]),(events[2],events[3]))]
    # Source code observes the API return before the next cycle decrement.
    final=copy.deepcopy(events[3]);final.pop('kind');final.pop('serial')
    caller=copy.deepcopy(events[4]);caller.pop('kind');caller.pop('serial')
    final['CPU_Cycles']-=1
    final.pop('memory_file');caller.pop('memory_file')
    # VGA snapshot filenames are observer metadata. Compare every other byte
    # of their state and whole buffers separately, without field masking.
    for k in ('linear','fastmem'):
        a=final['memory_context']['vga'][k];b=caller['memory_context']['vga'][k]
        assert (source/a['file']).read_bytes()==(source/b['file']).read_bytes()
        a['file']=b['file']
    assert final==caller
    assert (source/events[3]['memory_file']).read_bytes()==(source/events[4]['memory_file']).read_bytes()
    case=properties(prefix,events,fetches)
    if reference:assert case==json.loads((repo/'tools/oracle/software_dos_case.json').read_text()),'complete original software-DOS property reference differs'
    proof=dict(scope=case['scope'],legacy=legacy,transitions=transitions,
               properties=case,memory_sha256={p:digest(source/p) for p in sorted(memory_files)},
               VGA_sha256={p:digest(source/p) for p in sorted(vga_files)},
               producer_manifest_sha256=digest(root/'producers.json'),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: actual software INT21/RETF pairs,380 startup/158 handler fetches,20 complete RAM/provider/cache/VGA boundaries; complete original39frames/27518PCM/end600 unchanged')
    return proof


def capture(repo,root,reference=True):
    assert root.is_relative_to(Path('/tmp'))
    with tempfile.TemporaryDirectory(prefix='wasm-fist-software-probe-') as temp:
        probe=Path(temp)/'software.gdb'
        text=observer(repo,'device')
        assert text.count('\nend\nrun')==1
        probe.write_text(text.replace('\nend\nrun','\n'+(repo/'tools/oracle/software_dos.gdb.inc').read_text()+'\nend\nrun'))
        tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
        extra=[Path(__file__).resolve(),repo/'tools/oracle/software_dos.gdb.inc',
               *[repo/'tools/oracle'/n for n in ('memory_context.py','memory_context.gdb.inc',
                 'capture_pit_irq_frames.py','physical_provider.gdb.inc','paging_control.gdb.inc','capture_physical_provider.py')],
               *[tree/n for n in ('src/cpu/paging.cpp','src/hardware/memory.cpp','src/hardware/vga_memory.cpp',
                                  'include/mem.h','include/paging.h','include/vga.h')],probe]
        capture_prefix(repo,root,stop=0x5cdd,probe=probe,extra_producers=extra)
        retained=root/'observer.gdb';retained.write_bytes(probe.read_bytes())
        manifest=json.loads((root/'producers.json').read_text());manifest.pop(str(probe))
        manifest[str(retained)]=digest(retained)
        (root/'producers.json').write_text(json.dumps(manifest,indent=2)+'\n')
        return verify(repo,root,reference)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    return verify(repo,root) if args.verify_only else capture(repo,root)


if __name__=='__main__':main()
