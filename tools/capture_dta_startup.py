#!/usr/bin/env python3
"""Observe the actual module DTA initialization and both ports at cooperative tick120."""
import argparse,base64,gzip,hashlib,json,os,re,shutil,struct,subprocess,sys
from pathlib import Path

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def verify(output,repo):
    fixture=json.loads((repo/'tools/oracle/detail_loader_case.json').read_text())
    expected=fixture['dta_init']['states']['after-store']['dta_value']
    image=(repo/'re_out/fist_image.bin').read_bytes()
    assert hashlib.sha256(image).hexdigest()==fixture['image_sha256']
    producers=json.loads((output/'producers.json').read_text())
    for path,sha in producers.items():assert digest(path)==sha,path
    native=output/'native-init'
    assert (native/'gdb.exit').read_text()=='0\n'
    assert 'Python Exception' not in (native/'probe.log').read_text()
    initial=json.loads((native/'state.json').read_text())
    assert initial['ready']==1
    assert initial['stored_DTA_DWORD']==expected,initial
    assert initial['reserved_module_DTA_hex']==image[expected:expected+128].hex()
    assert initial['original_initializer_code_hex']==image[0xa88:0xa93].hex()
    cases=[]
    for target in ('native','wasm'):
        folder=output/target
        assert (folder/'exit').read_text()=='0\n'
        log=(folder/'port.log').read_text()
        assert 'FIST_DUMPTICK: dumping frame' in log
        module=int(re.search(r'KDV module loaded @g_mem\+0x([0-9a-f]+)',log)[1],16)
        assert module==initial['module_offset']
        memory=(folder/'startup.memory').read_bytes();assert len(memory)==16777216
        value=struct.unpack_from('<I',memory,module+0x927)[0]
        tick=struct.unpack_from('<H',memory,0x1c452)[0]
        assert value==expected,(target,value,expected)
        assert tick==120,(target,tick)
        assert memory[module+0xa88:module+0xa93]==image[0xa88:0xa93]
        cases.append({'target':target,'tick':tick,'module_offset':module,'stored_DTA_DWORD':value,'resolved_guest_DTA_address':module+value,'DTA_buffer_hex':memory[module+value:module+value+128].hex(),'memory_sha256':digest(folder/'startup.memory'),'log_sha256':digest(folder/'port.log'),'exit':0})
    proof={'scope':'Actual Native module constructor storage and complete16MiB Native/WASM startup dumps at cooperative tick120. DTA storage/address diagnostic only: no original CPU/device clock, GP/flags or complete frame/PCM acceptance.','producers':producers,'original_DTA_offset':expected,'actual_native_init':initial,'startup_cases':cases,'complete_original_acceptance':False}
    (output/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: actual Native initialization and both production targets store the original module DTA offset; reserved initial buffer/code preserved. Tick120 is a diagnostic boundary.')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--native',type=Path)
    parser.add_argument('--wasm',type=Path)
    parser.add_argument('--node',default='node')
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);output=args.output.resolve()
    if args.verify_only:verify(output,repo);return
    if args.native is None or args.wasm is None:parser.error('--native and --wasm are required for capture')
    if not output.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    native=args.native.resolve(strict=True);wasm=args.wasm.resolve(strict=True)
    node=Path(shutil.which(args.node) or args.node).resolve(strict=True)
    output.mkdir(parents=True,exist_ok=False);prefix=output/'start-state'
    for suffix in ('text','bda'):
        source=repo/('tools/oracle/start_state.'+suffix+'.gz.b64')
        Path(str(prefix)+'.'+suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
    shutil.copyfile(repo/'tools/oracle/start_state.vga',Path(str(prefix)+'.vga'))
    fixture=repo/'tools/oracle/detail_loader_case.json';expected=json.loads(fixture.read_text())['dta_init']['states']['after-store']['dta_value']
    probe=Path(__file__).resolve().with_name('dta_startup_native.gdb')
    producers={str(p):digest(p) for p in (native,wasm,wasm.with_suffix('.wasm'),node,probe,Path(__file__).resolve(),fixture,repo/'re_out/fist_image.bin',*[Path(str(prefix)+'.'+s) for s in ('text','bda','vga')])}
    (output/'producers.json').write_text(json.dumps(producers,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith('FIST_')}
    env.update(FIST_SB='1',FIST_TEXT_STATE=str(prefix))
    folder=output/'native-init';folder.mkdir();shutil.copytree(repo/'armoredfist',folder/'game')
    diag=dict(env,FIST_DATADIR=str(folder/'game'),FIST_DTA_DIAG_DIR=str(folder),FIST_DTA_EXPECTED_OFFSET=str(expected))
    with (folder/'probe.log').open('w') as log:
        result=subprocess.run(['gdb','-q','-batch','-x',str(probe),'--args',str(native)],cwd=repo,env=diag,stdout=log,stderr=subprocess.STDOUT,timeout=120)
    (folder/'gdb.exit').write_text(str(result.returncode)+'\n')
    for target,command in [('native',[str(native)]),('wasm',[str(node),str(wasm)])]:
        folder=output/target;folder.mkdir();shutil.copytree(repo/'armoredfist',folder/'game')
        diag=dict(env,FIST_DATADIR=str(folder/'game'),FIST_DUMPTICK='120',FIST_MEMDUMP=str(folder/'startup.memory'))
        with (folder/'port.log').open('w') as log:
            result=subprocess.run(command,cwd=repo,env=diag,stdout=log,stderr=subprocess.STDOUT,timeout=240)
        (folder/'exit').write_text(str(result.returncode)+'\n')
    verify(output,repo)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,TypeError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
