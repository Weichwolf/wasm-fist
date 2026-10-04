#!/usr/bin/env python3
"""Verify the original checkpoint-free CPU/RAM contract and consume the full prefix owner."""
from pathlib import Path
import ast,json,os,shutil,struct,tempfile
from sequence_format import validate,validate_endpoint
from verify_device_start_prefix import digest,producer_paths,verify as verify_prefix

PREFIX_BOUNDARIES={'77e2','77e9','1280','12ab','77ee','23c4','133a','77ff','7809'}

def additional_producers(repo):
    return tuple(repo/'tools/oracle'/p for p in (
        'capture_device_checkpoint.py','verify_device_checkpoint.py',
        'check_device_checkpoint_transitions.py','cpu_instructions_probe.cpp'))

def physical_owner(repo):
    shared=repo/'tools/oracle/file_error.gdb'
    program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical']
    assert len(nodes)==1
    ns={'struct':struct}
    exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),ns)
    return ns['physical']


def verify_prefix_projection(root,repo):
    rows=(root/'source/prefix-fetches.jsonl').read_text().splitlines()
    assert len(rows)==368
    with tempfile.TemporaryDirectory(prefix='wasm-fist-checkpoint-prefix-') as temporary:
        clone=Path(temporary)
        shutil.copytree(root/'baseline',clone/'baseline',copy_function=os.link,ignore=shutil.ignore_patterns('game'))
        (clone/'source').mkdir()
        for p in (root/'source').iterdir():
            if not p.is_file() or p.name=='prefix-fetches.jsonl':continue
            if p.suffix in ('.memory','.json') and p.stem not in PREFIX_BOUNDARIES:continue
            os.link(p,clone/'source'/p.name)
        (clone/'source/prefix-fetches.jsonl').write_text('\n'.join(rows[:242])+'\n')
        for name in ('baseline.exit','source.exit','original-hashes.json'):shutil.copy2(root/name,clone/name)
        producers=json.loads((root/'producers.json').read_text())
        selected={str(p):producers[str(p)] for p in producer_paths(repo)}
        (clone/'producers.json').write_text(json.dumps(selected,indent=2)+'\n')
        return verify_prefix(clone,repo)


