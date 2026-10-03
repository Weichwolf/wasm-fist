import hashlib
import json
import re
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class ResourceOpenTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        text = patched_unit(cls.directory, 'fist.c')
        signature = 'void __allregs FUN_1000_26fc(undefined2 param_1,int param_2,undefined2 param_3,undefined2 param_4)'
        body = text.split(signature+'\n\n{',1)[1].split('\n}\n',1)[0]
        defines = '\n'.join(line for line in text.splitlines() if re.match(r'#define [iu]Ram000f[0-9a-f]{4} ',line))
        wrapper = cls.directory / 'resource.c'
        wrapper.write_text('#include "ghidra_compat.h"\n'
                           'void fist_resource_test_int_dispatch(void);\n'
                           '#define fist_int_dispatch fist_resource_test_int_dispatch\n'+
                           defines+'\n'+signature+'\n{'+body+'\n}\n')
        cls.case = json.loads((ROOT / 'tools/oracle/resource_open_26fc_case.json').read_text())
        flags = ['-I'+str(ROOT/'re_out'), '-I'+str(ROOT/'tests'),
                 '-ffunction-sections','-fdata-sections','-fno-strict-aliasing',
                 '-Wno-int-conversion','-Wno-incompatible-pointer-types',
                 '-Wno-implicit-function-declaration','-Wno-return-mismatch','-w']
        sources = [str(ROOT/'tests/resource_open.c'),str(wrapper),
                   *(str(ROOT/'re_out'/name) for name in
                     ('fist_dos.c','fist_vga.c','fist_pic.c','fist_sb.c'))]
        native,wasm = (str(cls.directory/name) for name in ('resource','resource.js'))
        cls.commands = []
        for target,build,run in [
            ('native',['gcc','-m32','-O0',*flags,*sources,'-Wl,--gc-sections','-lm','-o',native],[native]),
            ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),'-O2',*flags,*sources,
                     '-sNODERAWFS=1','-sASSERTIONS=1','-sEXIT_RUNTIME=1','-o',wasm],
             [tool('node','Git/emsdk/node/*/bin/node'),wasm])]:
            result = subprocess.run(build,capture_output=True,text=True,timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target,run))

    def test_missing_backland_variants_preserve_word_registers_and_complete_memory(self):
        case = self.case
        image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(),case['image_sha256'])
        code = bytes.fromhex(case['code_bytes'])
        self.assertEqual(image[case['code_offset']:case['code_offset']+len(code)],code)
        memory = bytearray(b'\xa5'*(16*1024*1024))
        memory[:len(image)] = image
        filename = bytes.fromhex(case['filename_bytes'])
        memory[0x1c000+case['filename_offset']:0x1c000+case['filename_offset']+64] = filename
        table = bytes.fromhex(case['variant_bytes'])
        memory[case['variant_table_offset']:case['variant_table_offset']+len(table)] = table
        # The original does not assign ES before AH43; preserve its inherited
        # bridge lane. BP is the actual nonzero source caller, adjacent to DI.
        struct.pack_into('<H',memory,0xf0010,case['source_es'])
        source = self.directory/'input.memory';source.write_bytes(memory)
        expected = memory.copy();expected[case['variant_table_offset']] = filename[10]
        for offset,value in [(0,2),(2,case['descriptor_offset']),(4,0x1c00),
                             (6,case['filename_offset']),(8,case['filename_offset']),
                             (10,2),(12,case['source_bp']),(14,0x1c00),(18,1),(20,0x21)]:
            struct.pack_into('<H',expected,0xf0000+offset,value)
        expected_text = ''.join(
            f"probe {i} {row['di']:04x} {row['bp']:04x} {row['bx']:04x} "
            f"1c00 {row['dx']:04x} {row['filename']}\n"
            for i,row in enumerate(case['probes'])) + 'calls 3\n'
        for target,run in self.commands:
            with self.subTest(target=target):
                output = self.directory/(target+'.memory')
                result = subprocess.run([*run,str(source),str(output),str(case['source_bp']),str(self.directory)],
                                        capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                self.assertEqual(result.stdout,expected_text)
                actual = output.read_bytes();self.assertEqual(len(actual),len(expected))
                differences = [(hex(i),x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]
                self.assertFalse(differences,f'Unexpected memory writes: {differences[:24]}')
