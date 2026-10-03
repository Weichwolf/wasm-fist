#!/usr/bin/env python3
"""Recover the original file-error exit through CRT restart and the main-loop carry decision."""
import argparse
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys

from capture_file_error import digest as sha, verify as verify_body

ROOT = Path(__file__).resolve().parents[2]


def verify(root, repo):
    labels = {'main-service-return', 'main-abort-jump', 'crt-restart-entry', 'crt-restart-jump',
              'main-loop-resume', 'main-loop-poll-entry', 'main-loop-carry-test', 'main-loop-exit'}
    image = (repo/'re_out/fist_dat_image.bin').read_bytes()
    results = []
    for stage in ('find', 'open'):
        case = root/stage
        body_proof = verify_body(case, repo, stage)
        folder = case/'error'
        states = {p.stem: json.loads(p.read_text()) for p in folder.glob('*.json')
                  if p.stem.startswith(('main', 'crt'))}
        assert set(states) == labels
        memory = {label: (folder/(label+'.memory')).read_bytes() for label in labels}
        assert all(len(m) == 16777216 for m in memory.values())
        entry, resumed = states['crt-restart-entry'], states['main-loop-resume']
        code = bytes.fromhex(entry['fetched_code_hex'])
        scratch = struct.unpack_from('<H', code, 1)[0]
        engine_cs = states['main-service-return']['segments'][1]['value']
        crt_cs = entry['segments'][1]['value']
        dgroup = entry['segments'][3]['value']
        restart_offset = ((crt_cs-engine_cs) << 4)+0x314
        original = bytearray(image[restart_offset:restart_offset+len(code)])
        assert original[0] == 0xbb and original[3:6] == bytes.fromhex('b80004')
        assert original[23] == 0xea and struct.unpack_from('<HH', original, 24) == (0xe0, 0)
        struct.pack_into('<H', original, 1, struct.unpack_from('<H', original, 1)[0]+engine_cs)
        struct.pack_into('<H', original, 26, engine_cs)
        assert code == original
        assert dgroup-engine_cs == 0x1c00 and crt_cs-engine_cs == 0xf69
        regs = entry['registers'].copy()
        stack_size = struct.unpack_from('<H', code, 4)[0]
        stack_base = (scratch << 4)+stack_size
        expected = bytearray(memory['crt-restart-entry'])
        # CRT324 PUSH SS/POP DS overwrites the earlier PUSH CS word.
        struct.pack_into('<H', expected, stack_base-2, dgroup)
        struct.pack_into('<H', expected, stack_base-4, 0x324)
        expected[(dgroup << 4)+0x6a] = 0xff
        assert memory['main-loop-resume'] == expected
        # f69e preserves the stack's physical address when rebasing SS to DGROUP.
        ax = ((scratch-dgroup) << 4)+(stack_size-4)
        regs[0] = (regs[0] & 0xffff0000) | ax
        regs[3] = (regs[3] & 0xffff0000) | dgroup
        regs[4] = (regs[4] & 0xffff0000) | (ax+4)
        assert resumed['registers'] == regs
        assert resumed['segments'][2]['value'] == resumed['segments'][3]['value'] == dgroup
        assert resumed['cpu_regs.ip.dword[0]'] == 0xe0
        for label in ('main-loop-carry-test', 'main-loop-exit'):
            assert states[label]['cpu_regs.flags'] & 1 and states[label]['lflags.type'] == 0
        assert states['main-loop-carry-test']['registers'] == states['main-loop-exit']['registers']
        assert memory['main-loop-carry-test'] == memory['main-loop-exit']
        assert bytes.fromhex(states['main-loop-carry-test']['fetched_code_hex'])[:2] == bytes.fromhex('73f1')
        trace = [json.loads(line) for line in (folder/'caller-fetches.jsonl').read_text().splitlines()]
        assert (crt_cs, 0x65f8) in [(q['segments'][1]['value'], q['cpu_regs.ip.dword[0]']) for q in trace]
        assert trace[-1]['cpu_regs.ip.dword[0]'] == 0xe7
        assert not any(q['segments'][1]['value'] == engine_cs and
                       q['cpu_regs.ip.dword[0]'] in (0x6dfd, 0xe5a1) for q in trace)
        results.append(dict(stage=stage, error_body=body_proof, states=states,
                            memory_sha256={label: sha(folder/(label+'.memory')) for label in labels},
                            fetches_sha256=sha(folder/'caller-fetches.jsonl'), fetches=len(trace),
                            CRT_to_main_whole_RAM_exact=True, restart_marker_physical=(dgroup << 4)+0x6a,
                            stack_physical_top=stack_base,
                            main_loop_branch='CF1: JAE not taken; original exits at00e7',
                            frames=38, mixed_samples=27518, endpoint_ms=600,
                            capture_sha256=body_proof['capture_sha256']))
    proof = dict(scope='Source-only original find/open file-error continuation after0f5d through actuale339 '
                       'abort jump, relocated CRT314/f69e stack rebase and00e0/5c5f poll. Eight full '
                       'GP/segments/raw+lazyflags/control/time and whole16MiB continuation boundaries per '
                       'failure. CRT-to-main changes exactly two stack WORDs plus BYTE[DGROUP:6a]=ff; '
                       'registers/SP derived from runtime-relocated instructions, preserving upper halves. '
                       'Actual poll STC supplies CF1, JAE skips00d8 and reaches00e7. No ordinary6032 '
                       'return. Both complete38-frame/27518-mixed-sample/end600 pairs remain unchanged. '
                       'No port/error-inventory/CPU/device-time or complete output acceptance.',
                 cases=results,
                 original_binary_sha256=sha(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
                 engine_image_sha256=sha(repo/'re_out/fist_dat_image.bin'),
                 code=[dict(image_offset=a, bytes=image[a:b].hex()) for a,b in
                       ((0xf9a4, 0xf9c0), (0xf69e, 0xf6ba), (0xd0, 0xe8),
                        (0x15c5f, 0x15c8a), (0xe339, 0xe36c))],
                 script_sha256={str(p.relative_to(repo)): sha(p) for p in
                                (Path(__file__).resolve(), Path(__file__).resolve().with_name('file_error_main.gdb'),
                                 repo/'tools/oracle/capture_file_error.py', repo/'tools/oracle/file_error.gdb',
                                 repo/'tools/oracle/capture_sequence.sh', repo/'tools/oracle/sequence_format.py')},
                 reproduction='python3 -B tools/oracle/capture_file_error_main.py --output /tmp/wasm-fist-file-error-main-source',
                 complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: both original errors abandon the nested loader, use relocated CRT stack/restart marker, '
          'and exit the original main loop via actual poll CF1. Whole RAM CRT stores and complete '
          '38-frame/27518-PCM/end600-ms pairs checked.')
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
    probe = Path(__file__).resolve().with_name('file_error_main.gdb')
    for stage in ('find', 'open'):
        case = root/stage
        case.mkdir()
        for name, observe in [('baseline', False), ('error', True)]:
            folder = case/name
            folder.mkdir()
            wrapper = case/('dosbox-'+name)
            command = (['gdb', '-q', '-batch', '-x', str(probe), '--args']
                       if observe or stage == 'open' else [])+[str(binary)]
            removal = ('' if stage == 'open' else
                       'mv '+shlex.quote(str(folder/'game/FISTDATA/HIGH.DTL'))+' '+
                       shlex.quote(str(folder/'removed.HIGH.DTL'))+'\n')
            wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\n'+removal+
                               'exec '+shlex.join(command)+' "$@"\n')
            wrapper.chmod(0o755)
            env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
            env.update(FIST_SEQUENCE_END_MS='600', DOSBOX=str(wrapper),
                       FIST_DETAIL_OPERANDS_DIR=str(folder), FIST_DETAIL_REPO=str(repo),
                       FIST_FILE_ERROR_STAGE=stage, FIST_FILE_ERROR_OBSERVE=str(int(observe)))
            with (case/(name+'.log')).open('w') as log:
                result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(folder)],
                                        cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
            (case/(name+'.exit')).write_text(str(result.returncode)+'\n')
            assert result.returncode == 0, (stage, name, result.returncode)
    return verify(root, repo)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, TypeError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
