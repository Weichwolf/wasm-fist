#!/usr/bin/env python3
"""Observe complete missing-detail output and independent tick120 port memory diagnostics."""
import argparse,hashlib,json,os,shlex,shutil,struct,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(output,repo,original):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from sequence_format import validate,validate_endpoint
    from compare_sequences import compare
    assert validate_endpoint(original,600)==600
    assert validate(str(original)+'.frames','F')['records']==38
    assert validate(str(original)+'.pcm','A')['samples']==27518
    source=json.loads((repo/'tools/oracle/file_error_case.json').read_text())
    for suffix,sha in source['capture_sha256'].items():assert digest(str(original)+'.'+suffix)==sha,suffix
    producers=json.loads((output/'producers.json').read_text())
    for path,sha in producers.items():assert digest(path)==sha,path
    cases=[]
    for target in ('native','wasm'):
        assert (output/(target+'.exit')).read_text()==(output/(target+'-startup.exit')).read_text()=='0\n'
        prefix=output/target/'sequence';assert validate_endpoint(prefix,600)==600
        assert not (output/target/'game/FISTDATA/HIGH.DTL').exists()
        assert digest(output/target/'game/HIGH.DTL.removed')==source['removed_asset_sha256']
        memory=(output/(target+'-startup.memory')).read_bytes();assert len(memory)==16777216
        tick=struct.unpack_from('<H',memory,0x1c452)[0];assert tick==120
        offset,segment=struct.unpack_from('<HH',memory,0x2aa2c);task=(segment<<4)+offset
        status=struct.unpack_from('<H',memory,task)[0];reason=struct.unpack_from('<I',memory,0x100000+0xd82)[0]
        cases.append({'target':target,'frames_600ms':validate(str(prefix)+'.frames','F'),'first_original_frame_error':compare(original,prefix,'F'),'endpoint_ms':600,'task':{'segment':segment,'offset':offset,'address':task,'actual_status':status,'source_error_status':source['task_status']},'actual_module_error_reason':reason,'source_module_error_reason':source['reason'],'stored_DTA_DWORD':struct.unpack_from('<I',memory,0x100000+0x927)[0],'memory_diagnostic_tick':tick,'memory_sha256':digest(output/(target+'-startup.memory')),'frame_sha256':digest(str(prefix)+'.frames'),'nonlocal_error_status_reason_match':status==source['task_status'] and reason==source['reason']})
    proof={'scope':'Complete600-ms missing-HIGH.DTL port frame streams versus the original error case. Independent cooperative tick120 memory diagnostics do not match the original CPU/memory boundary. Observed states only: no corrected error/GP/flags/stack/IRQ/time or complete frame/PCM acceptance.','source_receipt_sha256':digest(repo/'tools/oracle/file_error_case.json'),'producers':producers,'cases':cases,'complete_original_acceptance':False}
    (output/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(cases,indent=2))
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=ROOT);parser.add_argument('--native',type=Path);parser.add_argument('--wasm',type=Path);parser.add_argument('--node',default='node');parser.add_argument('--original',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--verify-only',action='store_true');args=parser.parse_args();repo=args.repo.resolve(strict=True);output=args.output.resolve();original=args.original.resolve()
    if args.verify_only:return verify(output,repo,original)
    if args.native is None or args.wasm is None:parser.error('--native and --wasm are required for capture')
    if not output.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    native=args.native.resolve(strict=True);wasm=args.wasm.resolve(strict=True);node=Path(shutil.which(args.node) or args.node).resolve(strict=True);output.mkdir(parents=True,exist_ok=False)
    wrappers={}
    for name,command in [('native',[str(native)]),('node',[str(node)])]:
        path=output/(name+'-wrapper');path.write_text('#!/usr/bin/env bash\nset -euo pipefail\nmv "$FIST_DATADIR/FISTDATA/HIGH.DTL" "$FIST_DATADIR/HIGH.DTL.removed"\nexec '+shlex.join(command)+' "$@"\n');path.chmod(0o755);wrappers[name]=path
    prefix=output/'start-state'
    import base64,gzip
    for suffix in ('text','bda'):Path(str(prefix)+'.'+suffix).write_bytes(gzip.decompress(base64.b64decode((repo/('tools/oracle/start_state.'+suffix+'.gz.b64')).read_bytes())))
    shutil.copyfile(repo/'tools/oracle/start_state.vga',Path(str(prefix)+'.vga'))
    producers={str(p):digest(p) for p in (native,wasm,wasm.with_suffix('.wasm'),node,*wrappers.values(),Path(__file__).resolve(),repo/'tools/capture_port_sequence.sh',repo/'tools/oracle/file_error_case.json',*[Path(str(prefix)+'.'+s) for s in ('text','bda','vga')])};(output/'producers.json').write_text(json.dumps(producers,indent=2)+'\n')
    for target in ('native','wasm'):
        for diagnostic in (False,True):
            name=target+('-startup' if diagnostic else '')
            env={k:v for k,v in os.environ.items() if not k.startswith('FIST_') and k not in ('NATIVE','NODE','OUTJS')};env.update(FIST_SB='1',FIST_TEXT_STATE=str(prefix),NATIVE=str(wrappers['native']),NODE=str(wrappers['node']),OUTJS=str(wasm))
            if diagnostic:env['FIST_MEMDUMP']=str(output/(name+'.memory'))
            else:env['FIST_SEQUENCE_END_MS']='600'
            with (output/(name+'.log')).open('w') as log:result=subprocess.run(['bash','tools/capture_port_sequence.sh',target,'120' if diagnostic else '0',str(output/name)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=240)
            (output/(name+'.exit')).write_text(str(result.returncode)+'\n');assert result.returncode==0,(name,result.returncode)
    return verify(output,repo,original)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,TypeError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
