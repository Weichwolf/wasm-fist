#!/usr/bin/env python3
"""Capture source-backed task-mode inputs without retaining a repository scratch tree."""
import argparse
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


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(root):
    baseline = root/'baseline'
    assert validate_endpoint(baseline/'sequence', 600) == 600
    image = (ROOT/'re_out/fist_dat_image.bin').read_bytes()
    flags = ('cpu_regs.flags', 'lflags.type', 'lflags.prev_type', 'lflags.oldcf',
             'lflags.var1.dword[0]', 'lflags.var2.dword[0]', 'lflags.res.dword[0]')
    controls = ('cpu.cr0', 'paging.cr3', 'cpu.code.big', 'cpu.stack.big', 'cpu.pmode', 'cpu.cpl')
    cases = []
    for name in ('natural', 'nonzero', 'exchange', 'null'):
        p = root/name
        assert (root/(name+'.exit')).read_text() == '0\n'
        assert validate_endpoint(p/'sequence', 600) == 600
        assert validate(str(p/'sequence.frames'), 'F')['records'] == 39
        assert validate(str(p/'sequence.pcm'), 'A')['samples'] == 27518
        for suffix in ('frames', 'pcm', 'end'):
            assert (p/('sequence.'+suffix)).read_bytes() == (baseline/('sequence.'+suffix)).read_bytes()
        inputs = json.loads((p/'inputs.json').read_text())
        labels = ('get-entry', 'get-return', 'put-entry', 'put-return', 'restored-return')
        states = {label: json.loads((p/(label+'.json')).read_text()) for label in labels}
        memory = {label: (p/(label+'.memory')).read_bytes() for label in labels}
        assert all(len(m) == 16777216 for m in memory.values())
        a,b,c,d,r = (states[label] for label in labels)
        ma,mb,mc,md,mr = (memory[label] for label in labels)
        ss = a['segments'][2]['base'];cs = a['segments'][1]['base']
        mode = cs+0x2d59;assert mode-inputs['load_bias'] == 0x123e9
        assert inputs['eax'] == 0xea and inputs['mode'] == 0 and inputs['neighbor'] == 0
        code = cs+0x2d2a
        assert ma[code:code+37] == image[0x123ba:0x123df]
        if name == 'natural':
            assert ma[code:mode+8] == image[0x123ba:0x123f1]
        assert mb == ma
        expected = a['registers'].copy()
        expected[0] = (expected[0]&0xffffff00)|ma[mode]
        expected[4] = (expected[4]&0xffff0000)|((expected[4]+4)&65535)
        assert b['registers'] == expected
        expected_segments = [segment.copy() for segment in a['segments']]
        expected_segments[1] = dict(value=0x1119, base=inputs['load_bias'])
        assert b['segments'] == expected_segments
        for key in flags+controls:
            assert a[key] == b[key], (name, key)
        # The actual far caller publishes the getter's AL. Only the independent
        # setter-input cases alter it at the reached callee boundary.
        expected = b['registers'].copy()
        if name in ('exchange','null'):expected[0] = (expected[0]&0xffffff00)|0xa5
        expected[4] = (expected[4]&0xffff0000)|((expected[4]-4)&65535)
        assert c['registers'] == expected and c['segments'] == a['segments']
        for key in flags+controls:assert c[key] == b[key], (name,key)
        expected_memory = bytearray(mb);sp=b['registers'][4]&65535
        struct.pack_into('<H', expected_memory, ss+((sp-2)&65535), 0x1119)
        struct.pack_into('<H', expected_memory, ss+((sp-4)&65535), 0xda4f)
        if name=='null':expected_memory[inputs['pointer_address']:inputs['pointer_address']+4] = b'\0'*4
        assert mc == expected_memory
        off,seg = struct.unpack_from('<HH',mc,ss+0xea2c)
        expected_memory = bytearray(mc);sp=c['registers'][4]&65535
        struct.pack_into('<H',expected_memory,ss+((sp-2)&65535),c['segments'][0]['value'])
        struct.pack_into('<H',expected_memory,ss+((sp-4)&65535),c['registers'][7]&65535)
        if seg|off:expected_memory[(seg<<4)+((off+0x496)&65535)] = c['registers'][0]&255
        expected_memory[mode] = c['registers'][0]&255
        assert md == expected_memory
        expected = c['registers'].copy();expected[0]=(expected[0]&0xffffff00)|mc[mode]
        expected[4] = (expected[4]&0xffff0000)|((expected[4]+4)&65535)
        assert d['registers'] == expected and d['segments'] == b['segments']
        for key in controls:assert c[key] == d[key]
        assert c['lflags.type'] == d['lflags.type'] == 0
        for key in ('lflags.prev_type','lflags.oldcf'):assert c[key] == d[key], (name,key)
        # Original WORD OR leaves the high union words intact; RETF fills flags.
        for key,value in [('lflags.var1.dword[0]',seg),('lflags.var2.dword[0]',off),('lflags.res.dword[0]',seg|off)]:
            assert d[key] == (c[key]&0xffff0000)|value, (name,key)
        assert d['cpu_regs.flags'] == 0x3246 if name=='null' else d['cpu_regs.flags'] == 0x3206
        rows = [json.loads(line) for line in (p/'fetches.jsonl').read_text().splitlines()]
        assert len(rows) == (13 if name=='null' else 15)
        first = a['PIC_Ticks']*30000+30000-a['CPU_Cycles']-a['CPU_CycleLeft']
        last = d['PIC_Ticks']*30000+30000-d['CPU_Cycles']-d['CPU_CycleLeft']
        assert last-first == len(rows)-1
        assert r['PIC_Ticks'] == d['PIC_Ticks'] and r['CPU_Cycles'] == d['CPU_Cycles'] and r['CPU_CycleLeft'] == d['CPU_CycleLeft']
        expected_memory = bytearray(md)
        expected_memory[mode:mode+2] = bytes((inputs['mode'],inputs['neighbor']))
        expected_memory[inputs['pointer_address']:inputs['pointer_address']+4] = bytes.fromhex(inputs['pointer'])
        expected_memory[inputs['task_address']] = inputs['task_mode']
        assert mr == expected_memory
        assert r['registers'][0] == 0
        if name=='null':
            normal = json.loads((root/'natural/put-return.json').read_text())
            for key in flags:assert r[key] == normal[key]
        cases.append(dict(case=name,scope='Actual getter entry through original caller/setter return. Controlled inputs and restored architectural test state; no clock or budget adjustment.',
                          inputs=inputs,states=states,fetches=len(rows)-1,cycles=[first,last],
                          measured=dict(mode_before_get=ma[mode],mode_neighbor=ma[mode+1],getter_byte=b['registers'][0]&255,setter_input_byte=c['registers'][0]&255,setter_return_byte=d['registers'][0]&255,task_pointer=dict(offset=off,segment=seg)),
                          memory_sha256={label:digest(p/(label+'.memory')) for label in labels},
                          capture_sha256={suffix:digest(p/('sequence.'+suffix)) for suffix in ('frames','pcm','end')}))
    proof = dict(scope='Original byte/CS/pointer and real getter-to-setter data contract, complete states and16MiB interval comparisons. Complete600-ms source output retained after control restoration. No port CPU/flags/stack/time or full original output acceptance.',
                 image_sha256=digest(ROOT/'re_out/fist_dat_image.bin'),original_binary_sha256=digest(ROOT/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
                 frames=39,mixed_samples=27518,endpoint_ms=600,cases=cases,
                 reproduction='python3 -B tools/oracle/capture_task_mode.py --output /tmp/wasm-fist-task-source',
                 script_sha256={str(p.relative_to(ROOT)):digest(p) for p in [ROOT/'tools/oracle/task_mode.gdb',ROOT/'tools/oracle/task_mode_gdb.sh',Path(__file__).resolve()]})
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('Original natural/nonzero/independent/null task-mode paths: all complete states and16MiB writes exact;600-ms frame/PCM/end bytes retain unobserved original.14/14/14/12 fetches; clock unchanged by restoration.')
    return proof


def capture(output):
    output.mkdir(parents=True, exist_ok=True)
    for name in ('baseline','natural','nonzero','exchange','null'):
        p=output/name
        if p.exists():
            # A completed existing observation is validated below, rather than overwritten.
            assert (output/(name+'.exit')).read_text()=='0\n'
            continue
        environment=os.environ.copy()
        environment['FIST_SEQUENCE_END_MS']='600'
        if name!='baseline':
            environment.update(DOSBOX=str(ROOT/'tools/oracle/task_mode_gdb.sh'),
                               FIST_TASK_MODE_OUTPUT=str(p),FIST_TASK_MODE_CASE=name,
                               FIST_TASK_MODE_NORMAL_RETURN=str(output/'natural/put-return.json'))
        with (output/(name+'.log')).open('wb') as log:
            result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(p)],cwd=ROOT,env=environment,stdout=log,stderr=subprocess.STDOUT,timeout=60)
        (output/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode==0,(name,result.returncode)
    return verify(output)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args()
    (verify if args.verify_only else capture)(args.output.resolve())
