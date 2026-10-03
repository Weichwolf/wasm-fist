set pagination off
set confirm off
python
import gdb, json, os, struct
from pathlib import Path

root = Path(os.environ['FIST_CALLBACK_OUTPUT'])
case = os.environ['FIST_CALLBACK_CASE']
assert case in ('natural', 'exchange-if1', 'exchange-if0', 'null-install')
fields = ('PIC_Ticks', 'CPU_Cycles', 'CPU_CycleLeft', 'CPU_CycleMax',
          'cpu_regs.ip.dword[0]', 'cpu.cr0', 'paging.cr3', 'cpu.code.big',
          'cpu.stack.big', 'cpu.pmode', 'cpu.cpl', 'cpu_regs.flags', 'lflags.type',
          'lflags.prev_type', 'lflags.oldcf', 'lflags.var1.dword[0]',
          'lflags.var2.dword[0]', 'lflags.res.dword[0]')
inside = False
original = None

def state():
    row = {name: int(gdb.parse_and_eval(name)) for name in fields}
    row['registers'] = [int(gdb.parse_and_eval('cpu_regs.regs[%d].dword[0]' % i)) for i in range(8)]
    row['segments'] = [{'value': int(gdb.parse_and_eval('Segs.val[%d]' % i)),
                        'base': int(gdb.parse_and_eval('Segs.phys[%d]' % i))} for i in range(6)]
    return row

def memory():
    return bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase')), 16777216))

def write(address, data):
    gdb.selected_inferior().write_memory(int(gdb.parse_and_eval('MemBase')) + address, data)

def snapshot(name, row):
    (root/(name+'.json')).write_text(json.dumps(row, indent=2)+'\n')
    (root/(name+'.memory')).write_bytes(memory())

class Capture(gdb.Breakpoint):
    def stop(self):
        global inside, original
        row = state()
        cs, ip = row['segments'][1]['value'], row['cpu_regs.ip.dword[0]']
        if not inside:
            assert cs == 0x2082 and ip == 0x3def
            inside = True
            snapshot('caller-before', row)
        if cs == 0x2082 and ip == 0x5026:
            data = memory()
            address = row['segments'][1]['base'] + 0x4f98
            original = dict(state=row, callback_address=address,
                            callback=data[address:address+4].hex())
            (root/'inputs.json').write_text(json.dumps(original, indent=2)+'\n')
            if case != 'natural':
                offset, segment = (0, 0) if case == 'null-install' else (0xbeef, 0x4567)
                write(address, struct.pack('<HH', 0x5678, 0x3456))
                gdb.execute('set cpu_regs.regs[0].dword[0] = %u' % ((0x89ab << 16) | (row['registers'][0] & 65535)))
                gdb.execute('set cpu_regs.regs[3].dword[0] = %u' % ((0xcdef << 16) | offset))
                gdb.execute('set Segs.val[0] = %u' % segment)
                gdb.execute('set Segs.phys[0] = %u' % (segment << 4))
                gdb.execute('set cpu_regs.flags = %u' % (0x3003 if case == 'exchange-if0' else 0x3203))
                gdb.execute('set lflags.type = 0')
                row = state()
            snapshot('helper-before', row)
        if cs == 0x2082 and ip == 0x3df3:
            snapshot('caller-after', row)
            if case != 'natural':
                normal_path = Path(os.environ['FIST_CALLBACK_NORMAL_AFTER'])
                normal = json.loads(normal_path.read_text())
                write(original['callback_address'], struct.pack('<HH',
                      original['state']['registers'][3] & 65535,
                      original['state']['segments'][0]['value']))
                for index in (0, 3):
                    gdb.execute('set cpu_regs.regs[%d].dword[0] = %u' % (index, normal['registers'][index]))
                gdb.execute('set Segs.val[0] = %u' % normal['segments'][0]['value'])
                gdb.execute('set Segs.phys[0] = %u' % normal['segments'][0]['base'])
                for name in ('cpu_regs.flags', 'lflags.type', 'lflags.prev_type', 'lflags.oldcf',
                             'lflags.var1.dword[0]', 'lflags.var2.dword[0]', 'lflags.res.dword[0]'):
                    gdb.execute('set %s = %u' % (name, normal[name]))
                stack = original['state']['segments'][2]['base'] + ((original['state']['registers'][4]-2) & 65535)
                normal_memory = normal_path.with_suffix('.memory').read_bytes()
                write(stack, normal_memory[stack:stack+2])
            snapshot('restored-after', state())
            self.enabled = False
        with (root/'fetches.jsonl').open('a') as stream:
            stream.write(json.dumps(row)+'\n')
        return False

capture = Capture('fist_cpu_trace')
capture.enabled = False

class ESWrite(gdb.Breakpoint):
    def stop(self):
        if int(gdb.parse_and_eval('Segs.val[1]')) != 0x2082 or int(gdb.parse_and_eval('cpu_regs.ip.dword[0]')) != 0x3dee:
            return False
        capture.enabled = True
        self.enabled = False
        return False

class Arm(gdb.Breakpoint):
    def stop(self):
        # Python stop callbacks execute even when a native condition is false.
        if int(gdb.parse_and_eval('PIC_Ticks')) != 19:
            return False
        ESWrite('Segs.val[0]', type=gdb.BP_WATCHPOINT, wp_class=gdb.WP_WRITE, internal=True)
        self.enabled = False
        return False

Arm('TIMER_AddTick')
end
run
