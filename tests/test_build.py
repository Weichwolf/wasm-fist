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
args = sys.argv[1:]
if '-c' in args and any(Path(a).name == os.environ.get('FAIL_SOURCE') for a in args):
    print('compiler terminated unsuccessfully', file=sys.stderr)
    sys.exit(23)
if '-c' not in args:
    assert all(Path(a).is_file() for a in args if a.endswith('.o'))
Path(args[args.index('-o') + 1]).write_bytes(b'object-compatible-fixture')
'''


class BuildTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        for folder in ('tools', 'build', 're_out', 'armoredfist', 'compiler'):
            (self.root / folder).mkdir()
        (self.root / 'build/fist.c').write_text('')
        (self.root / 're_out/image.bin').write_bytes(b'image')
        compiler = self.root / 'compiler/emcc'
        compiler.write_text(COMPILER)
        compiler.chmod(0o755)
        (self.root / 'compiler/em++').symlink_to('emcc')
        for script in ('build.sh', 'build_web.sh'):
            shutil.copyfile(ROOT / 'tools' / script, self.root / 'tools' / script)

    def build(self, script, source):
        objects = self.root / (script + source + '.objects')
        objects.mkdir()
        prefix = 'wasm_' if script == 'build.sh' else ''
        (objects / (prefix + Path(source).stem + '.o')).write_bytes(b'previous-object')
        output = objects / 'output.js'
        env = dict(os.environ, EMCC=str(self.root / 'compiler/emcc'),
                   OBJDIR=str(objects), FAIL_SOURCE=source)
        result = subprocess.run(['bash', str(self.root / 'tools' / script), str(output)],
                                env=env, capture_output=True, text=True, timeout=10)
        return result, output

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


if __name__ == '__main__':
    unittest.main()
