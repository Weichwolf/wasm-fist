import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

from test_detail_service import block
from test_port_io import ROOT, patched_unit, tool


class DetailReturnTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='wasm-fist-detail-return-')
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        engine = patched_unit(cls.directory, 'fist.c')
        producers = []
        declarations = []
        for name in ('6de2','de89','e339','c008','e2df'):
            match = re.search(r'(?:void|undefined2) __allregs FUN_0000_'+name+r'\([^;]*?\)\n\n\{', engine)
            if not match:
                raise RuntimeError('Missing real engine producer '+name)
            producers.append(block(engine,match.group(0)))
            declarations.append(match.group(0).split('{',1)[0].strip()+';')
        wrapper = cls.directory/'engine.c'
        defines = '\n'.join(line for line in engine.splitlines() if line.startswith('#define DAT_'))
        wrapper.write_text('#include "ghidra_compat.h"\n'+defines+'\n'+
                           '\n'.join(declarations)+'\n'+'\n'.join(producers)+'\n')
        main = (ROOT/'tools/native_main.c').read_text()
        helper = block(main,'static uint32_t fist_detail_load(')
        operation = block(main,'    if (op == 0x44 && g_ext_ready) {')
        gate = cls.directory/'gate.c'
        gate.write_text('#include "ghidra_compat.h"\n#define FIST_EXT_BASE 0x100000u\n'
                        'extern int g_fist_ext_int;\n'
                        'extern unsigned short g_fist_ext_ecx,g_fist_ext_edx,g_fist_ext_edi;\n'
                        +helper+'\nint detail_service_gate(void) {\n'
                        'uint8_t *dg=g_mem+0x1c000; unsigned op=*(uint16_t *)(dg+0xea10); int g_ext_ready=1;\n'
                        +operation+'\nabort();\n}\n')
        extender = patched_unit(cls.directory,'fist_ext.c')
        (cls.directory/'fist_ext.c').write_text('#define fist_int_dispatch detail_service_int_dispatch\n'
                                              'void detail_service_int_dispatch(void);\n'+extender)
        flags = ['-I'+str(ROOT/'re_out'),'-I'+str(ROOT/'tests'),'-ffunction-sections','-fdata-sections',
                 '-fno-strict-aliasing','-Wno-int-conversion','-Wno-incompatible-pointer-types',
                 '-Wno-implicit-function-declaration','-Wno-return-mismatch','-w']
        if 'FistDetailRegisters' not in (ROOT/'re_out/ghidra_compat.h').read_text():
            flags.append('-DFIST_TEST_DETAIL_LEGACY')
        sources = [str(ROOT/'tests/detail_return.c'),str(wrapper),str(gate),str(cls.directory/'fist_ext.c'),
                   *(str(ROOT/'re_out'/name) for name in ('fist_dos.c','fist_vga.c','fist_pic.c'))]
        native,wasm = (str(cls.directory/name) for name in ('return','return.js'))
        cls.commands = []
        for target,build,run in [
            ('native',['gcc','-m32','-O0',*flags,*sources,'-Wl,--gc-sections','-lm','-o',native],[native]),
            ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc'),'-O2',*flags,*sources,
                     '-sNODERAWFS=1','-sASSERTIONS=1','-sEXIT_RUNTIME=1','--pre-js',
                     str(ROOT/'tools/wasm_pre.js'),'-o',wasm],[tool('node','Git/emsdk/node/*/bin/node'),wasm])]:
            result = subprocess.run(build,capture_output=True,text=True,timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target,run))
        cls.game = cls.directory/'game'
        (cls.game/'FISTDATA').mkdir(parents=True)
        for name in ('LOW','MEDIUM','HIGH'):
            shutil.copyfile(ROOT/'armoredfist/FISTDATA'/(name+'.DTL'),cls.game/'FISTDATA'/(name+'.DTL'))
        (cls.game/'PEER.DAT').write_bytes(b'Still open')

    def check_case(self,sky,detail,first,second,peers,inbox):
        source = json.loads((ROOT/'tools/oracle/detail_return_state_case.json').read_text())
        image = ROOT/'re_out/fist_image.bin'
        loader = json.loads((ROOT/'tools/oracle/detail_loader_case.json').read_text())
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         loader['image_sha256'])
        engine_image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        original = json.loads((ROOT/'tools/oracle/detail_return_case.json').read_text())
        for code in original['code']:
            raw = bytes.fromhex(code['bytes'])
            self.assertEqual(engine_image[code['image_offset']:code['image_offset']+len(raw)],raw)
        name = 'LOW' if detail==0 else 'MEDIUM' if detail==1 else 'HIGH'
        table = (self.game/'FISTDATA'/(name+'.DTL')).read_bytes()
        self.assertEqual(len(table),source['returned_EAX'])
        expected_handle = source['returned_EBX']+peers
        for target,run in self.commands:
            with self.subTest(target=target,sky=sky,detail=detail,peers=peers):
                output = self.directory/f'{target}-{sky}-{detail}-{peers}.table'
                args = [str(image),*(str(v) for v in (sky,detail,first,second,peers,inbox)),str(output)]
                result = subprocess.run([*run,*args],env=dict(os.environ,FIST_DATADIR=str(self.game)),
                                        capture_output=True,text=True,timeout=30)
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                self.assertEqual(result.stdout.splitlines(),[
                    f'op44 {inbox:08x}',
                    f'op68 {expected_handle:08x} eax {second>>1:08x} config {first>>1} '
                    f'result {len(table)} dos-ebx {expected_handle} '
                    f'sky {0x6877 if sky==0 else 0x689a} mode {1 if sky==0 else sky}',
                    'dos 1a 4e 1a 3d 3f 3e',
                    'peers'+''.join(' '+str(source['returned_EBX']+i) for i in range(peers))])
                self.assertEqual(output.read_bytes(),bytes([0xa5])*4+table+bytes([0xa5])*4)

    def test_source_normal_return_reaches_real_engine_op68_inputs(self):
        source = json.loads((ROOT/'tools/oracle/detail_return_state_case.json').read_text())
        self.check_case(0,2,*source['config_words'],0,0x89ab7654)

    def test_actual_peer_handles_and_word_configurations_replace_stale_inputs(self):
        for sky,detail,first,second,peers,inbox in [
            (0,0,6,2,1,0xf1234567),
            (1,1,2,0,2,0x89ab7654),
            (0,4,0x80fe,0xfffe,3,0x76543210)]:
            self.check_case(sky,detail,first,second,peers,inbox)
