#!/usr/bin/env python3
"""Observe the actual keyboard callback exchange, then stop at the first 6c poster."""
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
import struct

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(output):
    assert (output/'gdb.exit').read_text() == '0\n'
    assert 'Python Exception' not in (output/'probe.log').read_text()
    labels = ('before', 'after', 'first-6c')
    states = {name: json.loads((output/(name+'.json')).read_text()) for name in labels}
    memory = {name: (output/(name+'.memory')).read_bytes() for name in labels}
    assert all(len(m) == 16777216 for m in memory.values())
    before, after, end = (states[name] for name in labels)
    case = json.loads((ROOT/'tools/oracle/callback_exchange_case.json').read_text())
    owner, stale = case['callback_image_address'], case['stale_image_address']
    assert before['incoming_offset'] == 0x3e0b
    assert before['incoming_segment'] == case['rebased_module_segment'] == 0x0f69
    assert before['callback'] == [1, 0]
    assert after['return_pointer'] == 1
    assert after['callback'] == end['callback'] == [0x3e0b, 0x0f69]
    expected = bytearray(memory['before'])
    struct.pack_into('<HH', expected, owner, before['incoming_offset'], before['incoming_segment'])
    assert memory['after'] == expected
    assert before['clock'] == after['clock']
    assert before['cf'] == after['cf'] == end['cf'] == 0
    assert all(row['stale_alias'] == case['stale_bytes'] for row in states.values())
    assert all(m[stale:stale+4].hex() == case['stale_bytes'] for m in memory.values())
    assert 'FUN_1000_3446' in before['backtrace']
    producers = json.loads((output/'producers.json').read_text())
    for path, sha in producers.items():
        assert digest(path) == sha, path
    proof = dict(scope='Actual natural3446/46b6 WORD exchange, packed old offset/segment, '
                 'existing carry lane and intentional first-6c stop. No port full CPU/flags/'
                 'stack/IRQ/IF/time, live keyboard dispatch or full output acceptance.',
                 producers=producers, states=states,
                 memory_sha256={name: digest(output/(name+'.memory')) for name in labels})
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: actual3446/46b6 four-byte callback exchange, packed return, unchanged leaf clock; first-6c stop.')


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
    probe = ROOT/'tools/callback_exchange_native.gdb'
    producers = {str(path): digest(path) for path in
                 (native, probe, Path(__file__).resolve(), ROOT/'tools/oracle/callback_exchange_case.json',
                  *[Path(str(prefix)+'.'+suffix) for suffix in ('text', 'bda', 'vga')])}
    (output/'producers.json').write_text(json.dumps(producers, indent=2)+'\n')
    env = {name: value for name, value in os.environ.items() if not name.startswith('FIST_')}
    env.update(FIST_CALLBACK_DIAG_DIR=str(output), FIST_DATADIR=str(output/'game'),
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
