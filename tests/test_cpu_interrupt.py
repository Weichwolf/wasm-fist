"""Replay reached original system/IRQ/IRET API boundaries with complete CPU/RAM."""
import json
import hashlib
import copy
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from device_cpu_fixture import words
from memory_context_fixture import memory_packet, expected_memory_context
from test_port_io import ROOT, tool

sys.path.insert(0, str(ROOT/'tools/oracle'))
from capture_pit_irq_frames import SYSTEM_FIELDS, source_constants
from capture_memory_context import capture


class CpuInterruptTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-cpu-interrupt-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        cls.original = cls.directory/'original'
        cls.case = capture(ROOT, cls.original)['legacy']['original']
        # Architectural expectations are the unchanged original owner. Actual added
        # provider/cache/VGA inputs come from these same captured API boundaries.
        raw_events = [json.loads(l) for l in (cls.original/'source/irq-events.jsonl').read_text().splitlines()]
        raw_system = [json.loads(l) for l in (cls.original/'source/system-events.jsonl').read_text().splitlines()]
        cls.case['events'] = raw_events; cls.case['system_events'] = raw_system
        events = {q['serial']:q for q in cls.case['events']}
        cls.pairs = [(events[t['before']], events[t['after']]) for t in cls.case['transitions']]
        system = cls.case['system_events']
        cls.pairs += list(zip(system[::2], system[1::2]))
        cls.commands = cls.build(cls.directory, ROOT/'tests/cpu_interrupt.c')

    @classmethod
    def build(cls, directory, source):
        native, wasm = directory/'interrupt', directory/'interrupt.js'
        # Exercise release builds too: descriptor loads must survive NDEBUG.
        flags = ['-O2', '-DNDEBUG', '-I'+str(ROOT/'tests'), '-I'+str(ROOT/'re_out')]
        commands = []
        for target, build, run in [
            ('native', ['gcc', '-m32', *flags, str(source), '-o', str(native)], [str(native)]),
            ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc'), *flags,
                      str(source), '-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1', '-sALLOW_MEMORY_GROWTH=1',
                      '-o', str(wasm)], [tool('node', 'Git/emsdk/node/*/bin/node'), str(wasm)])]:
            result = subprocess.run(build, capture_output=True, text=True, timeout=120)
            (directory/(target+'-build.log')).write_text(result.stdout+result.stderr)
            if result.returncode: raise RuntimeError(result.stdout+result.stderr)
            commands.append((target, run))
        return commands

    @staticmethod
    def state(q):
        return words(q)+[q[k]&0xffffffff for k in SYSTEM_FIELDS]

    def run_pair(self, index, commands=None, directory=None):
        directory = directory or self.directory
        before, after = self.pairs[index]
        if before['kind'] == 'before-hardware':
            op = [0, before['num'], before['oldeip']]
        elif before['kind'] == 'before-iret':
            op = [1, before['use32'], before['oldeip']]
        elif before['operation'] == 'CPU_LTR':
            op = [5, before['arguments']['selector'], 0]
        else:
            op = [{'CPU_LGDT':2, 'CPU_LIDT':3}[before['operation']],
                  before['arguments']['limit'], before['arguments']['base']]
        source = directory/(f'case-{index}.input')
        ram = self.original/'source'
        source.write_bytes(struct.pack('<61I', *op, *self.state(before))+memory_packet(before,ram))
        expected = ' '.join(f'{w:08x}' for w in self.state(after))+'\ncomplete-RAM 16777216\n'
        if op[0] == 5: expected += f"return {after['return_value']}\n"
        for target, run in commands or self.commands:
            output = directory/(target+'.memory')
            context = directory/(target+'.context')
            result = subprocess.run([*run, str(source), str(ram/before['memory_file']),
                                     str(output), str(ram/after['memory_file']),str(context)],
                                    capture_output=True, text=True, timeout=30)
            (directory/(target+f'-{index}.log')).write_text(result.stdout+result.stderr)
            yield target, result, expected, output, ram/after['memory_file']

    def check_pair(self, index):
        for target, result, expected, output, ram in self.run_pair(index):
            with self.subTest(target=target, case=index):
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout, expected)
                self.assertEqual(output.read_bytes(), ram.read_bytes(), 'complete16MiB differs')
                context = output.with_suffix('.context')
                self.assertEqual(hashlib.sha256(context.read_bytes()).digest(),
                                 hashlib.sha256(expected_memory_context(self.pairs[index][1],self.original/'source')).digest(),
                                 'complete linked cache or VGA storage differs')

    def test_all_reached_irq_and_return_boundaries_match_original(self):
        self.assertEqual(len(self.case['transitions']), 9)
        for index in range(9): self.check_pair(index)

    def test_reached_system_loads_preserve_cache_flags_and_full_ram(self):
        self.assertEqual(len(self.pairs), 17)
        self.assertEqual([a['operation'] for a,b in self.pairs[9:]],
                         ['CPU_LGDT', 'CPU_LIDT', 'CPU_LTR', 'CPU_LIDT',
                          'CPU_LIDT', 'CPU_LGDT', 'CPU_LIDT', 'CPU_LTR'])
        for index in range(9, 17): self.check_pair(index)

    def test_flag_and_descriptor_constants_bind_original_source(self):
        constants, offsets = source_constants(ROOT)
        header = (ROOT/'re_out/fist_interrupt.h').read_text()
        for name in ('IF', 'TF', 'DF', 'IOPL', 'NT', 'VM'):
            self.assertIn('FIST_FLAG_'+name+'=0x%xu'%constants['FLAG_'+name], header)
        for name in ('NORMAL', 'ALL'):
            self.assertIn('FIST_FMASK_'+name+'=0x%xu'%constants['FMASK_'+name], header)
        self.assertEqual((offsets[32]['esp0'], offsets[16]['sp0']), (4, 2))
        self.assertEqual(len(SYSTEM_FIELDS), 21)

    def test_ltr_device_descriptor_callbacks_keep_original_dword_widths(self):
        self.check_ltr_device(self.commands,self.directory)

    def check_ltr_device(self,commands,directory,byte_stores=False):
        # Controlled relocation of the reached available descriptor into the
        # captured chained VGA aperture. This proves API callback widths; the
        # independent extracted-source VGA composition proves buffer semantics.
        before,after = (copy.deepcopy(q) for q in self.pairs[-1])
        source = self.original/'source'; context = before['memory_context']
        vga = context['vga']; selector = before['arguments']['selector']
        address = vga['pages']['base']*4096+318
        from capture_pit_irq_frames import shared_physical
        ram = (source/before['memory_file']).read_bytes()
        physical = shared_physical(ROOT)
        self.assertEqual(physical(address,ram,before),address)
        descriptor = physical(before['cpu.gdt.table_base']+(selector&~7),ram,before)
        low,high = struct.unpack_from('<2I',ram,descriptor)
        self.assertEqual(high&0x200,0)
        before['cpu.gdt.table_base'] = after['cpu.gdt.table_base'] = address-(selector&~7)
        packet = bytearray(memory_packet(before,source))
        from memory_context import packet as context_packet
        offset = len(context_packet(context))
        for i,value in enumerate(ram[descriptor:descriptor+8]):
            a = 318+i; planar = ((a&~3)<<2)|(a&3)
            packet[offset+planar] = value
        input_file = directory/'ltr-vga.input'
        input_file.write_bytes(struct.pack('<61I',6,selector,0,*self.state(before))+packet)
        expected_calls = [f'read {address:08x} 4 {low:08x}',f'read {address+4:08x} 4 {high:08x}']*2
        if byte_stores:
            expected_calls += [f'write {address+i:08x} 1 {value:08x}'
                               for i,value in enumerate(struct.pack('<2I',low,high|0x200))]
        else:
            expected_calls += [f'write {address:08x} 4 {low:08x}',f'write {address+4:08x} 4 {high|0x200:08x}']
        expected = '\n'.join(expected_calls)+'\n'
        expected += ' '.join('%08x'%v for v in self.state(after))+'\ncomplete-RAM 16777216\nreturn 0\n'
        cpu_source = (ROOT/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/cpu.cpp').read_text()
        save = cpu_source.split('void Descriptor:: Save(PhysPt address) {',1)[1].split('\n}',1)[0]
        self.assertEqual(save.count('mem_writed('),2)
        self.assertIn('mem_writed(address,*data);',save)
        self.assertIn('mem_writed(address+4,*(data+1));',save)
        for target,run in commands:
            output = directory/(target+'-ltr-vga.memory'); cache = directory/(target+'-ltr-vga.context')
            result = subprocess.run([*run,str(input_file),str(source/before['memory_file']),str(output),
                                     str(source/before['memory_file']),str(cache)],capture_output=True,text=True,timeout=30)
            (directory/(target+'-ltr-vga.log')).write_text(result.stdout+result.stderr)
            with self.subTest(target=target,byte_stores=byte_stores):
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertEqual(result.stdout,expected)
                self.assertEqual(output.read_bytes(),ram)
                if byte_stores:
                    original = (self.directory/(target+'-ltr-vga.context')).read_bytes()
                    self.assertNotEqual(hashlib.sha256(cache.read_bytes()).digest(),hashlib.sha256(original).digest(),
                                        'byte stores must differ in the original whole-width VGA mirror')

    def test_byte_descriptor_stores_reach_equal_cpu_ram_but_different_vga(self):
        self.check_ltr_device(self.commands,self.directory)
        header = (ROOT/'re_out/fist_interrupt.h').read_text()
        old = '    fist_ram_write(bus,address,4,d.low);\n    fist_ram_write(bus,address+4,4,d.high);'
        self.assertEqual(header.count(old),1)
        new = '    for(unsigned i=0;i<8;i++)fist_ram_write(bus,address+i,1,(uint8_t)((i<4 ? d.low : d.high)>>((i&3)*8)));'
        directory = self.directory/'byte-descriptor';directory.mkdir()
        (directory/'fist_interrupt.h').write_text(header.replace(old,new))
        source = directory/'cpu_interrupt.c';source.write_bytes((ROOT/'tests/cpu_interrupt.c').read_bytes())
        self.check_ltr_device(self.build(directory,source),directory,byte_stores=True)

    def test_first_and_repeated_bootstrap_match_cold_paging_and_full_context(self):
        directory = self.directory/'bootstrap';directory.mkdir()
        commands = self.build(directory,ROOT/'tests/cpu_bootstrap.c')
        self.check_bootstrap(commands,directory)

    def check_bootstrap(self,commands,directory):
        source = self.original/'source'
        fetches = [json.loads(l) for l in (source/'boot-fetches.jsonl').read_text().splitlines()]
        system = self.case['system_events']
        clock_fields = ('CPU_Cycles','CPU_CycleLeft','PIC_Ticks','CPU_CycleMax')
        def report(label,q):
            return label+' '+' '.join('%08x'%v for v in self.state(q))+' clock '+' '.join(str(q[k]) for k in clock_fields)
        for group in (1,2):
            rows = [q for q in fetches if q['group']==group]
            self.assertEqual(len(rows),9)
            context = rows[0]['context']
            before = [q for q in system if q['kind']=='before-system' and q['context']==context]
            first = next(q for q in before if q['operation']=='CPU_LGDT')
            loads = [first,next(q for q in before if q['operation']=='CPU_LIDT' and q['label']>first['label']),
                     next(q for q in before if q['operation']=='CPU_LTR')]
            after = [next(q for q in system if q['kind']=='after-system' and q['label']==a['label']) for a in loads]
            initial = directory/('bootstrap-%d.input'%group)
            initial.write_bytes(struct.pack('<62I',*self.state(first),*[first[k] for k in clock_fields])+memory_packet(first,source))
            expected = [report('LGDT',after[0]),report('fetch-1',rows[0]),report('LIDT',after[1])]
            expected += [report('fetch-%d'%q['ordinal'],q) for q in rows[1:]]
            expected += [report('LTR',after[2]),'return 0 instructions 10']
            for target,run in commands:
                output = directory/('%s-%d.memory'%(target,group))
                result = subprocess.run([*run,str(initial),str(source/first['memory_file']),str(output)],
                                        capture_output=True,text=True,timeout=30)
                (directory/('%s-%d.log'%(target,group))).write_text(result.stdout+result.stderr)
                with self.subTest(target=target,bootstrap=group):
                    self.assertEqual(result.returncode,0,result.stderr)
                    self.assertEqual(result.stdout.splitlines(),expected)
                    self.assertEqual(output.read_bytes(),(source/after[2]['memory_file']).read_bytes())
                    self.assertEqual(hashlib.sha256(Path(str(output)+'.context').read_bytes()).digest(),
                                     hashlib.sha256(expected_memory_context(after[2],source)).digest())
                    for q in rows:
                        boundary = str(output)+'-fetch-%d'%q['ordinal']
                        self.assertEqual(Path(boundary+'.memory').read_bytes(),(source/q['memory_file']).read_bytes())
                        self.assertEqual(hashlib.sha256(Path(boundary+'.context').read_bytes()).digest(),
                                         hashlib.sha256(expected_memory_context(q,source)).digest())

    def test_causal_tss_stack_iret_tag_and_busy_cache_mutants_fail(self):
        header = (ROOT/'re_out/fist_interrupt.h').read_text()
        mutants = [
            ('wrong-stack', 5, 'uint32_t esp=fist_ram_read(bus,tss,sys->tss_is386 ? 4 : 2);',
             'uint32_t esp=cpu->esp;'),
            ('dirty-iret-tag', 4, 'cpu->flags.type=FIST_LAZY_UNKNOWN;', '/* incorrectly retained lazy tag */'),
            ('normalized-tss-kind', 0, 'sys->lastint=(uint8_t)num;',
             'sys->lastint=(uint8_t)num;sys->tss_is386=1;'),
            ('missing-tss-busy', 11, 'd.high|=0x200u;', '/* incorrectly left TSS available */')]
        for name, index, old, new in mutants:
            self.assertEqual(header.count(old), 1)
            directory = self.directory/name; directory.mkdir()
            (directory/'fist_interrupt.h').write_text(header.replace(old, new))
            source = directory/'cpu_interrupt.c'; source.write_bytes((ROOT/'tests/cpu_interrupt.c').read_bytes())
            commands = self.build(directory, source)
            for target, result, expected, output, ram in self.run_pair(index, commands, directory):
                with self.subTest(mutant=name, target=target):
                    self.assertEqual(len(result.stdout.splitlines()[0].split()), 58)
                    self.assertFalse(result.returncode == 0 and result.stdout == expected
                                     and output.exists() and output.read_bytes() == ram.read_bytes())


if __name__ == '__main__': unittest.main()
