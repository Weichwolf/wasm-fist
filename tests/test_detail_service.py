import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


def block(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth = 1
    end = opening+1
    while depth:
        depth += (text[end] == '{')-(text[end] == '}')
        end += 1
    return text[start:end]


class DetailServiceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='wasm-fist-detail-service-')
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        main = (ROOT/'tools/native_main.c').read_text()
        operation = block(main, '    if (op == 0x44 && g_ext_ready) {')
        helper = (block(main, 'static uint32_t fist_detail_load(')
                  if 'static uint32_t fist_detail_load(' in main else '')
        owner = re.search(r'^FistDetailRegisters g_fist_detail_registers;[^\n]*',main,re.M)
        wrapper = cls.directory/'gate.c'
        wrapper.write_text('#include "ghidra_compat.h"\n'
                           '#define FIST_EXT_BASE 0x100000u\n'
                           'extern int g_fist_ext_int;\n'
                           'extern unsigned short g_fist_ext_ecx, g_fist_ext_edx, g_fist_ext_edi;\n'
                           'extern unsigned m_ext_FUN_0000_6032(int,unsigned short,unsigned short,unsigned short,unsigned,unsigned short);\n'
                           +(owner.group(0)+'\n' if owner else '')
                           +helper+'\nint detail_service_gate(void) {\n'
                           'uint8_t *dg=g_mem+0x1c000; unsigned op=*(uint16_t *)(dg+0xea10); int g_ext_ready=1;\n'
                           +operation+'\nabort();\n}\n')
        text = patched_unit(cls.directory, 'fist_ext.c')
        (cls.directory/'fist_ext.c').write_text(
            '#define fist_int_dispatch detail_service_int_dispatch\n'
            'void detail_service_int_dispatch(void);\n'+text)
        flags = ['-I'+str(ROOT/'re_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion',
                 '-Wno-incompatible-pointer-types', '-Wno-implicit-function-declaration',
                 '-Wno-return-mismatch', '-w']
        sources = [str(ROOT/'tests/detail_service.c'), str(wrapper), str(cls.directory/'fist_ext.c'),
                   *(str(ROOT/'re_out'/name) for name in ('fist_dos.c','fist_vga.c','fist_pic.c'))]
        native, wasm = (str(cls.directory/name) for name in ('detail', 'detail.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc','-m32','-O0',*flags,*sources,'-Wl,--gc-sections','-lm','-o',native], [native]),
            ('wasm', [tool('emcc','Git/emsdk/upstream/emscripten/emcc'),'-O2',*flags,*sources,
                      '-sNODERAWFS=1','-sASSERTIONS=1','-sEXIT_RUNTIME=1','--pre-js',
                      str(ROOT/'tools/wasm_pre.js'),'-o',wasm],
             [tool('node','Git/emsdk/node/*/bin/node'),wasm])]:
            result = subprocess.run(build,capture_output=True,text=True,timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target,run))

    def test_actual_op44_uses_filemgr_for_complete_detail_tables_and_handle(self):
        source = json.loads((ROOT/'tools/oracle/detail_loader_case.json').read_text())
        image = ROOT/'re_out/fist_image.bin'
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(), source['image_sha256'])
        for code in source['code']:
            if code['image'] == 'ext':
                raw = bytes.fromhex(code['bytes'])
                self.assertEqual(image.read_bytes()[code['image_offset']:code['image_offset']+len(raw)],raw)
        returned = source['loader_trace']['handlers'][0][-1]['registers']
        game = self.directory/'game'
        (game/'FISTDATA').mkdir(parents=True)
        for name in ('LOW','MEDIUM','HIGH'):
            shutil.copyfile(ROOT/'armoredfist/FISTDATA'/(name+'.DTL'),game/'FISTDATA'/(name+'.DTL'))
        for sky, detail, name in [(0,0,'LOW'),(0,1,'MEDIUM'),(1,2,'HIGH'),(0,4,'HIGH')]:
            original = (ROOT/'armoredfist/FISTDATA'/(name+'.DTL')).read_bytes()
            self.assertEqual(len(original), returned[0])
            for target,run in self.commands:
                with self.subTest(target=target,sky=sky,detail=detail):
                    output = self.directory/f'{target}-{sky}-{detail}.table'
                    result = subprocess.run([*run,str(image),str(sky),str(detail),str(output)],
                                            env=dict(os.environ,FIST_DATADIR=str(game)),
                                            capture_output=True,text=True,timeout=30)
                    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                    self.assertEqual(result.stdout.splitlines(),[
                        f'result {returned[0]} size {len(original)} ebx {returned[3]} '
                        f'sky {0x6877 if sky==0 else 0x689a} mode {1 if sky==0 else sky} task 0 flat 0',
                        'dos 1a 4e 1a 3d 3f 3e'])
                    self.assertEqual(output.read_bytes(),bytes([0xa5])*4+original+bytes([0xa5])*4)
