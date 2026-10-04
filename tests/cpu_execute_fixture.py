"""Continuous source-backed actual-byte execution with test-owned DOS observers."""
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess

from test_port_io import ROOT, tool
from device_cpu_fixture import words, clock
from memory_context_fixture import system_words, memory_packet, expected_memory_context


def layout(directory, source):
    """Read the original packed structures; addresses are not inferred from dumps."""
    header = (ROOT/'third_party/dosbox-build/dosbox-0.74-3/include/dos_inc.h').read_text()
    fields = {}
    for name, owner in (('sSDA', 'SDA'), ('sPSP', 'PSP'), ('sDTA', 'DTA')):
        body = re.sub(r'/\*.*?\*/', '', header.split('struct '+name+' {', 1)[1].split('}', 1)[0], flags=re.S)
        at = 0
        for typ, field, count in re.findall(r'\b(Bit8u|Bit16u|Bit32u|RealPt|char)\s+(\w+)(?:\[(\d+|DOS_NAMELENGTH_ASCII)\])?\s*;', body):
            fields[owner+'_'+field] = at
            at += {'Bit8u': 1, 'Bit16u': 2, 'Bit32u': 4, 'RealPt': 4, 'char': 1}[typ] * (13 if count == 'DOS_NAMELENGTH_ASCII' else int(count or 1))
            if owner == 'PSP' and field == 'stack':break
        if owner == 'DTA':assert at == 43
    fields['SDA'] = (int(re.search(r'#define DOS_SDA_SEG\s+(0x\w+)', header)[1], 16) << 4) + int(re.search(r'#define DOS_SDA_OFS\s+(\d+)', header)[1])
    fields['SDA_PSP'] = fields.pop('SDA_current_psp')
    fields['SDA_DTA'] = fields.pop('SDA_current_dta')
    fields['PSP_STACK'] = fields.pop('PSP_stack')
    fetches = read(source/'software-fetches.jsonl')
    code = next(bytes.fromhex(q['fetched_code_hex']) for q in fetches if q['segments'][1]['value'] == 0xf000 and q['fetched_code_hex'].startswith('fe38'))
    fields['DOS_CALLBACK'] = struct.unpack_from('<H', code, 2)[0]
    (directory/'source_layout.h').write_text(''.join('#define SOURCE_%s %d\n' % (name, value) for name, value in fields.items()))


def read(path):
    return [json.loads(line) for line in path.read_text().splitlines()]


def build(directory, source, header=None):
    directory.mkdir(parents=True, exist_ok=True)
    layout(directory, source)
    unit = directory/'cpu_execute.c'
    unit.write_bytes((ROOT/'tests/cpu_execute.c').read_bytes())
    if header is not None:(directory/'fist_exec.h').write_text(header)
    flags = ['-O2', '-DNDEBUG', '-I'+str(directory), '-I'+str(ROOT/'tests'), '-I'+str(ROOT/'re_out'),
             '-ffunction-sections', '-fdata-sections', '-fno-strict-aliasing', '-w']
    sources = [str(unit), str(ROOT/'tests/cpu_execute_clock.c'),
               *(str(ROOT/'re_out'/n) for n in ('fist_dos.c', 'fist_pic.c', 'fist_sb.c'))]
    runs = []
    for target, compiler, options, output, runner in (
        ('native', ['gcc', '-m32'], ['-Wl,--gc-sections', '-lm'], directory/'native', []),
        ('wasm', [tool('emcc', 'Git/emsdk/upstream/emscripten/emcc')],
         ['-sNODERAWFS=1', '-sASSERTIONS=1', '-sEXIT_RUNTIME=1', '-sALLOW_MEMORY_GROWTH=1'], directory/'wasm.js',
         [tool('node', 'Git/emsdk/node/*/bin/node')])):
        result = subprocess.run([*compiler, *flags, *sources, *options, '-o', str(output)], capture_output=True, text=True, timeout=120)
        (directory/(target+'-build.log')).write_text(result.stdout+result.stderr)
        if result.returncode:raise RuntimeError(result.stderr)
        runs.append((target, [*runner, str(output)]))
    return runs