def verify(root,repo):
    folder=root/'source';baseline=root/'baseline'
    assert (root/'source.exit').read_text()=='0\n'
    assert 'Python Exception' not in (folder/'dosbox.log').read_text()
    producers=json.loads((root/'producers.json').read_text())
    assert set(producers)=={str(p) for p in (*producer_paths(repo),*additional_producers(repo))}
    for p,h in producers.items():assert digest(p)==h,p
    originals=json.loads((root/'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()}==set(originals)
    for p,h in originals.items():assert digest(repo/p)==h,p
    prefix=folder/'sequence'
    assert validate_endpoint(prefix,600)==600
    assert validate(str(prefix)+'.frames','F')['records']==39
    assert validate(str(prefix)+'.pcm','A')['samples']==27518
    for suffix in ('frames','pcm','end'):assert Path(str(prefix)+'.'+suffix).read_bytes()==(baseline/('sequence.'+suffix)).read_bytes(),suffix
    for suffix in ('text','bda','vga'):assert (folder/('start-state.'+suffix)).read_bytes()==(baseline/('start-state.'+suffix)).read_bytes(),suffix
    rows=[json.loads(line) for line in (folder/'prefix-fetches.jsonl').read_text().splitlines()]
    old=verify_prefix_projection(root,repo)['fetches']
    assert len(rows)==368 and rows[:242]==old
    entry=rows[242];returned=rows[-1];assert entry['cpu_regs.ip.dword[0]']==0x3322 and returned['cpu_regs.ip.dword[0]']==0x780e
    assert rows[241]['cpu_regs.ip.dword[0]']==0x7809
    image=(repo/'re_out/fist_image.bin').read_bytes();assert image[0x7809]==0xe8
    assert 0x780e+struct.unpack_from('<i',image,0x780a)[0]==0x3322
    physical=physical_owner(repo)
    memories={p.stem:p.read_bytes() for p in folder.glob('*.memory')}
    assert set(memories)=={'77e2','77e9','1280','12ab','77ee','23c4','133a','77ff','7809','3322','780e'}
    assert all(len(m)==0x1000000 for m in memories.values())
    assert {p.stem for p in folder.glob('*.json')}==set(memories)
    for q in rows:
     label='%04x'%q['cpu_regs.ip.dword[0]']
     if label in memories:assert q==json.loads((folder/(label+'.json')).read_text()),label
    address=lambda q,s,off:physical((q['segments'][s]['base']+off)&0xffffffff,memories['3322'],q)
    for q in rows:
     p=address(q,1,q['cpu_regs.ip.dword[0]'])
     assert q['fetched_code_physical']==p and bytes.fromhex(q['fetched_code_hex'])==image[q['cpu_regs.ip.dword[0]']:q['cpu_regs.ip.dword[0]']+32]
     assert q['entry_return_ip']==0xf57
    for a,b in zip(rows[241:],rows[242:]):
     assert b['CPU_Cycles']==a['CPU_Cycles']-1
     for field in ('PIC_Ticks','CPU_CycleLeft','CPU_CycleMax','segments','cpu.cr0','paging.cr3','paging.enabled','cpu.code.big','cpu.stack.big','cpu.stack.mask','cpu.stack.notmask','cpu.pmode','cpu.cpl'):assert a[field]==b[field],field
    expected=bytearray(memories['7809']);stack=address(entry,2,entry['registers'][4]);struct.pack_into('<I',expected,stack,0x780e)
    assert expected==memories['3322'] and entry['registers'][4]==rows[241]['registers'][4]-4
    expected=bytearray(memories['3322'])
    writes=[]
    for offset,value in ((0x2f5c,0x334c),(0x2f58,0xbc98),(0x2f50,0x54200),(0xbc98,0),(0x2f54,7)):
     p=address(entry,3,offset);before=struct.unpack_from('<I',expected,p)[0];struct.pack_into('<I',expected,p,value)
     writes.append(dict(segment='DS',offset=offset,physical=p,before=before,after=value))
    for offset,value in ((0x3d006,1),(0x3d002,7),(0x3cffe,0x334c),(0x3cffa,0x338d)):
     p=address(entry,2,offset);before=struct.unpack_from('<I',expected,p)[0];struct.pack_into('<I',expected,p,value)
     writes.append(dict(segment='SS',offset=offset,physical=p,before=before,after=value))
    assert expected==memories['780e'],'Complete post-free RAM differs from the original instruction stores'
    assert struct.unpack_from('<I',memories['3322'],address(entry,3,0x2f54))[0]==8
    assert returned['registers']==[0,0,0xbc98,1,0x3d00e,0,6,0x140]
    assert returned['cpu_regs.flags']==0x3246 and returned['lflags.type']==30
    assert struct.unpack_from('<I',memories['3322'],address(entry,3,0x2f60))[0]==struct.unpack_from('<I',memories['780e'],address(entry,3,0x2f60))[0]==0
    new=rows[242:];labels=[q['cpu_regs.ip.dword[0]'] for q in new]
    assert labels.count(0x3661)==2 and labels.count(0x3376)==1
    proof=dict(scope='Original-source contract: complete original77e2 prefix extended through the actual3322 checkpoint-free return780e. 368 full CPU states,126 additional transitions, eleven complete16MiB RAM boundaries and exact instruction-store RAM proof. No port implementation or complete original acceptance.',fetches=rows,actual_call=dict(ip=0x7809,target=0x3322,return_ip=0x780e,stack_physical=stack),new_fetches=126,actual_3661_index_owner='ESI register; DS:2f60 remains zero. The legacy C shim publishes ESI into this RAM word, which is not an original store.',callee_3661_calls=2,callee_3376_calls=1,whole_RAM_writes=writes,lazy_types_reached=sorted({q['lflags.type'] for q in new}),memory_sha256={k:digest(folder/(k+'.memory')) for k in memories},original_frames=39,original_mixed_samples=27518,endpoint_ms=600,producers=json.loads((root/'producers.json').read_text()),verifier_sha256=digest(__file__),original_files=len(originals),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS:368 full original CPU states, actualCALL3322, eleven whole RAM boundaries and exact post-free memory; all source output unchanged')

    return proof
