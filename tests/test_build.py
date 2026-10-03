import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
COMPILER = '''#!/usr/bin/env python3
import os
from pathlib import Path
import sys
import subprocess
args = sys.argv[1:]
if '-c' in args and any(Path(a).name == os.environ.get('FAIL_SOURCE') for a in args):
    print('compiler terminated unsuccessfully', file=sys.stderr)
    sys.exit(23)
if '-c' in args and os.environ.get('CHECK_CAPTURE_HEADERS'):
    source = next((a for a in args if Path(a).name == 'fist_vga.c'), None)
    if source:
        result = subprocess.run(['/usr/bin/gcc', '-E',
                                 *(a for a in args if a.startswith('-I')), source],
                                stdout=subprocess.DEVNULL)
        if result.returncode:
            sys.exit(result.returncode)
if '-c' not in args:
    assert all(Path(a).is_file() for a in args if a.endswith('.o'))
Path(args[args.index('-o') + 1]).write_bytes(b'object-compatible-fixture')
'''


class BuildTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for folder in ('tools', 're_out', 'armoredfist', 'compiler'):
            (self.root / folder).mkdir()
        self.work = self.root / 'temporary-work'
        (self.work / 'build').mkdir(parents=True)
        (self.work / 'build/fist.c').write_text('')
        (self.root / 're_out/image.bin').write_bytes(b'image')
        compiler = self.root / 'compiler/emcc'
        compiler.write_text(COMPILER)
        compiler.chmod(0o755)
        (self.root / 'compiler/em++').symlink_to('emcc')
        for compiler_name in ('gcc', 'g++'):
            (self.root / 'compiler' / compiler_name).symlink_to('emcc')
        for script in ('build_native.sh', 'build.sh', 'build_web.sh', 'patch.sh', 'work_dir.sh', 'clean.sh'):
            shutil.copyfile(ROOT / 'tools' / script, self.root / 'tools' / script)

    def build(self, script, source, compiler_on_path=False, check_headers=False):
        objects = self.root / (script + source + '.objects')
        objects.mkdir()
        prefix = 'wasm_' if script == 'build.sh' else ''
        (objects / (prefix + Path(source).stem + '.o')).write_bytes(b'previous-object')
        output = objects / 'output.js'
        env = dict(os.environ, EMCC=str(self.root / 'compiler/emcc'),
                   OBJDIR=str(objects), FAIL_SOURCE=source, FIST_WORKDIR=str(self.work))
        if compiler_on_path:
            env.update(EMCC='emcc', PATH=str(self.root / 'compiler') + os.pathsep + env['PATH'])
        if check_headers:
            env['CHECK_CAPTURE_HEADERS'] = '1'
        result = subprocess.run(['bash', str(self.root / 'tools' / script), str(output)],
                                env=env, capture_output=True, text=True, timeout=10)
        return result, output

    def test_external_staging_resolves_the_actual_shared_capture_headers(self):
        shutil.copyfile(ROOT / 're_out/fist_vga.c', self.work / 'build/fist_vga.c')
        for header in (ROOT / 're_out').glob('*.h'):
            shutil.copyfile(header, self.work / 'build' / header.name)
        oracle = self.root / 'tools/oracle'
        oracle.mkdir()
        for name in ('fist_sequence_capture.h', 'fist_sequence_endpoint.h'):
            shutil.copyfile(ROOT / 'tools/oracle' / name, oracle / name)
        for script in ('build_native.sh', 'build.sh', 'build_web.sh'):
            with self.subTest(script=script):
                result, output = self.build(script, 'no-failure.c',
                                            compiler_on_path=True, check_headers=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertTrue(output.exists())

    def test_patch_stages_sources_outside_checkout(self):
        (self.root / 're_out/fist.c').write_text('int source_owner;\n')
        env = dict(os.environ, FIST_WORKDIR=str(self.work))
        result = subprocess.run(['bash', str(self.root / 'tools/patch.sh')],
                                env=env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        self.assertEqual((self.work / 'build/fist.c').read_text(), 'int source_owner;\n')
        self.assertFalse((self.root / 'build').exists())

    def test_clean_removes_only_owned_temporary_artifacts(self):
        for name in ('verify/run.completed', 'sequence-capture/original'):
            directory = self.work / name
            directory.mkdir(parents=True)
            (directory / 'old.log').write_text('obsolete')
        outside = self.root / 'keep.txt'
        outside.write_text('source')
        result = subprocess.run(['bash', str(self.root / 'tools/clean.sh')],
                                env=dict(os.environ, FIST_WORKDIR=str(self.work)),
                                capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
        for name in ('build', 'verify', 'sequence-capture'):
            self.assertFalse((self.work / name).exists())
        self.assertEqual(outside.read_text(), 'source')

    def test_default_work_directory_is_external_and_workspace_specific(self):
        env = os.environ.copy()
        env.pop('FIST_WORKDIR', None)
        directories = []
        for root in (self.root, self.root / 'other-workspace'):
            command = 'ROOT="$1"; source "$2"; printf "%s\n" "$FIST_WORKDIR"'
            result = subprocess.run(['bash', '-c', command, 'paths', str(root),
                                     str(self.root / 'tools/work_dir.sh')],
                                    env=env, capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            directory = Path(result.stdout.strip())
            self.assertTrue(directory.is_relative_to('/tmp'))
            self.assertFalse(directory.is_relative_to(self.root))
            directories.append(directory)
        self.assertNotEqual(*directories)

    def test_compiler_failure_cannot_link_stale_objects(self):
        for script in ('build.sh', 'build_web.sh'):
            for source in ('fist_vga.c', 'fist_opl_dbopl.cpp'):
                with self.subTest(script=script, source=source):
                    result, output = self.build(script, source)
                    self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertFalse(output.exists())

    def test_successful_compilation_links(self):
        for script in ('build.sh', 'build_web.sh'):
            with self.subTest(script=script):
                result, output = self.build(script, 'no-failure.c')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertTrue(output.exists())

    def test_compiler_on_path_compiles_cpp_and_links(self):
        for script in ('build.sh', 'build_web.sh'):
            with self.subTest(script=script):
                result, output = self.build(script, 'no-failure.c', compiler_on_path=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertTrue(output.exists())


if __name__ == '__main__':
    unittest.main()
