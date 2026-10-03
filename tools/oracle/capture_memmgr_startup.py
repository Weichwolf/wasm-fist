#!/usr/bin/env python3
"""Recover the original startup allocations and shared task at the first KDV lookup."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(root, repo):
    sys.path.insert(0, str(repo/'tools/oracle'))
    from sequence_format import validate, validate_endpoint
    for name in ('baseline', 'observed'):
        assert (root/(name+'.exit')).read_text() == '0\n'
        folder = root/name
        assert 'Python Exception' not in (folder/'dosbox.log').read_text()
        assert validate_endpoint(folder/'sequence', 600) == 600
        assert validate(str(folder/'sequence.frames'), 'F')['records'] == 39
        assert validate(str(folder/'sequence.pcm'), 'A')['samples'] == 27518
    for suffix in ('frames', 'pcm', 'end'):
        assert (root/'baseline'/('sequence.'+suffix)).read_bytes() == (root/'observed'/('sequence.'+suffix)).read_bytes(), suffix
    folder = root/'observed'
    states = {p.stem: json.loads(p.read_text()) for p in folder.glob('*.json')}
    labels = {'init-entry', 'tcb-clear', 'init-return', 'kdv-entry', 'kdv-free-entry',
              'lookup-entry', 'lookup-return', 'free-return'}
    labels |= {f'alloc-{i:02d}-{phase}' for i in range(1, 9) for phase in ('entry', 'return')}
    assert set(states) == labels
    memory = {name: (folder/(name+'.memory')).read_bytes() for name in states}
    assert all(len(m) == 16777216 for m in memory.values())
    image = (repo/'re_out/fist_image.bin').read_bytes()
    code = image[0x84c0:0x85a4]
    slots = [struct.unpack_from('<I', code, i+2)[0] for i in range(len(code)-5) if code[i:i+2] == b'\x8d\x15']
    assert len(slots) == 8
    initial = states['init-entry']['memmgr_slots']
    assert initial['0x2f50'] == initial['0x2f54'] == initial['0xbc98'] == 0
    assert not states['init-entry']['blocks']
    allocations = []
    for i, slot in enumerate(slots, 1):
        a, b = states[f'alloc-{i:02d}-entry'], states[f'alloc-{i:02d}-return']
        regs = a['registers']
        assert regs[0] & 255 == 1 and regs[2] == slot
        assert a['memmgr_slots']['0x2f54'] == i-1 and b['memmgr_slots']['0x2f54'] == i
        assert a['stack_return'] == b['cpu_regs.ip.dword[0]']
        assert [block['slot'] for block in b['blocks']] == slots[:i]
        block = b['blocks'][-1]
        assert block['size'] == regs[1] and block['alignment'] == regs[3] and block['flags'] == (regs[0] & 255)
        assert block['address'] == block['slot_value'] and block['address'] % block['alignment'] == 0
        allocations.append({'entry': f'alloc-{i:02d}-entry', 'return': f'alloc-{i:02d}-return',
                            'slot': slot, 'size': regs[1], 'alignment': regs[3], 'flags': regs[0] & 255})
    complete = states['alloc-08-return']
    assert complete['blocks'][-1]['size'] == 0
    assert complete['memmgr_slots']['0xbc98'] == complete['memmgr_slots']['0x2f50']
    before = states['tcb-clear']
    expected = bytearray(memory['tcb-clear'])
    struct.pack_into('<I', expected, before['tcb_physical']+0x488, 0)
    assert memory['init-return'] == expected
    opened = states['kdv-entry']
    assert opened['memmgr_slots']['0x2f54'] == 9
    assert [block['slot'] for block in opened['blocks'][:7]] == slots[:7]
    assert opened['blocks'][-1]['slot'] == slots[-1] and opened['blocks'][-1]['size'] == 0
    for label in ('kdv-entry', 'kdv-free-entry', 'lookup-entry', 'lookup-return', 'free-return'):
        q = states[label]
        assert q['engine_task']['physical'] == q['tcb_physical'], label
    assert states['kdv-free-entry']['registers'][2] == states['lookup-entry']['registers'][2] == slots[-1]
    found = states['lookup-return']
    index = found['registers'][6]  # actual3661 returns the found table index in ESI
    assert found['memmgr_slots']['0x2f60'] == states['lookup-entry']['memmgr_slots']['0x2f60']
    assert found['blocks'][index]['slot'] == slots[-1]
    freed = states['free-return']
    assert freed['blocks'] == found['blocks'][:index]
    assert freed['memmgr_slots']['0xbc98'] == 0
    proof = {
        'scope': 'Source-only actual84c0 startup allocations, zero-sized bc98 checkpoint and first6e95→3322→3661 lookup/free. Complete GP/segments/raw+lazyflags/time/control snapshots and whole16MiB RAM are retained. The tcb-clear→init-return interval has only the TCB+488 zero-store. At first KDV open the engine far task and extender logical task resolve to the same original physical block. Actual3661 returns the found index in ESI, without storing it at2f60. All39 frames/27518 mixed samples/end600ms match a fresh unobserved baseline. No port allocation/GP/flags/stack/CPU/IRQ/device-time or complete original output acceptance.',
        'original_binary_sha256': digest(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
        'image_sha256': digest(repo/'re_out/fist_image.bin'),
        'code': [{'image_offset': a, 'bytes': image[a:b].hex()} for a,b in
                 ((0x84c0,0x85a4), (0x6e95,0x6ebd), (0x3322,0x3352), (0x3661,0x36bf), (0x36bf,0x38e8))],
        'allocations': allocations, 'states': states,
        'memory_sha256': {name: digest(folder/(name+'.memory')) for name in states},
        'frames': 39, 'mixed_samples': 27518, 'endpoint_ms': 600,
        'capture_sha256': {s: digest(root/'baseline'/('sequence.'+s)) for s in ('frames','pcm','end')},
        'script_sha256': {str(p.relative_to(repo)): digest(p) for p in
                          (Path(__file__).resolve(), Path(__file__).resolve().with_name('memmgr_startup.gdb'),
                           repo/'tools/oracle/capture_sequence.sh', repo/'tools/oracle/sequence_format.py')},
        'reproduction': 'python3 -B tools/oracle/capture_memmgr_startup.py --output /tmp/wasm-fist-memmgr-startup-source',
        'complete_original_acceptance': False,
    }
    (root/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: eight original startup allocations/checkpoint, real first KDV lookup/free and shared engine/extender task. All39 frames/27518 mixed samples/end600ms unchanged.')
    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    repo, root = args.repo.resolve(strict=True), args.output.resolve()
    if args.verify_only:
        return verify(root, repo)
    if not root.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True, exist_ok=False)
    binary = repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'
    probe = Path(__file__).resolve().with_name('memmgr_startup.gdb')
    for name, observe in [('baseline', False), ('observed', True)]:
        folder = root/name
        folder.mkdir()
        wrapper = root/('dosbox-'+name)
        command = (['gdb', '-q', '-batch', '-x', str(probe), '--args'] if observe else [])+[str(binary)]
        wrapper.write_text('#!/usr/bin/env bash\nexec '+shlex.join(command)+' "$@"\n')
        wrapper.chmod(0o755)
        env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
        env.update(DOSBOX=str(wrapper), FIST_SEQUENCE_END_MS='600',
                   FIST_MEMMGR_SOURCE_DIR=str(folder), FIST_MEMMGR_SOURCE_REPO=str(repo))
        with (root/(name+'.log')).open('w') as log:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(folder)],
                                    cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode == 0, (name, result.returncode)
    return verify(root, repo)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, TypeError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
