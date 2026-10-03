#!/usr/bin/env python3
"""Reach the real startup registrar and poster on both complete port builds."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

from oracle.sequence_format import validate, validate_endpoint

ROOT = Path(__file__).resolve().parents[1]


def word(data, offset):
    return struct.unpack_from('<H', data, offset)[0]


def check(output):
    output.mkdir(parents=True, exist_ok=False)
    case = json.loads((ROOT/'tools/oracle/resource_bx_case.json').read_text())
    image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
    assert hashlib.sha256(image).hexdigest() == case['image_sha256']
    for code in case['code']:
        raw = bytes.fromhex(code['bytes'])
        assert image[code['offset']:code['offset']+len(raw)] == raw
    original = (ROOT/'armoredfist/FISTDATA/MSPRITE0.BIN').read_bytes()
    assert hashlib.sha256(original).hexdigest() == case['resource_sha256']
    targets = [('native', [os.environ.get('NATIVE', '/tmp/fist_native')]),
               ('wasm', [os.environ.get('NODE', shutil.which('node') or 'node'),
                         os.environ.get('OUTJS', '/tmp/fisttest/fistrun.js')])]
    results, failures = [], []
    for mode in ('normal', 'dimensions', 'empty'):
        resource = bytearray(original)
        if mode == 'dimensions':
            # Same 40 reached records; only the final MUL operands change.
            struct.pack_into('<H', resource, case['last_dimensions_offset'], 0x0705)
        elif mode == 'empty':
            # Original zero-length branch preserves its incoming BX.
            struct.pack_into('<H', resource, 4, 0)
        expected_bx = case['incoming_bx']
        si, count = 4, 0
        while word(resource, si):
            length, dimensions = word(resource, si), word(resource, si+2)
            expected_bx = (dimensions & 255) * (dimensions >> 8)
            count += 1
            si += length
        assert count == (0 if mode == 'empty' else case['records'])
        pair = []
        for target, command in targets:
            directory = output/(mode+'-'+target)
            directory.mkdir()
            shutil.copytree(ROOT/'armoredfist', directory/'game')
            (directory/'game/FISTDATA/MSPRITE0.BIN').write_bytes(resource)
            environment = os.environ.copy()
            for name in ('FIST_DUMPTICK', 'FIST_RUNMS', 'FIST_KDV', 'FIST_KDV_DUMPFRAME',
                         'FIST_MOUSE', 'FIST_SIMRUN', 'FIST_TERRAIN', 'FIST_SOUND_REGLOG'):
                environment.pop(name, None)
            environment.update(FIST_DATADIR=str(directory/'game'), FIST_SB='1', FIST_EXTLOG='1',
                               FIST_SEQUENCE=str(directory/'sequence'), FIST_SEQUENCE_END_MS='600')
            (directory/'producer.json').write_text(json.dumps(dict(
                command=command, cwd=str(ROOT),
                hashes={path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
                        for path in command if Path(path).is_file()},
                environment={name: value for name, value in environment.items()
                             if name.startswith('FIST_')}), indent=2)+'\n')
            with (directory/'port.log').open('wb') as log:
                result = subprocess.run(command, cwd=ROOT, env=environment,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=120)
            (directory/'exit').write_text(str(result.returncode)+'\n')
            assert result.returncode == 0, (directory, result.returncode)
            assert validate_endpoint(directory/'sequence', 600) == 600
            frames = validate(str(directory/'sequence.frames'), 'F')
            packets = re.findall(r'^\[ext\] service op 0x6c \(display-list cmd\) '
                                 r'inbox=([0-9a-f]{8}) args ([0-9a-f]{4}/[0-9a-f]{4}/[0-9a-f]{4})$',
                                 (directory/'port.log').read_text(), re.M)
            assert len(packets) == 1, (directory, packets)
            if int(packets[0][0], 16) != expected_bx:
                failures.append(dict(case=mode, target=target, packet=packets[0], expected=hex(expected_bx)))
            pair.append((directory, packets[0]))
            results.append(dict(case=mode, target=target, records=count, bx=expected_bx,
                                packet=packets[0], frames=frames,
                                input_resource_sha256=hashlib.sha256(resource).hexdigest()))
        if pair[0][1] != pair[1][1]:
            failures.append(dict(case=mode, cross_target_packets=[pair[0][1], pair[1][1]]))
        for suffix in ('frames', 'end'):
            assert (pair[0][0]/('sequence.'+suffix)).read_bytes() == (pair[1][0]/('sequence.'+suffix)).read_bytes()
    (output/'proof.json').write_text(json.dumps(dict(scope=case['scope'], cases=results, failures=failures,
                                                   complete_original_acceptance=False), indent=2)+'\n')
    assert not failures, failures
    print('PASS: six complete startup runs; actual normal/dynamic/empty BX and complete cross-target 600-ms frame/end bytes. Mixed PCM and whole-memory equality are outside this regression.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    check(args.output.resolve())
