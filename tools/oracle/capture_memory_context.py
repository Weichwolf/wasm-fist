#!/usr/bin/env python3
"""Capture actual complete memory inputs for original IRQ and device consumers."""
import argparse
import json
from pathlib import Path
import tempfile

from capture_pit_events import digest, lines
from memory_context import legacy_view, observer, portable, validate_context


def verify(repo, root, kind='irq', baseline=None, reference=True, bootstrap=True):
    for path,h in json.loads((root/'producers.json').read_text()).items():
        assert digest(path) == h, path
    with legacy_view(root,kind,repo) as view:
        if kind == 'irq':
            from capture_pit_irq_frames import verify as shared
            base = shared(repo,view,reference=reference,bootstrap=bootstrap)
        elif kind == 'device':
            from verify_device_start_prefix import verify as shared
            base = shared(view,repo)
        else:
            from capture_device_config_cpu import verify as shared
            with legacy_view(baseline,'device',repo) as baseline_view:
                # The baseline's verified legacy proof has exactly the old architectural rows.
                (baseline_view/'proof.json').write_text(json.dumps(json.loads((baseline/'proof.json').read_text())['legacy'])+'\n')
                base = shared(view,repo,baseline_view)
    names = {'irq':('irq-events.jsonl','system-events.jsonl','boot-fetches.jsonl'),
             'device':('prefix-fetches.jsonl',), 'config':('reset-fetches.jsonl',)}[kind]
    rows = {name:lines(root/'source'/name) for name in names}
    contexts = [q for group in rows.values() for q in group if 'memory_context' in q]
    assert len(contexts) == {'irq':52,'device':9,'config':3}[kind], 'incomplete memory contexts'
    inventory = {}
    states = []
    for q in contexts:
        c = validate_context(q['memory_context'],root/'source',repo)
        states.append(dict(memory_file=q.get('memory_file','%04x.memory'%q['cpu_regs.ip.dword[0]']),state=c))
        for key in ('linear','fastmem'):
            name = c['vga'][key]['file']; inventory[name] = digest(root/'source'/name)
    assert set(inventory) == {p.name for p in (root/'source').glob('*.memory.*')}, 'incomplete VGA inventory'
    proof = dict(scope='Actual complete CPU/system/RAM boundaries with all physical slots, first-MB/A20, linked cache and whole VGA storage. Legacy complete original contracts remain unchanged. Other handlers, fault routes, application integration and complete original parity remain open.',
                 kind=kind,legacy=base,states=states,vga_sha256=inventory,
                 producer_manifest_sha256=digest(root/'producers.json'),
                 complete_original_acceptance=False)
    if kind != 'irq':proof['fetches'] = rows[names[0]]
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: %s %d actual provider/cache/VGA contexts with complete unchanged original contracts'%(kind,len(contexts)))
    return proof


def capture(repo, root, kind='irq', baseline=None):
    assert root.is_relative_to(Path('/tmp'))
    with tempfile.TemporaryDirectory(prefix='wasm-fist-memory-probe-') as temp:
        probe = Path(temp)/'memory-context.gdb'; probe.write_text(observer(repo,kind))
        extra = [Path(__file__).resolve(),repo/'tools/oracle/memory_context.py',repo/'tools/oracle/capture_physical_provider.py',
                 *(repo/'tools/oracle'/name for name in ('memory_context.gdb.inc','physical_provider.gdb.inc','paging_control.gdb.inc')),
                 repo/'tools/oracle'/{'irq':'pit_irq_frame.gdb','device':'device_start_prefix.gdb','config':'device_config_cpu.gdb'}[kind],
                 *(repo/'third_party/dosbox-build/dosbox-0.74-3'/name for name in ('src/cpu/paging.cpp','src/hardware/memory.cpp','src/hardware/vga_memory.cpp','include/mem.h','include/paging.h','include/vga.h'))]
        # Verify once, after retaining the exact probe and binding its final path.
        def deferred(*args, **kwargs):return None
        if kind == 'irq':
            from capture_pit_irq_frames import capture as shared
            shared(repo,root,bootstrap=True,probe=probe,extra_producers=extra,verifier=deferred)
        elif kind == 'device':
            from capture_device_start_prefix import capture as shared
            shared(repo,root,probe=probe,extra_producers=extra)
        else:
            from capture_device_config_cpu import capture as shared
            shared(repo,root,baseline,probe=probe,extra_producers=extra,verifier=deferred)
        # Retain the exact generated observer and its binding after temporary probe cleanup.
        retained = root/'memory-context.gdb'; retained.write_bytes(probe.read_bytes())
        manifest = json.loads((root/'producers.json').read_text());manifest.pop(str(probe),None)
        manifest[str(retained)] = digest(retained)
        (root/'producers.json').write_text(json.dumps(manifest,indent=2)+'\n')
        return verify(repo,root,kind,baseline)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--kind',choices=('irq','device','config'),default='irq')
    parser.add_argument('--baseline',type=Path);parser.add_argument('--verify-only',action='store_true')
    args = parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    baseline=args.baseline.resolve(strict=True) if args.baseline else None
    assert args.kind != 'config' or baseline is not None
    return verify(repo,root,args.kind,baseline) if args.verify_only else capture(repo,root,args.kind,baseline)


if __name__ == '__main__':main()
