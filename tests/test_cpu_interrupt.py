"""Replay reached original system/IRQ/IRET API boundaries with complete CPU/RAM."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from device_cpu_fixture import words
from test_port_io import ROOT, tool

sys.path.insert(0, str(ROOT/'tools/oracle'))
from capture_pit_irq_frames import SYSTEM_FIELDS, capture, source_constants, verify


class CpuInterruptTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-cpu-interrupt-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        cls.original = cls.directory/'original'
        capture(ROOT, cls.original)
        cls.case = verify(ROOT, cls.original)['original']
        events = {q['serial']:q for q in cls.case['events']}
        cls.pairs = [(events[t['before']], events[t['after']]) for t in cls.case['transitions']]
        system = cls.case['system_events']
        cls.pairs += list(zip(system[::2], system[1::2]))
        cls.commands = cls.build(cls.directory, ROOT/'tests/cpu_interrupt.c')

    @classmethod
    def build(cls, directory, source):
        native, wasm = directory/'interrupt', directory/'interrupt.js'
        # Exercise release builds too: descriptor loads must survive NDEBUG.
        flags = ['-O2', '-DNDEBUG', '-I'+str(ROOT/'re_out')]
        commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', *flags, str(source), '-o', str(native)], [str(native)]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags,
                      str(source), '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1',
                      '-o', str(wasm)], [tool('node', 'Git/emsdk/node/*/bin/node'), str(wasm)])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            (directory/(target+'-build.log')).write_text(result.stdout+result.stderr)
            if result.returncode: raise RuntimeError(result.stdout+result.stderr)
            commands.append((target, run))
        return commands

    @staticmethod
    def state(q):
        return words(q)+[q[k]&0xffffffff for k in SYSTEM_FIELDS]

    def run_pair(self, index, commands=None, directory=None):
        directory = directory or self.directory
        before, after = self.pairs[index]
        if before['kind'] == 'before-hardware':
            op = [0, before['num'], before['oldeip']]
        elif before['kind'] == 'before-iret':
            op = [1, before['use32'], before['oldeip']]
        elif before['operation'] == 'CPU_LTR':
            op = [5, before['arguments']['selector'], 0]
        else:
            op = [{'CPU_LGDT':2, 'CPU_LIDT':3}[before['operation']],
                  before['arguments']['limit'], before['arguments']['base']]
        source = directory/(f'case-{index}.input')
        source.write_bytes(struct.pack('<61I', *op, *self.state(before)))
        expected = ' '.join(f'{w:08x}' for w in self.state(after))+'\ncomplete-RAM 16777216\n'
        if op[0] == 5: expected += f"return {after['return_value']}\n"
        ram = self.original/'source'
        for target, run in commands or self.commands:
            output = directory/(target+'.memory')
            result = subprocess.run([*run, str(source), str(ram/before['memory_file']),
                                     str(output), str(ram/after['memory_file'])],
                                    capture_output=True, text=True, timeout=30)
            (directory/(target+f'-{index}.log')).write_text(result.stdout+result.stderr)
            yield target, result, expected, output, ram/after['memory_file']

    def check_pair(self, index):
        for target, result, expected, output, ram in self.run_pair(index):
            with self.subTest(target=target, case=index):
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)
                self.assertEqual(output.read_bytes(), ram.read_bytes(), 'complete16MiB differs')

    def test_all_reached_irq_and_return_boundaries_match_original(self):
        self.assertEqual(len(self.case['transitions']), 9)
        for index in range(9): self.check_pair(index)

    def test_reached_system_loads_preserve_cache_flags_and_full_ram(self):
        self.assertEqual(len(self.pairs), 13)
        self.assertEqual([a['operation'] for a,b in self.pairs[9:]],
                         ['CPU_LGDT', 'CPU_LIDT', 'CPU_LTR', 'CPU_LIDT'])
        for index in range(9, 13): self.check_pair(index)

    def test_flag_and_descriptor_constants_bind_original_source(self):
        constants, offsets = source_constants(ROOT)
        header = (ROOT/'re_out/fist_interrupt.h').read_text()
        for name in ('IF', 'TF', 'DF', 'IOPL', 'NT', 'VM'):
            self.assertIn('FIST_FLAG_'+name+'=0x%xu'%constants['FLAG_'+name], header)
        for name in ('NORMAL', 'ALL'):
            self.assertIn('FIST_FMASK_'+name+'=0x%xu'%constants['FMASK_'+name], header)
        self.assertEqual((offsets[32]['esp0'], offsets[16]['sp0']), (4, 2))
        self.assertEqual(len(SYSTEM_FIELDS), 21)

    def test_causal_tss_stack_iret_tag_and_busy_cache_mutants_fail(self):
        header = (ROOT/'re_out/fist_interrupt.h').read_text()
        mutants = [
            ('wrong-stack', 5, 'uint32_t esp=fist_cpu_ram_read(cpu,ram,size,tss,sys->tss_is386 ? 4 : 2);',
             'uint32_t esp=cpu->esp;'),
            ('dirty-iret-tag', 4, 'cpu->flags.type=FIST_LAZY_UNKNOWN;', '/* incorrectly retained lazy tag */'),
            ('normalized-tss-kind', 0, 'sys->lastint=(uint8_t)num;',
             'sys->lastint=(uint8_t)num;sys->tss_is386=1;'),
            ('missing-tss-busy', 11, 'd.high|=0x200u;', '/* incorrectly left TSS available */')]
        for name, index, old, new in mutants:
            self.assertEqual(header.count(old), 1)
            directory = self.directory/name; directory.mkdir()
            (directory/'fist_interrupt.h').write_text(header.replace(old, new))
            source = directory/'cpu_interrupt.c'; source.write_bytes((ROOT/'tests/cpu_interrupt.c').read_bytes())
            commands = self.build(directory, source)
            for target, result, expected, output, ram in self.run_pair(index, commands, directory):
                with self.subTest(mutant=name, target=target):
                    self.assertEqual(len(result.stdout.splitlines()[0].split()), 58)
                    self.assertFalse(result.returncode == 0 and result.stdout == expected
                                     and output.exists() and output.read_bytes() == ram.read_bytes())


if __name__ == '__main__': unittest.main()
