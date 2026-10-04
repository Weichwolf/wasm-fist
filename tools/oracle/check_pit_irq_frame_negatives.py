#!/usr/bin/env python3
"""Reject incomplete original IRQ runs and changed TSS/frame/fetch evidence."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from capture_pit_events import digest, lines
from capture_pit_irq_frames import shared_physical, source_constants, verify


def write(path, rows):
    path.write_text(''.join(json.dumps(q)+'\n' for q in rows))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args(); repo = args.repo.resolve(strict=True)
    source = args.source.resolve(strict=True); root = args.output.resolve()
    assert root.is_relative_to(Path('/tmp'))
    assert not root.is_relative_to(source), 'negative output must be outside the source capture'
    root.mkdir(parents=True, exist_ok=False)
    verify(repo, source); cases = []
    for name, expected in [('missing-handler-fetch', 'incomplete IRQ frame/fetch output'),
                           ('missing-observer-completion', 'irq-completion.json'),
                           ('missing-boot-tail-producer', 'shell-tail-copy.json'),
                           ('unfinished-IRQ-return', 'incomplete IRQ observer'),
                           ('changed-TSS-kernel-stack', 'complete CPU transition'),
                           ('missing-interrupt-stack-write', 'complete IRQ RAM transition'),
                           ('coherent-changed-caller-register', 'complete original IRQ reference changed')]:
        with tempfile.TemporaryDirectory(dir=root, prefix=name+'-') as temporary:
            clone = Path(temporary)/'evidence'; shutil.copytree(source, clone)
            folder = clone/'source'; events = lines(folder/'irq-events.jsonl')
            before = next(q for q in events if q['kind'] == 'before-hardware' and q['cpu.pmode'])
            after = next(q for q in events if q['kind'] == 'after-hardware' and q['index'] == before['index'])
            if name == 'missing-handler-fetch':
                rows = lines(folder/'irq-fetches.jsonl'); rows.pop(); write(folder/'irq-fetches.jsonl', rows)
            elif name == 'missing-observer-completion':
                (folder/'irq-completion.json').unlink()
            elif name == 'missing-boot-tail-producer':
                (folder/'shell-tail-copy.json').unlink()
            elif name == 'unfinished-IRQ-return':
                path = folder/'irq-completion.json'; q = json.loads(path.read_text())
                q['active'] = [before['index']]; path.write_text(json.dumps(q)+'\n')
            elif name == 'changed-TSS-kernel-stack':
                path = folder/before['memory_file']; memory = bytearray(path.read_bytes())
                physical = shared_physical(repo); _, offsets = source_constants(repo)
                bits = 32 if before['cpu_tss.is386'] else 16
                field = 'esp0' if bits == 32 else 'sp0'
                address = physical(before['cpu_tss.base']+offsets[bits][field], memory, before)
                value = int.from_bytes(memory[address:address+bits//8], 'little')
                memory[address:address+bits//8] = (value+4).to_bytes(bits//8, 'little'); path.write_bytes(memory)
            elif name == 'missing-interrupt-stack-write':
                path = folder/after['memory_file']; memory = bytearray(path.read_bytes())
                old = (folder/before['memory_file']).read_bytes(); physical = shared_physical(repo)
                address = physical(after['segments'][2]['base']+(after['registers'][4] & after['cpu.stack.mask']), memory, after)
                assert memory[address] != old[address]
                memory[address] = old[address]; path.write_bytes(memory)
            else:
                # All boundary/fetch/header register records change together: semantic transitions
                # still hold, but the immutable complete original observation must reject the claim.
                for q in events:
                    if q['index'] == before['index']: q['registers'][1] ^= 0x10000000
                rows = lines(folder/'irq-fetches.jsonl')
                for q in rows:
                    if q['index'] == before['index']: q['registers'][1] ^= 0x10000000
                write(folder/'irq-events.jsonl', events); write(folder/'irq-fetches.jsonl', rows)
            try:
                verify(repo, clone)
            except (AssertionError, FileNotFoundError) as error:
                assert expected in str(error), (name, str(error))
                cases.append(dict(name=name, rejected=True, reason=str(error)))
            else:
                raise AssertionError('negative accepted: '+name)
    nested = source/'must-not-create-negative-output'
    result = subprocess.run([sys.executable, '-B', str(Path(__file__).resolve()),
                            '--repo', str(repo), '--source', str(source), '--output', str(nested)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 1 and 'negative output must be outside the source capture' in result.stderr
    assert not nested.exists()
    cases.append(dict(name='recursive-negative-destination', rejected=True,
                      reason='negative output must be outside the source capture'))
    proof = dict(scope='Original IRQ verifier negatives; no port CPU/IRQ/time or full-output acceptance.',
                 cases=cases, checker_sha256=digest(Path(__file__)),
                 verifier_sha256=digest(repo/'tools/oracle/capture_pit_irq_frames.py'),
                 source_fixture_sha256=digest(repo/'tools/oracle/pit_irq_frame_case.json'),
                 source_manifest_sha256=digest(source/'producers.json'), complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: all eight original IRQ completion/boot/TSS/frame/coherent-state/destination negatives reject; clones retired')


if __name__ == '__main__': main()
