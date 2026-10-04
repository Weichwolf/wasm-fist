import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


from device_cpu_fixture import DIRECTORY, commands, original, check, clock, words


class DeviceResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = DIRECTORY
        cls.commands = commands()
        cls.source, cls.controlled = original()
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
        label = '%04x' % rows[0]['cpu_regs.ip.dword[0]']
        check(self, rows, self.source/'source'/(label+'.memory'),
              self.source/'source/77ff.memory')

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
