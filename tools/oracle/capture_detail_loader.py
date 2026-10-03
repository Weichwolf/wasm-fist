#!/usr/bin/env python3
"""Recover original detail-loader DTA initialization and full EBX operand provenance."""
import argparse,hashlib,json,os
from pathlib import Path
import shlex,struct,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]

def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def verify(root,repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from cpu_trace import records
    from sequence_format import validate,validate_endpoint
    baseline=root/'baseline/sequence';assert (root/'baseline.exit').read_text()=='0\n'
    assert validate_endpoint(baseline,600)==600 and validate(str(baseline)+'.frames','F')['records']==39
    assert validate(str(baseline)+'.pcm','A')['samples']==27518
    for name in ('init','loader'):
        assert (root/(name+'.exit')).read_text()=='0\n'
        prefix=root/name/'sequence';assert validate_endpoint(prefix,600)==600
        for suffix in ('frames','pcm','end'):assert Path(str(prefix)+'.'+suffix).read_bytes()==Path(str(baseline)+'.'+suffix).read_bytes(),(name,suffix)
        assert 'Python Exception' not in (root/name/'dosbox.log').read_text()
    p=root/'init';labels=('before-load','before-store','after-store');states={n:json.loads((p/(n+'.json')).read_text()) for n in labels};memory={n:(p/(n+'.memory')).read_bytes() for n in labels}
    assert all(len(m)==16777216 for m in memory.values())
    a,b,c=(states[n] for n in labels)
    assert [q['cpu_regs.ip.dword[0]'] for q in (a,b,c)]==[0xa88,0xa8d,0xa93]
    assert all(q['segments'][1]['value']==0x2b and q['segments'][1]['base']==0x10000000 for q in (a,b,c))
    expected=a['registers'].copy();expected[2]=0x80b;assert b['registers']==c['registers']==expected
    assert a['dta_value']==b['dta_value']==0 and c['dta_value']==b['registers'][2]
    assert memory['before-load']==memory['before-store']
    expected=bytearray(memory['before-store']);struct.pack_into('<I',expected,b['dta_physical'],b['registers'][2]);assert memory['after-store']==expected
    for key in a:
        if key not in ('registers','cpu_regs.ip.dword[0]','CPU_Cycles','dta_value'):assert a[key]==b[key]==c[key],key
    def cycle(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
    assert cycle(b)-cycle(a)==cycle(c)-cycle(b)==1
    fetches=[json.loads(line) for line in (p/'fetches.jsonl').read_text().splitlines()];assert len(fetches)==2
    assert [q['cpu_regs.ip.dword[0]'] for q in fetches]==[0xa8d,0xa93]
    trace=root/'loader.trace';count=0;selected=[]
    locations={0x7660,0x7666,0x766b,0x7670,0x7695,0x769b,0x76a1,0x76c0,0x76e2,0x76e7,0x76ec,0x76fc,0x6032,0x6044,0x604a,0x6050,0x606a,0x607c,0x6083,0x6087,0x6093,0x609d,0x60a8}
    main={0x6dfd,0xde89,0x6e00,0x6e02,0x6e05,0x6e08,0xe2df}
    for row in records(trace):
        count+=1;time,(cs,ip),regs,segs=row
        if (cs==0x2b and ip in locations) or (cs==0x1119 and ip in main):selected.append(dict(cycle=time,cs=cs,ip=ip,registers=list(regs),segments=segs))
    calls=[];handlers=[]
    for row in selected:
        if row['cs']==0x2b:
            if row['ip']==0x7660:handlers.append([])
            if handlers and (not handlers[-1] or handlers[-1][-1]['ip']!=0x76fc) and row['ip'] in locations:handlers[-1].append(row)
    assert len(handlers)==2
    for handler in handlers:
        assert handler[0]['ip']==0x7660 and handler[-1]['ip']==0x76fc
        at={r['ip']:r for r in handler}
        assert at[0x766b]['registers'][3]==0xf0010000
        assert at[0x769b]['registers'][3]==0xf0010000
        assert at[0x76a1]['registers'][0]&255==4
        assert at[0x6032]['registers'][0]==0x3a20 and at[0x6032]['registers'][6]==0x76f3
        assert at[0x604a]['registers'][3]==0xf0010000 and at[0x6050]['registers'][3]==c['dta_value']==0x80b
        assert at[0x6083]['registers'][3]==5
        assert at[0x6087]['registers'][:4]==[0x804,0,0,5]
        assert at[0x60a8]['registers'][:4]==[0x804,0,0,5]
        assert at[0x76fc]['registers'][:4]==[0x804,0,0,5]
    returned=[r for r in selected if r['cs']==0x1119 and r['ip']==0x6e00]
    posted=[r for r in selected if r['cs']==0x1119 and r['ip']==0xe2df]
    assert len(returned)==1 and returned[0]['registers'][:4]==[2,0,0,5]
    assert posted and posted[0]['registers'][:4]==[2,0,0,5]
    image=(repo/'re_out/fist_image.bin').read_bytes();data=(repo/'re_out/fist_dat_image.bin').read_bytes()
    assert image[0xa88:0xa93]==bytes.fromhex('ba0b080000891527090000')
    assert image[0x604a:0x6050]==bytes.fromhex('8b1d27090000')
    proof={'scope':'Source-only actual DTA DWORD initialization and natural op44→6032→de89→op68 full EBX provenance. Init whole16MiB/GP/segments/raw+lazyflags/controls is verified; loader CPU trace lacks raw flags. No port acceptance or generic upper-word zeroing contract.',
           'original_binary_sha256':digest(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),
           'image_sha256':digest(repo/'re_out/fist_image.bin'),'data_image_sha256':digest(repo/'re_out/fist_dat_image.bin'),
           'code':[dict(image='ext',image_offset=a,bytes=image[a:b].hex()) for a,b in ((0xa88,0xa97),(0x6032,0x60a9),(0x7660,0x76fd))]+[dict(image='data',image_offset=a,bytes=data[a:b].hex()) for a,b in ((0x6de2,0x6e0e),(0xde89,0xdeb2))],
           'dta_init':{'states':states,'fetches':len(fetches),'memory_sha256':{n:digest(p/(n+'.memory')) for n in labels}},
           'loader_trace':{'sha256':digest(trace),'complete_fetches':count,'handlers':handlers,'returned':returned,'posted':posted},
           'capture_sha256':{s:digest(str(baseline)+'.'+s) for s in ('frames','pcm','end')},'frames':39,'mixed_samples':27518,'endpoint_ms':600,
           'script_sha256':{str(path.relative_to(repo)):digest(path) for path in (Path(__file__).resolve(),Path(__file__).resolve().with_name('detail_dta_init.gdb'),repo/'tools/oracle/cpu_trace.py',repo/'tools/oracle/capture_sequence.sh')},
           'reproduction':'python3 -B tools/oracle/capture_detail_loader.py --output /tmp/wasm-fist-detail-loader-source'}
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original DTA init whole16MiB/raw+lazyflags; actual op44 FILEMGR clears old EBX upper word by DWORD DTA load, then retains saved WORD handle; 39frames/27518PCM unchanged.')
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args()
    repo=args.repo.resolve(strict=True);root=args.output.resolve()
    if args.verify_only:return verify(root,repo)
    if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True,exist_ok=False)
    for name in ('baseline','init','loader'):
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'};env['FIST_SEQUENCE_END_MS']='600'
        if name=='init':
            (root/name).mkdir();probe=Path(__file__).resolve().with_name('detail_dta_init.gdb');binary=repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox';wrapper=root/'dosbox-init'
            wrapper.write_text('#!/usr/bin/env bash\nexec gdb -q -batch -x '+shlex.quote(str(probe))+' --args '+shlex.quote(str(binary))+' "$@"\n');wrapper.chmod(0o755)
            env.update(DOSBOX=str(wrapper),FIST_DETAIL_INIT_DIR=str(root/name),FIST_DETAIL_REPO=str(repo))
        elif name=='loader':env.update(FIST_CPU_TRACE_WINDOW='524:526',FIST_CPU_TRACE=str(root/'loader.trace'))
        with (root/(name+'.log')).open('w') as log:result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(root/name)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=60)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n');assert result.returncode==0,(name,result.returncode)
    return verify(root,repo)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
