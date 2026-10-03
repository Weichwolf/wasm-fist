#!/usr/bin/env python3
"""Recover the original77e2 DSOUNDS load and checkpoint before the first KDV lookup."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def verify(root, repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from sequence_format import validate,validate_endpoint
    producers=json.loads((root/'producers.json').read_text())
    for path,sha in producers.items():assert digest(path)==sha,path
    for name in ('baseline','observed'):
     assert (root/(name+'.exit')).read_text()=='0\n'
     assert 'Python Exception' not in (root/name/'dosbox.log').read_text()
     p=root/name/'sequence';assert validate_endpoint(p,600)==600
     assert validate(str(p)+'.frames','F')['records']==39
     assert validate(str(p)+'.pcm','A')['samples']==27518
    for s in ('frames','pcm','end'):assert (root/'baseline'/('sequence.'+s)).read_bytes()==(root/'observed'/('sequence.'+s)).read_bytes(),s
    expected={'device-start-entry':0x77e2,'after-device-configuration':0x77ee,'after-checkpoint-free':0x780e,'after-size-query':0x781a,'allocation-entry':0x36bf,'allocation-return':0x7832,'after-bank-read':0x7841,'after-checkpoint':0x7858,'device-start-return':0x7869,'kdv-entry':0x6e95}
    states={p.stem:json.loads(p.read_text()) for p in (root/'observed').glob('*.json')};assert set(states)==set(expected)
    def read(q,m,a,n):
     out=bytearray()
     while n:
      linear=(q['segments'][3]['base']+a)&0xffffffff
      if q['paging.enabled']:
       d=struct.unpack_from('<I',m,(q['paging.cr3']&0xfffff000)+(linear>>22)*4)[0];assert d&1
       if d&128:p=(d&0xffc00000)|(linear&0x3fffff)
       else:
        t=struct.unpack_from('<I',m,(d&0xfffff000)+((linear>>12)&1023)*4)[0];assert t&1
        p=(t&0xfffff000)|(linear&4095)
      else:p=linear
      k=min(n,4096-(linear&4095));out+=m[p:p+k];a+=k;n-=k
     return bytes(out)
    image=(repo/'re_out/fist_image.bin').read_bytes()
    mem={}
    for label,q in states.items():
     assert q['cpu_regs.ip.dword[0]']==expected[label] and q['segments'][1]['value']==states['device-start-entry']['segments'][1]['value']
     m=(root/'observed'/(label+'.memory')).read_bytes();assert len(m)==16777216;mem[label]=m
     assert q['tcb_physical']==q['engine_task']['physical']
     code=q['segments'][1]['base']-q['segments'][3]['base']+expected[label]
     assert read(q,m,code,32)==image[expected[label]:expected[label]+32],label
    image=(repo/'re_out/fist_image.bin').read_bytes();assert image[0x85a4:0x85b0]==b'dsounds.bin\0'
    assert [b['slot'] for b in states['after-checkpoint-free']['blocks']]==[b['slot'] for b in states['device-start-entry']['blocks'][:-1]]
    assert states['device-start-entry']['blocks'][-1]['slot']==0xbc98
    assert states['after-checkpoint-free']['memmgr_slots']['0xbc98']==0
    bank=(repo/'armoredfist/FISTDATA/DSOUNDS.BIN').read_bytes()
    allocation=states['allocation-entry'];assert allocation['registers'][1:4]==[len(bank),0x85b0,4]
    assert allocation['registers'][0]&255==3 and allocation['stack_return']==0x7832
    returned=states['allocation-return'];assert returned['blocks'][:-1]==allocation['blocks']
    b=returned['blocks'][-1];assert b['slot']==0x85b0 and b['size']==len(bank) and b['alignment']==4 and b['flags']==3 and b['address']==b['slot_value']
    loaded=states['after-bank-read'];assert loaded['blocks']==returned['blocks']
    assert read(loaded,mem['after-bank-read'],b['address'],len(bank))==bank
    for label in ('after-size-query','after-bank-read'):
     assert struct.unpack('<I',read(states[label],mem[label],0x937,4))[0]==len(bank)
     assert states[label]['registers'][0]==len(bank)
    checkpoint=states['after-checkpoint'];assert checkpoint['blocks'][:-1]==loaded['blocks']
    assert checkpoint['blocks'][-1]['slot']==0xbc98 and checkpoint['blocks'][-1]['size']==0
    assert checkpoint['memmgr_slots']['0xbc98']==b['address']+len(bank)
    assert states['kdv-entry']['blocks']==states['device-start-return']['blocks']==checkpoint['blocks']
    assert read(states['device-start-return'],mem['device-start-return'],0x77e0,1)==b'\x01'
    proof=dict(scope='Source-only actual77e2 device-start boundaries, original DSOUNDS size query, allocator entry/return, complete bank load and checkpoint relocation before first KDV. The85b0 block is DSOUNDS, disproving the prior sky attribution. Full GP/segments/raw+lazyflags/control/time and whole16MiB snapshots are retained at ten boundaries; every reached code boundary is checked against the original image with guest paging. No port service/device/IRQ/time or complete original output acceptance.',original_binary_sha256=digest(repo/'third_party/dosbox-fist'),image_sha256=digest(repo/'re_out/fist_image.bin'),code=[dict(image_offset=a,bytes=image[a:z].hex()) for a,z in [(0x77e2,0x786a),(0x85a4,0x85b0),(0x36bf,0x38e8),(0x1280,0x12b3)]],bank_sha256=digest(repo/'armoredfist/FISTDATA/DSOUNDS.BIN'),bank_bytes=len(bank),allocation=b,states=states,memory_sha256={label:digest(root/'observed'/(label+'.memory')) for label in states},capture_sha256={s:digest(root/'baseline'/('sequence.'+s)) for s in ('frames','pcm','end')},frames=39,mixed_samples=27518,endpoint_ms=600,producers=producers,reproduction='python3 -B tools/oracle/capture_sound_bank_startup.py --output /tmp/wasm-fist-sound-bank-startup-source',complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: original77e2 loads complete DSOUNDS and relocates checkpoint; ten original boundaries and all39frame/27518PCM/end600 unchanged.')

    return proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    repo, root = args.repo.resolve(strict=True), args.output.resolve()
    if args.verify_only:
        return verify(root, repo)
    if not root.is_relative_to(Path('/tmp')):
        parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True, exist_ok=False)
    binary = repo/'third_party/dosbox-fist'
    probe = Path(__file__).resolve().with_name('sound_bank_startup.gdb')
    files=(binary,probe,Path(__file__).resolve(),repo/'tools/oracle/memmgr_startup.gdb',repo/'tools/oracle/capture_sequence.sh',repo/'tools/oracle/sequence_format.py',repo/'re_out/fist_image.bin',repo/'armoredfist/FISTDATA/DSOUNDS.BIN')
    (root/'producers.json').write_text(json.dumps({str(p):digest(p) for p in files},indent=2)+'\n')
    for name, observe in [('baseline', False), ('observed', True)]:
        folder = root/name
        folder.mkdir()
        wrapper = root/('dosbox-'+name)
        command = (['gdb', '-q', '-batch', '-x', str(probe), '--args'] if observe else [])+[str(binary)]
        wrapper.write_text('#!/usr/bin/env bash\nexec '+shlex.join(command)+' "$@"\n')
        wrapper.chmod(0o755)
        env = {k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k != 'DOSBOX'}
        env.update(DOSBOX=str(wrapper), FIST_SEQUENCE_END_MS='600',
                   FIST_MEMMGR_SOURCE_DIR=str(folder), FIST_MEMMGR_SOURCE_REPO=str(repo))
        with (root/(name+'.log')).open('w') as log:
            result = subprocess.run(['bash', 'tools/oracle/capture_sequence.sh', '1', str(folder)],
                                    cwd=repo, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n')
        assert result.returncode == 0, (name, result.returncode)
    return verify(root, repo)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, TypeError, subprocess.SubprocessError) as error:
        sys.exit('FAIL: '+str(error))
