#!/usr/bin/env python3
"""Capture original startup through CALL3322 and its checkpoint-free return780e."""
import argparse
from pathlib import Path
import shlex,subprocess
from capture_device_start_prefix import capture
from check_device_checkpoint_transitions import check
from verify_device_checkpoint import additional_producers


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    if not args.verify_only:
        capture(repo,root,stop=0x780e,extra_producers=additional_producers(repo))
        tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
        compiler=shlex.split(subprocess.check_output(['sdl-config','--cflags'],text=True))
        subprocess.run(['g++','-std=gnu++11',*compiler,'-I'+str(tree/'include'),'-I'+str(tree),
                        '-ffunction-sections','-fdata-sections',str(repo/'tools/oracle/cpu_instructions_probe.cpp'),
                        '-Wl,--gc-sections','-o',str(root/'instructions-probe')],check=True,capture_output=True,text=True)
    return check(root,repo)


if __name__=='__main__':main()
