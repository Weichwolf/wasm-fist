import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


def words(q):
    result = [*q['registers'], q['cpu_regs.ip.dword[0]']]
    for segment in q['segments']:
        result += [segment['value'], segment['base']]
    result += [q[k] for k in ('cpu_regs.flags', 'lflags.var1.dword[0]',
                             'lflags.var2.dword[0]', 'lflags.res.dword[0]',
                             'lflags.type', 'lflags.prev_type', 'lflags.oldcf',
                             'cpu.code.big', 'cpu.stack.big', 'cpu.stack.mask',
                             'cpu.stack.notmask', 'cpu.pmode', 'cpu.cpl', 'cpu.cr0',
                             'paging.cr3', 'paging.enabled')]
    assert len(result) == 37
    return result


def clock(q):
    return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']


class DeviceResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        patched_unit(cls.directory, 'fist_ext.c')
        wrapper = cls.directory/'reset.c'
        wrapper.write_text('#define fist_clock_charge_cpu_instructions fist_reset_test_charge\n'
                           '#include "fist_ext.c"\n')
        flags = ['-I'+str(ROOT/'re_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion',
                 '-Wno-incompatible-pointer-types', '-Wno-implicit-function-declaration',
                 '-Wno-return-mismatch', '-w']
        sources = [str(ROOT/'tests/device_reset.c'), str(wrapper),
                   *(str(ROOT/'re_out'/name) for name in
                     ('fist_dos.c', 'fist_vga.c', 'fist_pic.c', 'fist_sb.c'))]
        native, wasm = (str(cls.directory/name) for name in ('reset', 'reset.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources,
                        '-Wl,--gc-sections', '-lm', '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2',
                      *flags, *sources, '-sNODERAWFS=1', '-sASSERTIONS=1',
                      '-sEXIT_RUNTIME=1', '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target, run))
        cls.case = json.loads((ROOT/'tools/oracle/device_reset_case.json').read_text())
        tree = ROOT/'third_party/dosbox-build/dosbox-0.74-3'
        cls.flags_probe = str(cls.directory/'cpu-flags-probe')
        subprocess.run(['g++', '-std=gnu++11',
                        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                        '-I'+str(tree/'include'), '-I'+str(tree),
                        '-ffunction-sections', '-fdata-sections',
                        str(ROOT/'tools/oracle/cpu_flags_probe.cpp'),
                        '-Wl,--gc-sections', '-o', cls.flags_probe],
                       check=True, capture_output=True, text=True, timeout=60)

    def check_entry(self, first):
        rows = self.case['fetches'][first:]
        image = (ROOT/'re_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), self.case['producers'][str(ROOT/'re_out/fist_image.bin')])
        memory = bytearray(b'\xa5' * 0x1000000)
        memory[0x100000:0x100000+len(image)] = image
        initial = rows[0]
        stack = initial['segments'][2]['base']+initial['registers'][4]
        memory[stack:stack+4] = struct.pack('<I', self.case['entry_return_ip'])
        source = self.directory/'reset.input'
        source.write_bytes(struct.pack('<37I', *words(initial))+memory)
        expected = ''.join(f"fetch {clock(q)} {q['CPU_Cycles']} {q['CPU_CycleLeft']} "
                           +' '.join(f'{value:08x}' for value in words(q))+'\n' for q in rows)
        expected += f'pumps 0 fetches {len(rows)}\n'
        for target, run in self.commands:
            with self.subTest(target=target, entry=hex(initial['cpu_regs.ip.dword[0]'])):
                output = self.directory/(target+'.memory')
                result = subprocess.run([*run, str(source), str(output),
                                         str(clock(initial)-1), str(initial['cpu_regs.ip.dword[0]'])],
                                        capture_output=True, text=True, timeout=30,
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory/'device.wav')))
                self.assertEqual(result.returncode, 0, (result.stdout+result.stderr)[:1200])
                self.assertEqual(result.stdout, expected)
                actual = output.read_bytes()
                self.assertEqual(len(actual), len(memory))
                differences = [(hex(i), a, b) for i, (a, b) in enumerate(zip(actual, memory)) if a != b]
                self.assertFalse(differences, f'Complete memory differs: {differences[:20]}')

    def test_actual_tail_dispatch_matches_full_original_reset_sequence(self):
        self.check_entry(0)

    def test_actual_reset_entry_matches_full_original_sequence(self):
        self.check_entry(6)

    def test_recovered_lazy_flag_materialization_matches_original_cpu(self):
        inputs = []
        for type_, bits, kind in ((0, 8, 'unknown'), (2, 16, 'add'), (19, 8, 'xor'),
                                 (21, 32, 'xor'), (22, 8, 'cmp'), (23, 16, 'cmp'),
                                 (31, 8, 'test')):
            mask = (1 << bits)-1
            values = (0, 1, 15, mask//2, mask//2+1, mask-1, mask)
            for a in values:
                for b in values:
                    result = a+b if kind=='add' else a-b if kind=='cmp' else a & b if kind=='test' else a ^ b
                    for raw in (0x3206, 0xffffffff, 0x20460202):
                        operands = [((dirty & ~mask) | (value & mask)) & 0xffffffff
                                    for dirty, value in zip((0x89abcdef, 0x76543210, 0x12345678), (a, b, result))]
                        inputs.append([raw, type_, 0x31, 1, *operands])
        script = ''.join(' '.join(f'{v:x}' for v in row)+'\n' for row in inputs)
        expected = subprocess.run([self.flags_probe], input=script, text=True,
                                  check=True, capture_output=True, timeout=30).stdout
        self.assertEqual(len(expected.splitlines()), len(inputs))
        for target, run in self.commands:
            with self.subTest(target=target, cases=len(inputs)):
                result = subprocess.run([*run, 'flags'], input=script, text=True,
                                        capture_output=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)
