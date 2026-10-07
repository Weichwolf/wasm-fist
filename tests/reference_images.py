"""Provide frozen instruction-oracle images outside the rewrite checkout."""
import argparse
import functools
import hashlib
import os
import pathlib
import runpy
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
REFERENCE = '349ad31a9fd21b350d435651bb2e90afda40cf60'
NAMES = frozenset(('fist_dat_image.bin', 'fist_mga_image.bin', 'fist_image.bin'))


def frozen_file(name):
    return subprocess.check_output(['git', 'show', f'{REFERENCE}:{name}'], cwd=ROOT)


def atomic_write(path, data):
    with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.reference-', delete=False) as file:
        temporary = pathlib.Path(file.name)
        try:
            file.write(data)
            file.close()
            temporary.replace(path)
        finally:
            temporary.unlink(missing_ok=True)


@functools.lru_cache(maxsize=3)
def load_image(name, expected_sha256):
    if name not in NAMES:
        raise ValueError('Unknown frozen instruction image')
    cache = pathlib.Path(os.environ.get('FIST_REFERENCE_ROOT', '/tmp/wasm-fist-reference-images')).resolve()
    if not cache.is_relative_to(pathlib.Path('/tmp')) or cache == pathlib.Path('/tmp'):
        raise ValueError('Instruction reference cache must use a dedicated /tmp directory')
    cache.mkdir(parents=True, exist_ok=True)
    destination = cache / name
    if destination.exists():
        data = destination.read_bytes()
    elif name != 'fist_image.bin':
        data = frozen_file(f're_out/{name}')
        if hashlib.sha256(data).hexdigest() != expected_sha256:
            raise RuntimeError(f'Frozen {name} does not match its original pin')
        atomic_write(destination, data)
    else:
        extractor = cache / 'extract_image.py'
        # The kernel image was ignored in the reference tree. Reuse its exact
        # frozen extractor, rather than duplicating the original decryption.
        atomic_write(extractor, frozen_file('tools/extract_image.py'))
        module = runpy.run_path(str(extractor))
        source = (ROOT / 'armoredfist/FIST.RUN').read_bytes()
        if len(source) != module['DATA_BASE'] + module['IMG_SIZE']:
            raise ValueError('Provisioned FIST.RUN has an unexpected complete size')
        data = bytearray(source[module['DATA_BASE']:])
        computed, stored = module['verify_checksum'](data)
        if computed != stored and stored != 0x1234dead:
            raise ValueError('Provisioned FIST.RUN encrypted checksum differs')
        module['decrypt'](data)
        data = bytes(data)
        if hashlib.sha256(data).hexdigest() != expected_sha256:
            raise RuntimeError('Extracted original kernel does not match its pin')
        atomic_write(destination, data)
    if hashlib.sha256(data).hexdigest() != expected_sha256:
        raise RuntimeError(f'Cached {name} does not match its original pin')
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    from original_asset_oracle import IMAGE_SHA256 as kernel
    from original_unit_oracle import IMAGE_SHA256 as engine
    from original_vehicle_oracle import MGA_SHA256 as mga
    for name, digest in (('fist_dat_image.bin', engine), ('fist_mga_image.bin', mga), ('fist_image.bin', kernel)):
        data = load_image(name, digest)
        print(f'{name}: {len(data)} bytes, SHA256 {digest}')


if __name__ == '__main__':
    main()
