import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class TaskModeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        cls.case = json.loads((ROOT/'tools/oracle/task_mode_case.json').read_text())
        cls.image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        assert hashlib.sha256(cls.image).hexdigest() == cls.case['image_sha256']
        for code in cls.case['code']:
            raw = bytes.fromhex(code['bytes'])
            assert cls.image[code['offset']:code['offset']+len(raw)] == raw
        source = cls.case['original']
        assert source['image_sha256'] == cls.case['image_sha256']
        assert (source['frames'], source['mixed_samples'], source['endpoint_ms']) == (39, 27518, 600)
        for path, checksum in source['script_sha256'].items():
            assert hashlib.sha256((ROOT/path).read_bytes()).hexdigest() == checksum
        measured = {row['case']: row['measured'] for row in source['cases']}
        assert measured['natural']['getter_byte'] == measured['natural']['setter_input_byte'] == 0
        assert measured['nonzero']['getter_byte'] == measured['nonzero']['setter_input_byte'] == 0x42
        assert measured['nonzero']['mode_neighbor'] == 0x7d
        for name in ('exchange', 'null'):
            assert measured[name]['setter_input_byte'] == 0xa5 and measured[name]['setter_return_byte'] == 0x42
        assert measured['null']['task_pointer'] == dict(offset=0, segment=0)
        text = patched_unit(cls.directory, 'fist.c')
        bodies = []
        for name in ('FUN_1000_23ba', 'FUN_1000_23bf'):
            match = re.search(r'undefined1 __allregs '+name+r'\([^;]*?\)\n\n\{.*?\n\}\n', text, re.S)
            if not match:
                raise RuntimeError('Missing actual task-mode body: '+name)
            bodies.append(match.group(0))
        caller = text.split('void __allregs FUN_0000_d99b(undefined4 param_1)\n\n{', 1)[1].split('\n}\n', 1)[0]
        start = '    *(uint16_t *)(dg+0x78e) = 0xea; }\n'
        end = '  { uint16_t s = *(uint16_t *)(dg+0xea2e), o = *(uint16_t *)(dg+0xea2c);\n    *(uint32_t *)(g_mem + ((uint32_t)s<<4) + o + 0x3f2) = 0; }'
        block = caller.split(start, 1)[1].split(end, 1)[0]
        names = ('DAT_1000_2d59', 'DAT_1000_c354', 'DAT_1000_c358')
        defines = '\n'.join(line for line in text.splitlines()
                            if any(line.startswith('#define '+name+' ') for name in names))
        wrapper = cls.directory/'task.c'
        wrapper.write_text('#include "ghidra_compat.h"\n'+defines+'\n'+'\n'.join(bodies)+
                           '\nvoid observed_task_calls(void)\n{\n'+block+'\n}\n')
        flags = ['-I'+str(ROOT/'re_out'), '-fno-strict-aliasing', '-w']
        sources = [str(ROOT/'tests/task_mode.c'), str(wrapper)]
        native, wasm = (str(cls.directory/name) for name in ('task', 'task.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags, *sources,
                      '-sNODERAWFS=1', '-sEXIT_RUNTIME=1', '-sINITIAL_MEMORY=33554432',
                      '-sEMULATE_FUNCTION_POINTER_CASTS=1', '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target, run))

    def check_case(self, name, old, mode, segment, offset, adjacent=None):
        case = self.case
        memory = bytearray(b'\xa5'*(16*1024*1024))
        memory[:len(self.image)] = self.image
        struct.pack_into('<II', memory, 0x1c354, *case['port_vectors'])
        pointer = (case['rebased_ss'] << 4)+case['task_pointer_offset']
        struct.pack_into('<HH', memory, pointer, offset, segment)
        memory[case['mode_address']] = old
        if adjacent is not None:
            memory[case['mode_address']+1] = adjacent
        # Actual stale SS bytes form an independent decoy. All bytes must survive
        # except the measured task and CS BYTE writes; old code aliases are checked.
        stale = bytes.fromhex(case['stale_ss_bytes'])
        memory[case['stale_ss_address']:case['stale_ss_address']+len(stale)] = stale
        expected = memory.copy()
        incoming = old if mode == 'calls' else 0xa5
        expected[case['mode_address']] = incoming
        if segment | offset:
            expected[(segment << 4)+((offset+case['task_mode_offset']) & 0xffff)] = incoming
        stdout = f'calls 2 mode {old:02x}\n' if mode == 'calls' else f'return {old:02x} mode a5\n'
        source = self.directory/(name+'.input')
        source.write_bytes(memory)
        for target, run in self.commands:
            with self.subTest(target=target, case=name):
                output = self.directory/(name+'-'+target+'.memory')
                result = subprocess.run([*run, mode, str(source), str(output), str(case['mode_address'])],
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                self.assertEqual(result.stdout, stdout)
                actual = output.read_bytes()
                self.assertEqual(len(actual), len(expected))
                differences = [(hex(i), x, y) for i, (x, y) in enumerate(zip(expected, actual)) if x != y]
                self.assertFalse(differences, f'Unexpected whole-memory writes: {differences[:24]}')

    def test_actual_caller_threads_natural_getter_byte_to_task(self):
        self.check_case('natural', 0, 'calls', 0x9000, 0)

    def test_nonzero_getter_and_adjacent_byte_survive_actual_caller(self):
        self.check_case('nonzero', 0x42, 'calls', 0x9000, 0, adjacent=0x7d)

    def test_setter_returns_old_mode_after_storing_independent_input(self):
        self.check_case('exchange', 0x42, 'exchange', 0x9000, 0)

    def test_null_task_pointer_still_exchanges_mode_and_returns_old_byte(self):
        self.check_case('null', 0x42, 'exchange', 0, 0)
