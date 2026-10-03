#!/usr/bin/env python3
"""Recover op44's actual return and the two following configuration WORD inputs."""
import argparse,hashlib,json,os
from pathlib import Path
import subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(root,repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from cpu_trace import records
    from sequence_format import validate,validate_endpoint
    for name in ('baseline','caller'):
        assert (root/(name+'.exit')).read_text()=='0\n'
        prefix=root/name/'sequence';assert validate_endpoint(prefix,600)==600
        assert validate(str(prefix)+'.frames','F')['records']==39 and validate(str(prefix)+'.pcm','A')['samples']==27518
    for suffix in ('frames','pcm','end'):assert (root/'caller'/('sequence.'+suffix)).read_bytes()==(root/'baseline'/('sequence.'+suffix)).read_bytes(),suffix
    complete=list(records(root/'caller.trace'));selected=[];active=False
    for cycle,(cs,ip),regs,segs in complete:
        if cs==0x2b and ip==0x76fc and not selected:active=True
        if active:selected.append(dict(cycle=cycle,cs=cs,ip=ip,registers=list(regs),segments=segs))
        if active and cs==0x1119 and ip==0xe2df:break
    assert selected and selected[-1]['cs']==0x1119 and selected[-1]['ip']==0xe2df
    at={(r['cs'],r['ip']):r for r in selected}
    for loc in ((0x2b,0x76fc),(0x1119,0xe34c),(0x1119,0xdea5),(0x1119,0x6dfd)):
        assert at[loc]['registers'][:4]==[0x804,0,0,5],loc
    states={label:at[0x1119,ip] for label,ip in [('de89-return',0x6dfd),('first-config-loaded',0x6e00),('first-config-shifted',0x6e02),('second-config-load',0x6e05),('second-config-loaded',0x6e08),('second-config-shifted',0x6e0a),('op68-entry',0xe2df)]}
    assert [states[n]['registers'][0] for n in states]==[0x804,2,1,1,4,2,2]
    assert all(states[n]['registers'][1:4]==[0,0,5] for n in states)
    image=(repo/'re_out/fist_dat_image.bin').read_bytes();assert image[0x6dfd:0x6e02]==bytes.fromhex('a1498bd1e8') and image[0x6e05:0x6e0a]==bytes.fromhex('a14b8bd1e8')
    proof={'scope':'Source-only natural op44/de89 EAX804 and independently loaded/shifted AX configuration values. EBX5 persists. Complete bounded CPU trace has GP/segments/time but no raw flags or whole memory. No port register/CPU/stack/IRQ/time or full output acceptance.',
           'correction':'The older detail_loader_case returned row at6e00 is after MOV AX,[8b49], not the de89 return. Actual return at6dfd retains804. The op68 input2 comes from MOV AX,[8b4b] then SHR AX,1.',
           'original_binary_sha256':digest(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),'data_image_sha256':digest(repo/'re_out/fist_dat_image.bin'),'code':[dict(image_offset=a,bytes=image[a:b].hex()) for a,b in ((0x6de2,0x6e0e),(0xde89,0xdea6),(0xe339,0xe36d))],
           'trace_sha256':digest(root/'caller.trace'),'complete_fetches':len(complete),'handler_to_op68_fetches':len(selected),'states':states,'handler_to_op68':selected,
           'frames':39,'mixed_samples':27518,'endpoint_ms':600,'capture_sha256':{s:digest(root/'baseline'/('sequence.'+s)) for s in ('frames','pcm','end')},
           'script_sha256':{str(p.relative_to(repo)):digest(p) for p in (Path(__file__).resolve(),repo/'tools/oracle/cpu_trace.py',repo/'tools/oracle/capture_sequence.sh')},
           'reproduction':'python3 -B tools/oracle/capture_detail_return.py --output /tmp/wasm-fist-detail-return-source'}
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: actual de89 EAX804; first config WORD2→1, second WORD4→2 before op68; fullEBX5 unchanged. All39 originalframes/27518PCM unchanged.')
    return proof
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args();root=args.output.resolve();repo=args.repo.resolve(strict=True)
    if args.verify_only:return verify(root,repo)
    if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True,exist_ok=False)
    for name in ('baseline','caller'):
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'};env['FIST_SEQUENCE_END_MS']='600'
        if name=='caller':env.update(FIST_CPU_TRACE_WINDOW='524:526',FIST_CPU_TRACE=str(root/'caller.trace'))
        with (root/(name+'.log')).open('w') as log:r=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(root/name)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=60)
        (root/(name+'.exit')).write_text(str(r.returncode)+'\n');assert r.returncode==0
    return verify(root,repo)
if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
