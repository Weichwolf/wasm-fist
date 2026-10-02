import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('provision_game', ROOT / 'tools/provision_game.py')
provision = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(provision)


class ProvisionGameTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.archive = self.root / 'game.zip'
        self.destination = self.root / 'installation'

    def archive_with(self, extras=(), omit=()):
        files = {'armoredfist/FIST.DAT': b'engine', 'armoredfist/FIST.RUN': b'kernel',
                 'armoredfist/FIST.SET': b'settings', 'armoredfist/FISTDATA/MISSION': b'mission'}
        files.update(extras)
        with zipfile.ZipFile(self.archive, 'w') as archive:
            for name, content in files.items():
                if name not in omit:
                    archive.writestr(name, content)
        return patch.object(provision, 'SHA256',
                            hashlib.sha256(self.archive.read_bytes()).hexdigest())

    def test_complete_installation_and_existing_saves_are_preserved(self):
        with self.archive_with({'armoredfist/FISTDATA/EDITOR.MAP': b'editor'}):
            provision.install(self.archive, self.destination)
        self.assertEqual((self.destination / 'FISTDATA/EDITOR.MAP').read_bytes(), b'editor')
        (self.destination / 'FIST.SET').write_bytes(b'custom settings')
        provision.install(self.root / 'missing.zip', self.destination)
        self.assertEqual((self.destination / 'FIST.SET').read_bytes(), b'custom settings')

    def test_checksum_failure_does_not_install_or_cache(self):
        self.archive.write_bytes(b'corrupt archive')
        with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
            provision.install(self.archive, self.destination)
        self.assertFalse(self.destination.exists())
        cache = self.root / 'cache/game.zip'
        with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
            provision.download(self.archive.as_uri(), cache)
        self.assertFalse(cache.exists())

    def test_missing_assets_do_not_leave_partial_installation(self):
        with self.archive_with(omit=('armoredfist/FIST.RUN',)):
            with self.assertRaisesRegex(ValueError, 'missing FIST.RUN'):
                provision.install(self.archive, self.destination)
        self.assertFalse(self.destination.exists())
        self.assertEqual(sorted(p.name for p in self.root.iterdir()), ['game.zip'])

    def test_archive_cannot_write_outside_game_directory(self):
        for name in ('armoredfist/../outside', '/outside', 'other/file'):
            with self.subTest(name=name), self.archive_with({name: b'outside'}):
                with self.assertRaisesRegex(ValueError, 'Unexpected archive member'):
                    provision.install(self.archive, self.destination)
            self.assertFalse(self.destination.exists())
            self.assertFalse((self.root / 'outside').exists())


if __name__ == '__main__':
    unittest.main()
