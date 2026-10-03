#!/usr/bin/env python3
"""Recover the actual scheduler caller and keyboard FAR-CALL operand exchange."""
import argparse
from copy import deepcopy
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'tools/oracle'))
from sequence_format import validate, validate_endpoint

FLAGS = ('cpu_regs.flags', 'lflags.type', 'lflags.prev_type', 'lflags.oldcf',
         'lflags.var1.dword[0]', 'lflags.var2.dword[0]', 'lflags.res.dword[0]')
CASES = ('natural', 'exchange-if1', 'exchange-if0', 'null-install')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def effective_flags(row):
    if row['lflags.type'] == 0:
        return row['cpu_regs.flags']
    assert row['lflags.type'] == 2  # Original lazy WORD ADD at 1346c.
    first, second, result = (row[name] & 65535 for name in
                            ('lflags.var1.dword[0]', 'lflags.var2.dword[0]', 'lflags.res.dword[0]'))
    assert result == (first+second) & 65535
    bits = ((first+second > 65535) | ((result & 255).bit_count() % 2 == 0) << 2 |
            ((first ^ second ^ result) & 16) | (result == 0) << 6 |
            (result & 32768) >> 8 | ((~(first ^ second) & (first ^ result)) & 32768) >> 4)
    return (row['cpu_regs.flags'] & ~0x8d5) | bits


