#!/usr/bin/env python3
"""Observe the original successful detail return without altering the game."""
import argparse,hashlib,json,os,shlex,subprocess
from pathlib import Path
from verify_detail_return_state import verify
def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args()
 repo=args.repo.resolve(strict=True);root=args.output.resolve()
 if args.verify_only:return verify(root,repo)
 if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
 root.mkdir(parents=True,exist_ok=False);probe=Path(__file__).resolve().with_name('detail_return_state.gdb');binary=repo/'third_party/dosbox-fist'
 files=[Path(__file__).resolve(),Path(__file__).resolve().with_name('verify_detail_return_state.py'),probe,binary,*[repo/name for name in ('tools/oracle/file_error.gdb','tools/oracle/detail_return_case.json','tools/oracle/capture_sequence.sh','tools/oracle/sequence_format.py','re_out/fist_image.bin','re_out/fist_dat_image.bin','armoredfist/FISTDATA/HIGH.DTL','third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h')]]
 (root/'producers.json').write_text(json.dumps({str(p):digest(p) for p in files},indent=2)+'\n')
 for name in ('baseline','return'):
  folder=root/name;folder.mkdir();wrapper=root/('dosbox-'+name)
  command=(['gdb','-q','-batch','-x',str(probe),'--args'] if name=='return' else [])+[str(binary)]
  wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec '+shlex.join(command)+' "$@"\n');wrapper.chmod(0o755)
  env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
  env.update(FIST_SEQUENCE_END_MS='600',DOSBOX=str(wrapper),FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder))
  with (root/(name+'.log')).open('w') as log:r=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(folder)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90)
  (root/(name+'.exit')).write_text(str(r.returncode)+'\n');assert r.returncode==0,(name,r.returncode)
 for path,sha in json.loads((root/'producers.json').read_text()).items():assert digest(path)==sha,path
 return verify(root,repo)
if __name__=='__main__':main()
