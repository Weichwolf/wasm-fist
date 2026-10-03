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
        extra = []
        for name in ('FUN_0000_222f', 'FUN_1000_223c', 'FUN_0000_f842',
                     'FUN_0000_f7c3', 'FUN_1000_288e', 'FUN_1000_2cc8'):
            match = re.search(r'(?:undefined4|void) __allregs\s+' + name +
                              r'\([^;]*?\)\n\n\{(.*?)\n\}\n', text, re.S)
            if not match:
                raise RuntimeError('Missing producer body: ' + name)
            extra.append(match.group(0))
        needed = ('DAT_1000_c388', 'DAT_2000_2b7a', 'DAT_2000_bb06',
                  'DAT_1000_2b50', 'DAT_1000_2b52', 'DAT_1000_2b54',
                  'DAT_1000_2b56', 'DAT_1000_2b58', 'DAT_1000_2b5a', 'DAT_1000_2d57')
        defines += '\n' + '\n'.join(line for line in text.splitlines()
                                      if any(line.startswith('#define '+name+' ') for name in needed))
        main = (ROOT/'tools/native_main.c').read_text()
        applier = re.search(r'int fist_apply_reloc_section\([^;]*?\) \{.*?^\}\n', main, re.S|re.M)
        if not applier:
            raise RuntimeError('Missing relocation owner')
        wrapper.write_text('#include "ghidra_compat.h"\n'
                           'void fist_resource_test_int_dispatch(void);\n'
                           'extern unsigned short g_fist_1345_bp;\n'
                           'undefined4 FUN_0000_f7c3(undefined2,undefined2);\n'
                           'void FUN_1000_288e(void), FUN_1000_2cc8(void);\n'
                           'undefined4 FUN_1000_0d2e(undefined2,int,undefined2);\n'
                           '#define fist_int_dispatch fist_resource_test_int_dispatch\n'+
                           defines+'\n'+signature+'\n{'+body+'\n}\n'+
                           '\n'.join(extra)+'\n#define RELOC_TAB_LIN 0x33520u\n'
                           '#define DGROUP_LIN 0x1c000u\n'+applier.group(0))
        cls.case = json.loads((ROOT / 'tools/oracle/resource_open_26fc_case.json').read_text())
        cls.boot = json.loads((ROOT / 'tools/oracle/boot_resource_case.json').read_text())
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

    def test_early_crt_installs_all_original_resource_vectors(self):
        case = self.boot
        image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        for code in case['code']:
            raw = bytes.fromhex(code['bytes'])
            self.assertEqual(image[code['offset']:code['offset']+len(raw)], raw)
        memory = bytearray(b'\xa5'*(16*1024*1024))
        memory[:len(image)] = image
        struct.pack_into('<I',memory,0x1c012,0x0f6901b2)
        source = self.directory/'crt-input.memory';source.write_bytes(memory)
        for target,run in self.commands:
            with self.subTest(target=target):
                output = self.directory/(target+'-crt.memory')
                result = subprocess.run([*run,str(source),str(output),str(self.case['source_bp']),
                                         str(self.directory),'crt'],
                                        capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                self.assertEqual(result.stdout,'calls 0\n')
                before = Path(str(output)+'.before').read_bytes()
                self.assertEqual(len(before),len(memory))
                expected = bytearray(before)
                for row in case['vectors']:
                    struct.pack_into('<HH',expected,0x1c000+row['offset'],row['value'],case['port_segment'])
                actual = output.read_bytes()
                self.assertEqual(len(actual),len(expected))
                differences = [(hex(i),x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]
                self.assertFalse(differences,f'Unexpected CRT-interval writes: {differences[:24]}')

    def test_boot_caller_passes_actual_lanes_and_skips_loaded_resource(self):
        case = self.case
        image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        for loaded in (False,True):
            memory = bytearray(b'\xa5'*(16*1024*1024));memory[:len(image)] = image
            filename = bytes.fromhex(case['filename_bytes'])
            memory[0x1c000+case['filename_offset']:0x1c000+case['filename_offset']+64] = filename
            variants = bytes.fromhex(case['variant_bytes'])
            memory[case['variant_table_offset']:case['variant_table_offset']+len(variants)] = variants
            struct.pack_into('<H',memory,0xf0010,case['source_es'])
            struct.pack_into('<H',memory,0x22b7a,0x80 if loaded else 0)
            # A loaded original returns before vector dispatch. Its vector is
            # deliberately invalid so accidentally calling it fails immediately.
            struct.pack_into('<I',memory,0x1c388,0xdeadbeef if loaded else 0x0f69306c)
            expected = memory.copy()
            text = 'calls 0\n'
            if not loaded:
                expected[case['variant_table_offset']] = filename[10]
                for offset,value in [(0,2),(2,case['descriptor_offset']),(4,0x1c00),
                                     (6,case['filename_offset']),(8,case['filename_offset']),
                                     (10,2),(12,case['source_bp']),(14,0x1c00),(18,1),(20,0x21)]:
                    struct.pack_into('<H',expected,0xf0000+offset,value)
                text = ''.join(f"probe {i} {r['di']:04x} {r['bp']:04x} {r['bx']:04x} "
                               f"1c00 {r['dx']:04x} {r['filename']}\n"
                               for i,r in enumerate(case['probes']))+'calls 3\n'
            source = self.directory/f'caller-{loaded}.memory';source.write_bytes(memory)
            for target,run in self.commands:
                with self.subTest(target=target,loaded=loaded):
                    output = self.directory/f'{target}-caller-{loaded}.memory'
                    result = subprocess.run([*run,str(source),str(output),str(case['source_bp']),
                                             str(self.directory),'caller'],
                                            capture_output=True,text=True,timeout=30)
                    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                    self.assertEqual(result.stdout,text)
                    actual = output.read_bytes();self.assertEqual(len(actual),len(expected))
                    differences = [(hex(i),x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]
                    self.assertFalse(differences,f'Unexpected caller writes: {differences[:24]}')
