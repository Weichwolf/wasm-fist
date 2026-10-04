"""One full CPU/RAM fixture and ordered producer build for configuration and reset tests."""
import json
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from test_port_io import ROOT, patched_unit, tool
from memory_context_fixture import system_words, memory_packet, expected_memory_context
from capture_memory_context import capture

_STORAGE = tempfile.TemporaryDirectory(prefix='wasm-fist-cpu-fixture-')
DIRECTORY = Path(_STORAGE.name)
_commands = None
_original = None


def words(q):
    out = [*q['registers'], q['cpu_regs.ip.dword[0]']]
    for s in q['segments']:out += [s['value'],s['base']]
    out += [q[k] for k in ('cpu_regs.flags','lflags.var1.dword[0]','lflags.var2.dword[0]',
                          'lflags.res.dword[0]','lflags.type','lflags.prev_type','lflags.oldcf',
                          'cpu.code.big','cpu.stack.big','cpu.stack.mask','cpu.stack.notmask',
                          'cpu.pmode','cpu.cpl','cpu.cr0','paging.cr3','paging.enabled')]
    assert len(out) == 37
    return out


def clock(q):
    return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']


def original():
    global _original
    if _original is None:
        default = DIRECTORY/'original'
        controlled = DIRECTORY/'controlled'
        capture(ROOT,default,kind='device')
        capture(ROOT,controlled,kind='config',baseline=default)
        _original = default,controlled
    return _original


def commands(directory=None):
    global _commands
    if directory is not None:return build(directory)
    if _commands is None:
        _commands = build(DIRECTORY)
    return _commands


def build(directory):
    patched_unit(directory,'fist_ext.c')
    wrapper = directory/'device.c'
    wrapper.write_text('#define fist_clock_charge_cpu_instructions fist_cpu_test_charge\n#include "fist_ext.c"\n')
    flags = ['-I'+str(ROOT/'re_out'),'-ffunction-sections','-fdata-sections',
             '-fno-strict-aliasing','-Wno-int-conversion','-Wno-incompatible-pointer-types',
             '-Wno-implicit-function-declaration','-Wno-return-mismatch','-w']
    sources = [str(ROOT/'tests/device_cpu.c'),str(wrapper),
               *(str(ROOT/'re_out'/n) for n in ('fist_dos.c','fist_vga.c','fist_pic.c','fist_sb.c'))]
    native,wasm = str(directory/'device'),str(directory/'device.js')
    runs = []
    for target,build,run in [('native',['gcc','-m32','-O0',*flags,*sources,'-Wl,--gc-sections','-lm','-o',native],[native]),
                             ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),'-O2',*flags,*sources,
                                      '-sNODERAWFS=1','-sASSERTIONS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1','-o',wasm],
                              [tool('node','Git/emsdk/node/*/bin/node'),wasm])]:
        result = subprocess.run(build,capture_output=True,text=True,timeout=120)
        if result.returncode:raise RuntimeError(result.stdout+result.stderr)
        runs.append((target,run))
    return runs


def check(test,rows,before,after,timing=None,runs=None,context_equal=True):
    initial = rows[0]
    memory = Path(before).read_bytes()
    expected_memory = Path(after).read_bytes()
    test.assertEqual(len(memory),0x1000000);test.assertEqual(len(expected_memory),len(memory))
    source = DIRECTORY/'device.input'
    source.write_bytes(struct.pack('<58I',*words(initial),*system_words(initial))+memory+
                       memory_packet(initial,Path(before).parent))
    timing = [(clock(q),q['CPU_Cycles'],q['CPU_CycleLeft']) for q in rows] if timing is None else timing
    test.assertEqual(len(timing),len(rows))
    expected = ''.join(f'fetch {at} {remaining} {left} '+' '.join(f'{v:08x}' for v in words(q)+system_words(q))+'\n'
                       for q,(at,remaining,left) in zip(rows,timing))
    expected += f'pumps 0 fetches {len(rows)}\n'
    for target,run in runs or commands():
        with test.subTest(target=target,entry=hex(initial['cpu_regs.ip.dword[0]']),start=timing[0][0]-1):
            output = DIRECTORY/(target+'.memory')
            context = DIRECTORY/(target+'.context')
            result = subprocess.run([*run,str(source),str(output),str(timing[0][0]-1),str(initial['cpu_regs.ip.dword[0]']),str(context)],
                                    capture_output=True,text=True,timeout=30,
                                    env=dict(os.environ,FIST_AUDIO_WAV=str(DIRECTORY/'device.wav')))
            test.assertEqual(result.returncode,0,(result.stdout+result.stderr)[:1600])
            test.assertEqual(result.stdout,expected)
            actual = output.read_bytes();test.assertEqual(len(actual),len(expected_memory))
            test.assertEqual(actual,expected_memory,'Complete raw16MiB guest memory differs')
            test.assertEqual(hashlib.sha256(context.read_bytes()).digest()==
                             hashlib.sha256(expected_memory_context(rows[-1],Path(after).parent)).digest(),context_equal,
                             'Complete linked cache or VGA storage differs')
