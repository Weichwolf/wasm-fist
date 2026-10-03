#!/usr/bin/env python3
"""Observe the actual d99b task-mode pair, then stop at the first 6c poster."""
import argparse
import base64
import gzip
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(output):
    assert (output/'gdb.exit').read_text() == '0\n'
    assert 'Python Exception' not in (output/'probe.log').read_text()
    assert json.loads((output/'counts.json').read_text()) == dict(get=1, put=1, poster=1)
    labels = ('get-before', 'get-after', 'put-before', 'put-after', 'first-6c')
    states = {name: json.loads((output/(name+'.json')).read_text()) for name in labels}
    memory = {name: (output/(name+'.memory')).read_bytes() for name in labels}
    assert all(len(m) == 16777216 for m in memory.values())
    before, after, put, returned = (states[name] for name in labels[:4])
    assert before['mode'] == after['return_byte'] == put['incoming_byte'] == 0
    assert after['mode'] == put['mode'] == returned['return_byte'] == returned['mode'] == 0
    assert memory['get-before'] == memory['get-after'] == memory['put-before']
    case = json.loads((ROOT/'tools/oracle/task_mode_case.json').read_text())
    expected = bytearray(memory['put-before'])
    expected[case['mode_address']] = put['incoming_byte']
    expected[before['task_mode_address']] = put['incoming_byte']
    assert memory['put-after'] == expected
    assert before['pointer_segment'] == 0x9000 and before['pointer_offset'] == 0
    assert before['task_mode_address'] == 0x90496
    assert before['clock'] == after['clock'] and put['clock'] == returned['clock']
    assert all('FUN_0000_d99b' in state['backtrace'] for state in (before, put))
    assert all(m[0x12d59:0x12d5b] == bytes.fromhex('ff78') for m in memory.values())
    producers = json.loads((output/'producers.json').read_text())
    for path, sha in producers.items():
        assert digest(path) == sha, path
    proof = dict(scope='Actual natural d99b BYTE/pointer transport and intentional first-6c stop; '
                 'no port CPU/flags/stack/IRQ/IF/device-time or full output acceptance. '
                 'Leaves retain clock individually; inherited resolver work remains open.',
                 producers=producers, states=states,
                 memory_sha256={name: digest(output/(name+'.memory')) for name in labels})
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: actual d99b getter/setter BYTE, task pointer and all16-MiB writes; first-6c stop.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    output = args.output.resolve()
    if args.verify_only:
        verify(output)
        return
    if args.native is None:
        parser.error('--native is required for capture')
    native = args.native.resolve(strict=True)
    if not output.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    output.mkdir(parents=True, exist_ok=False)
    shutil.copytree(ROOT/'armoredfist', output/'game')
    prefix = output/'start-state'
    for suffix in ('text', 'bda'):
        source = ROOT/('tools/oracle/start_state.'+suffix+'.gz.b64')
        Path(str(prefix)+'.'+suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
    shutil.copyfile(ROOT/'tools/oracle/start_state.vga', Path(str(prefix)+'.vga'))
    probe = ROOT/'tools/task_mode_native.gdb'
    producers = {str(path): digest(path) for path in
                 (native, probe, Path(__file__).resolve(), ROOT/'tools/oracle/task_mode_case.json',
                  *[Path(str(prefix)+'.'+suffix) for suffix in ('text', 'bda', 'vga')])}
    (output/'producers.json').write_text(json.dumps(producers, indent=2)+'\n')
    env = {name: value for name, value in os.environ.items() if not name.startswith('FIST_')}
    env.update(FIST_TASK_MODE_DIAG_DIR=str(output), FIST_DATADIR=str(output/'game'),
               FIST_SB='1', FIST_TEXT_STATE=str(prefix))
    with (output/'probe.log').open('w') as log:
        result = subprocess.run(['gdb', '-q', '-batch', '-x', str(probe), '--args', str(native)],
                                cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=120)
    (output/'gdb.exit').write_text(str(result.returncode)+'\n')
    verify(output)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
