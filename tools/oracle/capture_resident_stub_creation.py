#!/usr/bin/env python3
"""Observe original resident CALL-stub table creation without altering the game."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
from verify_resident_stub_creation import verify


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args()
    repo=args.repo.resolve(strict=True);root=args.output.resolve()
    if args.verify_only:return verify(root,repo)
    if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True,exist_ok=False)
    probe=Path(__file__).resolve().with_name('resident_stub_creation.gdb')
    binary=repo/'third_party/dosbox-fist'
    files=[Path(__file__).resolve(),probe,Path(__file__).resolve().with_name('verify_resident_stub_creation.py'),binary,
           *[repo/p for p in ('tools/oracle/file_error.gdb','tools/oracle/capture_sequence.sh',
                             'tools/oracle/sequence_format.py','re_out/fist_image.bin',
                             'tools/oracle/sb_irq_frame_case.json',
                             'tools/oracle/sound_vector_init_case.json',
                             'tools/oracle/resident_image.py',
                             'third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h',
                             'third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp',
                             'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h')]]
    originals={str(p.relative_to(repo)):digest(p) for p in (repo/'armoredfist').rglob('*') if p.is_file()}
    (root/'original-hashes.json').write_text(json.dumps(originals,indent=2)+'\n')
    (root/'producers.json').write_text(json.dumps({str(p):digest(p) for p in files},indent=2)+'\n')
    for name in ('baseline','source'):
        folder=root/name;folder.mkdir()
        wrapper=root/('dosbox-'+name)
        command=(['gdb','-q','-batch','-x',str(probe),'--args'] if name=='source' else [])+[str(binary)]
        wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec '+shlex.join(command)+' "$@"\n')
        wrapper.chmod(0o755)
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
        env.update(FIST_SEQUENCE_END_MS='600',DOSBOX=str(wrapper),FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder))
        with (root/(name+'.log')).open('w') as log:
            result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(folder)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode==0,(name,result.returncode)
    return verify(root,repo)


if __name__=='__main__':main()
