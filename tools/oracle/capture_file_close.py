#!/usr/bin/env python3
"""Capture the original configuration-read and overlay-size file lifetimes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'tools/oracle'))
from cpu_trace import records
from sequence_format import validate, validate_endpoint


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(output):
    baseline = output/'baseline/sequence'
    assert (output/'baseline.exit').read_text() == '0\n'
    assert validate_endpoint(baseline, 600) == 600
    assert validate(str(baseline)+'.frames', 'F')['records'] == 39
    assert validate(str(baseline)+'.pcm', 'A')['samples'] == 27518
    image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
    cases = []
    for name, first, last in (('config', 0x086b, 0x0898), ('drivers', 0x5a38, 0x5a55)):
        assert (output/(name+'.exit')).read_text() == '0\n'
        prefix = output/name/'sequence'
        assert validate_endpoint(prefix, 600) == 600
        for suffix in ('frames', 'pcm', 'end'):
            assert Path(str(prefix)+'.'+suffix).read_bytes() == Path(str(baseline)+'.'+suffix).read_bytes()
        trace = output/(name+'.trace')
        complete = list(records(trace))
        selected = [dict(cycle=cycle, cs=cs, ip=ip, registers=registers, segments=segments)
                    for cycle, (cs, ip), registers, segments in complete
                    if cs == 0x2082 and first <= ip <= last]
        assert selected
        calls = []
        for row in selected:
            if row['ip'] == first:
                calls.append([])
            assert calls
            calls[-1].append(row)
        assert len(calls) == 2 and all(call[-1]['ip'] == last for call in calls)
        if name == 'config':
            normal, missing = ({row['ip']: row for row in call} for call in calls)
            assert normal[0x0878]['registers'][0] & 65535 == 5
            assert normal[0x088a]['registers'][0] & 65535 == 10
            assert normal[0x088d]['registers'][0] & 65535 == 5
            assert normal[0x0895]['registers'][3] & 65535 == 5
            assert normal[0x0898]['registers'][0] & 65535 == 0x3e05
            assert missing[0x0878]['registers'][0] & 65535 == 2
            assert all(row['ip'] not in (0x0888, 0x0895) for row in calls[1])
        else:
            for call, size in zip(calls, (0x433c, 0x7c9c)):
                at = {row['ip']: row for row in call}
                assert at[0x5a3e]['registers'][0] & 65535 == 5
                assert at[0x5a51]['registers'][3] & 65535 == 5
                assert at[0x5a55]['registers'][0] & 65535 == size
                assert at[0x5a55]['registers'][2] & 65535 == 0
        cases.append(dict(case=name, trace_sha256=digest(trace), complete_fetches=len(complete),
                          calls=calls, capture_sha256={suffix: digest(str(prefix)+'.'+suffix)
                                                     for suffix in ('frames', 'pcm', 'end')}))
    scripts = (Path(__file__).resolve(), ROOT/'tools/oracle/cpu_trace.py', ROOT/'tools/oracle/capture_sequence.sh')
    proof = dict(scope='Original reached configuration read/close and two overlay size/close WORD '
                 'contracts; source-only evidence. Full GP/segment fetch records diagnose data transport, '
                 'not port CPU/flags/stack/IRQ/time acceptance. PUSHF/POPF read-flag preservation is '
                 'bound to actual code; controlled read-error flags remain a separate source observation.',
                 original_binary_sha256=digest(ROOT/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
                 image_sha256=digest(ROOT/'re_out/fist_dat_image.bin'),
                 code=[dict(image_offset=a, module_cs=0x0f69, module_ip=a-0xf690,
                            bytes=image[a:b].hex()) for a, b in ((0xfefb, 0xff29), (0x150c8, 0x150e6))],
                 frames=39, mixed_samples=27518, endpoint_ms=600, cases=cases,
                 script_sha256={str(path.relative_to(ROOT)): digest(path) for path in scripts},
                 reproduction='python3 -B tools/oracle/capture_file_close.py --output /tmp/wasm-fist-file-close-source')
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: original successful/missing config opens and both driver close/size paths; '
          'all39 frames/27518 mixed samples/end600 unchanged.')
    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    output = args.output.resolve()
    if args.verify_only:
        return verify(output)
    if not output.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    output.mkdir(parents=True, exist_ok=False)
    for name, window in (('baseline', None), ('config', '328:330'), ('drivers', '404:408')):
        env = {name: value for name, value in os.environ.items()
               if not name.startswith('FIST_') and name != 'DOSBOX'}
        env['FIST_SEQUENCE_END_MS'] = '600'
        if window:
            env.update(FIST_CPU_TRACE_WINDOW=window, FIST_CPU_TRACE=str(output/(name+'.trace')))
        with (output/(name+'.log')).open('w') as log:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(output/name)],
                                    cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=60)
        (output/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode == 0, (name, result.returncode)
    return verify(output)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
