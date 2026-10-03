#!/usr/bin/env python3
"""Recover original missing-detail CF and nonlocal FILEMGR service error behavior."""
import argparse,hashlib,json,os,shlex,struct,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(root,repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from sequence_format import validate,validate_endpoint
    for name in ('baseline','error'):
        assert (root/(name+'.exit')).read_text()=='0\n'
        prefix=root/name/'sequence';assert validate_endpoint(prefix,600)==600
        assert validate(str(prefix)+'.frames','F')['records']==38 and validate(str(prefix)+'.pcm','A')['samples']==27518
        assert not (root/name/'game/FISTDATA/HIGH.DTL').exists()
        assert digest(root/name/'removed.HIGH.DTL')==digest(repo/'armoredfist/FISTDATA/HIGH.DTL')
    for suffix in ('frames','pcm','end'):assert (root/'baseline'/('sequence.'+suffix)).read_bytes()==(root/'error'/('sequence.'+suffix)).read_bytes()
    folder=root/'error';assert 'Python Exception' not in (folder/'dosbox.log').read_text()
    states={p.stem:json.loads(p.read_text()) for p in folder.glob('at-*.json')}
    assert set(states)=={'at-6032','at-6044','at-5e3a','at-0f64','at-0f57','at-0f5d'}
    assert states['at-6032']['filename']=='high.dtl'
    assert states['at-6044']['cpu_regs.flags']&1 and states['at-6044']['lflags.type']==0
    assert states['at-5e3a']['registers']==states['at-6044']['registers']
    a,b,c=(states[name] for name in ('at-0f64','at-0f57','at-0f5d'))
    memory={name:(folder/(name+'.memory')).read_bytes() for name in states};assert all(len(m)==16777216 for m in memory.values())
    expected=bytearray(memory['at-0f64']);struct.pack_into('<I',expected,a['error_reason_physical'],a['registers'][2]);struct.pack_into('<H',expected,a['tcb_physical'],0xffff)
    assert memory['at-0f57']==expected and memory['at-0f5d']==memory['at-0f57']
    regs=a['registers'].copy();regs[3]=a['tcb_logical'];assert b['registers']==regs
    for key in a:
        if key not in ('registers','cpu_regs.ip.dword[0]','CPU_Cycles','task_status','error_reason'):assert a[key]==b[key],key
    assert b['task_status']==0xffff and b['error_reason']==a['registers'][2]==1
    regs=b['registers'].copy();regs[4]=b['saved_service_ESP'];assert c['registers']==regs
    for key in b:
        if key not in ('registers','cpu_regs.ip.dword[0]','CPU_Cycles'):assert b[key]==c[key],key
    cycle=lambda q:q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
    fetches=[json.loads(line) for line in (folder/'fetches.jsonl').read_text().splitlines()]
    assert [q['cpu_regs.ip.dword[0]'] for q in fetches]==[0xf6a,0xf70,0xf75,0xf57,0xf5d]
    assert [cycle(q)-cycle(a) for q in fetches]==[1,2,3,4,5]
    assert cycle(b)-cycle(a)==4 and cycle(c)-cycle(b)==1
    image=(repo/'re_out/fist_image.bin').read_bytes()
    assert image[0x38e8:0x38ed]==bytes.fromhex('e977d6ffff')
    assert image[0xf64:0xf77]==bytes.fromhex('8915820d00008b1d930c000066c703ffffebe0')
    proof={'scope':'Source-only original missing HIGH.DTL: actual CF1 at6044 and nonlocal file-error→0f64→0f57→0f5d. Exact whole16MiB reason/task writes, four error-leaf fetches/one ESP restore fetch and preserved GP/segments/raw+lazyflags/controls. Complete paired original error output is unchanged by observation. No port/error-inventory/CPU-device-time or complete original parity acceptance.',
           'original_binary_sha256':digest(repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox'),'image_sha256':digest(repo/'re_out/fist_image.bin'),'removed_asset_sha256':digest(repo/'armoredfist/FISTDATA/HIGH.DTL'),
           'code':[{'image_offset':x,'bytes':image[x:y].hex()} for x,y in ((0x6032,0x60a9),(0x5e3a,0x5e5d),(0x38e8,0x38ed),(0xf30,0xf77))],
           'states':states,'fetches':fetches,'memory_sha256':{name:digest(folder/(name+'.memory')) for name in states},'reason':1,'task_status':0xffff,'normal_loader_return_reached':False,'error_leaf_fetches':4,'stack_restore_fetches':1,'frames':38,'mixed_samples':27518,'endpoint_ms':600,
           'capture_sha256':{s:digest(root/'baseline'/('sequence.'+s)) for s in ('frames','pcm','end')},
           'script_sha256':{str(p.relative_to(repo)):digest(p) for p in (Path(__file__).resolve(),Path(__file__).resolve().with_name('file_error.gdb'),repo/'tools/oracle/capture_sequence.sh',repo/'tools/oracle/sequence_format.py')},
           'reproduction':'python3 -B tools/oracle/capture_file_error.py --output /tmp/wasm-fist-file-error-source-final'}
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original missing-file CF1, exact nonlocal reason/task writes, five real fetches and saved ESP restoration. All38 frames/27518 mixed samples/end600ms equal the unobserved error baseline.')
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    if args.verify_only:return verify(root,repo)
    if not root.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    root.mkdir(parents=True,exist_ok=False);binary=repo/'third_party/dosbox-build/dosbox-0.74-3/src/dosbox';probe=Path(__file__).resolve().with_name('file_error.gdb')
    for name,observe in [('baseline',False),('error',True)]:
        folder=root/name;folder.mkdir();wrapper=root/('dosbox-'+name)
        command='exec '+('gdb -q -batch -x '+shlex.quote(str(probe))+' --args ' if observe else '')+shlex.quote(str(binary))+' "$@"\n'
        wrapper.write_text('#!/usr/bin/env bash\nset -euo pipefail\nmv '+shlex.quote(str(folder/'game/FISTDATA/HIGH.DTL'))+' '+shlex.quote(str(folder/'removed.HIGH.DTL'))+'\n'+command);wrapper.chmod(0o755)
        env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k!='DOSBOX'}
        env.update(FIST_SEQUENCE_END_MS='600',DOSBOX=str(wrapper),FIST_DETAIL_OPERANDS_DIR=str(folder),FIST_DETAIL_REPO=str(repo))
        with (root/(name+'.log')).open('w') as log:result=subprocess.run(['bash','tools/oracle/capture_sequence.sh','1',str(folder)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=90)
        (root/(name+'.exit')).write_text(str(result.returncode)+'\n');assert result.returncode==0,(name,result.returncode)
    return verify(root,repo)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,TypeError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
