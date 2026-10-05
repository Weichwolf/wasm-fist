"""Original-backed programs for the reached real/protected startup bridge."""
import itertools
import struct
from capture_cpu_cmp import cases as address_cases
from capture_cpu_segment_push import cases as segment_cases, packet
from capture_cpu_shr_instructions import operand_mutation

MODES = ('push-byte', 'push-ss', 'sub-word', 'push-full', 'pop-ss', 'xchg', 'shl')
COUNTS = dict(zip(MODES, (8192, 1024, 5137, 512, 1024, 19520, 19520)))


def programs(mode, tree):
    if mode in ('push-ss', 'pop-ss'):
        values, _ = segment_cases(tree)
        opcode = 0x16 if mode == 'push-ss' else 0x17
        result = [p[:-1] + bytes((opcode,)) for p in values if p[-1] == 0x06]
    elif mode in ('push-byte', 'push-full'):
        result = []
        values = range(256) if mode == 'push-byte' else (
            0, 1, 0x7f, 0x80, 0xff, 0x100, 0x7fff, 0x8000, 0xffff, 0x10000,
            0x7fffffff, 0x80000000, 0xffffffff, 0x12345678, 0x89abcdef, 0xff008000)
        for big, operand, address, stack, flags, value in itertools.product(
                (0, 1), (0, 1), (0, 1), (0, 1), (0x202, 0xfedcba98), values):
            width = 4 if big != operand else 2
            n = len(result)
            sp = ((0, 1, 2, 3, 0xfffc, 0xfffd, 0xfffe, 0xffff, 0xcafe0000,
                   0xfeedffff)[n % 10] if not stack else
                  (4, 5, 0xffff, 0x10000, 0x1ffff, 0x100000)[n % 6])
            code = (b'\x66' if operand else b'') + (b'\x67' if address else b'')
            code += (bytes((0x6a, value)) if mode == 'push-byte' else
                     b'\x68' + (value & ((1 << (width * 8)) - 1)).to_bytes(width, 'little'))
            result.append(packet(code, big=big, stack=stack,
                ip=(0x3abb, 0xfffd, 0xfffe, 0xffff)[n % 4], sp=sp, lazy=n % 65,
                flags=flags, selectors=(0x10, 0x2082, (0, 1, 0x26e, 0xffff)[n % 4],
                                         0x19f5, 0, 0),
                direction=1 if n & 1 else -1, budget=(1, 2, 17, 1796)[n % 4]))
    else:
        result = []
        opcodes = (0x87, 0x86) if mode == 'xchg' else (0xc1, 0xc0) if mode == 'shl' else (0x81,)
        for opcode in opcodes:
            for p in address_cases():
                big = struct.unpack_from('<I', p)[0]
                code = bytearray(p[112:])
                at = 0
                width = 4 if big else 2
                while code[at] != 0x3b:
                    if code[at] == 0x66:
                        width = 2 if big else 4
                    at += 1
                if mode == 'sub-word' and width != 2:
                    continue
                code[at] = opcode
                if mode == 'sub-word':
                    code[at + 1] = (code[at + 1] & 0xc7) | (5 << 3)
                    code += b'\x80\x01'
                elif mode == 'shl':
                    code[at + 1] = (code[at + 1] & 0xc7) | ((4 if len(result) & 1 else 6) << 3)
                    code.append((0, 1, 4, 7, 8, 16, 31, 32, 33, 63, 64, 127, 128, 255)[len(result) % 14])
                q = bytearray(p[:112])
                struct.pack_into('<I', q, 60, len(code))
                result.append(bytes(q + code))
        if mode == 'sub-word':
            for code in [bytes((0x83, 0xe8, n)) for n in range(256)] + [bytes.fromhex('26812e14018001')]:
                q = bytearray(result[0][:112])
                struct.pack_into('<I', q, 60, len(code))
                result.append(bytes(q + code))
    assert len(result) == COUNTS[mode], (mode, len(result))
    return result


