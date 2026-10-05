#!/usr/bin/env python3
"""Compare actual original JS through the shared original conditional/sign owner."""
import argparse,json,resource
from pathlib import Path
from capture_cpu_jns import contract,programs
from capture_cpu_byte_instructions import digest

def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False)
    limit=resource.getrlimit(resource.RLIMIT_CORE)
    resource.setrlimit(resource.RLIMIT_CORE,(0,limit[1]))
    try:
        recovered=contract(repo)
        (root/'source-contract.json').write_text(json.dumps(recovered,indent=2)+'\n')
        coded=programs(repo,root,recovered,opcode=0x78)
        tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
        paths=[Path(__file__),repo/'tools/oracle/capture_cpu_jns.py',
            repo/'tools/oracle/capture_cpu_byte_instructions.py',
            repo/'tools/oracle/cpu_execute_probe.py',repo/'tools/oracle/cpu_execute_probe.cpp',
            repo/'tools/oracle/rep_probe.cpp',repo/'tests/cpu_execute_controlled.c',repo/'tests/test_cpu_execute.py',
            *sorted((repo/'re_out').glob('*.h')),*tree.rglob('*.h'),
            *[tree/p for p in ('config.h','src/cpu/flags.cpp','src/cpu/cpu.cpp','src/cpu/core_normal.cpp','src/cpu/modrm.cpp')],
            *sorted((root/'programs/source').glob('*.h'))]
        proof=dict(scope='Actual original CASE_W/D JS/TFLG_S uses the shared get_SF and JumpCond16/32_b owner. Complete8448 encoded programs and8 causal results compare both targets. Complete sign-query evidence remains with JNS; actual reached/runtime/frame/audio acceptance is separate.',
            source_contract=recovered,programs=coded,
            inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
        (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
        return proof
    finally:
        resource.setrlimit(resource.RLIMIT_CORE,limit)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();capture(a.repo.resolve(),a.output.resolve())
