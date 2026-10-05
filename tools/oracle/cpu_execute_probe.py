"""Compile verbatim original normal-core branch bodies and DoString for controlled inputs."""
from pathlib import Path
import re
import subprocess


def build(repo, directory, *, extra_opcodes=(), byte_opcodes=(), trace=False, lazy_input=False, segment_input=False):
    tree = repo/'third_party/dosbox-build/dosbox-0.74-3'
    core = tree/'src/cpu/core_normal'
    support = (core/'support.h').read_text().split('#include "helpers.h"', 1)[0]
    normal = (tree/'src/cpu/core_normal.cpp').read_text()
    macros = '\n'.join(re.search(r'^#define '+name+r'\s+.*$', normal, re.M)[0] for name in ('GETIP', 'SAVEIP', 'LOADIP'))
    cpu = (tree/'src/cpu/cpu.cpp').read_text()
    pushes = '\n'.join(re.search(r'void CPU_Push'+str(bits)+r'\(Bitu value\) \{.*?\n\}', cpu, re.S)[0] for bits in (16, 32))
    opcodes = (0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xb8, 0xb9, 0xe1, 0xe2, 0xe8, 0xe9, 0xeb)+tuple(extra_opcodes)
    for width, filename in ((2, 'prefix_none.h'), (4, 'prefix_66.h')):
        text = (core/filename).read_text()
        cases = []
        for op in opcodes:
            match = re.search(r'\bCASE_[WD]\(0x'+format(op, '02x')+r'\).*?(?=\s*CASE_[BWD]\(|\Z)', text, re.S)
            assert match is not None, (filename, hex(op))
            cases.append(re.sub(r'CASE_[WD]\(0x'+format(op, '02x')+r'\)', 'case 0x'+format(op, '02x')+':', match[0], count=1))
        for op in byte_opcodes:
            match = re.search(r'\bCASE_B\(0x'+format(op,'02x')+r'\).*?(?=\s*CASE_[BWD]\(|\Z)',(core/'prefix_none.h').read_text(),re.S)
            assert match is not None
            cases.append(re.sub(r'CASE_B\(0x'+format(op,'02x')+r'\)','case 0x'+format(op,'02x')+':',match[0],count=1))
        near = (core/('prefix_0f.h' if width==2 else 'prefix_66_0f.h')).read_text()
        for op in (0x84,0x85,0x86):
            match = re.search(r'\bCASE_0F_[WD]\(0x'+format(op,'02x')+r'\).*?(?=\s*CASE_0F_[BWD]\(|\Z)',near,re.S)
            assert match is not None
            cases.append(re.sub(r'CASE_0F_[WD]\(0x'+format(op,'02x')+r'\)', 'case 0x1'+format(op,'02x')+':',match[0],count=1))
        (directory/('original_cases_'+str(width)+'.h')).write_text('\n'.join(cases))
    if byte_opcodes:
        assert set(byte_opcodes)=={0x24},byte_opcodes
        helpers=(core/'helpers.h').read_text()
        macro=re.search(r'^#define ALIb\(inst\).*?\n\s*\{[^\n]*\}',helpers,re.M)[0]
        support += '\n#include "'+str(tree/'src/cpu/instructions.h')+'"\n'+macro+'\n'
    (directory/'original_branch_support.h').write_text(macros+'\n'+support+'\n'+pushes+'\n')
    output = directory/'original-execute'
    options=(['-DFIST_EXECUTE_FETCH_TRACE'] if trace else [])+(['-DFIST_EXECUTE_LAZY_INPUT'] if lazy_input else [])
    if segment_input:options.append('-DFIST_EXECUTE_SEGMENT_INPUT')
    result = subprocess.run(['g++', '-O2', '-std=gnu++11', *options,
                    *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                    '-I'+str(tree/'include'), '-I'+str(tree), '-I'+str(directory), '-I'+str(repo/'tools/oracle'),
                    '-ffunction-sections', '-fdata-sections', str(repo/'tools/oracle/cpu_execute_probe.cpp'),
                    '-Wl,--gc-sections', '-o', str(output)], capture_output=True, text=True, timeout=60)
    if result.returncode:raise RuntimeError(result.stderr)
    return str(output)
