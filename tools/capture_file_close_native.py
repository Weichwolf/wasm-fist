#!/usr/bin/env python3
"""Capture actual startup file lifetime and WORD returns; stop at the first op68 poster."""
import argparse,base64,gzip,hashlib,json,os
from pathlib import Path
import shutil,subprocess,sys

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def verify(output,repo):
    assert (output/'gdb.exit').read_text()=='0\n'
    assert 'Python Exception' not in (output/'probe.log').read_text()
    counts=json.loads((output/'counts.json').read_text())
    assert counts=={'config':2,'size':2,'de89':1,'op68':1},counts
    events=json.loads((output/'dos-calls.json').read_text())
    all_opens=[e for e in events if e['command']==0x3d]
    names=[e['name'].replace('\\','/').split('/')[-1].upper() for e in all_opens]
    assert names==['SOUND.CFG','FIST.CD','SOUNDDVR.DVR','MGAVIDEO.DVR','FIST.SET'],names
    missing_open=all_opens[1];assert missing_open['after']['dos_regs'][0]==2 and missing_open['after']['dos_regs'][9]==1 and not missing_open['after']['handles']
    opens=[e for e in all_opens if e['after']['dos_regs'][9]==0]
    closes=[e for e in events if e['command']==0x3e]
    assert len(all_opens)==5
    assert len(closes)==len(opens)==4
    for opened,closed in zip(opens,closes):
        h=opened['after']['dos_regs'][0];assert opened['after']['dos_regs'][9]==0
        assert closed['before']['dos_regs'][1]==h
        assert closed['after']['dos_regs'][9]==0 and not closed['after']['handles']
        assert opened['after']['handles']==[{'handle':h,'file':opened['name'].replace('\\','/').split('/')[-1].upper()}]
    states={p.stem:json.loads(p.read_text()) for p in sorted(output.glob('*-before.json'))+sorted(output.glob('*-after.json'))}
    config=states['config-1-after'];assert config['return_ax']==0x3e00|(opens[0]['after']['dos_regs'][0]&255)
    assert config['dos_regs'][1]==opens[0]['after']['dos_regs'][0] and config['dos_regs'][9]==0 and config['dos_regs'][2]==10
    assert not config['handles']
    missing=states['config-2-after'];assert missing['return_ax']==2 and missing['dos_regs'][9]==1 and not missing['handles']
    sizes=[states['size-'+str(i)+'-after'] for i in (1,2)]
    fixture=json.loads((repo/'tools/oracle/file_close_case.json').read_text())
    assert [r['return_ax'] for r in sizes]==[fixture['source_assets'][name]['size'] for name in ('SOUNDDVR.DVR','MGAVIDEO.DVR')]
    assert all(r['dos_regs'][3]==0 and not r['handles'] for r in sizes)
    before=states['de89-before'];end=json.loads((output/'op68-gate.json').read_text())
    assert not before['handles'] and not end['handles']
    assert before['param_1']==end['inbox_ebx']
    # Record the complete DWORD. The unrelated upper-word constructor remains unresolved.
    producers=json.loads((output/'producers.json').read_text())
    for path,sha in producers.items():assert digest(path)==sha,path
    proof={'scope':'Actual fefb/50c8 startup file lifetime and WORD results; independent probe stops at first op68. No full CPU/flags/stack/IRQ/time or whole original output acceptance.','producers':producers,'counts':counts,'dos_calls':events,'helper_states':states,'op68_gate':end,'all_startup_files_closed':True,'op68_upper_word_accepted':False}
    (output/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: actual startup config/overlay handles are closed, WORD read/size returns match, first op68 observed without masking.')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--native',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);output=args.output.resolve()
    if args.verify_only:verify(output,repo);return
    if args.native is None:parser.error('--native is required for capture')
    if not output.is_relative_to(Path('/tmp')):parser.error('disposable captures must be under /tmp')
    native=args.native.resolve(strict=True);output.mkdir(parents=True,exist_ok=False)
    shutil.copytree(repo/'armoredfist',output/'game');prefix=output/'start-state'
    for suffix in ('text','bda'):
        source=repo/('tools/oracle/start_state.'+suffix+'.gz.b64')
        Path(str(prefix)+'.'+suffix).write_bytes(gzip.decompress(base64.b64decode(source.read_bytes())))
    shutil.copyfile(repo/'tools/oracle/start_state.vga',Path(str(prefix)+'.vga'))
    probe=Path(__file__).resolve().with_name('file_close_native.gdb')
    producers={str(p):digest(p) for p in (native,probe,Path(__file__).resolve(),repo/'tools/oracle/file_close_case.json',*[Path(str(prefix)+'.'+s) for s in ('text','bda','vga')])}
    (output/'producers.json').write_text(json.dumps(producers,indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith('FIST_')}
    env.update(FIST_FILE_CLOSE_DIAG_DIR=str(output),FIST_DATADIR=str(output/'game'),FIST_SB='1',FIST_TEXT_STATE=str(prefix))
    with (output/'probe.log').open('w') as log:
        result=subprocess.run(['gdb','-q','-batch','-x',str(probe),'--args',str(native)],cwd=repo,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
    (output/'gdb.exit').write_text(str(result.returncode)+'\n');verify(output,repo)

if __name__=='__main__':
    try:main()
    except (AssertionError,OSError,ValueError,subprocess.SubprocessError) as error:sys.exit('FAIL: '+str(error))
