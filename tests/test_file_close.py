import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, patched_unit, tool


class FileCloseTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        cls.case = json.loads((ROOT/'tools/oracle/file_close_case.json').read_text())
        image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
        assert hashlib.sha256(image).hexdigest() == cls.case['image_sha256']
        for row in cls.case['code']:
            data = bytes.fromhex(row['bytes'])
            assert image[row['image_offset']:row['image_offset']+len(data)] == data
        text = patched_unit(cls.directory, 'fist.c')
        bodies = []
        for name in ('FUN_1000_4c9a', 'FUN_0000_fefb', 'FUN_1000_50c8'):
            match = re.search(r'(?:undefined4|undefined2) __allregs\s+'+name+r'\([^;]*?\)\n\n\{.*?\n\}\n', text, re.S)
            if not match:
                raise RuntimeError('Missing actual filename/file-close body: '+name)
            bodies.append(match.group(0))
        defines = '\n'.join(line for line in text.splitlines() if re.match(r'#define [iu]Ram000f[0-9a-f]{4} ', line))
        wrapper = cls.directory/'file_close.c'
        wrapper.write_text('#include "ghidra_compat.h"\nvoid fist_close_test_dispatch(void);\n'
                           '#define fist_int_dispatch fist_close_test_dispatch\n'+defines+'\n'+'\n'.join(bodies))
        flags = ['-I'+str(ROOT/'re_out'), '-I'+str(ROOT/'tests'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion', '-w']
        sources = [str(ROOT/'tests/file_close.c'), str(wrapper),
                   *(str(ROOT/'re_out'/name) for name in ('fist_dos.c', 'fist_vga.c', 'fist_pic.c'))]
        native, wasm = (str(cls.directory/name) for name in ('close', 'close.js'))
        cls.commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', '-O0', *flags, *sources, '-Wl,--gc-sections', '-lm', '-o', native], [native]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2', *flags, *sources,
                      '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1',
                      '--pre-js', str(ROOT/'tools/wasm_pre.js'), '-o', wasm],
             [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(result.stdout+result.stderr)
            cls.commands.append((target, run))

    def check_close(self, mode='config', size=10, peers=0, unrelated=0):
        sample = self.directory/'sample.bin'
        data = bytes((i*19+7) & 255 for i in range(min(size,64)))
        with sample.open('wb') as stream:
            stream.write(data)
            stream.truncate(size)
        name = 'missing.bin' if mode.startswith('missing') else 'sample.bin'
        command = 'size' if mode.endswith('size') else 'read-error' if mode=='read-error' else 'config'
        expected = bytearray(b'\xa5'*65)
        terminator = b'\0' if command=='size' else b'\0\0'
        expected[:len(name)+len(terminator)] = name.encode()+terminator
        if command=='config' and name=='sample.bin':expected[:len(data)]=data
        handle = 5+peers
        if name=='missing.bin':
            result,cf,dx,cx,calls,opened,closed,read_ax,read_cf = 2,1,0x740,0,1,0,0,0,0
            # fefb marshals the caller's CX; 50c8 does so independently.
        elif command=='size':
            result,cf,dx,cx,calls,opened,closed,read_ax,read_cf = size & 65535,0,size>>16,0,3,handle,handle,0,0
        else:
            read_ax = 6 if command=='read-error' else min(size,64)
            read_cf = command=='read-error'
            result,cf,dx,cx,calls,opened,closed = 0x3e00 | (handle & 255),int(read_cf),0x740,read_ax,3,handle,handle
        line = (f'result {result} cf {cf} dx {dx} cx {cx} calls {calls} opened {opened} closed {closed} '
                f'read-ax {read_ax} read-cf {int(read_cf)} next {handle} peers-alive {peers}\n')
        for target, run in self.commands:
            with self.subTest(target=target,mode=mode,size=size,peers=peers,unrelated=unrelated):
                output = self.directory/(target+'.buffer')
                actual = subprocess.run([*run,command,name,str(output),str(peers),str(unrelated)],
                                        env=dict(os.environ,FIST_DATADIR=str(self.directory)),
                                        capture_output=True,text=True,timeout=30)
                self.assertEqual(actual.returncode,0,actual.stdout+actual.stderr)
                self.assertEqual(actual.stdout,line)
                self.assertEqual(output.read_bytes(),expected)

    def test_short_config_read_closes_saved_handle_and_returns_close_ax(self):
        self.check_close()

    def test_full_and_empty_config_reads_release_the_handle(self):
        for size in (0,64):self.check_close(size=size)

    def test_config_read_error_preserves_read_carry_after_closing(self):
        self.check_close(mode='read-error')

    def test_config_close_preserves_existing_peer_handles(self):
        self.check_close(peers=3,unrelated=5)

    def test_both_original_overlay_sizes_release_the_handle(self):
        for size in (0x433c,0x7c9c):self.check_close(mode='size',size=size,unrelated=0x1366)

    def test_overlay_size_keeps_both_words_and_existing_peers(self):
        self.check_close(mode='size',size=0x12345678,peers=3,unrelated=5)

    def test_missing_files_skip_read_seek_and_close(self):
        self.check_close(mode='missing-config')
        self.check_close(mode='missing-size')