def verify(output):
    image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
    code = {0x1347a: bytes.fromhex('bb0b3e0e070ee83312f8cb'),
            0x146b6: bytes.fromhex('9cfa8cc02e87069a4f8ec02e871e984f9dcb'),
            0x14627: bytes.fromhex('9a010000005bcb'),
            0x14517: bytes.fromhex('e8deff1ebaa74eb8690f8ed8b80925cd211fc3'),
            0x14537: bytes.fromhex('501eb8001c8ed8e4608ae0e4610c80e661247fe661b020e620')}
    for address, data in code.items():
        assert image[address:address+len(data)] == data
    assert 0x13483 + struct.unpack_from('<h', image, 0x13481)[0] == 0x146b6
    baseline = output/'baseline'
    assert (output/'baseline.exit').read_text() == '0\n'
    assert validate_endpoint(baseline/'sequence', 600) == 600
    assert validate(str(baseline/'sequence.frames'), 'F')['records'] == 39
    assert validate(str(baseline/'sequence.pcm'), 'A')['samples'] == 27518
    cases = []
    for case in CASES:
        p = output/case
        assert (output/(case+'.exit')).read_text() == '0\n'
        assert 'Python Exception' not in (p/'dosbox.log').read_text()
        assert validate_endpoint(p/'sequence', 600) == 600
        for suffix in ('frames', 'pcm', 'end'):
            assert (p/('sequence.'+suffix)).read_bytes() == (baseline/('sequence.'+suffix)).read_bytes()
        rows = [json.loads(line) for line in (p/'fetches.jsonl').read_text().splitlines()]
        assert len(rows) == 11
        expected_ips = (0x3def, 0x3df0, 0x5026, 0x5027, 0x5028,
                        0x502a, 0x502f, 0x5031, 0x5036, 0x5037, 0x3df3)
        assert [row['cpu_regs.ip.dword[0]'] for row in rows] == list(expected_ips)
        assert all(row['segments'][1]['value'] == 0x2082 and row['CPU_CycleMax'] == 30000
                   and row['cpu.cr0'] == 0 and row['cpu.pmode'] == 0 for row in rows)
        labels = ('caller-before', 'helper-before', 'caller-after', 'restored-after')
        states = {name: json.loads((p/(name+'.json')).read_text()) for name in labels}
        memory = {name: (p/(name+'.memory')).read_bytes() for name in labels}
        assert all(len(data) == 16777216 for data in memory.values())
        assert [states[name] for name in labels[:3]] == [rows[i] for i in (0, 2, 10)]
        inputs = json.loads((p/'inputs.json').read_text())
        before, helper, after, restored = (states[name] for name in labels)
        cs_base = helper['segments'][1]['base']
        load_bias = cs_base + helper['cpu_regs.ip.dword[0]'] - 0x146b6
        owner = inputs['callback_address']
        assert owner == cs_base+0x4f98 == load_bias+0x14628
        assert memory['helper-before'][load_bias+0x146b6:load_bias+0x146c8] == code[0x146b6]
        assert memory['caller-before'][load_bias+0x1347a:load_bias+0x13485] == code[0x1347a]
        assert memory['caller-before'][load_bias+0x14627] == 0x9a
        ss_base = before['segments'][2]['base']
        sp = before['registers'][4] & 65535
        expected_memory = bytearray(memory['caller-before'])
        struct.pack_into('<H', expected_memory, ss_base+((sp-2) & 65535), before['segments'][1]['value'])
        struct.pack_into('<H', expected_memory, ss_base+((sp-4) & 65535), expected_ips[-1])
        if case != 'natural':
            struct.pack_into('<HH', expected_memory, owner, 0x5678, 0x3456)
        assert memory['helper-before'] == expected_memory
        # PUSH CS and near CALL affect only the WORD stack and instruction location.
        first = deepcopy(rows[0])
        for i in (1, 2):
            first['CPU_Cycles'] -= 1
            first['cpu_regs.ip.dword[0]'] = expected_ips[i]
            first['registers'][4] = (first['registers'][4] & 0xffff0000) | ((sp-i*2) & 65535)
            if i == 2 and case != 'natural':
                offset, segment = (0, 0) if case == 'null-install' else (0xbeef, 0x4567)
                first['registers'][0] = 0x89ab0000 | (first['registers'][0] & 65535)
                first['registers'][3] = 0xcdef0000 | offset
                first['segments'][0] = dict(value=segment, base=segment << 4)
                first['cpu_regs.flags'] = 0x3003 if case == 'exchange-if0' else 0x3203
                first['lflags.type'] = 0
            assert rows[i] == first
        old_offset, old_segment = struct.unpack_from('<HH', expected_memory, owner)
        new_offset, new_segment = helper['registers'][3] & 65535, helper['segments'][0]['value']
        flags = effective_flags(helper)
        expected = deepcopy(helper)
        for step, index in enumerate(range(3, 11)):
            expected['CPU_Cycles'] -= 1
            expected['cpu_regs.ip.dword[0]'] = expected_ips[index]
            if step == 0:  # PUSHF materializes the lazy arithmetic state.
                expected['registers'][4] = (expected['registers'][4] & 0xffff0000) | ((sp-6) & 65535)
                expected['cpu_regs.flags'], expected['lflags.type'] = flags, 0
                struct.pack_into('<H', expected_memory, ss_base+((sp-6) & 65535), flags & 65535)
            elif step == 1:
                expected['cpu_regs.flags'] &= ~0x200
            elif step == 2:
                expected['registers'][0] = (expected['registers'][0] & 0xffff0000) | new_segment
            elif step == 3:
                expected['registers'][0] = (expected['registers'][0] & 0xffff0000) | old_segment
                struct.pack_into('<H', expected_memory, owner+2, new_segment)
            elif step == 4:
                expected['segments'][0] = dict(value=old_segment, base=old_segment << 4)
            elif step == 5:
                expected['registers'][3] = (expected['registers'][3] & 0xffff0000) | old_offset
                struct.pack_into('<H', expected_memory, owner, new_offset)
            elif step == 6:
                expected['cpu_regs.flags'], expected['lflags.type'] = flags, 0
                expected['registers'][4] = (expected['registers'][4] & 0xffff0000) | ((sp-4) & 65535)
            else:
                expected['registers'][4] = before['registers'][4]
            assert rows[index] == expected, (case, step)
        assert memory['caller-after'] == expected_memory
        assert restored['CPU_Cycles'] == after['CPU_Cycles']
        assert restored['PIC_Ticks'] == after['PIC_Ticks'] and restored['CPU_CycleLeft'] == after['CPU_CycleLeft']
        expected_restore = deepcopy(after)
        if case != 'natural':
            normal = json.loads((output/'natural/caller-after.json').read_text())
            for index in (0, 3):
                expected_restore['registers'][index] = normal['registers'][index]
            expected_restore['segments'][0] = normal['segments'][0]
            for name in FLAGS:
                expected_restore[name] = normal[name]
            struct.pack_into('<HH', expected_memory, owner,
                             inputs['state']['registers'][3] & 65535,
                             inputs['state']['segments'][0]['value'])
            normal_memory = (output/'natural/caller-after.memory').read_bytes()
            address = ss_base+((sp-6) & 65535)
            expected_memory[address:address+2] = normal_memory[address:address+2]
        assert restored == expected_restore and memory['restored-after'] == expected_memory
        start, stop = (row['PIC_Ticks']*30000+30000-row['CPU_CycleLeft']-row['CPU_Cycles'] for row in (rows[0], rows[-1]))
        assert stop-start == 10
        cases.append(dict(case=case, inputs=inputs, states=states, fetches=10, cycles=[start, stop],
                          measured=dict(old_offset=old_offset, old_segment=old_segment,
                                        new_offset=new_offset, new_segment=new_segment, effective_flags=flags),
                          memory_sha256={name: digest(p/(name+'.memory')) for name in labels},
                          capture_sha256={suffix: digest(p/('sequence.'+suffix)) for suffix in ('frames', 'pcm', 'end')}))
    proof = dict(scope='Actual scheduler PUSH-CS/near-CALL and keyboard FAR-CALL operand exchange. '
                 'All original GP/segments/control/raw+lazyflags and16-MiB writes, including high-word and IF variants. '
                 'Controlled inputs are explicitly restored after observation without changing clock/budget. '
                 'Source-only evidence; no port CPU/IRQ/time or full output acceptance.',
                 image_sha256=digest(ROOT/'re_out/fist_dat_image.bin'),
                 original_binary_sha256=digest(ROOT/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
                 callback_image_address=0x14628, rebased_module_segment=0x0f69,
                 code=[dict(offset=address, bytes=data.hex()) for address, data in code.items()],
                 frames=39, mixed_samples=27518, endpoint_ms=600, cases=cases,
                 reproduction='python3 -B tools/oracle/capture_callback_exchange.py --output /tmp/wasm-fist-callback-exchange',
                 script_sha256={str(p.relative_to(ROOT)): digest(p) for p in
                                (ROOT/'tools/oracle/callback_exchange.gdb', ROOT/'tools/oracle/callback_exchange_gdb.sh', Path(__file__).resolve())})
    (output/'proof.json').write_text(json.dumps(proof, indent=2)+'\n')
    print('PASS: four actual callback-exchange source cases, all architectural states and16-MiB writes; '
          '10 fetches each, original39 frames/27518 mixed samples/end600ms unchanged.')
    return proof


def capture(output):
    if not output.is_relative_to(Path('/tmp')):
        raise ValueError('disposable captures must be under /tmp')
    output.mkdir(parents=True, exist_ok=True)
    for case in ('baseline', *CASES):
        p = output/case
        if p.exists():
            assert (output/(case+'.exit')).read_text() == '0\n'
            continue
        env = {name: value for name, value in os.environ.items() if not name.startswith('FIST_') and name != 'DOSBOX'}
        env['FIST_SEQUENCE_END_MS'] = '600'
        if case != 'baseline':
            env.update(DOSBOX=str(ROOT/'tools/oracle/callback_exchange_gdb.sh'),
                       FIST_CALLBACK_OUTPUT=str(p), FIST_CALLBACK_CASE=case,
                       FIST_CALLBACK_NORMAL_AFTER=str(output/'natural/caller-after.json'))
        with (output/(case+'.log')).open('w') as log:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(p)],
                                    cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=60)
        (output/(case+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode == 0, (case, result.returncode)
    return verify(output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    try:
        (verify if args.verify_only else capture)(args.output.resolve())
    except (AssertionError, OSError, ValueError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
