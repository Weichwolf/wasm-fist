#!/usr/bin/env python3
"""Complete original startup through both reached INT21 handlers and caller5df6."""
import argparse
import copy
import datetime
import json
from pathlib import Path
import tempfile

from capture_device_start_prefix import capture as capture_prefix
from capture_software_dos import verify as verify_first, read_lines
from verify_device_start_prefix import digest
from memory_context import observer, portable, validate_context


HANDLER_KINDS = tuple('fetch-%04x-%04x' % (cs, ip) for cs, ip in (
    (8,0x1abb),(8,0x1abf),(8,0x142f),(8,0x1432),
    (0x2dd,0x1437),(0x2dd,0x143a),(0x2dd,0x143d),(0x2dd,0x143f),
    (0x2dd,0x1548),(0x2dd,0x154b),(0x2dd,0x1550),(0x2dd,0x1553),
    (8,0x1558),(8,0x155d),(8,0x1563),(8,0x1569)))
FIND_KINDS = ('before-software','after-software','before-DOS21',
              'before-FindFirst','before-SetupSearch','after-SetupSearch',
              'before-directory','after-directory','before-SetResult',
              'after-SetResult','after-FindFirst','after-DOS21',
              'before-far-ret','after-far-ret','caller')
HOST_KINDS = ('initial77e2','before-directory','after-directory','before-SetResult')


def properties(repo, source, handler, following, following_end, find, events, host):
    def row(value):
        q=copy.deepcopy(value)
        if 'memory_context' in q:q['memory_context']=portable(q['memory_context'])
        args=q.get('arguments',{})
        if 'path' in args:
            # Capture directory is observer metadata, never guest memory or state.
            assert args['path']==str(source/'game/FISTDATA')+'/'
            args['path']='game/FISTDATA/'
        return q
    return dict(scope='Complete original startup and both reached DOS handlers:1014 instruction fetches,1034 CPU/time observations,52 replay memory contexts and four complete host states. Same-run raw boot RAM and host-name buffer bytes are matched inputs, never normalized. First INT21 properties remain owned by software_dos_case.json. No runtime bootstrap or complete original application parity acceptance.',
                first_case_sha256=digest(repo/'tools/oracle/software_dos_case.json'),
                handler=[row(q) for q in handler],following=[row(q) for q in following],
                following_end=row(following_end),find_fetches=[row(q) for q in find],
                find_events=[row(q) for q in events],host=[row(q) for q in host])


