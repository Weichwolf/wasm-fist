#!/usr/bin/env python3
"""Observe original DOS loader hardware/renderer palettes without changing output."""
import argparse
import json
import os
from pathlib import Path
import shlex
import subprocess
from palette_control_fixture import digest
from sequence_format import validate, validate_endpoint


def capture(repo, root, baseline=None):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True, exist_ok=False)
    env = {k: v for k, v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
    env.update(FIST_SEQUENCE_END_MS='600', FIST_ORACLE_WALL_SECONDS='90')
    originals = {str(p): digest(p) for p in (repo / 'armoredfist').rglob('*') if p.is_file()}
    assert originals
    (root / 'originals.json').write_text(json.dumps(originals, indent=2) + '\n')
    if baseline is None:
        baseline = root / 'baseline'
        with (root / 'baseline.log').open('w') as output:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(baseline)],
                cwd=repo, env=dict(env, DOSBOX=str(repo / 'third_party/dosbox-fist')),
                stdout=output, stderr=subprocess.STDOUT, timeout=100)
        (root / 'baseline.exit').write_text(str(result.returncode) + '\n')
        assert result.returncode == 0, ('Incomplete baseline', result.returncode)
    probe = repo / 'tools/oracle/dac_startup.gdb'
    wrapper = root / 'dosbox-gdb'
    wrapper.write_text('#!/usr/bin/env bash\nexec ' + shlex.join([
        'gdb', '-q', '-batch', '-x', str(probe), '--args', str(repo / 'third_party/dosbox-fist')]) + ' "$@"\n')
    wrapper.chmod(0o755)
    tree = repo / 'third_party/dosbox-build/dosbox-0.74-3'
    paths = [Path(__file__), probe, repo / 'tools/oracle/dac_state.gdb.inc',
             repo / 'tools/oracle/capture_sequence.sh', repo / 'tools/oracle/file_error.gdb',
             repo / 'tools/oracle/palette_control_fixture.py', repo / 'third_party/dosbox-fist',
             *[tree / p for p in ('src/hardware/vga_dac.cpp', 'src/hardware/vga_attr.cpp',
                 'src/ints/int10_modes.cpp', 'src/gui/render.cpp', 'include/vga.h', 'include/render.h',
                 'src/gui/sdlmain.cpp', 'src/gui/render_scalers.h')]]
    inputs = {str(p): digest(p) for p in paths}
    (root / 'inputs.json').write_text(json.dumps(inputs, indent=2) + '\n')
    env.update(DOSBOX=str(wrapper), FIST_DETAIL_REPO=str(repo), FIST_DETAIL_OPERANDS_DIR=str(root))
    with (root / 'capture.log').open('w') as output:
        result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(root / 'source')],
            cwd=repo, env=env, stdout=output, stderr=subprocess.STDOUT, timeout=100)
    (root / 'capture.exit').write_text(str(result.returncode) + '\n')
    assert result.returncode == 0, ('Incomplete observed run', result.returncode)
    assert 'Python Exception' not in (root / 'source/dosbox.log').read_text()
    rows = [json.loads(q) for q in (root / 'startup.jsonl').read_text().splitlines()]
    assert [q['kind'] for q in rows] == ['before-DOS-Execute-FIST-RUN', 'after-DOS-Execute-FIST-RUN',
                                      'before-DOS-Execute-FIST-DAT', 'after-DOS-Execute-FIST-DAT']
    assert all(q['dac']['mode'] == 9 for q in rows)
    outputs = {}
    for folder in (baseline, root / 'source'):
        assert validate_endpoint(folder / 'sequence', 600) == 600
        assert validate(str(folder / 'sequence.frames'), 'F')['records'] == 39
        assert validate(str(folder / 'sequence.pcm'), 'A')['samples'] == 27518
    for suffix in ('frames', 'pcm', 'end'):
        output = root / 'source' / ('sequence.' + suffix)
        assert output.read_bytes() == (baseline / ('sequence.' + suffix)).read_bytes(), suffix
        outputs[suffix] = digest(output)
    assert {str(p) for p in (repo / 'armoredfist').rglob('*') if p.is_file()} == set(originals)
    for p, h in {**inputs, **originals}.items():
        assert digest(Path(p)) == h, p
    proof = dict(scope='Complete original DOS_Execute(FIST.RUN/FIST.DAT) hardware DAC,renderer palette,attribute and host-surface states. All39 frames/27518 mixedPCM/end600 equal unobserved output. Actual BIOS CPU cost and complete renderer/mixer/runtime parity remain separate.',
                 events=rows, inputs_sha256=inputs, outputs_sha256=outputs,
                 frames=39, samples=27518, endpoint_ms=600, terminal_exit=0,
                 complete_original_acceptance=False)
    (root / 'proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    print('PASS original DOS startup palettes and unchanged complete output', flush=True)
    return proof


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    capture(args.repo.resolve(), args.output.resolve(), args.baseline.resolve() if args.baseline else None)
