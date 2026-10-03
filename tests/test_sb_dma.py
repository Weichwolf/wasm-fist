import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import hashlib

from test_port_io import patched_unit

ROOT = Path(__file__).resolve().parents[1]


def write(port, value):
    return f'w {port:x} {value:x}\n'


def dsp(*values):
    return ''.join(write(0x22c, value) for value in values)


def program(addr=0x2de0, count=2047, auto=True, masked=False):
    return (write(0xc, 0) + write(2, addr & 255) + write(2, (addr >> 8) & 255)
            + write(0x83, addr >> 16) + write(3, count & 255) + write(3, count >> 8)
            + write(0xb, 0x59 if auto else 0x49) + write(0xa, 5 if masked else 1))


def registers():
    return write(0xc, 0) + 'r 2\nr 2\nr 3\nr 3\n'


class SoundBlasterDmaTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = Path(cls.tmp.name)
        flags = ['-I' + str(ROOT / 're_out'), '-ffunction-sections', '-fdata-sections']
        text = patched_unit(cls.directory, 'fist_ext.c')
        declarations = '\n'.join(line for line in text.splitlines() if line.startswith('#define '))
        signatures = ['undefined4 __allregs FUN_0000_12eb(int *param_1,char param_2,undefined4 param_3,char *param_4)',
                      'ushort __allregs FUN_0000_2810(void)']
        bodies = []
        for signature in signatures:
            body = text.split(signature+'\n\n{',1)[1].split('\n}\n',1)[0]
            bodies.append(signature+'\n{'+body+'\n}\n')
        producer = cls.directory / 'dma_init.c'
        producer.write_text('#include "ghidra_compat.h"\nextern uint32_t fist_ext_base;\n'+
                            declarations+'\n'+''.join(bodies))
        clock = cls.directory / 'clock.c'
        # Demand-only cases feed device ports without instruction I/O costs.
        # Keep their recording endpoint and link the real queue/clock owner.
        clock.write_text('#define in sb_demand_clock_in\n'
                         '#define out sb_demand_clock_out\n#include "fist_vga.c"\n')
        sources = [str(ROOT / 'tests/sb_dma.c'), str(ROOT / 're_out/fist_sb.c'), str(ROOT / 're_out/fist_pic.c'),
                   str(clock),
                   str(ROOT / 're_out/fist_dos.c'), str(producer)]
        native, wasm = (str(cls.directory / name) for name in ('sb', 'sb.js'))
        emcc = os.environ.get('EMCC') or shutil.which('emcc')
        node = os.environ.get('NODE') or shutil.which('node')
        targets = [(['gcc', '-m32', *flags, *sources, '-Wl,--gc-sections', '-lm', '-o', native], [native]),
                   ([emcc, '-O2', *flags, *sources, '-sNODERAWFS=1', '-sEXIT_RUNTIME=1',
                     '-sASSERTIONS=1', '-o', wasm], [node, wasm])]
        cls.commands = []
        for build, run in targets:
            subprocess.run(build, check=True, capture_output=True, text=True, timeout=120)
            cls.commands.append(run)
        tree = ROOT / 'third_party/dosbox-build/dosbox-0.74-3'
        driver = str(cls.directory / 'driver.o')
        subprocess.run(['gcc', *flags, '-DORIGINAL_SB', '-c', str(ROOT / 'tests/sb_dma.c'),
                        '-o', driver], check=True, capture_output=True, text=True)
        cls.original = str(cls.directory / 'original')
        subprocess.run(['g++', '-std=gnu++11',
                        *subprocess.check_output(['sdl-config', '--cflags'], text=True).split(),
                        '-I' + str(tree / 'include'), '-I' + str(tree), *flags, driver,
                        str(ROOT / 'tools/oracle/sb_dma_probe.cpp'), '-Wl,--gc-sections', '-lm',
                        '-o', cls.original], check=True, capture_output=True, text=True, timeout=120)

    def run_case(self, commands):
        expected = subprocess.run([self.original], input=commands, capture_output=True,
                                  text=True, check=True, timeout=30).stdout
        for run in self.commands:
            with self.subTest(target=run[0]):
                result = subprocess.run(run, input=commands, capture_output=True, text=True,
                                        env=dict(os.environ, FIST_AUDIO_WAV=str(self.directory / 'device.wav')),
                                        timeout=30)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)
        return expected.splitlines()

    def test_original_2810_programs_complete_dma_ring_and_device_input(self):
        case = json.loads((ROOT / 'tools/oracle/dma_2810_case.json').read_text())
        image = ROOT / 're_out/fist_image.bin'
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(), case['image_sha256'])
        writes = ''.join(write(row['port'],row['value']) for row in case['outputs'])
        log = ''.join(f"out {row['port']:x} {row['value']:x}\n" for row in case['outputs'])
        log += f"return {case['return_ax']:x}\n"
        log += '[sb] WAV finalized: 4096 PCM bytes (2048 samples @ 11111 Hz, 0.18s)\n'
        # The independent original DMA/DSP owners receive the captured original
        # OUTs. Port owners receive the actual translated 2810 invocation.
        tail = (registers()+'r 83\ns\n'+dsp(0x40,0xa6,0xd1,0x48,0xff,3,0x1c)+'s\n'+
                'd 1024\np\ns\n'+registers()+'r 22e\n'+
                'd 1024\np\ns\n'+registers()+'r 22e\n')
        expected = subprocess.run([self.original], input=writes+tail, capture_output=True,
                                  text=True, check=True, timeout=30).stdout
        for run in self.commands:
            with self.subTest(target=run[0]):
                commands = f"i {case['channel']:x} {case['physical_address']:x} {image}\n"+tail
                result = subprocess.run(run,input=commands,capture_output=True,text=True,
                                        env=dict(os.environ,FIST_AUDIO_WAV=str(self.directory/'init.wav')),
                                        timeout=30)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertEqual(result.stderr,log)
                self.assertEqual(result.stdout,expected)

    def test_every_captured_original_pcm8_demand_preserves_dma_and_dsp_state(self):
        case = json.loads((ROOT / 'tools/oracle/sb_pcm8_demand_case.json').read_text())
        commands = program() + dsp(0x40, 0xa6, 0xd1, 0x48, 0xff, 3, 0x1c) + 's\n'
        consumed, remaining, completions = 0, case['block'], 0
        before = []
        for tick, size, left, addr, count in case['requests']:
            before.append((left, consumed, completions, addr, count))
            commands += 's\n' + registers() + f'd {size}\np\n'
            consumed += size
            remaining -= size
            if remaining == 0:
                completions += 1
                remaining = case['block']
                commands += 'r 22e\n'
        commands += 's\n'
        lines = self.run_case(commands)
        self.assertEqual(lines.pop(0), 'state 1024 0 0 11111')
        for left, consumed_before, irq_before, addr, count in before:
            self.assertEqual(lines.pop(0), f'state {left} {consumed_before} {irq_before} 11111')
            self.assertEqual([lines.pop(0) for _ in range(4)],
                             [f'read 2 {addr & 255:x}', f'read 2 {addr >> 8:x}',
                              f'read 3 {count & 255:x}', f'read 3 {count >> 8:x}'])
            data = lines.pop(0).split()
            n = int(data[1])
            # Complete device bytes are also compared to the actual original DMA reader.
            self.assertEqual(bytes.fromhex(data[2]),
                             bytes(((i * 29) ^ (i >> 8)) & 255 for i in range(addr, addr + n)))
            if n == left:
                # The original read_sb returns 7f for an empty DSP read buffer,
                # independently of clearing the pending 8-bit completion IRQ.
                self.assertEqual(lines.pop(0), 'read 22e 7f')
        self.assertEqual(lines, [f'state {remaining} {consumed} {completions} 11111'])
        self.assertEqual(completions, len(case['irq_ticks']))

    def test_controller_flipflops_page_count_and_terminal_status(self):
        for auto in (False, True):
            commands = (program(0x12000, 63, auto) + dsp(0x40, 0xa6, 0x48, 127, 0, 0x1c)
                        # Another channel's byte shares the low/high flip-flop.
                        + write(0xc, 0) + write(0, 0xab) + write(2, 0x12)
                        + registers() + 'r 83\nr 8\ns\nd 90\np\ns\n' + registers()
                        + 'r 8\nr 8\nd 38\np\ns\n' + registers() + 'r 8\n')
            self.run_case(commands)
        # Controller two has its own flip-flop and word-address page register.
        self.run_case(write(0xd8, 0) + write(0xc0, 0x11) + write(0xc4, 0x22)
                      + write(0x8b, 3) + write(0xd8, 0) + 'r c4\nr c4\nr 8b\n')

    def test_mask_pause_and_irq_acknowledgement_follow_original_consumption(self):
        commands = (program(masked=True) + dsp(0x40, 0xa6, 0x48, 0xff, 3, 0x1c)
                    + 'd 16\ns\n' + write(0xa, 1) + 'd 16\ns\n'
                    + write(0xa, 5) + 's\nd 16\n' + write(0xa, 5) + 's\n'
                    + write(0xa, 1) + dsp(0xd0) + 'd 16\ns\n'
                    + dsp(0xd4, 0xd3) + 'd 2000\np\np\ns\n'
                    + 'd 1024\np\ns\nr 22e\nd 1024\np\ns\n')
        self.run_case(commands)

    def test_irq_status_and_wrong_width_ack_follow_original_hardware_reads(self):
        # Original 14e0 selects mixer index 82 at 152a/152c and tests bit 1
        # of the read at 152f to choose the DSP acknowledgement width. Reading
        # the 16-bit acknowledgement must leave a pending 8-bit IRQ intact.
        case = json.loads((ROOT / 'tools/oracle/sb_irq_ack_case.json').read_text())
        image = (ROOT / 're_out/fist_image.bin').read_bytes()
        self.assertEqual(hashlib.sha256(image).hexdigest(), case['image_sha256'])
        code = bytes.fromhex(case['irq_code_bytes'])
        self.assertEqual(image[case['irq_code_offset']:case['irq_code_offset']+len(code)], code)
        self.assertEqual([(row['port'], row['value']) for row in case['original_reads']],
                         [(0x225, 1), (0x22e, 0x7f)])
        commands = (write(0x224, 0x82) + 'r 224\nr 225\nr 22e\nr 22f\n'
                    + program() + dsp(0x40, 0xa6, 0x48, 0xff, 3, 0x1c)
                    + 'd 1024\np\ns\nr 225\nr 22f\nr 225\n'
                    + 'd 1024\np\ns\nr 225\nr 22e\nr 225\n'
                    + 'd 1024\np\ns\nr 225\nr 22e\nr 225\n')
        lines = self.run_case(commands)
        reads = [line for line in lines if line.startswith('read ')]
        self.assertEqual(reads, ['read 224 82', 'read 225 0', 'read 22e 7f', 'read 22f ff',
                                'read 225 1', 'read 22f ff', 'read 225 1',
                                'read 225 1', 'read 22e 7f', 'read 225 0',
                                'read 225 1', 'read 22e 7f', 'read 225 0'])
        self.assertEqual([line for line in lines if line.startswith('state ')],
                         ['state 1024 1024 1 11111', 'state 1024 2048 1 11111',
                          'state 1024 3072 2 11111'])

    def test_single_cycle_and_exit_auto_init_finish_on_demand(self):
        for command in (0x14, 0x15, 0x91):
            self.run_case(program() + dsp(0x40, 0xa6, command, 99, 0)
                          + 's\nd 80\ns\nd 1\np\np\ns\nd 8\nr 22e\np\ns\n'
                          + registers())
        self.run_case(program() + dsp(0x40, 0xa6, 0x48, 0xff, 3, 0x1c)
                      + 'd 100\n' + dsp(0xda) + 'd 924\np\ns\nd 1024\ns\n')

    def test_irq_ack_preserves_complete_available_dsp_data(self):
        commands = (program() + dsp(0x40, 0xa6, 0x48, 0xff, 3, 0x1c)
                    + 'd 1024\np\n' + write(0x224, 0x82) + dsp(0xe1)
                    + 'r 225\nr 22e\nr 225\nr 22a\nr 22e\nr 22a\nr 22e\n')
        lines = self.run_case(commands)
        self.assertEqual([line for line in lines if line.startswith('read ')],
                         ['read 225 1', 'read 22e ff', 'read 225 0',
                          'read 22a 4', 'read 22e ff', 'read 22a 5', 'read 22e 7f'])


if __name__ == '__main__':
    unittest.main()
