#!/usr/bin/env python3
"""Capture full controlled1280 CPU/RAM states; restore inputs before the original caller resumes."""
import argparse
import ast
import copy
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
from verify_device_start_prefix import digest, producer_paths
from sequence_format import validate, validate_endpoint


def verify(root, repo, baseline):
    assert (root/'source.exit').read_text() == '0\n'
    folder = root/'source'
    assert 'Python Exception' not in (folder/'dosbox.log').read_text()
    for p, h in json.loads((root/'producers.json').read_text()).items():
        assert digest(p) == h, p
    prefix = folder/'sequence'
    assert validate_endpoint(prefix, 600) == 600
    assert validate(str(prefix)+'.frames', 'F')['records'] == 39
    assert validate(str(prefix)+'.pcm', 'A')['samples'] == 27518
    for suffix in ('frames', 'pcm', 'end'):
        assert Path(str(prefix)+'.'+suffix).read_bytes() == (baseline/'baseline'/('sequence.'+suffix)).read_bytes()
    for suffix in ('text', 'bda', 'vga'):
        assert (folder/('start-state.'+suffix)).read_bytes() == (baseline/'baseline'/('start-state.'+suffix)).read_bytes()
    originals = json.loads((baseline/'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()} == set(originals)
    for p, h in originals.items():
        assert digest(repo/p) == h, p
    rows = [json.loads(s) for s in (folder/'reset-fetches.jsonl').read_text().splitlines()]
    widths = json.loads((repo/'tools/oracle/device_config_1280_widths_case.json').read_text())
    assert len(rows) == 9
    original = json.loads((baseline/'proof.json').read_text())['fetches'][2:11]
    plain = lambda q: {k:v for k,v in q.items() if k not in ('entry_return_ip','fetched_code_physical','fetched_code_hex')}
    before = json.loads((folder/'unmodified-entry.json').read_text())
    restored = json.loads((folder/'restored-return.json').read_text())
    assert before == plain(original[0]) and restored == plain(original[-1])
    unmodified = (folder/'unmodified-entry.memory').read_bytes()
    assert unmodified == (folder/'restored-return.memory').read_bytes()
    shared = repo/'tools/oracle/file_error.gdb'
    program = shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    nodes = [n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name == 'physical']
    ns = {'struct':struct}
    exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),ns)
    physical = ns['physical']
    address = lambda q,s,a: physical((q['segments'][s]['base']+a)&0xffffffff,unmodified,q)
    memory = bytearray(unmodified)
    tcb = struct.unpack_from('<I',memory,address(before,3,0xc93))[0]
    p = address(before,3,tcb+0x490);memory[p:p+6] = struct.pack('<HHH',*widths['packet_words'])
    p = address(before,3,0x12ce);memory[p:p+2] = b'\x5a\xc3'
    assert memory == (folder/'1280.memory').read_bytes()
    initial = plain(rows[0]);expected = copy.deepcopy(before)
    expected['registers'][0] = widths['input_eax'];expected['registers'][3] = widths['input_ebx']
    assert initial == expected
    return_ip = struct.unpack_from('<I',memory,address(before,2,before['registers'][4]&before['cpu.stack.mask']))[0]
    assert return_ip == 0x77ee
    image = (repo/'re_out/fist_image.bin').read_bytes()
    live_code_memory=bytearray(memory)
    for i,q in enumerate(rows):
        expected = copy.deepcopy(initial)
        if i < 8:
            old = widths['instructions'][i]
            expected['cpu_regs.ip.dword[0]'] = old['ip']
            expected['registers'][0] = old['eax'];expected['registers'][3] = old['ebx']
        else:
            expected['cpu_regs.ip.dword[0]'] = return_ip
            expected['registers'][0] = widths['return_eax'];expected['registers'][3] = widths['return_ebx']
            expected['registers'][4] += 4
        expected['CPU_Cycles'] -= i
        assert plain(q) == expected, i
        if i==3:
            p=address(before,3,0x12cc);live_code_memory[p:p+2]=struct.pack('<H',widths['packet_words'][0])
        if i==5:
            p=address(before,3,0x12c4);live_code_memory[p:p+4]=struct.pack('<I',widths['packet_words'][1])
        if i==7:
            p=address(before,3,0x12c8);live_code_memory[p:p+4]=struct.pack('<I',widths['packet_words'][2])
        ip = q['cpu_regs.ip.dword[0]'];p = address(q,1,ip)
        assert q['entry_return_ip'] == return_ip
        assert q['fetched_code_physical'] == p and bytes.fromhex(q['fetched_code_hex']) == live_code_memory[p:p+32]
    after = bytearray(memory)
    for field in widths['after']:
        p = address(before,3,field['offset']);data = bytes.fromhex(field['hex']);after[p:p+len(data)] = data
    assert len(memory) == len(after) == 16777216
    assert after == (folder/'12ab.memory').read_bytes() == (folder/'77ee.memory').read_bytes()
    result = dict(scope='Controlled original1280 full nine-state caller/RET fixture. Only documented EAX/EBX, three TCB words and adjacent guards change; full input/RAM is restored before caller77ee executes. No hardware-variant or production CPU integration acceptance.',fetches=rows,return_ip=return_ip,original_frames=39,original_mixed_samples=27518,endpoint_ms=600,memory_sha256={p.name:digest(p) for p in folder.glob('*.memory')},producers=json.loads((root/'producers.json').read_text()),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS: nine controlled full CPU states, complete16MiB memory and restored original39frame/27518PCM/end600 output')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--baseline',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true')
    args = parser.parse_args();repo = args.repo.resolve(strict=True);baseline = args.baseline.resolve(strict=True);root = args.output.resolve()
    if args.verify_only:return verify(root,repo,baseline)
    assert root.is_relative_to(Path('/tmp'));root.mkdir(parents=True,exist_ok=False)
    probe = Path(__file__).resolve().with_name('device_config_cpu.gdb')
    producers = (*producer_paths(repo),Path(__file__).resolve(),probe,repo/'tools/oracle/device_config_1280_widths_case.json',repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/paging.cpp',repo/'third_party/dosbox-build/dosbox-0.74-3/include/paging.h',baseline/'proof.json')
    (root/'producers.json').write_text(json.dumps({str(p):digest(p) for p in producers},indent=2)+'\n')
    folder = root/'source';folder.mkdir();wrapper = root/'dosbox-source'
    command = ['gdb','-q','-batch','-x',str(probe),'--args',str(repo/'third_party/dosbox-fist')]
    wrapper.write_text('#!/usr/bin/env bash\nexec '+shlex.join(command)+' "$@"\n');wrapper.chmod(0o755)
    env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
    env.update(DOSBOX=str(wrapper),FIST_SEQUENCE_END_MS='600',FIST_DETAIL_REPO=str(repo),FIST_DETAIL_OPERANDS_DIR=str(folder))
    with (root/'source.log').open('w') as log:
        result = subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(folder)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90)
    (root/'source.exit').write_text(str(result.returncode)+'\n');assert result.returncode == 0
    return verify(root,repo,baseline)


if __name__ == '__main__':main()