def verify(repo, root, reference=True):
    first=verify_first(repo,root)
    source=root/'source'
    handler=read_lines(source/'handler/events.jsonl')
    following=read_lines(source/'following/fetches.jsonl')
    following_end=json.loads((source/'following/before-next-DOS.json').read_text())
    find=read_lines(source/'findfirst/fetches.jsonl')
    events=read_lines(source/'findfirst/events.jsonl')
    host=read_lines(source/'host-events.jsonl')
    assert len(handler)==18 and [q['serial'] for q in handler]==list(range(1,19))
    assert tuple(q['kind'] for q in handler)==HANDLER_KINDS[:8]+('before-DOS21','after-DOS21')+HANDLER_KINDS[8:]
    assert len(following)==257 and len(find)==219
    assert following[-1]['segments'][1]['value']==0x2b
    assert following[-1]['cpu_regs.ip.dword[0]']==0x5df4
    assert following[-1]['fetched_code_hex'].startswith('cd21')
    last=copy.deepcopy(following_end)
    last.pop('memory_context');last.pop('memory_file')
    assert last==following[-1]
    assert find[0]['segments'][1]['value']==8 and find[0]['cpu_regs.ip.dword[0]']==0x197d
    assert find[-2]['segments'][1]['value']==8 and find[-2]['cpu_regs.ip.dword[0]']==0x1b41
    assert find[-2]['fetched_code_hex'].startswith('66cb')
    assert find[-1]['segments'][1]['value']==0x2b and find[-1]['cpu_regs.ip.dword[0]']==0x5df6
    assert tuple(q['kind'] for q in events)==FIND_KINDS
    assert [q['serial'] for q in events]==list(range(1,16))
    assert events[0]['arguments']==dict(num=33,type=1,oldeip=0x5df6)
    assert events[3]['arguments']==dict(search='DSOUNDS.BIN',attr=51,fcb_findfirst=0)
    assert events[4]['arguments']==dict(drive=2,attr=51,pattern='DSOUNDS.BIN',pt=44960)
    assert events[12]['arguments']==dict(use32=1,bytes=0,oldeip=0x1b43)
    assert tuple(q['kind'] for q in host)==HOST_KINDS
    assert [q['serial'] for q in host]==list(range(1,5))
    initial,before,after,result=host
    fields=('current_drive','current_directory','next_free','occupied')
    assert {k:initial[k] for k in fields}=={k:before[k] for k in fields}
    assert initial['current_drive']==events[4]['arguments']['drive']
    assert initial['current_directory']=='FISTDATA' and len(initial['occupied'])==2048
    directory=events[6]['arguments']
    assert directory['path']==before['arguments']['path']==str(source/'game/FISTDATA')+'/'
    assert directory['next_free']==before['next_free'] and directory['occupied']==before['occupied']
    free=before['next_free'];slots=len(before['occupied']);scans=0
    assert 0<=free<slots
    while before['occupied'][free] and scans<slots:
        free=(free+1)%slots;scans+=1
    assert scans<slots
    assert after['arguments']['id']==free and after['next_free']==(free+1)%slots
    occupied=list(before['occupied']);occupied[free]=True
    assert after['occupied']==occupied
    assert after['current_drive']==initial['current_drive']
    assert after['current_directory']==initial['current_directory']
    assert {k:result[k] for k in fields}=={k:after[k] for k in fields}
    args=result['arguments'];assert args==events[8]['arguments']
    assert args['name']=='DSOUNDS.BIN'
    name=bytes.fromhex(args['name_raw_hex'])
    assert len(name)==13 and name[:12]==b'DSOUNDS.BIN\0'
    stat=(source/'game/FISTDATA/DSOUNDS.BIN').stat()
    clock=datetime.datetime.fromtimestamp(stat.st_mtime)
    assert args['size']==stat.st_size and args['attr']==32 and args['pt']==44960
    assert args['date']==((clock.year-1980)<<9 | clock.month<<5 | clock.day)
    assert args['time']==(clock.hour<<11 | clock.minute<<5 | clock.second//2)
    memory_files=set();vga_files=set()
    contexts=handler+[following_end]+events
    assert len(contexts)==34
    for q in contexts:
        p=source/q['memory_file'];assert p.stat().st_size==16777216
        validate_context(q['memory_context'],source,repo)
        memory_files.add(q['memory_file'])
        vga_files.update(q['memory_context']['vga'][k]['file'] for k in ('linear','fastmem'))
    for folder,rows in (('handler',handler),('following',[following_end]),('findfirst',events)):
        assert {str(p.relative_to(source)) for p in (source/folder).glob('*.memory')}=={q['memory_file'] for q in rows}
        expected={q['memory_context']['vga'][k]['file'] for q in rows for k in ('linear','fastmem')}
        assert {str(p.relative_to(source)) for p in (source/folder).glob('*.memory.*')}==expected
    case=properties(repo,source,handler,following,following_end,find,events,host)
    if reference:
        assert case==json.loads((repo/'tools/oracle/software_startup_dos_case.json').read_text()),'complete startup/DOS source reference differs'
    proof=dict(scope=case['scope'],first=first,properties=case,
               added_memory_sha256={p:digest(source/p) for p in sorted(memory_files)},
               added_VGA_sha256={p:digest(source/p) for p in sorted(vga_files)},
               producer_manifest_sha256=digest(root/'producers.json'),
               validator_sha256=digest(Path(__file__)),complete_original_acceptance=False)
    (root/'startup-dos-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: complete1014-fetch startup/both-DOS source,34 added memory boundaries/four host contexts; original39frames/27518PCM/end600 unchanged')
    return proof


def capture(repo,root,reference=True):
    assert root.is_relative_to(Path('/tmp'))
    with tempfile.TemporaryDirectory(prefix='wasm-fist-startup-dos-probe-') as temp:
        probe=Path(temp)/'startup-dos.gdb'
        text=observer(repo,'device');assert text.count('\nend\nrun')==1
        includes=[repo/'tools/oracle'/name for name in ('software_dos.gdb.inc','software_startup_dos.gdb.inc')]
        probe.write_text(text.replace('\nend\nrun','\n'+'\n'.join(p.read_text() for p in includes)+'\nend\nrun'))
        tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
        extra=[Path(__file__).resolve(),*includes,probe,
               *[repo/'tools/oracle'/n for n in ('capture_software_dos.py','memory_context.py',
                 'memory_context.gdb.inc','capture_pit_irq_frames.py','physical_provider.gdb.inc',
                 'paging_control.gdb.inc','capture_physical_provider.py')],
               *[tree/n for n in ('src/cpu/paging.cpp','src/cpu/cpu.cpp','src/cpu/callback.cpp',
                 'src/cpu/core_normal.cpp','src/cpu/core_normal/prefix_none.h',
                 'src/cpu/core_normal/string.h','src/hardware/memory.cpp','src/hardware/vga_memory.cpp',
                 'src/dos/dos.cpp','src/dos/dos_files.cpp','src/dos/dos_classes.cpp',
                 'src/dos/drive_local.cpp','src/dos/drive_cache.cpp','include/dos_inc.h',
                 'include/dos_system.h','include/mem.h','include/paging.h','include/vga.h')]]
        capture_prefix(repo,root,stop=0x5cdd,probe=probe,extra_producers=extra,wall_seconds=80)
        retained=root/'observer.gdb';retained.write_bytes(probe.read_bytes())
        manifest=json.loads((root/'producers.json').read_text());manifest.pop(str(probe))
        manifest[str(retained)]=digest(retained)
        (root/'producers.json').write_text(json.dumps(manifest,indent=2)+'\n')
        return verify(repo,root,reference)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve(strict=True);root=args.output.resolve()
    return verify(repo,root) if args.verify_only else capture(repo,root)


if __name__=='__main__':main()
