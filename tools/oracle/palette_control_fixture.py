"""Shared complete-output comparisons for DAC and renderer source fixtures."""
import hashlib
import subprocess
import sys


def digest(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def build(repo, root, *, original=False, includes=()):
    sys.path.insert(0, str(repo / 'tests'))
    from test_port_io import tool
    tree = repo / 'third_party/dosbox-build/dosbox-0.74-3'
    common = ['-O2', '-ffunction-sections', '-fdata-sections',
              *['-I' + str(p) for p in (root, *includes, repo / 're_out')]]
    if original:
        choices = [('original', ['g++'], ['-std=gnu++11',
            *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
            '-I' + str(tree / 'include'), '-I' + str(tree / 'src/gui'),
            '-I' + str(tree), '-Wl,--gc-sections'], root / 'original.cpp', root / 'original', [])]
    else:
        choices = [
            ('native', ['gcc', '-m32'], ['-DNDEBUG', '-D_FILE_OFFSET_BITS=64', '-Wl,--gc-sections'],
             root / 'portable.c', root / 'native', []),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc')],
             ['-DNDEBUG', '-sNODERAWFS=1', '-sEXIT_RUNTIME=1', '-sALLOW_MEMORY_GROWTH=1'],
             root / 'portable.c', root / 'wasm.js', [tool('node', 'Git/emsdk/node/*/bin/node')])]
    commands = []
    for target, compiler, flags, source, output, runner in choices:
        result = subprocess.run([*compiler, *common, str(source), *flags, '-o', str(output)],
                                capture_output=True, text=True, timeout=120)
        (root / (target + '-build.log')).write_text(result.stdout + result.stderr)
        assert result.returncode == 0, result.stderr
        commands.append((target, [*runner, str(output)]))
    return commands


def run(command, output, *, timeout=180):
    result = subprocess.run([*command, str(output)], capture_output=True, text=True, timeout=timeout)
    output.with_suffix('.log').write_text(result.stdout + result.stderr)
    assert result.returncode == 0, (command, result.returncode, result.stderr)
    assert output.is_file() and output.stat().st_size > 0, ('Missing complete output', output)
    return result


def compare(repo, root, programs, *, expected_log=None):
    commands = build(repo, root, original=True) + build(repo, root)
    reference = root / 'original.raw'
    results = []
    for target, command in commands:
        output = root / (target + '.raw')
        process = run(command, output)
        if expected_log is not None:
            assert process.stderr.strip() == expected_log, process.stderr
        if target != 'original':
            assert output.stat().st_size == reference.stat().st_size, (target, 'Unequal complete lengths')
            with reference.open('rb') as a, output.open('rb') as b:
                offset = 0
                while True:
                    expected, actual = a.read(1048576), b.read(1048576)
                    assert actual == expected, (target, offset + next(
                        i for i, (x, y) in enumerate(zip(expected, actual)) if x != y))
                    if not expected:
                        break
                    offset += len(expected)
        results.append(dict(target=target, programs=programs, bytes=output.stat().st_size,
                            sha256=digest(output)))
        if target != 'original':
            output.unlink()
        print('PASS complete original palette comparison', target, programs, flush=True)
    reference.unlink()
    return results
