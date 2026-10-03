"""Verify the reached sound IRQ-vector getter/setter and actual IF transition."""
import argparse,ast,hashlib,json,re,struct,sys
from pathlib import Path

def verify(root,repo):
    sys.path.insert(0,str(repo/'tools/oracle'))
    from sequence_format import validate,validate_endpoint
    digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    assert (root/'source.exit').read_text()=='0\n'
    for p,h in json.loads((root/'producers.json').read_text()).items():assert digest(Path(p))==h,p
    originals=json.loads((root/'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()}==set(originals)
    for p,h in originals.items():assert digest(repo/p)==h,p
    prefix=root/'source/sequence';baseline=root/'baseline/sequence'
    assert (root/'baseline.exit').read_text()=='0\n'
    for name,p in (('source',prefix),('baseline',baseline)):
        assert 'Python Exception' not in (root/name/'dosbox.log').read_text()
        assert validate_endpoint(p,600)==600
        assert validate(str(p)+'.frames','F')['records']==39
        assert validate(str(p)+'.pcm','A')['samples']==27518
    for s in ('frames','pcm','end'):assert Path(str(prefix)+'.'+s).read_bytes()==Path(str(baseline)+'.'+s).read_bytes(),s
    effects=json.loads((repo/'tools/oracle/effects_mode_case.json').read_text())
    assert {s:digest(Path(str(prefix)+'.'+s)) for s in ('frames','pcm','end')}==effects['capture_sha256']
    program=(repo/'tools/oracle/file_error.gdb').read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical'];assert len(nodes)==1
    ns={'struct':struct};exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(repo/'tools/oracle/file_error.gdb'),'exec'),ns);physical=ns['physical']
    lazy=(repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h').read_text().split('//Types of Flag changing instructions',1)[1].split('enum {',1)[1].split('};',1)[0]
    types={name:i for i,name in enumerate(re.findall(r'\bt_[A-Za-z0-9_]+\b',lazy))}
    image=(repo/'re_out/fist_image.bin').read_bytes();blob=(repo/'armoredfist/FIST.RUN').read_bytes()
    irq_case=json.loads((repo/'tools/oracle/sb_irq_frame_case.json').read_text());region=irq_case['resident_entry_region'];bias=region['asset_file_offset']-region['ip']
    assert blob[region['asset_file_offset']:region['asset_file_offset']+len(bytes.fromhex(region['bytes']))]==bytes.fromhex(region['bytes'])
    folder=root/'source';rows=[json.loads(s) for s in (folder/'vector-fetches.jsonl').read_text().splitlines()]
    states={p.stem:json.loads(p.read_text()) for p in folder.glob('*.json')}
    ram={name:(folder/(name+'.memory')).read_bytes() for name in states}
    ext_ips=[0x138d,0x138e,0x1394,0x1397,0x139c,0x139f,0x13a0,0x13a4,0x13a6,0x13a7,0x13ad,0x13ae,0x13b0,0x13b2,0x13b7,0x13bb,0x13bd,0x13be]
    resident_ips=[0x1fba,0x1fbe,0x1fc2,0x1fd6,0x1fdd,0x2237,0x223b,0x223e,0x2241,0x2246,0x225b,0x225c]
    assert set(states)=={'%04x'%ip for ip in ext_ips}|{'resident-%04x'%ip for ip in resident_ips}
    assert len(rows)==507 and all(len(m)==16777216 for m in ram.values())
    ext_cs=states['138d']['segments'][1]['value'];resident_cs=states['resident-1fbe']['segments'][1]['value'];assert ext_cs!=resident_cs
    for name,q in states.items():
        assert [r for r in rows if r['segments'][1]['value']==q['segments'][1]['value'] and r['cpu_regs.ip.dword[0]']==q['cpu_regs.ip.dword[0]'] and r['phase']==q['phase']]==[q],name
    assert [q['cpu_regs.ip.dword[0]'] for q in rows if q['segments'][1]['value']==ext_cs]==ext_ips
    assert rows[0]==states['138d'] and rows[-1]==states['13be']
    def clock(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
    gaps=[clock(b)-clock(a) for a,b in zip(rows,rows[1:])]
    assert gaps.count(1)==502 and gaps.count(0)==4 and len(gaps)==506
    def address(name,segment,offset):
        q=states[name];return physical((q['segments'][segment]['base']+offset)&0xffffffff,ram[name],q)
    def read(name,segment,offset,width):return int.from_bytes(ram[name][address(name,segment,offset):address(name,segment,offset)+width],'little')
    ext_lengths=dict(zip(ext_ips,(1,6,3,2,3,1,4,2,1,6,1,2,2,5,4,2,1,5)))
    resident_lengths=dict(zip(resident_ips,(4,4,4,7,6,4,3,3,5,2,1,3)))
    for name,q in states.items():
        ip=q['cpu_regs.ip.dword[0]'];p=address(name,1,ip);m=ram[name]
        assert p==q['fetched_code_physical'] and m[p:p+32].hex()==q['fetched_code_hex'],name
        source=image if q['segments'][1]['value']==ext_cs else blob
        off=ip if q['segments'][1]['value']==ext_cs else bias+ip
        n=(ext_lengths if q['segments'][1]['value']==ext_cs else resident_lengths)[ip]
        assert m[p:p+n]==source[off:off+n],(name,'complete fetched instruction')
    checks=[]
    def arithmetic(name,kind,v1,v2,res,width):
        q=states[name];updates={'lflags.type':types[kind]};mask=(1<<(width*8))-1
        for key,v,w in (('var1',v1,width),('var2',v2,1 if kind.startswith('t_SHL') else width),('res',res,width)):
            k='lflags.'+key+'.dword[0]';fieldmask=(1<<(8*w))-1;updates[k]=(q[k]&(~fieldmask&0xffffffff))|(v&fieldmask)
        return updates
    def transition(a,b,regs=None,writes=(),segments=None,flags=None):
        qa,qb=states[a],states[b];expected=bytearray(ram[a])
        for p,v,w in writes:expected[p:p+w]=v.to_bytes(w,'little')
        assert qb['registers']==(qa['registers'] if regs is None else regs),(a,b,'GP')
        assert qb['segments']==(qa['segments'] if segments is None else segments),(a,b,'segments')
        assert ram[b]==expected,(a,b,'whole RAM')
        f={k:v for k,v in qa.items() if k=='cpu_regs.flags' or k.startswith('lflags.')}
        if flags:f.update(flags)
        for k,v in f.items():assert qb[k]==v,(a,b,k,hex(qb[k]),hex(v))
        for k in qa:
            if k.startswith(('cpu.','paging.')) or k in ('PIC_Ticks','CPU_CycleMax','CPU_CycleLeft'):assert qa[k]==qb[k],(a,b,k)
        assert clock(qb)-clock(qa)==1,(a,b,'one fetched instruction')
        checks.append([a,b])
    def stack_pointer(q,delta):return (q['registers'][4]&q['cpu.stack.notmask'])|((q['registers'][4]+delta)&q['cpu.stack.mask'])
    transition('138d','138e',flags={'cpu_regs.flags':states['138d']['cpu_regs.flags']&~0x200})
    q=states['138e'];r=q['registers'].copy();r[1]=read('138e',3,0x12c4,4);transition('138e','1394',r)
    v=r[1]&255;transition('1394','1397',flags=arithmetic('1394','t_TESTb',v,8,v&8,1))
    assert v&8==0;transition('1397','139c')
    r=r.copy();r[1]=(r[1]&0xffffff00)|((v+8)&255);transition('139c','139f',r,flags=arithmetic('139c','t_ADDb',v,8,v+8,1))
    q=states['139f'];r=q['registers'].copy();r[4]=stack_pointer(q,-4);transition('139f','13a0',r,[(address('139f',2,r[4]&q['cpu.stack.mask']),r[1],4)])
    q=states['13a0'];r=q['registers'].copy();r[0]=(r[0]&0xffff0000)|struct.unpack_from('<H',image,0x13a2)[0];transition('13a0','13a4',r)
    q=states['13a6'];r=q['registers'].copy();r[1]=read('13a6',2,r[4]&q['cpu.stack.mask'],4);r[4]=stack_pointer(q,4);transition('13a6','13a7',r)
    q=states['13a7'];transition('13a7','13ad',writes=[(address('13a7',3,0x12c0),q['registers'][3],4)])
    q=states['13ad'];r=q['registers'].copy();r[4]=stack_pointer(q,-4);transition('13ad','13ae',r,[(address('13ad',2,r[4]&q['cpu.stack.mask']),q['segments'][3]['value'],4)])
    q=states['13ae'];r=q['registers'].copy();r[0]=q['segments'][1]['value'];transition('13ae','13b0',r)
    q=states['13b0'];segments=[s.copy() for s in q['segments']];segments[3]=q['segments'][1].copy();transition('13b0','13b2',segments=segments)
    q=states['13b2'];r=q['registers'].copy();r[2]=struct.unpack_from('<I',image,0x13b3)[0];transition('13b2','13b7',r)
    q=states['13b7'];r=q['registers'].copy();r[0]=(r[0]&0xffff0000)|struct.unpack_from('<H',image,0x13b9)[0];transition('13b7','13bb',r)
    q=states['13bd'];r=q['registers'].copy();saved=read('13bd',2,r[4]&q['cpu.stack.mask'],4);assert saved==states['13ad']['segments'][3]['value'];r[4]=stack_pointer(q,4)
    segments=[s.copy() for s in q['segments']];segments[3]=states['13ad']['segments'][3].copy();transition('13bd','13be',r,segments=segments)
    q=states['resident-1fba'];r=q['registers'].copy();sp=r[4]&q['cpu.stack.mask'];value=read('resident-1fba',2,sp,2);r[4]=stack_pointer(q,2)
    transition('resident-1fba','resident-1fbe',r,[(address('resident-1fba',3,(r[0]+4)&0xffffffff),value,2)])
    q=states['resident-1fbe'];slot=address('resident-1fbe',3,q['registers'][0]);transition('resident-1fbe','resident-1fc2',writes=[(slot,q['registers'][2],4)])
    q=states['resident-1fd6'];p=address('resident-1fd6',4,(q['registers'][0]+q['registers'][3]*4+3)&0xffffffff);v=ram['resident-1fd6'][p]
    transition('resident-1fd6','resident-1fdd',writes=[(p,v|1,1)],flags=arithmetic('resident-1fd6','t_ORb',v,1,v|1,1))
    q=states['resident-2237'];r=q['registers'].copy();v=r[3];r[3]=(v<<16)&0xffffffff;transition('resident-2237','resident-223b',r,flags=arithmetic('resident-2237','t_SHLd',v,16,r[3],4))
    q=states['resident-223b'];r=q['registers'].copy();r[3]=(r[3]&0xffff0000)|(r[1]&255);transition('resident-223b','resident-223e',r)
    q=states['resident-223e'];r=q['registers'].copy();v=r[3]&65535;r[3]=(r[3]&0xffff0000)|((v<<2)&65535);transition('resident-223e','resident-2241',r,flags=arithmetic('resident-223e','t_SHLw',v,2,v<<2,2))
    q=states['resident-2241'];r=q['registers'].copy();v=r[3]&65535;base=read('resident-2241',0,0x206,2);r[3]=(r[3]&0xffff0000)|((v+base)&65535);transition('resident-2241','resident-2246',r,flags=arithmetic('resident-2241','t_ADDw',v,base,v+base,2))
    transition('resident-225b','resident-225c',flags={'cpu_regs.flags':states['resident-225b']['cpu_regs.flags']|0x200})
    assert len(checks)==23
    vector=states['13a4']['registers'][1]&255
    before_ivt=struct.unpack_from('<I',ram['13a4'],vector*4)[0];after_ivt=struct.unpack_from('<I',ram['13bd'],vector*4)[0]
    a,b=states['13a4'],states['13a6'];r=a['registers'].copy();r[3]=before_ivt;assert b['registers']==r
    assert states['13bb']['registers']==states['13bd']['registers'] and states['13bb']['segments']==states['13bd']['segments']
    assert not a['cpu_regs.flags']&0x200 and not b['cpu_regs.flags']&0x200
    assert not states['13bb']['cpu_regs.flags']&0x200 and states['13bd']['cpu_regs.flags']&0x200
    assert states['13ad']['saved_vector']==before_ivt
    assert struct.unpack_from('<IH',ram['13bd'],slot)==(states['13bb']['registers'][2],states['13bb']['segments'][3]['value'])
    assert states['resident-2237']['registers'][3]&65535==states['resident-2237']['segments'][1]['base']>>4
    assert after_ivt==states['resident-2246']['registers'][3]
    stub=((after_ivt>>16)<<4)+(after_ivt&65535);assert p==stub+3 and ram['13bb'][p]==0 and ram['13bd'][p]==1
    assert ram['13bd'][stub:stub+4].hex()==irq_case['runtime_irq_stub']['bytes']
    proof=dict(scope='Source-only actual sound138d protected vector prefix: full507 observed fetch states/30 complete16MiB boundaries. Twenty-three direct instruction/wholeRAM transitions recover CLI, caller GP/segment/stack widths, saved BIOS vector, protected handler slot, resident stub activation, real-IVT pointer construction and actual kernel STI. Nested DOS services are traced/bracketed, not fully interpreted or accepted as a port implementation. Other vectors/errors/device/DMA/IRQ/CPU/time/finalPCM/full original output remain open.',states=states,fetches=rows,whole_RAM_transitions=checks,memory_sha256={n:digest(folder/(n+'.memory')) for n in states},vector=vector,original_real_vector=before_ivt,installed_real_vector=after_ivt,protected_handler_slot_physical=slot,stub_physical=stub,stub_activation_physical=p,resident_file_bias=bias,cycle_interval=clock(rows[-1])-clock(rows[0]),frames=39,mixed_samples=27518,endpoint_ms=600,capture_sha256={s:digest(Path(str(prefix)+'.'+s)) for s in ('frames','pcm','end')},producers=json.loads((root/'producers.json').read_text()),originals_unchanged=len(originals),verifier_sha256=digest(Path(__file__)),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: original507 fetches/30 state-RAM boundaries/23 direct instruction-RAM transitions; complete39frame/27518PCM/end600 preserved')
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    verify(args.output.resolve(strict=True),args.repo.resolve(strict=True))

if __name__=='__main__':main()
