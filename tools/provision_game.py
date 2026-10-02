#!/usr/bin/env python3
"""Install the pinned original game archive without replacing an existing installation."""

import argparse
import hashlib
from pathlib import Path, PurePosixPath
import shutil
import tempfile
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
URL = ('https://d1.xp.myabandonware.com/t/07386d7a-4319-4bcf-9eef-dd85b0f3d41b/'
       'Armored-Fist_DOS_EN.zip')
SHA256 = 'a8d8fcb64cc525ddb1562cccfef4cbe94cc3c8d3fcb380d638ce59c28527ea8c'
CACHE = ROOT / 'scratch/provision-game/Armored-Fist_DOS_EN.zip'


def verify_archive(path):
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != SHA256:
        raise ValueError(f'Archive SHA-256 mismatch: {digest}; expected {SHA256}')


def download(url, cache):
    if cache.exists():
        verify_archive(cache)
        return cache
    cache.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.download-', dir=cache.parent) as temp:
        archive = Path(temp) / 'game.zip'
        with urllib.request.urlopen(url, timeout=60) as source, archive.open('wb') as output:
            shutil.copyfileobj(source, output)
        verify_archive(archive)
        archive.replace(cache)
    return cache


def install(archive, destination):
    # Check before downloading/extracting; never modify an existing game or save state.
    if destination.exists() or destination.is_symlink():
        if not destination.is_dir():
            raise ValueError(f'Destination exists and is not a directory: {destination}')
        print(f'Existing installation left unchanged: {destination}')
        return
    verify_archive(archive)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive) as source:
        for entry in source.infolist():
            path = PurePosixPath(entry.filename)
            if (not path.parts or path.parts[0] != 'armoredfist' or
                    '..' in path.parts or '\\' in entry.filename or
                    (entry.external_attr >> 16) & 0o170000 == 0o120000):
                raise ValueError(f'Unexpected archive member: {entry.filename}')
        with tempfile.TemporaryDirectory(prefix='.armoredfist-', dir=destination.parent) as temp:
            source.extractall(temp)
            game = Path(temp) / 'armoredfist'
            for name in ('FIST.DAT', 'FIST.RUN', 'FIST.SET', 'FISTDATA'):
                if not (game / name).exists():
                    raise ValueError(f'Archive is missing {name}')
            # A concurrently created destination must not be replaced.
            if destination.exists() or destination.is_symlink():
                raise ValueError(f'Destination appeared during extraction: {destination}')
            game.rename(destination)
    print(f'Installed original game: {destination}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, help='use a local archive instead of downloading')
    parser.add_argument('--url', default=URL, help='download URL for the pinned archive')
    parser.add_argument('--destination', type=Path, default=ROOT / 'armoredfist')
    args = parser.parse_args()
    try:
        if args.destination.exists() or args.destination.is_symlink():
            install(None, args.destination)
        else:
            install(args.archive if args.archive is not None else download(args.url, CACHE),
                    args.destination)
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        parser.exit(1, f'Provisioning failed: {error}\n')


if __name__ == '__main__':
    main()
