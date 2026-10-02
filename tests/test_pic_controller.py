import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_port_io import ROOT, tool
from test_sb_dma import program, dsp


class PicControllerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        tree = ROOT / 'third_party/dosbox-build/dosbox-0.74-3'
        cls.original = str(cls.directory / 'original')
        subprocess.run(['g++', '-std=gnu++11', '-DFIST_SB_EVENT_CLOCK',
                        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                        '-I' + str(tree / 'include'), '-I' + str(tree),
                        '-ffunction-sections', '-fdata-sections',
                        str(ROOT / 'tools/oracle/pic_controller_probe.cpp'),
                        str(ROOT / 'tools/oracle/io_delay_probe.cpp'),
                        str(ROOT / 'tools/oracle/sb_dma_probe.cpp'),
                        '-Wl,--gc-sections', '-lm', '-o', cls.original],
                       check=True, capture_output=True, text=True, timeout=120)
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections',
                 '-fno-strict-aliasing', '-Wno-int-conversion']
        if os.environ.get('FIST_PARENT_NO_PIC_CLOCK'):
            flags.append('-DFIST_PARENT_NO_PIC_CLOCK')
        producer = Path(os.environ.get('FIST_PIC_PRODUCER_DIR', ROOT / 're_out'))
        sources = [str(ROOT / 'tests/pic_controller.c'), str(ROOT / 're_out/fist_pic.c'),
                   str(producer / 'fist_vga.c'), str(producer / 'fist_sb.c'),
                   str(ROOT / 're_out/fist_dos.c')]
        cls.commands = []
        for target in ('native', 'wasm'):
            output = str(cls.directory / ('pic' if target == 'native' else 'pic.js'))
            build = (['gcc', '-m32', '-O0'] if target == 'native' else
                     [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), '-O2'])
            build += [*flags, *sources]
            build += (['-Wl,--gc-sections', '-lm'] if target == 'native' else
                      ['-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1'])
            subprocess.run([*build, '-o', output], check=True, capture_output=True,
                           text=True, timeout=120)
            run = [output] if target == 'native' else [tool('node', 'Git/emsdk/node/*/bin/node'), output]
            cls.commands.append((target, run))

    def check_script(self, script, start=525*30000+100, serviced=False, reached=None):
        path = self.directory / 'script.txt'
        path.write_text(script)
        extra = ['sb-irq'] if serviced else []
        original = subprocess.run([self.original, str(start), str(path), *extra], check=True,
                                  capture_output=True, text=True, timeout=30).stdout
        if reached:
            clocks = [list(map(int, line.split()[1:])) for line in original.splitlines()
                      if line.startswith('clock ')]
            self.assertEqual(len(clocks), len(reached))
            self.assertEqual([c[0] for c in clocks], [r['cycle'] for r in reached])
            # The captured IRQ has other mixer/VGA deadlines. This fixture
            # tests its bytes/times; all fixture budgets are compared below.
        for target, run in self.commands:
            with self.subTest(target=target, start=start):
                result = subprocess.run([*run, str(start), str(path), *extra], capture_output=True,
                                        text=True, timeout=30,
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory/'device.wav')))
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual([s for s in result.stdout.splitlines() if s.startswith('read ')],
                                 [s for s in original.splitlines() if s.startswith('read ')])
                self.assertEqual(result.stdout, original + 'pumps 0\n')
        return [line for line in original.splitlines() if not line.startswith('clock ')]

    def test_icw_data_programs_vectors_and_keeps_initial_masks(self):
        self.check_script('r 21 f8\nr a1 fe\nw 20 11\nw 21 28\nw 21 4\nw 21 1\n'
                          'r 21 f8\nw a0 11\nw a1 70\nw a1 2\nw a1 1\nr a1 fe\n')

    def test_real_pcm8_completion_latches_pic_request_until_matching_ack(self):
        script = program(count=2047, auto=True) + dsp(0x40,0xa6,0xd1,0x48,0xff,3,0x1c)
        self.check_script(script + 'r 20 0\ng 0 200\nr 20 0\ng 0 200\nr 20 80\n'
                          'w 20 b\nr 20 0\nw 20 a\nr 20 80\nr 22f ff\nr 20 80\n'
                          'r 22e 7f\nr 20 0\n')

    def test_effective_if_trap_gate_coalescing_and_specific_eoi(self):
        lines = self.check_script(
            'w 21 78\nh 7 0\nr 20 80\nj 3093 0\nj 3202 1\nj 3202 0\n'
            'w 20 b\nr 20 80\nh 7 0\nh 7 0\nj 3202 0\n'
            'w 20 65\nr 20 80\nw 20 67\nr 20 0\nj 3202 0\n'
            'w 20 20\nr 20 0\nj 3202 0\nw 20 a\nr 20 0\n')
        self.assertEqual([line for line in lines if line.startswith('irq ')],
                         ['irq -1 -1', 'irq -1 -1', 'irq 7 15',
                          'irq -1 -1', 'irq 7 15', 'irq -1 -1'])

    def test_slave_cascade_masks_and_nested_service_eoi_order(self):
        lines = self.check_script(
            'w 21 78\nh 7 0\nj 3202 0\nh a 0\nr 20 0\nr a0 4\n'
            'w a1 fb\nr 20 4\nw 21 7c\nr 20 0\nr a0 4\nj 3202 0\n'
            'w 21 78\nj 3202 0\nw 20 b\nw a0 b\nr 20 80\nr a0 4\n'
            'w 20 20\nr 20 80\nr a0 4\nw a0 20\nr a0 0\nr 20 80\n'
            'w 20 20\nr 20 0\n')
        self.assertEqual([line for line in lines if line.startswith('irq ')],
                         ['irq 7 15', 'irq -1 -1', 'irq 10 114'])

    def test_all_sixteen_irq_priorities_and_deactivation_match_original(self):
        order = [0,1,2,8,9,10,11,12,13,14,15,3,4,5,6,7]
        script = 'w 21 0\nw a1 0\n'
        script += ''.join(f'h {i:x} 0\n' for i in reversed(order))
        for irq in order:
            script += 'j 3202 0\n' + ('w a0 20\n' if irq >= 8 else 'w 20 20\n')
        script += 'j 3202 0\nh 5 0\nl 5 0\nj 3202 0\nr 20 0\nr a0 0\n'
        lines = self.check_script(script)
        self.assertEqual([line for line in lines if line.startswith('irq ')],
                         [f'irq {i} {8+i if i<8 else 0x70+i-8}' for i in order] +
                         ['irq -1 -1', 'irq -1 -1'])

    def test_icw_single_auto_eoi_and_special_mask_priorities_match_original(self):
        # ICW1 single mode skips ICW3. Auto EOI leaves no in-service bit.
        lines = self.check_script(
            'w 20 13\nw 21 20\nw 21 3\nw 21 78\nh 7 0\nj 3202 0\n'
            'w 20 b\nr 20 0\nh 7 0\nj 3202 0\nr 20 0\n'
            'w a0 11\nw a1 90\nw a1 2\nw a1 1\nw a1 f3\n'
            'h 8 0\nm 8 0\nj 3202 0\nh 9 0\nm 9 0\nj 3202 0\n'
            'w a0 68\nj 3202 0\nw a0 b\nr a0 3\nw a0 20\nr a0 1\n'
            'w a0 20\nr a0 0\nw a0 48\n')
        self.assertEqual([line for line in lines if line.startswith('irq ')],
                         ['irq 7 39', 'irq 7 39', 'irq 8 144', 'irq -1 -1', 'irq 9 145'])

    def test_pic_command_io_and_mask_requeue_match_thin_active_budgets(self):
        script = ('w 20 b\nr 20 0\nw 20 a\nr 20 0\nw a0 b\nr a0 0\n'
                  'h 7 0\nw 21 78\nr 20 80\nj 3093 0\nj 3202 0\n'
                  'r 20 0\nw 20 b\nr 20 80\nw 20 20\nr 20 0\n')
        for index in (29900,29937,29969,29998,29999):
            self.check_script(script,525*30000+index)

    def test_reached_14e0_pic_isr_query_and_eoi_prefix_matches_original(self):
        case = json.loads((ROOT / 'tools/oracle/pic_irq_case.json').read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        code = bytes.fromhex(case['code_bytes']); at = case['code_offset']
        self.assertEqual(image[at:at+len(code)], code)
        rows = case['original_rows']
        self.assertEqual(len(rows), 36)
        script = ''.join(f"{r['op']} {r['port']:x} {r['value']:x}\n" for r in rows)
        self.check_script(script, rows[0]['cycle']-1, serviced=True, reached=rows)
