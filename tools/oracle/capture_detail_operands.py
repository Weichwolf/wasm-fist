#!/usr/bin/env python3
"""Recover actual op44 detail selection, sky setup and complete FILEMGR result operands."""
import argparse,hashlib,json,os,shlex,struct,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
CASES={'natural':{},'low':{'detail':0},'medium':{'detail':1},'high':{'detail':2},'sky-off':{'sky':0}}
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(root,repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from sequence_format import validate,validate_endpoint
    baseline=root/'baseline/sequence'
    assert (root/'baseline.exit').read_text()=='0\n' and validate_endpoint(baseline,600)==600
    assert validate(str(baseline)+'.frames','F')['records']==39 and validate(str(baseline)+'.pcm','A')['samples']==27518
    files={n:{'size':len((repo/'armoredfist/FISTDATA'/n).read_bytes()),'sha256':digest(repo/'armoredfist/FISTDATA'/n)} for n in ('LOW.DTL','MEDIUM.DTL','HIGH.DTL')}
    observed={}
    for name,control in CASES.items():
        assert (root/(name+'.exit')).read_text()=='0\n'
        folder=root/name;prefix=folder/'sequence';assert validate_endpoint(prefix,600)==600
        assert 'Python Exception' not in (folder/'dosbox.log').read_text()
        for suffix in ('frames','pcm','end'):assert Path(str(prefix)+'.'+suffix).read_bytes()==Path(str(baseline)+'.'+suffix).read_bytes(),(name,suffix)
        states={p.stem:json.loads(p.read_text()) for p in folder.glob('handler-*.json')}
        expected_names={f'handler-{i}-{stage}' for i in (1,2) for stage in ('before-control','loader','return')}
        if control:expected_names|={f'handler-{i}-after-control' for i in (1,2)}
        assert set(states)==expected_names,(name,set(states))
        memory={label:(folder/(label+'.memory')).read_bytes() for label in states}
        assert all(len(m)==16777216 for m in memory.values())
        for i in (1,2):
            stem=f'handler-{i}-';before=states[stem+'before-control'];loader=states[stem+'loader'];returned=states[stem+'return']
            assert [q['cpu_regs.ip.dword[0]'] for q in (before,loader,returned)]==[0x7660,0x6032,0x76fc]
            assert all(q['segments'][1]['value']==43 for q in (before,loader,returned))
            if control:
                after=states[stem+'after-control'];expected=bytearray(memory[stem+'before-control'])
                for key,value in control.items():expected[before['tcb_physical']+{'detail':0xd1,'sky':0xcc}[key]]=value
                assert memory[stem+'after-control']==expected,(name,i,'control memory')
                assert after==dict(before,**control),(name,i,'control architecture')
            level=control.get('detail',before['detail']);sky=control.get('sky',before['sky'])
            filename=('LOW.DTL' if level==0 else 'MEDIUM.DTL' if level==1 else 'HIGH.DTL')
            raw=(repo/'armoredfist/FISTDATA'/filename).read_bytes()
            assert loader['detail']==returned['detail']==level and loader['sky']==returned['sky']==sky
            assert loader['filename']==filename.lower() and loader['registers'][0]==0x3a20
            assert returned['registers'][:4]==[len(raw),0,0,5]
            assert returned['ext_slots']['0x937']==len(raw)
            assert returned['ext_slots']['0x3a20']==struct.unpack_from('<I',raw)[0]
            assert bytes.fromhex(returned['detail_table_hex'])==raw
            for q in (loader,returned):
                assert q['ext_slots']['0x927']==0x80b
                assert q['ext_slots']['0x3958']==(0x689a if sky else 0x6877)
                assert q['detail_mode']==(sky if sky else 1)
        observed[name]={'controls':control,'states':states,'memory_sha256':{label:digest(folder/(label+'.memory')) for label in states}}
    image=(repo/'re_out/fist_image.bin').read_bytes()
    proof={'scope':'Source-only original op44 LOW/MEDIUM/HIGH selection, default/nonzero sky setup and two actual6032 returns per case. Complete2052-byte tables, GP/segments/raw+lazyflags/controls at observed boundaries and exact control-byte memory effects; no whole loader interval write, port, error/CF or instruction/device-time acceptance.',
           'original_binary_sha256':digest(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),'image_sha256':digest(repo/'re_out/fist_image.bin'),'source_files':files,'code':[{'image_offset':a,'bytes':image[a:b].hex()} for a,b in ((0x6032,0x60a9),(0x7660,0x76fd))],
           'cases':observed,'frames':39,'mixed_samples':27518,'endpoint_ms':600,'capture_sha256':{s:digest(str(baseline)+'.'+s) for s in ('frames','pcm','end')},
           'script_sha256':{str(p.relative_to(repo)):digest(p) for p in (Path(__file__).resolve(),Path(__file__).resolve().with_name('detail_operands.gdb'),repo/'tools/oracle/capture_sequence.sh',repo/'tools/oracle/sequence_format.py')},
           'reproduction':'python3 -B tools/oracle/capture_detail_operands.py --output /tmp/wasm-fist-detail-operands-source-final'}
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: five actual op44 cases/two calls each, full detail files and returned EAX804/EBX5, sky default6877 versus689a, exact control-byte writes. All39 original frames/27518 mixed samples unchanged.')
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    if args.verify_only:return verify(root,repo)
    if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True,exist_ok=False);probe=Path(__file__).resolve().with_name('detail_operands.gdb');wrapper=root/'dosbox-probe'
    wrapper.write_text('#!/usr/bin/env bash\nexec gdb -q -batch -x '+shlex.quote(str(probe))+' --args '+shlex.quote(str(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'))+' "$@"\n');wrapper.chmod(0o755)
    for name,controls in [('baseline',{}),*CASES.items()]:
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'};env['FIST_SEQUENCE_END_MS']='600'
        if name!='baseline':
            (root/name).mkdir();env.update(DOSBOX=str(wrapper),FIST_DETAIL_OPERANDS_DIR=str(root/name),FIST_DETAIL_REPO=str(repo))
            for key,value in controls.items():env['FIST_DETAIL_'+{'detail':'LEVEL','sky':'SKY'}[key]]=str(value)
        with (root/(name+'.log')).open('w') as log:result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(root/name)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n');assert result.returncode==0,(name,result.returncode)
    return verify(root,repo)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,TypeError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
