"""Compare recovered lazy flags with the actual original CPU flag/instruction owners."""
import json
import re
import struct
import subprocess
import unittest

from test_port_io import ROOT
from device_cpu_fixture import DIRECTORY, commands, words


FLAG_FIELDS = ('cpu_regs.flags', 'lflags.type', 'lflags.prev_type', 'lflags.oldcf',
               'lflags.var1.dword[0]', 'lflags.var2.dword[0]', 'lflags.res.dword[0]')
OPERATIONS = (('t_UNKNOWN', 8, 'unknown'), ('t_ADDw', 16, 'add'),
              ('t_XORb', 8, 'xor'), ('t_XORd', 32, 'xor'),
              ('t_CMPb', 8, 'cmp'), ('t_CMPw', 16, 'cmp'), ('t_TESTb', 8, 'test'),
              ('t_ADDd', 32, 'add'), ('t_ORb', 8, 'or'), ('t_ORd', 32, 'or'),
              ('t_SUBd', 32, 'sub'), ('t_CMPd', 32, 'cmp'),
              ('t_INCd', 32, 'inc'), ('t_DECd', 32, 'dec'))
DOS_OPERATIONS = (('t_ANDb', 8, 'and'), ('t_ANDw', 16, 'and'),
                  ('t_ORw', 16, 'or'), ('t_XORw', 16, 'xor'),
                  ('t_SUBb', 8, 'sub'), ('t_INCb', 8, 'inc'), ('t_INCw', 16, 'inc'),
                  ('t_DECb', 8, 'dec'), ('t_DECw', 16, 'dec'), ('t_TESTw', 16, 'test'),
                  ('t_SHLb', 8, 'shl'), ('t_SHLw', 16, 'shl'))


class CpuFlagsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.commands = commands()
        cls.case = json.loads((ROOT/'tools/oracle/device_checkpoint_case.json').read_text())
        first = json.loads((ROOT/'tools/oracle/software_dos_case.json').read_text())
        complete = json.loads((ROOT/'tools/oracle/software_startup_dos_case.json').read_text())
        cls.dos_rows = first['prefix']+first['fetches']+complete['following']+complete['find_fetches']
        tree = ROOT/'third_party/dosbox-build/dosbox-0.74-3'
        lazy = (tree/'src/cpu/lazyflags.h').read_text()
        cls.types = re.findall(r'\bt_[A-Za-z0-9_]+\b', lazy.split(
            '//Types of Flag changing instructions', 1)[1].split('enum {', 1)[1].split('};', 1)[0])
        cls.flags_probe = str(DIRECTORY/'cpu-flags-probe')
        subprocess.run(['g++', '-std=gnu++11',
                        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                        '-I'+str(tree/'include'), '-I'+str(tree),
                        '-ffunction-sections', '-fdata-sections',
                        str(ROOT/'tools/oracle/cpu_flags_probe.cpp'),
                        '-Wl,--gc-sections', '-o', cls.flags_probe],
                       check=True, capture_output=True, text=True, timeout=60)
        cls.instructions_probe = str(DIRECTORY/'cpu-instructions-probe')
        subprocess.run(['g++', '-std=gnu++11',
                        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                        '-I'+str(tree/'include'), '-I'+str(tree),
                        '-ffunction-sections', '-fdata-sections',
                        str(ROOT/'tools/oracle/cpu_instructions_probe.cpp'),
                        '-Wl,--gc-sections', '-o', cls.instructions_probe],
                       check=True, capture_output=True, text=True, timeout=60)

    def compare_flags(self, inputs):
        script = ''.join(' '.join(f'{v:x}' for v in row)+'\n' for row in inputs)
        expected = subprocess.run([self.flags_probe], input=script, text=True,
                                  check=True, capture_output=True, timeout=30).stdout
        self.assertEqual(len(expected.splitlines()), len(inputs))
        for target, run in self.commands:
            with self.subTest(target=target, cases=len(inputs)):
                result = subprocess.run([*run, 'flags'], input=script, text=True,
                                        capture_output=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr[:1000])
                self.assertEqual(result.stdout, expected)

    def test_boundary_and_complete_observed_flags_match_original_cpu(self):
        inputs = []
        for name, bits, kind in OPERATIONS:
            mask = (1 << bits)-1
            values = (0, 1, 15, mask//2, mask//2+1, mask-1, mask)
            for a in values:
                for b in values:
                    result = (a+b if kind == 'add' else a-b if kind in ('cmp', 'sub') else
                              a+1 if kind == 'inc' else a-1 if kind == 'dec' else
                              a | b if kind == 'or' else a & b if kind == 'test' else a ^ b)
                    for raw in (0x3206, 0xffffffff, 0x20460202):
                        operands = [((dirty & ~mask) | (value & mask)) & 0xffffffff
                                    for dirty, value in zip((0x89abcdef, 0x76543210, 0x12345678),
                                                          (a, b, result))]
                        inputs.append([raw, self.types.index(name), 0x31, 1, *operands])
        self.assertEqual(len(inputs), 2058)
        inputs += [[q[k] for k in FLAG_FIELDS] for q in self.case['fetches']]
        self.assertEqual(len(inputs), 2426)
        self.compare_flags(inputs)

    def test_each_reached_startup_operation_matches_original_flags(self):
        for name, _, _ in OPERATIONS[7:]:
            kind = self.types.index(name)
            q = next(q for q in self.case['fetches'] if q['lflags.type'] == kind)
            with self.subTest(operation=name, original_ip=hex(q['cpu_regs.ip.dword[0]'])):
                self.compare_flags([[q[k] for k in FLAG_FIELDS]])

    def test_flag_materialization_preserves_all_complete_original_cpu_words(self):
        self.assertEqual(len(self.case['fetches']), 368)
        self.assertEqual(len(self.dos_rows), 1014)
        rows = self.case['fetches']+self.dos_rows
        script = ''.join(' '.join(f'{q[k]:x}' for k in FLAG_FIELDS)+'\n' for q in rows)
        original = subprocess.run([self.flags_probe], input=script, text=True,
                                  check=True, capture_output=True, timeout=30).stdout.splitlines()
        self.assertEqual(len(original), len(rows))
        expected = []
        for q, line in zip(rows, original):
            f = line.split()
            self.assertEqual(len(f), 9)
            state = words(q)
            state[21:28] = [int(f[i], 16) for i in (2, 6, 7, 8, 3, 4, 5)]
            expected.append(f'{f[0]} {f[1]} '+' '.join(f'{v:08x}' for v in state))
        source = b''.join(struct.pack('<37I', *words(q)) for q in rows)
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run, 'flags-full'], input=source,
                                        capture_output=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr[:1000])
                self.assertEqual(result.stdout.decode(), '\n'.join(expected)+'\n')

    def test_checkpoint_operations_match_actual_original_instruction_owner(self):
        observed = self.case['instruction_transition_proof']['original_ALU_and_condition_calls']
        self.assertEqual(len(observed), 75)
        script = ''.join(q['operation']+' '+' '.join(f'{v:x}' for v in
                         [*q['input_flags'], q['a'], q['b']])+'\n' for q in observed)
        expected = subprocess.run([self.instructions_probe], input=script, text=True,
                                  check=True, capture_output=True, timeout=30).stdout
        recorded = ''.join('%08x %x %x %x %08x %08x %08x %08x %u %u\n' %
                           tuple(q['output']) for q in observed)
        self.assertEqual(expected, recorded)
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run, 'instructions'], input=script, text=True,
                                        capture_output=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr[:1000])
                self.assertEqual(result.stdout, expected)

    def test_complete_startup_dos_flags_match_original_including_narrow_operations(self):
        self.assertEqual(len(self.dos_rows), 1014)
        # INCw uses the same original width owner, but is a controlled source
        # composition here; this capture reaches INCb/INCd and all DEC widths.
        expected = {self.types.index(name) for name,_,_ in OPERATIONS+DOS_OPERATIONS
                    if name!='t_INCw'}
        self.assertEqual({q['lflags.type'] for q in self.dos_rows},expected)
        self.compare_flags([[q[k] for k in FLAG_FIELDS] for q in self.dos_rows])

    def test_dos_byte_word_boundaries_and_all_shift_counts_match_original(self):
        inputs = []
        for name, bits, kind in DOS_OPERATIONS:
            mask = (1<<bits)-1
            values = (0,1,15,mask//2,mask//2+1,mask-1,mask)
            for a in values:
                for b in range(32) if kind=='shl' else values:
                    result = (a-b if kind=='sub' else a+1 if kind=='inc' else a-1 if kind=='dec'
                              else a|b if kind=='or' else a^b if kind=='xor'
                              else a<<b if kind=='shl' else a&b)
                    for raw in (0x3206,0xffffffff,0x20460202):
                        widths = (bits,8 if kind=='shl' else bits,bits)
                        operands = [((dirty & ~((1<<width)-1)) | (v & ((1<<width)-1))) & 0xffffffff
                                    for dirty,v,width in zip((0x89abcdef,0x76543210,0x12345678),
                                                             (a,b,result),widths)]
                        inputs.append([raw,self.types.index(name),0x31,1,*operands])
        self.assertEqual(len(inputs), 2814)
        self.compare_flags(inputs)

    def test_dos_instruction_writes_preserve_original_lazy_upper_bits_and_carry(self):
        operations = ('XW','OW','HB','HW','SB','IB','IW','DB','DW','TW','LB','LW')
        scripts = []
        for operation in operations:
            bits = 8 if operation[-1]=='B' else 16
            mask = (1<<bits)-1
            for a in (0,1,15,mask//2,mask//2+1,mask-1,mask):
                for b in range(64) if operation[0]=='L' else (0,1,15,mask//2,mask):
                    for tag in ('t_UNKNOWN','t_CMPb','t_ADDw'):
                        flags = [0x20460203,self.types.index(tag),0x31,1,0x89abcdef,0x76543210,0x12345678]
                        scripts.append(operation+' '+' '.join(f'{v:x}' for v in [*flags,a,b])+'\n')
        self.assertEqual(len(scripts), 3738)
        script = ''.join(scripts)
        expected = subprocess.run([self.instructions_probe],input=script,text=True,
                                  check=True,capture_output=True,timeout=30).stdout
        self.assertEqual(len(expected.splitlines()),len(scripts))
        for target,run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run,'instructions'],input=script,text=True,
                                        capture_output=True,timeout=30)
                self.assertEqual(result.returncode,0,result.stderr[:1000])
                self.assertEqual(result.stdout,expected)
