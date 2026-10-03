#!/usr/bin/env python3
"""Observe actual startup allocation and KDV task ownership on both production ports."""
import argparse
import base64
import gzip
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(output, repo):
    from oracle.sequence_format import validate, validate_endpoint
    producers = json.loads((output/'producers.json').read_text())
    for path, sha in producers.items():
        assert digest(path) == sha, path
    source = json.loads((repo/'tools/oracle/memmgr_startup_case.json').read_text())
    assert source['image_sha256'] == digest(repo/'re_out/fist_image.bin')
    cases = []
    for target in ('native', 'wasm'):
        folder = output/target
        assert (folder/'exit').read_text() == '0\n'
        prefix = folder/'capture/sequence'
        assert validate_endpoint(prefix, 600) == 600
        frames = validate(str(prefix)+'.frames', 'F')
        assert frames['records'] == 39
        module = 0x100000
        if target == 'native':
            assert 'Python Exception' not in (folder/'capture/port.log').read_text()
            initial = json.loads((folder/'constructor.json').read_text())
            entry = json.loads((folder/'kdv-entry.json').read_text())
            final = json.loads((folder/'final.json').read_text())
            assert initial['module_offset'] == entry['module_offset'] == final['module_offset'] == module
            errors = [json.loads(line) for line in (folder/'errors.jsonl').read_text().splitlines()] if (folder/'errors.jsonl').exists() else []
            guest = (folder/'final.memory').read_bytes()
            task, host, status = final['current_TCB'], final['g_mem_host'], final['task_status']
        else:
            assert (folder/'anchor.exit').read_text() == '0\n'
            anchor = (folder/'anchor.memory').read_bytes()
            anchor_heap = (folder/'anchor-linear.memory').read_bytes()
            assert len(anchor) == 16777216
            host = anchor_heap.find(anchor)
            assert host >= 0 and anchor_heap.find(anchor, host+1) < 0, 'guest dump is not a unique exact live-heap interval'
            linear = (folder/'linear.memory').read_bytes()
            # g_mem is a static linked array. Derive its address from the same immutable
            # WASM producer's exact tick120 guest/linear pair, then read it at end600ms.
            guest = linear[host:host+16777216]
            task = struct.unpack_from('<I', guest, module+0xc93)[0]
            status = struct.unpack_from('<H', linear, task)[0]
            initial, entry, errors = None, None, None
        assert len(guest) == 16777216
        off, seg = struct.unpack_from('<HH', guest, 0x2aa2c)
        reason = struct.unpack_from('<I', guest, module+0xd82)[0]
        row = dict(target=target, constructor=initial, kdv_entry=entry, actual_native_errors=errors,
                   g_mem_host=host, current_TCB=task, actual_task_status=status,
                   engine_task={'segment':seg,'offset':off,'address':host+(seg<<4)+off},
                   shared_task=task == host+(seg<<4)+off, actual_reason=reason,
                   frames=frames, endpoint_ms=600, guest_sha256=hashlib.sha256(guest).hexdigest(),
                   frame_sha256=digest(str(prefix)+'.frames'), end_sha256=digest(str(prefix)+'.end'))
        if target == 'wasm':
            row['linear_sha256'] = digest(folder/'linear.memory')
            row['address_anchor'] = dict(tick=struct.unpack_from('<H',anchor,0x1c452)[0],
                                        guest_sha256=digest(folder/'anchor.memory'),
                                        linear_sha256=digest(folder/'anchor-linear.memory'))
            assert row['address_anchor']['tick'] == 120
        cases.append(row)
    for suffix in ('frames', 'end'):
        assert (output/'native/capture'/('sequence.'+suffix)).read_bytes() == (output/'wasm/capture'/('sequence.'+suffix)).read_bytes(), suffix
    proof = dict(scope='Actual Native constructor/first KDV snapshots and real0f64 callback observations; both complete600-ms frame/end streams and final16MiB guest dumps. WASM additionally observes the unmodified live linear heap at exit to resolve the actual task, including historical private storage. Source startup allocation/task ownership is a reference, not an assumed result. No original GP/flags/time or final mixed PCM/full surface acceptance.',
                 source_receipt_sha256=digest(repo/'tools/oracle/memmgr_startup_case.json'),
                 producers=producers, cases=cases, complete_original_acceptance=False)
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: complete cross-target600-ms frame/end observation and actual constructor/task diagnostics.')
    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--native', type=Path)
    parser.add_argument('--wasm', type=Path)
    parser.add_argument('--node', default='node')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    repo, output = args.repo.resolve(strict=True), args.output.resolve()
    if args.verify_only:
        return verify(output, repo)
    if args.native is None or args.wasm is None:
        parser.error('--native and --wasm are required')
    if not output.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    native, wasm = args.native.resolve(strict=True), args.wasm.resolve(strict=True)
    node = Path(shutil.which(args.node) or args.node).resolve(strict=True)
    output.mkdir(parents=True, exist_ok=False)
    prefix = output/'start-state'
    for suffix in ('text', 'bda'):
        Path(str(prefix)+'.'+suffix).write_bytes(gzip.decompress(base64.b64decode((repo/('tools/oracle/start_state.'+suffix+'.gz.b64')).read_bytes())))
    shutil.copyfile(repo/'tools/oracle/start_state.vga', Path(str(prefix)+'.vga'))
    gdb = Path(__file__).resolve().with_name('memmgr_startup_native.gdb')
    heap = Path(__file__).resolve().with_name('wasm_heap_dump.cjs')
    wrappers = {'native': output/'native-wrapper', 'wasm': output/'node-wrapper'}
    wrappers['native'].write_text('#!/usr/bin/env bash\nexec '+shlex.join(['gdb','-q','-batch','-x',str(gdb),'--args',str(native)])+' "$@"\n')
    wrappers['wasm'].write_text('#!/usr/bin/env bash\nexec '+shlex.join([str(node),str(heap)])+' "$@"\n')
    for wrapper in wrappers.values():
        wrapper.chmod(0o755)
    files = (native, wasm, wasm.with_suffix('.wasm'), node, gdb, heap, Path(__file__).resolve(),
             repo/'tools/capture_port_sequence.sh', repo/'tools/oracle/memmgr_startup_case.json',
             *wrappers.values(), *(Path(str(prefix)+'.'+s) for s in ('text','bda','vga')))
    (output/'producers.json').write_text(json.dumps({str(p): digest(p) for p in files}, indent=2)+'\n')
    for target in ('native', 'wasm'):
        folder = output/target
        folder.mkdir()
        env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k not in ('NATIVE','NODE','OUTJS')}
        env.update(NATIVE=str(wrappers['native']), NODE=str(wrappers['wasm']), OUTJS=str(wasm),
                   FIST_SB='1', FIST_TEXT_STATE=str(prefix), FIST_SEQUENCE_END_MS='600',
                   FIST_MEMMGR_PORT_DIR=str(folder),
                   FIST_LINEAR_HEAP=str(folder/'linear.memory'))
        if target == 'wasm':
            anchor_env = dict(env, FIST_MEMDUMP=str(folder/'anchor.memory'),
                              FIST_LINEAR_HEAP=str(folder/'anchor-linear.memory'))
            anchor_env.pop('FIST_SEQUENCE_END_MS')
            with (folder/'anchor.log').open('w') as log:
                result = subprocess.run(['bash','tools/capture_port_sequence.sh',target,'120',str(folder/'anchor-capture')],
                                        cwd=repo, env=anchor_env, stdout=log, stderr=subprocess.STDOUT, timeout=120)
            (folder/'anchor.exit').write_text(str(result.returncode)+'\n')
            assert result.returncode == 0, (target, 'address anchor', result.returncode)
        with (folder/'capture.log').open('w') as log:
            result = subprocess.run(['bash','tools/capture_port_sequence.sh',target,'0',str(folder/'capture')],
                                    cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=120)
        (folder/'exit').write_text(str(result.returncode)+'\n')
        assert result.returncode == 0, (target, result.returncode)
    return verify(output, repo)


if __name__ == '__main__':
    main()
