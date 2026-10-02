import base64
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('sequence_format', ROOT / 'tools/oracle/sequence_format.py')
FORMAT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(FORMAT)


def tool(name, pattern):
    return os.environ.get(name.upper()) or shutil.which(name) or str(next(Path.home().glob(pattern)))


def patched_unit(directory, filename):
    unit = directory / filename
    unit.write_bytes((ROOT / 're_out' / filename).read_bytes())
    for patch in sorted((ROOT / 'patches').glob('*.diff')):
        sections = re.findall(r'^--- a/' + re.escape(filename) + r'(?:[ \t][^\n]*)?\n.*?(?=^--- a/|\Z)',
                              patch.read_text(), re.M | re.S)
        if sections:
            result = subprocess.run(['patch', '--batch', '-s', '-p1', '-F0', '--fuzz=0', '-d', directory],
                                    input=''.join(sections), text=True, capture_output=True)
            if result.returncode:
                raise RuntimeError(f'{patch.name}: {result.stdout}{result.stderr}')
    return unit.read_text()


def build_pic_probe(directory, sb_events=False):
    tree = ROOT / 'third_party/dosbox-build/dosbox-0.74-3'
    dos_source = (tree / 'src/dos/dos.cpp').read_text()
    signature = 'static inline void modify_cycles(Bits value) {'
    helper = signature + dos_source.split(signature, 1)[1].split('\n#else', 1)[0]
    (directory / 'dos_modify_cycles.h').write_text(helper + '\n')
    output = str(directory / ('pic-sb-probe' if sb_events else 'pic-probe'))
    device = ['-DFIST_SB_EVENT_CLOCK', str(ROOT / 'tools/oracle/sb_dma_probe.cpp')] if sb_events else []
    subprocess.run(['g++', '-std=gnu++11',
                    *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                    '-I' + str(tree / 'include'), '-I' + str(tree), '-I' + str(directory),
                    '-ffunction-sections', '-fdata-sections',
                    str(ROOT / 'tools/oracle/pic_slice_probe.cpp'),
                    str(ROOT / 'tools/oracle/io_delay_probe.cpp'),
                    str(ROOT / 'tools/oracle/dos_delay_probe.cpp'),
                    str(ROOT / 'tools/oracle/rep_probe.cpp'), *device, '-Wl,--gc-sections', '-lm',
                    '-o', output], check=True, capture_output=True, text=True)
    return output


class PortIoTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        # Compile the reached producer bodies after their ordered, exact Extender patches.
        text = patched_unit(cls.directory, 'fist_ext.c')
        declarations = '\n'.join(line for line in text.splitlines() if line.startswith('#define '))
        bodies = []
        for address in ('7120', '2630', '2294', '22ab'):
            signature = (f'void __allregs FUN_0000_{address}(void)' if address != '22ab' else
                         'ushort * __allregs FUN_0000_22ab(ushort *param_1,uint param_2,byte param_3,byte param_4)')
            body = text.split(signature+'\n\n{', 1)[1].split('\n}\n', 1)[0]
            bodies.append(signature+'\n{'+body+'\n}\n')
        blit = cls.directory / 'ext_producers.c'
        blit.write_text('#include "ghidra_compat.h"\nextern uint32_t fist_ext_base;\n' + declarations +
                        '\n'+'\n'.join(bodies))
        engine_text = patched_unit(cls.directory, 'fist.c')
        sound = cls.directory / 'sound_script.c'
        helpers = []
        for signature in ('static void fist_intro_sound(uint16_t cur)',
                          'static void fist_intro_stop(undefined4 inbox)'):
            body = engine_text.split(signature+'\n{', 1)[1].split('\n}\n', 1)[0]
            helpers.append(signature.removeprefix('static ')+'\n{'+body+'\n}\n')
        sound.write_text('#include "ghidra_compat.h"\nvoid FUN_0000_e2c2(undefined4);\n'+''.join(helpers))
        sources = [str(ROOT / 'tests/port_io.c'), str(ROOT / 're_out/fist_vga.c'),
                   str(ROOT / 're_out/fist_dos.c'), str(blit), str(sound)]
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections', '-Wno-int-conversion']
        native, wasm = (str(cls.directory / name) for name in ('ports', 'ports.js'))
        targets = [('native', ['gcc', '-m32', *flags, *sources, '-Wl,--gc-sections', '-lm', '-o', native], [native]),
                   ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags, *sources,
                             '-sASSERTIONS=1', '-sNODERAWFS=1', '-sEXIT_RUNTIME=1',
                             '--pre-js', str(ROOT / 'tools/wasm_pre.js'), '-o', wasm],
                    [tool('node', 'Git/emsdk/node/*/bin/node'), wasm])]
        cls.commands = []
        for target, build, run in targets:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append((target, run))
        tree = ROOT / 'third_party/dosbox-build/dosbox-0.74-3'
        cls.pit_probe = str(cls.directory / 'pit-probe')
        sdl_flags = subprocess.check_output(['sdl-config', '--cflags'], text=True).split()
        subprocess.run(['g++', '-std=gnu++11', *sdl_flags, '-I' + str(tree / 'include'), '-I' + str(tree),
                        '-ffunction-sections', '-fdata-sections', str(ROOT / 'tools/oracle/pit_latch_probe.cpp'),
                        '-Wl,--gc-sections', '-lm', '-o', cls.pit_probe], check=True, capture_output=True, text=True)
        cls.pic_probe = build_pic_probe(cls.directory)

    def test_pit_latches_match_original_float_period_and_rounding(self):
        for mode in (2, 3):
            for period in (200, 8191, 17023, 65536):
                for elapsed in sorted({1, 2, 123, 17022, period // 2, period - 1, period, period + 1, 2 * period + 17}):
                    args = [str(value) for value in (mode, period, elapsed)]
                    expected = subprocess.check_output([self.pit_probe, *args], text=True)
                    for target, run in self.commands:
                        with self.subTest(target=target, mode=mode, period=period, elapsed=elapsed):
                            result = subprocess.run([*run, 'pit', *args], capture_output=True, text=True, timeout=30)
                            self.assertEqual(result.returncode, 0, result.stderr)
                            self.assertEqual(result.stdout, expected)

    def test_unrelated_ports_preserve_speaker_and_pit2(self):
        # Original float latches include fractional I/O time; whole-count subtraction is invalid.
        before = int(subprocess.check_output([self.pit_probe, '2', '50000', '1'], text=True))
        expected = {}
        for mask in (False, True):
            elapsed = 4 + 21 * 1193182 / 30000000 if mask else 5
            expected[mask] = int(subprocess.check_output([self.pit_probe, '2', '50000', str(elapsed)], text=True))
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run(run, capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                lines = result.stdout.splitlines()
                self.assertEqual(len(lines), 34)   # all 17 ports, both speaker-gate-on states
                for line in lines:
                    port, mode, actual_before, actual_after = map(int, line.split())
                    with self.subTest(target=target, port=port, speaker=mode):
                        self.assertEqual(actual_before, before)
                        self.assertEqual(actual_after, expected[port in (0x21, 0xa1)])

    def test_pit_latches_preserve_sub_count_cpu_time(self):
        for mode in (2, 3):
            for period in (200, 8191, 17023, 65536):
                for instructions in (1, 2, 20, 25, 26, 27, 123, 30000):
                    elapsed = 1 + instructions * 1193182 / 30000000
                    expected = subprocess.check_output([self.pit_probe, str(mode), str(period), str(elapsed)], text=True)
                    for target, run in self.commands:
                        for command in ('pit-cpu', 'pit-cpu-base'):
                            with self.subTest(target=target, mode=mode, period=period, instructions=instructions, command=command):
                                result = subprocess.run([*run, command, str(mode), str(period), str(instructions)],
                                                        capture_output=True, text=True, timeout=30)
                                self.assertEqual(result.returncode, 0, result.stderr)
                                self.assertEqual(result.stdout, expected)

    def test_capture_ends_on_the_requested_clock_boundary(self):
        for end_ms in (31, 32, 999, 1000, 3000):
            captures = []
            for target, run in self.commands:
                with self.subTest(target=target, end_ms=end_ms):
                    prefix = self.directory / f'{target}-{end_ms}'
                    env = dict(os.environ, FIST_SEQUENCE=str(prefix), FIST_SEQUENCE_END_MS=str(end_ms))
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    self.assertEqual(Path(str(prefix) + '.end').read_bytes(), f'FISTEND1\n{end_ms}\n'.encode())
                    frames = Path(str(prefix) + '.frames')
                    info = FORMAT.validate(frames, 'F')
                    self.assertLess(info['last'], end_ms * 1000)
                    captures.append(frames.read_bytes())
            if len(captures) == 2:
                self.assertEqual(*captures)

    def pic_fixture(self):
        prefix = self.directory / 'pic-start'
        for suffix in ('text', 'bda'):
            source = ROOT / f'tools/oracle/start_state.{suffix}.gz.b64'
            Path(str(prefix) + '.' + suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
        state = Path(str(prefix) + '.vga')
        state.write_bytes((ROOT / 'tools/oracle/start_state.vga').read_bytes())
        return prefix, state

    def test_cpu_slices_match_original_vga_queue(self):
        prefix, state = self.pic_fixture()
        cases = ((6354, 1198), (6354, 1546), (6354, 2972), (6354, 3322), (6355, 4221),
                 (6354, 2296), (6354, 2297), (6355, 5661), (6355, 5662),
                 (6358, 10993), (6358, 10994), (6358, 10995),
                 (76, 1000), (76, 2000), (76, 3843), (76, 3844))
        for tick, index in cases:
            scenario = ['transition'] if tick == 76 else []
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index), *scenario], text=True)
            for target, run in self.commands:
                for capture in ((False, True) if (tick, index) == (6354, 1546) else (False,)):
                    with self.subTest(target=target, tick=tick, index=index, capture=capture):
                        env = dict(os.environ, FIST_TEXT_STATE=str(prefix))
                        env.pop('FIST_SEQUENCE_END_MS', None)
                        env.pop('FIST_SEQUENCE', None)
                        if capture:
                            env['FIST_SEQUENCE'] = str(self.directory / f'pic-{target}')
                        result = subprocess.run([*run, 'pic-slice', str(tick), str(index)], env=env,
                                                capture_output=True, text=True, timeout=30)
                        self.assertEqual(result.returncode, 0, result.stderr)
                        self.assertEqual(result.stdout, expected)

    def test_initial_cpu_phase_matches_original_queue(self):
        prefix, state = self.pic_fixture()
        start_ns = int(state.read_text().splitlines()[1].split()[0])
        cycle = (start_ns * 3 + 50) // 100
        expected = subprocess.check_output([self.pic_probe, str(state), str(cycle // 30000),
                                            str(cycle % 30000)], text=True)
        for target, run in self.commands:
            with self.subTest(target=target):
                result = subprocess.run([*run, 'start-cpu'], env=dict(os.environ, FIST_TEXT_STATE=str(prefix)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)

    def test_file_read_mask_io_matches_original_budget_and_suppression(self):
        prefix, state = self.pic_fixture()
        cases = [(28814, count) for count in (1, 37, 38, 39, 1151)]
        cases += [(index, count) for index in (29900, 29913, 29914, 29937, 29999)
                  for count in (1, 8)]
        for scenario in ('file-read', 'masked-read'):
            for index, count in cases:
                expected = subprocess.check_output([self.pic_probe, str(state), '20', str(index),
                                                    scenario, str(count)], text=True)
                for target, run in self.commands:
                    with self.subTest(target=target, index=index, reads=count, scenario=scenario):
                        result = subprocess.run([*run, 'pic-' + scenario, '20', str(index), str(count)],
                                                env=dict(os.environ, FIST_TEXT_STATE=str(prefix)),
                                                capture_output=True, text=True, timeout=30)
                        self.assertEqual(result.returncode, 0, result.stderr)
                        self.assertEqual(result.stdout, expected)

    def test_rep_movs_matches_original_budget_chunks_zero_count_direction_and_overlap(self):
        prefix,state=self.pic_fixture()
        cases=[(6354,index,count,width,direction,displacement)
               for index,count in ((1198,0),(1198,1),(2295,0),(2295,1),(2295,2),
                                   (2296,4096),(29999,16000))
               for width,direction,displacement in ((1,1,0x10000),(4,1,0x10000),(2,-1,0x10000),
                                                    (1,1,1),(4,1,1),(4,-1,-1))]
        for tick,index,count,width,direction,displacement in cases:
            oracle=self.directory/'rep-original.memory'
            args=list(map(str,(count,width,direction,displacement)))
            expected=subprocess.check_output([self.pic_probe,str(state),str(tick),str(index),'rep',*args,str(oracle)],text=True)
            memory=oracle.read_bytes()
            for target,run in self.commands:
                with self.subTest(target=target,index=index,count=count,width=width,direction=direction,overlap=displacement):
                    output=self.directory/f'{target}-rep.memory'
                    result=subprocess.run([*run,'rep',str(tick),str(index),*args,str(output)],
                                          env=dict(os.environ,FIST_TEXT_STATE=str(prefix)),capture_output=True,text=True,timeout=30)
                    self.assertEqual(result.returncode,0,result.stderr)
                    self.assertEqual(result.stdout,expected)
                    self.assertEqual(output.read_bytes(),memory)

    def test_kdv_frame_blit_matches_original_copy_budget_and_aperture(self):
        prefix, state = self.pic_fixture()
        proof = json.loads((ROOT / 'tools/oracle/kdv_blit_cases.json').read_text())
        captured = proof['case']
        tick, index = divmod(captured['start_cycle'], 30000)
        cases = [(tick, index), (6354, 1198), (6354, 2295), (6354, 2296), (6354, 29999)]
        for tick, index in cases:
            oracle = self.directory / 'blit-original.memory'
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index),
                                                'blit', str(oracle)], text=True)
            if (tick, index) == cases[0]:
                self.assertEqual(list(map(int, expected.split())),
                                 [captured['end_cycle'], captured['end_remaining']])
            memory = oracle.read_bytes()
            for target, run in self.commands:
                with self.subTest(target=target, tick=tick, index=index):
                    output = self.directory / f'{target}-blit.memory'
                    result = subprocess.run([*run, 'blit', str(tick), str(index), str(output)],
                                            env=dict(os.environ, FIST_TEXT_STATE=str(prefix)),
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(output.read_bytes(), memory)
                    self.assertEqual(result.stdout, expected)

    def test_intro_script_forwards_every_original_sound_register_packet(self):
        proof = json.loads((ROOT / 'tools/oracle/sound_script_case.json').read_text())
        source = self.directory / 'sound-script.input'
        source.write_bytes(b''.join(struct.pack('<HBBI', row['ax'], row['pitch_offset'], row['dl'], row['ecx'])
                                    for row in proof['commands'][:-3]))
        expected = [(row['ax'], row['ecx'], row['dl']) for row in proof['commands']]
        partial = self.directory / 'sound-script.partial'
        partial.write_bytes(source.read_bytes()+b'\0')
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}-sound-script.log'
                result = subprocess.run([*run, 'sound-script', str(source)],
                                        env=dict(os.environ, FIST_SOUND_REGLOG=str(output)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                actual = [tuple(int(value, 16) for value in line.split()[1:])
                          for line in output.read_text().splitlines()]
                self.assertEqual(actual, expected)
                incomplete = subprocess.run([*run, 'sound-script', str(partial)],
                                            env=dict(os.environ, FIST_SOUND_REGLOG=str(output)+'.partial'),
                                            capture_output=True, text=True, timeout=30)
                self.assertNotEqual(incomplete.returncode, 0, 'partial input packet was accepted')

    def original_module_memory(self, proof, ring):
        asset = (ROOT / proof['sample_asset']).read_bytes()
        self.assertEqual(hashlib.sha256(asset).hexdigest(), proof['sample_asset_sha256'])
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        memory = bytearray(b'\xa5' * (0x100000 + 0x800))
        memory[:len(image)] = image
        memory[ring:ring+proof['ring_length']] = bytes([proof['ring_initial_fill']])*proof['ring_length']
        for index, value in proof.get('ring_initial_exceptions', []): memory[ring+index] = value
        for clip in proof['clips']:
            data = asset[clip['asset_offset']:clip['asset_offset']+clip['length']]
            self.assertEqual(hashlib.sha256(data).hexdigest(), clip['sha256'])
            memory[clip['module_offset']:clip['module_offset']+clip['length']] = data
        for field in proof['before']:
            data = bytes.fromhex(field['hex']); memory[field['offset']:field['offset']+len(data)] = data
        return memory

    def test_22ab_first_scripted_channel_matches_original_assignment_and_instruction_bytes(self):
        proof = json.loads((ROOT / 'tools/oracle/channel_22ab_case.json').read_text())
        memory = self.original_module_memory(proof, 0x100000)
        expected = memory.copy()
        for field in proof['after']:
            data = bytes.fromhex(field['hex']); expected[field['offset']:field['offset']+len(data)] = data
        source = self.directory / 'channel-input.memory'; source.write_bytes(memory)
        args = proof['arguments']
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}-channel.memory'
                result = subprocess.run([*run, 'channel', str(source), str(output),
                                         *(str(args[key]) for key in ('sample', 'pitch', 'normalization', 'channel'))],
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(int(result.stdout), proof['returned_sample'])
                self.assertEqual(output.read_bytes(), expected)

    def test_2630_active_channels_match_complete_original_buffer_and_state(self):
        self.check_original_mixer_case('mixer_2630_case.json')

    def test_2630_rollover_preserves_normalization_instruction_bytes(self):
        self.check_original_mixer_case('mixer_2630_rollover_case.json')

    def test_2630_default_callback_preserves_original_registers_and_mixed_bytes(self):
        self.check_original_mixer_case('mixer_2630_callback_case.json')

    def check_original_mixer_case(self, filename):
        proof = json.loads((ROOT / 'tools/oracle' / filename).read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image[proof['lookup_offset']:proof['lookup_offset']+512]).hexdigest(),
                         proof['lookup_sha256'])
        ring = 0x1621
        memory = self.original_module_memory(proof, ring)
        expected = memory.copy()
        for field in proof['after']:
            data = bytes.fromhex(field['hex']); expected[field['offset']:field['offset']+len(data)] = data
        for index, value in proof['ring_final_changes']: expected[ring+index] = value
        source = self.directory / 'mixer-input.memory'; source.write_bytes(memory)
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}-mixer.memory'
                result = subprocess.run([*run, 'mixer', str(source), str(output),
                                         str(proof.get('default_callback_count', 0))],
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                actual = output.read_bytes()
                self.assertEqual(len(actual), len(expected))
                self.assertEqual(actual[:0x100000], expected[:0x100000])
                start = 0x100000 + proof.get('output_offset', 0)
                end = start + proof['output_length']
                self.assertEqual(hashlib.sha256(actual[start:end]).hexdigest(), proof['output_sha256'])
                self.assertEqual(actual[0x100000:start], expected[0x100000:start])
                self.assertEqual(actual[end:], expected[end:])

    def test_extender_reads_match_original_callback_88_cpu_and_register_contract(self):
        prefix,state=self.pic_fixture()
        data=self.directory/'ext-read'; data.mkdir()
        proof=json.loads((ROOT/'tools/oracle/kernel_read_cases.json').read_text())
        for case in proof['cases']:
            n=case['requested']; raw=bytes((i*37+(i>>8)+11)&255 for i in range(n))
            (data/'READ.BIN').write_bytes(raw)
            tick,index=divmod(case['start_cycle'],30000)
            for target,run in self.commands:
                with self.subTest(target=target,case=case):
                    output=self.directory/f'{target}-ext-read.memory'
                    result=subprocess.run([*run,'ext-read',str(tick),str(index),str(n),'valid',str(output)],
                                          env=dict(os.environ,FIST_TEXT_STATE=str(prefix),FIST_DATADIR=str(data)),
                                          capture_output=True,text=True,timeout=30)
                    self.assertEqual(result.returncode,0,result.stderr)
                    self.assertEqual(list(map(int,result.stdout.split())),
                                     [case['end_cycle'],case['end_remaining'],case['returned_eax'],case['returned_ecx'],0])
                    self.assertEqual(output.read_bytes(),raw+b'\xa5'*16)

    def test_extender_disk_reads_preserve_full_eax_short_eof_and_error_results(self):
        prefix,state=self.pic_fixture()
        data=self.directory/'ext-eof'; data.mkdir()
        cases=((0,100,False),(8,0,False),(16384,16384,False),(16385,3,False),
               (32768,16384,False),(70000,70000,False),(131072,70001,False),(8,8,True))
        for requested,length,invalid in cases:
            raw=bytes((i*37+(i>>8)+11)&255 for i in range(length))
            (data/'READ.BIN').write_bytes(raw)
            returned=6 if invalid else min(requested,length)
            # 18fe computes unrequested remainder before the DOS result, not after actual bytes.
            if invalid:remaining=requested if requested<=16384 else 16384
            elif returned==requested:remaining=0
            else:remaining=max(0,requested-(returned//16384+1)*16384)
            for target,run in self.commands:
                with self.subTest(target=target,requested=requested,length=length,invalid=invalid):
                    output=self.directory/f'{target}-eof.memory'
                    result=subprocess.run([*run,'ext-read','6354','1110',str(requested),'invalid' if invalid else 'valid',str(output)],
                                          env=dict(os.environ,FIST_TEXT_STATE=str(prefix),FIST_DATADIR=str(data)),
                                          capture_output=True,text=True,timeout=30)
                    self.assertEqual(result.returncode,0,result.stderr)
                    self.assertEqual(list(map(int,result.stdout.split()))[2:],[returned,remaining,int(invalid)])
                    expected=b'\xa5'*(requested+16) if invalid else raw[:requested]+b'\xa5'*(requested+16-returned)
                    self.assertEqual(output.read_bytes(),expected)

    def test_dos_caps_and_credits_match_original_active_slice(self):
        prefix, state = self.pic_fixture()
        cases = [(6354, index, value, 0, 0)
                 for index in (1198, 2296, 29900, 29995, 29996, 29997, 29998, 29999)
                 for value in (0, 1, 6, 8, 768, 3072, 16384, 65535)]
        cases += [(6354, index, value, retire, after)
                  for index, retire in ((29999, 1), (29998, 2), (2296, 1))
                  for value in (0, 6, 16384) for after in (0, 1, 5, 6, 25)]
        for tick, index, value, retire, after in cases:
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index),
                                                'dos-cap', str(value), str(retire), str(after)], text=True)
            for target, run in self.commands:
                with self.subTest(target=target, tick=tick, index=index, value=value, retire=retire, after=after):
                    result = subprocess.run([*run, 'dos-cap', str(tick), str(index), str(value), str(retire), str(after)],
                                            env=dict(os.environ, FIST_TEXT_STATE=str(prefix)),
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout, expected)

    def test_dos_file_reads_match_original_buffer_register_and_budget_contract(self):
        prefix, state = self.pic_fixture()
        data = self.directory / 'dos-read'
        data.mkdir()
        cases = ((28814, 8, 8, False), (28814, 768, 768, False),
                 (28814, 16384, 16384, False), (28814, 65535, 65535, False),
                 (28814, 32, 7, False), (28814, 8, 0, False), (28814, 0, 32, False),
                 (29990, 8, 8, False), (29999, 8, 8, False),
                 (28814, 8, 8, True), (29999, 8, 8, True))
        for index, requested, length, invalid in cases:
            raw = bytes((i * 37 + 11) & 255 for i in range(length))
            (data / 'READ.BIN').write_bytes(raw)
            returned = 6 if invalid else min(length, requested)
            phase = subprocess.check_output([self.pic_probe, str(state), '20', str(index),
                                             'dos-read', str(returned), 'invalid' if invalid else '0'],
                                            text=True).strip()
            expected_memory = b'\xa5' * (requested + 16) if invalid else raw[:requested] + b'\xa5' * (requested + 16 - returned)
            for target, run in self.commands:
                with self.subTest(target=target, index=index, requested=requested, length=length, invalid=invalid):
                    output = self.directory / f'{target}-read.memory'
                    result = subprocess.run([*run, 'dos-read', '20', str(index), str(requested),
                                             'invalid' if invalid else 'valid', str(output)],
                                            env=dict(os.environ, FIST_TEXT_STATE=str(prefix), FIST_DATADIR=str(data)),
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout.strip(), f'{phase} {returned} {int(invalid)}')
                    self.assertEqual(output.read_bytes(), expected_memory)

    def test_mz_loader_matches_original_reads_relocations_and_short_eof(self):
        prefix, state = self.pic_fixture()
        data = self.directory / 'mz-data'
        data.mkdir()
        cases = []
        for name, segment, relocation in (('FIST.DAT', 0, 0), ('MGAVIDEO.DVR', 0x3400, 0x3400),
                                          ('SOUNDDVR.DVR', 0x4000, 0x4000)):
            raw = (ROOT / 'armoredfist' / name).read_bytes()
            (data / name).write_bytes(raw)
            header = struct.unpack_from('<14H', raw)
            module = bytearray(raw[header[4] * 16:])
            for offset in range(header[3]):
                off, seg = struct.unpack_from('<HH', raw, header[12] + 4 * offset)
                site = seg * 16 + off
                struct.pack_into('<H', module, site,
                                 (struct.unpack_from('<H', module, site)[0] + relocation) & 0xffff)
            rounded = (header[2] & 0x7ff) * 512 - header[4] * 16
            reads = 1 + (rounded + 0x7fff) // 0x8000 + header[3]
            cases.append((name, segment, relocation, bytes(module), reads))

        # The original ignores last-page byte count during reads and tolerates short EOF.
        for name, length, pages in (('BOUNDARY.DVR', 0x8000 + 13, 65),
                                    ('SHORT.DVR', 71, 65), ('ZM.DVR', 19, 0x8001)):
            module = bytes((i * 37 + 11) & 0xff for i in range(length))
            header = struct.pack('<14H', 0x5a4d if name != 'ZM.DVR' else 0x4d5a,
                                 (length + 32) % 512, pages, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0)
            (data / name).write_bytes(header + b'\0' * 4 + module)
            reads = 1 + (((pages & 0x7ff) * 512 - 32) + 0x7fff) // 0x8000
            cases.append((name, 0x3400, 0, module, reads))

        for name, segment, relocation, expected, reads in cases:
            expected_phase = subprocess.check_output([self.pic_probe, str(state), '20', '28814',
                                                     'file-read', str(reads)], text=True).split()
            base = segment << 4
            for target, run in self.commands:
                with self.subTest(target=target, file=name):
                    output = self.directory / f'{target}-{name}.memory'
                    result = subprocess.run([*run, 'mz-overlay', name, str(segment), str(relocation),
                                             str(base + len(expected) + 512), str(output)],
                                            env=dict(os.environ, FIST_TEXT_STATE=str(prefix),
                                                     FIST_DATADIR=str(data)),
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    status, cycle, remaining, span, registered = map(int, result.stdout.split())
                    self.assertEqual(status, 0, result.stderr)
                    self.assertEqual((cycle, remaining), tuple(map(int, expected_phase[:2])))
                    self.assertEqual((span, registered), (len(expected), 1))
                    memory = output.read_bytes()
                    self.assertEqual(memory[:base], b'\xa5' * base)
                    self.assertEqual(memory[base:base + len(expected)], expected)
                    self.assertEqual(memory[base + len(expected):], b'\xa5' * 512)

    def test_mz_relocation_segment_addition_wraps_at_16_bits(self):
        prefix, state = self.pic_fixture()
        data = self.directory / 'mz-wrap'
        data.mkdir()
        header = struct.pack('<14H', 0x5a4d, 64, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0)
        (data / 'WRAP.DVR').write_bytes(header + struct.pack('<HH', 0, 0x2000) + b'Q' * 32)
        phase = subprocess.check_output([self.pic_probe, str(state), '20', '28814',
                                         'file-read', '3'], text=True).split()
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}-wrap.memory'
                result = subprocess.run([*run, 'mz-overlay', 'WRAP.DVR', '0xf000', '0xf000',
                                         '0x110002', str(output)],
                                        env=dict(os.environ, FIST_TEXT_STATE=str(prefix),
                                                 FIST_DATADIR=str(data)),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(tuple(map(int, result.stdout.split())),
                                 (0, int(phase[0]), int(phase[1]), 32, 1))
                expected = bytearray(b'\xa5' * 0x110002)
                expected[0xf0000:0xf0020] = b'Q' * 32
                struct.pack_into('<H', expected, 0x10000, 0x95a5)
                self.assertEqual(output.read_bytes(), expected)

    def test_real_application_load_preserves_fixture_clock_and_bios_memory(self):
        prefix, state = self.pic_fixture()
        phase = subprocess.check_output([self.pic_probe, str(state), '20', '28814',
                                         'file-read', '1151'], text=True).split()
        cycle, remaining = map(int, phase[:2])
        raw = (ROOT / 'armoredfist/FIST.DAT').read_bytes()
        header_size = struct.unpack_from('<H', raw, 8)[0] * 16
        expected = bytearray(raw[header_size:])
        expected[0x400:0x500] = Path(str(prefix) + '.bda').read_bytes()
        for target, run in self.commands:
            with self.subTest(target=target):
                output = self.directory / f'{target}-startup.memory'
                result = subprocess.run([*run, 'mz-start', 'FIST.DAT', str(output)],
                                        env=dict(os.environ, FIST_TEXT_STATE=str(prefix),
                                                 FIST_DATADIR=str(ROOT / 'armoredfist')),
                                        capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout,
                                 f'{cycle} {remaining}\n{cycle} {remaining}\n{cycle+2} {remaining-2}\n')
                self.assertEqual(output.read_bytes(), expected)

    def test_cpu_retirement_matches_original_queue(self):
        prefix, state = self.pic_fixture()
        cases = ((6355, 6529, '290708'), (6355, 6529, '290712'),
                 (6355, 6529, '1,290707'), (6355, 6529, '100000,190708'),
                 (6358, 10990, '3'), (6358, 10990, '3,1'), (6358, 10990, '1,1,1,1'),
                 (6354, 1198, '1098'), (6354, 1198, '1098,1'),
                 (6354, 29998, '2'), (6354, 29998, '2,1'), (6354, 29998, '1,1,1'))
        for tick, index, counts in cases:
            expected = subprocess.check_output([self.pic_probe, str(state), str(tick), str(index),
                                                'retire', counts], text=True)
            for target, run in self.commands:
                with self.subTest(target=target, tick=tick, index=index, counts=counts):
                    env = dict(os.environ, FIST_TEXT_STATE=str(prefix))
                    env.pop('FIST_SEQUENCE_END_MS', None)
                    env.pop('FIST_SEQUENCE', None)
                    result = subprocess.run([*run, 'pic-retire', str(tick), str(index), counts], env=env,
                                            capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout, expected)

    def test_invalid_capture_endpoint_fails(self):
        for value in ('', '0', '-1', '3.5', '4294967296', 'x'):
            for target, run in self.commands:
                with self.subTest(target=target, value=value):
                    prefix = self.directory / f'invalid-{target}-{value}'
                    env = dict(os.environ, FIST_SEQUENCE=str(prefix), FIST_SEQUENCE_END_MS=value)
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn('FIST_SEQUENCE:', result.stderr)
                    self.assertFalse(Path(str(prefix) + '.end').exists())

    def test_start_fixture_requires_its_original_vga_queue_state(self):
        prefix = self.directory / 'start-state'
        for suffix in ('text', 'bda'):
            source = ROOT / f'tools/oracle/start_state.{suffix}.gz.b64'
            Path(str(prefix) + '.' + suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
        state = Path(str(prefix) + '.vga')
        valid = (ROOT / 'tools/oracle/start_state.vga').read_bytes()
        for target, run in self.commands:
            for data in (None, b'FISTVGA1\n20960467 20168067\n20 nan\n', valid + b'junk', valid):
                with self.subTest(target=target, data=data):
                    if data is None:
                        state.unlink(missing_ok=True)
                    else:
                        state.write_bytes(data)
                    capture = self.directory / f'state-{target}'
                    env = dict(os.environ, FIST_TEXT_STATE=str(prefix), FIST_SEQUENCE=str(capture),
                               FIST_SEQUENCE_END_MS='3000')
                    result = subprocess.run([*run, 'sequence'], env=env, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode == 0, data == valid, result.stderr)


if __name__ == '__main__':
    unittest.main()
