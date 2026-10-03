import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class CallbackExchangeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        cls.case = json.loads((ROOT/'tools/oracle/callback_exchange_case.json').read_text())
        cls.image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        assert hashlib.sha256(cls.image).hexdigest() == cls.case['image_sha256']
        source = cls.case['original']
        assert (source['frames'], source['mixed_samples'], source['endpoint_ms']) == (39, 27518, 600)
        for name, sha in source['script_sha256'].items():
            assert hashlib.sha256((ROOT/name).read_bytes()).hexdigest() == sha
        for code in source['code']:
            raw = bytes.fromhex(code['bytes'])
            assert cls.image[code['offset']:code['offset']+len(raw)] == raw
        cases = {row['case']: row for row in source['cases']}
        assert all(row['fetches'] == 10 for row in cases.values())
        assert cases['natural']['measured']['new_segment'] - cases['natural']['measured']['old_segment'] == cls.case['rebased_module_segment']
        for name in ('exchange-if1', 'exchange-if0', 'null-install'):
            before, after = (cases[name]['states'][label] for label in ('helper-before', 'caller-after'))
            assert before['registers'][0] >> 16 == after['registers'][0] >> 16 == 0x89ab
            assert before['registers'][3] >> 16 == after['registers'][3] >> 16 == 0xcdef
            assert after['cpu_regs.flags'] == before['cpu_regs.flags']
        text = patched_unit(cls.directory, 'fist.c')
        bodies = []
        for declaration in ('undefined4 __allregs FUN_1000_46b6', 'void __allregs FUN_1000_3446'):
            match = re.search(re.escape(declaration)+r'\([^;]*?\)\n\n\{.*?\n\}\n', text, re.S)
            if not match:
                raise RuntimeError('Missing actual callback/caller body: '+declaration)
            bodies.append(match.group(0))
        names = ('DAT_1000_4f98', 'DAT_1000_4f9a')
        defines = '\n'.join(line for line in text.splitlines()
                            if any(line.startswith('#define '+name+' ') for name in names))
        wrapper = cls.directory/'callback.c'
        wrapper.write_text('#include "ghidra_compat.h"\n'+defines+'\n'+'\n'.join(bodies)+'\n')
        flags = ['-I'+str(ROOT/'re_out'), '-fno-strict-aliasing', '-w']
        sources = [str(ROOT/'tests/callback_exchange.c'), str(wrapper)]
        native, wasm = (str(cls.directory/name) for name in ('callback', 'callback.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags, *sources,
                      '-sNODERAWFS=1', '-sEXIT_RUNTIME=1', '-sINITIAL_MEMORY=33554432', '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target, run))

    def check_case(self, name, old_offset, old_segment, offset, segment, cf, caller=False):
        owner = self.case['callback_image_address']
        memory = bytearray(b'\xa5'*(16*1024*1024))
        memory[:len(self.image)] = self.image
        struct.pack_into('<HH', memory, owner, old_offset, old_segment)
        expected = memory.copy()
        if caller:
            offset, segment = 0x3e0b, self.case['rebased_module_segment']
            dg = 0x1c000
            for field, value in ((0x18de, 0), (0x18e4, 65535), (0x18e6, 0x18e4), (0x18e8, 0x18ea)):
                struct.pack_into('<H', expected, dg+field, value)
            cursor = 0x18ea
            for index in range(63):
                following = (cursor+0x12) & 65535
                struct.pack_into('<H', expected, dg+cursor, following if index < 62 else 65535)
                cursor = following
            struct.pack_into('<H', expected, dg+0x18e0, cursor)
            stdout = 'caller cf 0\n'
        else:
            stdout = f'old {old_segment:04x}{old_offset:04x} cf {cf}\n'
        struct.pack_into('<HH', expected, owner, offset, segment)
        source = self.directory/(name+'.input')
        source.write_bytes(memory)
        for target, run in self.commands:
            with self.subTest(target=target, case=name):
                output = self.directory/(name+'-'+target+'.memory')
                result = subprocess.run([*run, 'caller' if caller else 'exchange', str(source),
                                         str(output), str(offset), str(segment), str(cf)],
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                actual = output.read_bytes()
                self.assertEqual(len(actual), len(expected))
                differences = [(hex(i), x, y) for i, (x, y) in enumerate(zip(expected, actual)) if x != y]
                self.assertFalse(differences, f'Unexpected whole-memory writes: {differences[:24]}')
                self.assertEqual(result.stdout, stdout)

    def test_natural_rebased_far_pointer_and_old_offset_return(self):
        self.check_case('natural', 1, 0, 0x3e0b, self.case['rebased_module_segment'], 0)

    def test_independent_segment_offset_and_old_pointer_return_preserve_carry(self):
        self.check_case('exchange', 0x5678, 0x3456, 0xbeef, 0x4567, 1)

    def test_null_install_still_returns_both_old_words(self):
        self.check_case('null-install', 0x5678, 0x3456, 0, 0, 1)

    def test_actual_scheduler_caller_supplies_cs_and_clears_carry(self):
        self.check_case('caller', 1, 0, 0, 0, 1, caller=True)

    def test_changed_old_callback_does_not_seed_scheduler_segment(self):
        self.check_case('changed-caller', 0x5678, 0x233d, 0, 0, 0, caller=True)