def inputs(directory, source):
    events, fetches = read(source/'software-events.jsonl'), read(source/'software-fetches.jsonl')
    following, detailed = read(source/'following/fetches.jsonl'), read(source/'handler/events.jsonl')
    next_events, next_fetches = read(source/'findfirst/events.jsonl'), read(source/'findfirst/fetches.jsonl')
    prefix = read(source/'prefix-fetches.jsonl')
    initial = copy.deepcopy(prefix[0]);initial['memory_file'] = '77e2.memory'
    host = read(source/'host-events.jsonl');h = host[0]
    packet = directory/'input'
    packet.write_bytes(struct.pack('<58I', *words(initial), *system_words(initial)) + memory_packet(initial, source) +
                       struct.pack('<III', initial['CPU_Cycles'], clock(initial)-1, h['next_free']) + bytes(h['occupied']) +
                       bytes.fromhex(host[-1]['arguments']['name_raw_hex']) + struct.pack('<I', h['current_drive']))
    expected = [*[('fetch', q) for q in prefix], ('before-software', events[0]), ('after-software', events[1])]
    for q in fetches:
        expected.append(('fetch', q))
        if q['segments'][1]['value'] == 0xf000 and q['fetched_code_hex'].startswith('fe38'):
            expected.extend((kind, next(r for r in detailed if r['kind'] == kind)) for kind in ('before-DOS21', 'after-DOS21'))
        if q['segments'][1]['value'] == 8 and q['cpu_regs.ip.dword[0]'] == 0x1b41:
            expected.extend((('before-software-ret', events[2]), ('after-software-ret', events[3])))
    expected.extend(('fetch', q) for q in following)
    expected.extend(('find-'+q['kind'], q) for q in next_events[:2])
    for q in next_fetches:
        expected.append(('fetch', q))
        if q['segments'][1]['value'] == 0xf000 and q['fetched_code_hex'].startswith('fe38'):
            expected.extend(('find-'+r['kind'], r) for r in next_events[2:12])
        if q['segments'][1]['value'] == 8 and q['cpu_regs.ip.dword[0]'] == 0x1b41:
            expected.extend(('find-'+r['kind'], r) for r in next_events[12:14])
    assert len(prefix+fetches+following+next_fetches) == 1014 and len(expected) == 1034
    text = ''.join('%s %d %d %d %d %s\n' % (kind, clock(q), q['PIC_Ticks'], q['CPU_Cycles'], q['CPU_CycleLeft'],
                  ' '.join('%08x' % v for v in words(q)+system_words(q))) for kind, q in expected) + 'fetches 1014\n'
    snapshots = {**{'prefix-'+p.stem: {**json.loads(p.read_text()), 'memory_file': p.stem+'.memory'} for p in source.glob('*.json')},
                 'after-software': events[1], **{q['kind']: q for q in detailed}, 'before-software-ret': events[2],
                 'after-software-ret': events[3], 'before-next-DOS': json.loads((source/'following/before-next-DOS.json').read_text()),
                 **{'find-'+q['kind']: q for q in next_events}}
    assert len(snapshots) == 52 and len(host) == 4
    return packet, text, snapshots, host


def replay(directory, source, runs):
    packet, expected, snapshots, host = inputs(directory, source)
    receipts = []
    for target, run in runs:
        output = directory/(target+'-output')
        try:
            result = subprocess.run([*run, str(packet), str(source/'77e2.memory'), str(output), str(source/'game/FISTDATA')],
                                    capture_output=True, text=True, timeout=30, env=dict(os.environ, FIST_SB='1'))
            (directory/(target+'.log')).write_text(result.stdout+result.stderr)
            assert result.returncode == 0, result.stderr
            actual_rows, expected_rows = result.stdout.splitlines(), expected.splitlines()
            assert len(actual_rows) == len(expected_rows), (target, len(actual_rows), len(expected_rows))
            different = [i for i, (a, b) in enumerate(zip(actual_rows, expected_rows)) if a != b]
            assert not different, (target, different[:3], actual_rows[different[0]], expected_rows[different[0]])
            for label, q in snapshots.items():
                assert Path(str(output)+'-'+label+'.memory').read_bytes() == (source/q['memory_file']).read_bytes(), (target, label, 'RAM')
                assert Path(str(output)+'-'+label+'.context').read_bytes() == expected_memory_context(q, source), (target, label, 'provider/cache/VGA')
            for label, q in zip(('initial-host', 'find-before-directory', 'find-after-directory', 'find-before-SetResult'), host):
                assert Path(str(output)+'-'+label+'.host').read_bytes() == struct.pack('<II', q['current_drive'], q['next_free'])+bytes(q['occupied']), (target, label, 'host')
            receipts.append(dict(target=target, exit=result.returncode, fetches=1014, CPU_time=1034, complete_memory=52, complete_host=4,
                                 output_sha256=hashlib.sha256(result.stdout.encode()).hexdigest()))
        finally:
            # All raw target dumps are disposable after complete comparison.
            for p in directory.glob(target+'-output-*'):p.unlink()
    (directory/'proof.json').write_text(json.dumps(receipts, indent=2)+'\n')
    return receipts
