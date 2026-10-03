"""Verify actual mode-2 effects branches; nested device work stays a boundary."""
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
    prefix=root/'source/sequence';ref=root/'baseline/sequence'
    assert (root/'baseline.exit').read_text()=='0\n'
    assert validate_endpoint(ref,600)==600
    assert validate(str(ref)+'.frames','F')['records']==39
    assert validate(str(ref)+'.pcm','A')['samples']==27518
    for name in ('baseline','source'):assert 'Python Exception' not in (root/name/'dosbox.log').read_text()
    assert validate_endpoint(prefix,600)==600
    assert validate(str(prefix)+'.frames','F')['records']==39
    assert validate(str(prefix)+'.pcm','A')['samples']==27518
    for s in ('frames','pcm','end'):assert Path(str(prefix)+'.'+s).read_bytes()==Path(str(ref)+'.'+s).read_bytes(),s
    program=(repo/'tools/oracle/file_error.gdb').read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical'];assert len(nodes)==1
    ns={'struct':struct};exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(repo/'tools/oracle/file_error.gdb'),'exec'),ns);physical=ns['physical']
    lazy=(repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h').read_text().split('//Types of Flag changing instructions',1)[1].split('enum {',1)[1].split('};',1)[0]
    types={name:index for index,name in enumerate(re.findall(r'\bt_[A-Za-z0-9_]+\b',lazy))}
    image=(repo/'re_out/fist_image.bin').read_bytes();folder=root/'source'
    assert struct.unpack_from('<I',image,0xcb3+0x68)[0]==0x76fd
    rows=[json.loads(s) for s in (folder/'effects-fetches.jsonl').read_text().splitlines()]
    states={p.stem:json.loads(p.read_text()) for p in folder.glob('*.json')}
    ram={name:(folder/(name+'.memory')).read_bytes() for name in states}
    expected={1:[0x76fd,0x7702,0x7704,0x7706,0x7708,0x770a,0x770c,0x773e,0x7745,0x7757,0x775c,0x7761,0xf57],2:[0x76fd,0x7702,0x7704,0x7706,0x7708,0x770a,0x770c,0x773e,0x7745,0x7747,0x774e,0x7750,0x7752,0x23ec,0x138d,0x7757,0x775c,0x7761,0x7869]}
    assert set(q['call'] for q in rows)==set(expected)
    for call,ips in expected.items():assert [q['cpu_regs.ip.dword[0]'] for q in rows if q['call']==call]==ips,call
    assert len(rows)==32 and len(states)==22 and all(len(m)==16777216 for m in ram.values())
    wanted={1:[0x76fd,0x7702,0x773e,0x7745,0x7757,0x775c,0x7761,0xf57],2:[0x76fd,0x7702,0x773e,0x7745,0x7747,0x774e,0x7750,0x7752,0x23ec,0x138d,0x7757,0x775c,0x7761,0x7869]}
    assert set(states)=={'%02d-%04x'%(call,ip) for call,ips in wanted.items() for ip in ips}
    label=lambda q:'%02d-%04x'%(q['call'],q['cpu_regs.ip.dword[0]'])
    for name,q in states.items():assert [r for r in rows if label(r)==name]==[q],name
    lengths={0x76fd:5,0x7702:2,0x7704:2,0x7706:2,0x7708:2,0x770a:2,0x770c:2,0x773e:7,0x7745:2,0x7747:7,0x774e:2,0x7750:2,0x7752:5,0x7757:5,0x775c:5,0x7761:1,0xf57:6,0x23ec:1,0x138d:1,0x7869:1}
    for q in rows:
     ip=q['cpu_regs.ip.dword[0]'];assert bytes.fromhex(q['fetched_code_hex'])[:lengths[ip]]==image[ip:ip+lengths[ip]],label(q)
    for name,q in states.items():
     m=ram[name];ip=q['cpu_regs.ip.dword[0]'];p=physical((q['segments'][1]['base']+ip)&0xffffffff,m,q)
     assert p==q['fetched_code_physical'] and m[p:p+32].hex()==q['fetched_code_hex'],name
     for off,key in ((0x77e0,'ready'),(0x77e1,'mode'),(0x2293,'mixer_active')):
      p=physical((q['segments'][3]['base']+off)&0xffffffff,m,q);assert m[p]==q[key],(name,key)
     p=physical((q['segments'][3]['base']+0x2716)&0xffffffff,m,q);assert struct.unpack_from('<I',m,p)[0]==q['mixer_pointer']
    def clock(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
    checks=[];memory_checks=[];current=None
    for q,b in zip(rows,rows[1:]):
     name=label(q)
     if name in ram:current=bytearray(ram[name])
     ip=q['cpu_regs.ip.dword[0]']
     if q['call']!=b['call'] or ip in (0x23ec,0x138d,0xf57,0x7869):continue
     assert current is not None
     instruction=image[ip:ip+lengths[ip]];regs=q['registers'].copy();next_ip=ip+len(instruction)
     flags={k:v for k,v in q.items() if k=='cpu_regs.flags' or k.startswith('lflags.')}
     def flat(off,seg=3):return physical((q['segments'][seg]['base']+off)&0xffffffff,current,q)
     def cmp(v1,v2):
      flags['lflags.type']=types['t_CMPb']
      for key,v in (('var1',v1),('var2',v2),('res',(v1-v2)&255)):
       k='lflags.'+key+'.dword[0]';flags[k]=(flags[k]&0xffffff00)|(v&255)
     if instruction[0]==0xa2:current[flat(struct.unpack_from('<I',instruction,1)[0])]=regs[0]&255
     elif instruction[0]==0x3c:cmp(regs[0]&255,instruction[1])
     elif instruction[:2]==b'\x80\x3d':cmp(current[flat(struct.unpack_from('<I',instruction,2)[0])],instruction[6])
     elif instruction[0] in (0x74,0x75):
      assert flags['lflags.type']==types['t_CMPb']
      zero=flags['lflags.res.dword[0]']&255==0
      if zero==(instruction[0]==0x74):next_ip+=struct.unpack_from('<b',instruction,1)[0]
     elif instruction[0]==0xb0:regs[0]=(regs[0]&0xffffff00)|instruction[1]
     elif instruction[0]==0xb8:regs[0]=struct.unpack_from('<I',instruction,1)[0]
     elif instruction[0]==0xa3:struct.pack_into('<I',current,flat(struct.unpack_from('<I',instruction,1)[0]),regs[0])
     elif instruction[0]==0xe8:
      regs[4]=(regs[4]-4)&0xffffffff;struct.pack_into('<I',current,flat(regs[4],2),next_ip)
      next_ip=(next_ip+struct.unpack_from('<i',instruction,1)[0])&0xffffffff
     elif instruction[0]==0xc3:
      next_ip=struct.unpack_from('<I',current,flat(regs[4],2))[0];regs[4]=(regs[4]+4)&0xffffffff
     else:raise AssertionError((name,instruction.hex()))
     assert b['cpu_regs.ip.dword[0]']==next_ip and b['registers']==regs,(name,label(b),'GP/IP')
     assert b['segments']==q['segments'],(name,'segments')
     for k,v in flags.items():assert b[k]==v,(name,k,hex(b[k]),hex(v))
     for k in q:
      if k.startswith(('cpu.','paging.')) or k in ('PIC_Ticks','CPU_CycleMax','CPU_CycleLeft'):assert b[k]==q[k],(name,k)
     assert clock(b)-clock(q)==1,(name,'one fetched instruction')
     checks.append([name,label(b)])
     if label(b) in ram:
      assert bytes(current)==ram[label(b)],(name,label(b),'whole RAM')
      memory_checks.append([name,label(b)])
    assert len(checks)==28 and len(memory_checks)==18
    assert states['01-76fd']['ready']==0 and states['01-76fd']['registers'][0]==2
    detail=json.loads((repo/'tools/oracle/detail_return_state_case.json').read_text())
    assert states['01-76fd']['registers'][0]==detail['op68_input_EAX']
    assert states['01-76fd']['registers'][3]==detail['returned_EBX']
    assert {s:digest(Path(str(prefix)+'.'+s)) for s in ('frames','pcm','end')}==detail['capture_sha256']
    assert states['02-76fd']['ready']==1 and states['02-76fd']['mixer_active']==0
    assert states['02-76fd']['registers'][0]==0x74e02
    assert states['02-23ec']['registers'][0]==0x74e01
    assert states['02-138d']['mixer_active']==1 and states['02-7757']['mixer_active']==1
    proof=dict(scope='Source-only actual mode-2 effects calls: ready0 skips23ec; ready1/mixer0 reaches23ec with fullEAX74e01 and enters actual138d. Direct fetched instructions prove BYTE mode/ready/active, fullEAX pointer and DWORD near CALL/RET. Nested23ec/device execution is bracketed only, not interpreted or accepted as a port implementation. No port fix or full output acceptance.',
     states=states,fetches=rows,instruction_transitions=checks,whole_RAM_transitions=memory_checks,memory_sha256={n:digest(folder/(n+'.memory')) for n in states},capture_sha256={s:digest(Path(str(prefix)+'.'+s)) for s in ('frames','pcm','end')},frames=39,mixed_samples=27518,endpoint_ms=600,originals_unchanged=len(originals),producers=json.loads((root/'producers.json').read_text()),original_binary_sha256=digest(repo/'third_party/dosbox-fist'),image_sha256=digest(repo/'re_out/fist_image.bin'),code_hex=image[0x76fd:0x7762].hex(),verifier_sha256=digest(Path(__file__)),lazyflags_sha256=digest(repo/"third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h"),complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('PASS: original32 fetches/22 complete state-RAM boundaries,28 instruction and18 whole-RAM transitions; complete39 frames/27518PCM unchanged')
    return proof

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    verify(args.output.resolve(strict=True),args.repo.resolve(strict=True))

if __name__=='__main__':main()