def mutations(mode, header, cpu, stack, values):
    """Cause-specific variants use original programs; expectations stay original-owned."""
    out = []
    def add(name, code, *, h=header, c=cpu, s=stack, **kwargs):
        out.append((name, h, c, s, packet(bytes.fromhex(code), **kwargs)))
    push = 'else if(op==0x68||op==0x6a)fist_cpu_push(e->bus,width,op==0x6a?(uint32_t)(int32_t)(int8_t)fist_exec_fetch_code(e,&ip,1):fist_exec_fetch_code(e,&ip,width));'
    if mode == 'push-byte':
        assert header.count(push) == 1
        add('zero-extension', '666aff', h=header.replace('(uint32_t)(int32_t)(int8_t)fist_exec_fetch_code', 'fist_exec_fetch_code'))
        add('address-derived-width', '676aff', h=header.replace(push, push.replace('fist_cpu_push(e->bus,width,', 'fist_cpu_push(e->bus,address,')))
        add('fixed16-width', '666aff', h=header.replace(push, push.replace('fist_cpu_push(e->bus,width,', 'fist_cpu_push(e->bus,2,')))
        add('eager-flags', '6aff', lazy=32, h=header.replace(push, push.replace(')fist_cpu_push', '){fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push') + '}'))
        add('lost-upper-ESP', '6a01', sp=0xcafe0080, s=stack.replace('cpu->esp=next;', 'cpu->esp=next&cpu->stack_mask;'))
        add('wrong-stack-segment', '6aff', s=stack.replace('fist_ram_resident_write(bus,2,next&cpu->stack_mask,width,value);', 'fist_ram_resident_write(bus,3,next&cpu->stack_mask,width,value);'))
    elif mode == 'push-full':
        assert header.count(push) == 1
        add('word-only-immediate', '666878563412', h=header.replace(':fist_exec_fetch_code(e,&ip,width)', ':fist_exec_fetch_code(e,&ip,2)'))
        add('address-width-immediate', '676878563412', h=header.replace(':fist_exec_fetch_code(e,&ip,width)', ':fist_exec_fetch_code(e,&ip,address)'))
        add('eager-flags', '6880ff', lazy=32, h=header.replace(push, push.replace(')fist_cpu_push', '){fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push') + '}'))
        add('lost-upper-ESP', '6880ff', sp=0xcafe0080, s=stack.replace('cpu->esp=next;', 'cpu->esp=next&cpu->stack_mask;'))
    elif mode == 'push-ss':
        route = 'else if(op==0x06||op==0x0e||op==0x16||op==0x1e)fist_cpu_push(e->bus,width,e->bus->cpu->segments[op==0x06?0:op==0x0e?1:op==0x16?2:3].value);'
        assert header.count(route) == 1
        add('wrong-segment', '16', h=header.replace('op==0x16?2:3', 'op==0x16?3:3'))
        add('fixed16-width', '6616', h=header.replace(route, route.replace('fist_cpu_push(e->bus,width,', 'fist_cpu_push(e->bus,op==0x16?2:width,')))
        add('eager-flags', '16', lazy=32, h=header.replace(route, route.replace(')fist_cpu_push', '){fist_cpu_fill_flags(e->bus->cpu);fist_cpu_push') + '}'))
        add('lost-upper-ESP', '16', sp=0xcafe0080, s=stack.replace('cpu->esp=next;', 'cpu->esp=next&cpu->stack_mask;'))
    elif mode == 'sub-word':
        variants = [
            ('ignored-es-override', operand_mutation(header, 'if(seg<6)q.seg=seg', 'if(seg<6 && seg!=0)q.seg=seg'), values[-1]),
            ('cmp-without-writeback', header.replace('if(operation!=7 && operation!=8)fist_exec_write_op(e,q,w,v);', 'if(operation!=5 && operation!=7 && operation!=8)fist_exec_write_op(e,q,w,v);'), values[-1]),
            ('wrong-word-tag', header.replace('width==2?FIST_LAZY_SUBW:FIST_LAZY_SUBD', 'width==2?FIST_LAZY_SUBD:FIST_LAZY_SUBD'), values[-1]),
            ('unsigned-byte-immediate', header.replace('(uint32_t)(int32_t)(int8_t)fist_exec_fetch_code(e,&ip,1),w*8', 'fist_exec_fetch_code(e,&ip,1),w*8'), values[-2])]
        out.extend((name, h, cpu, stack, p) for name, h, p in variants)
    elif mode == 'pop-ss':
        route = 'else if(op==0x07 || op==0x17 || op==0x1f){unsigned which=op==0x07?0:op==0x17?2:3;'
        start = header.index(route)
        end = header.index('}else if(op==0x9d)', start)
        body = header[start:end]
        read = 'uint32_t value=fist_ram_resident_read(e->bus,2,e->bus->cpu->esp&e->bus->cpu->stack_mask,2);fist_exec_select_segment(e,which,value);'
        assert body.count(read) == 1
        oldmask = body.replace(read, 'uint32_t next=fist_cpu_stack_advance(e->bus->cpu,e->bus->cpu->esp,width);' + read).replace('e->bus->cpu->esp=fist_cpu_stack_advance(e->bus->cpu,e->bus->cpu->esp,width);', 'e->bus->cpu->esp=next;')
        add('missing-SS-credit', '17', h=header.replace('   if(op==0x17)e->credit(e->opaque);\n', ''))
        add('old-stack-mask-advance', '17', stack=1, sp=0x1ffff, h=header[:start] + oldmask + header[end:])
        add('wide-selector-read', '6617', h=header[:start] + body.replace('e->bus->cpu->stack_mask,2);', 'e->bus->cpu->stack_mask,width);') + header[end:])
        add('wrong-segment', '17', h=header.replace('op==0x17?2:3', 'op==0x17?3:3'))
        add('eager-flags', '17', lazy=32, h=header.replace(route, route + 'fist_cpu_fill_flags(e->bus->cpu);'))
    elif mode == 'shl':
        route = 'else if(op==0xc0||op==0xc1){unsigned m=fist_exec_fetch_code(e,&ip,1),w=op==0xc0?1:width;FistExecOperand q=fist_exec_operand(e,&ip,m,address,seg);unsigned count=fist_exec_fetch_code(e,&ip,1)&31;fist_cpu_require(((m>>3)&7)==4 || ((m>>3)&7)==6);'
        assert header.count(route) == 1
        # ModRM /4 is SHL; /6 is its original SAL alias. /5 would test SHR.
        add('operand-read-for-zero-count', 'c126140120', h=header.replace('   if(count){uint32_t a=fist_exec_read_op(e,q,w);', '   uint32_t a=fist_exec_read_op(e,q,w);if(count){'))
        add('four-bit-immediate-count', '66c1e01f', h=header.replace('count=fist_exec_fetch_code(e,&ip,1)&31', 'count=fist_exec_fetch_code(e,&ip,1)&15'))
        add('word-only-operand', '66c1e004', h=header.replace('w=op==0xc0?1:width', 'w=op==0xc0?1:2'))
        add('eager-flags', '66c1e004', lazy=32, h=header.replace(route, route + 'fist_cpu_fill_flags(e->bus->cpu);'))
        add('ignored-es-override', '26c126140104', h=operand_mutation(header, 'if(seg<6)q.seg=seg', 'if(seg<6 && seg!=0)q.seg=seg'))
        add('widened-byte', '66c0e004', h=header.replace('w=op==0xc0?1:width', 'w=width'))
    elif mode == 'xchg':
        route = 'else if(op==0x86||op==0x87){unsigned w=op==0x86?1:width;uint32_t v=fist_exec_read_op(e,q,w),old=fist_exec_reg_read(e,index,w);fist_exec_reg_write(e,index,w,v);fist_exec_write_op(e,q,w,old);}'
        assert header.count(route) == 1
        add('memory-before-register', '87261401', h=header.replace(route, route.replace('fist_exec_reg_write(e,index,w,v);fist_exec_write_op(e,q,w,old);', 'fist_exec_write_op(e,q,w,old);fist_exec_reg_write(e,index,w,v);')))
        add('word-only-operand', '6687dc', h=header.replace('w=op==0x86?1:width', 'w=op==0x86?1:2'))
        add('lost-old-register', '87dc', h=header.replace('fist_exec_write_op(e,q,w,old);', 'fist_exec_write_op(e,q,w,fist_exec_reg_read(e,index,w));'))
        add('eager-flags', '87dc', lazy=32, h=header.replace(route, route.replace('{unsigned w=', '{fist_cpu_fill_flags(e->bus->cpu);unsigned w=')))
        add('ignored-es-override', '2687261401', h=operand_mutation(header, 'if(seg<6)q.seg=seg', 'if(seg<6 && seg!=0)q.seg=seg'))
        add('widened-byte', '6686dc', h=header.replace('w=op==0x86?1:width', 'w=width'))
    assert out and all((h, c, s) != (header, cpu, stack) for _, h, c, s, _ in out), mode
    return out
