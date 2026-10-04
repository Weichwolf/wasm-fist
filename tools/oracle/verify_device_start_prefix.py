#!/usr/bin/env python3
"""Verify complete original startup-prefix state, paged operands and real guest frames."""
from pathlib import Path
import argparse,ast,copy,hashlib,json,struct,sys
from sequence_format import validate,validate_endpoint


def digest(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def producer_paths(repo):
    return tuple(repo/p for p in (
        'tools/oracle/capture_device_start_prefix.py',
        'tools/oracle/device_start_prefix.gdb',
        'tools/oracle/verify_device_start_prefix.py',
        'third_party/dosbox-fist',
        'tools/oracle/file_error.gdb',
        'tools/oracle/capture_sequence.sh',
        'tools/oracle/sequence_format.py',
        're_out/fist_image.bin',
        'tools/oracle/sb_reset_clock_case.json',
        'tools/oracle/sound_bank_startup_case.json',
        'tools/oracle/device_reset_case.json',
        'tools/oracle/device_config_1280_case.json',
        'armoredfist/FIST.RUN',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/instructions.h',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal/prefix_66.h',
        'third_party/dosbox-build/dosbox-0.74-3/include/cpu.h',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal.cpp',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal/prefix_none.h',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/core_normal/prefix_0f.h',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/flags.cpp',
        'third_party/dosbox-build/dosbox-0.74-3/src/cpu/cpu.cpp',
        'third_party/dosbox-build/dosbox-0.74-3/src/hardware/pic.cpp',
        'third_party/dosbox-build/dosbox-0.74-3/src/hardware/iohandler.cpp',
        'third_party/dosbox-build/dosbox-0.74-3/src/hardware/sblaster.cpp',
    ))

def verify(root,repo):
    for name in ('baseline','source'):
     assert (root/(name+'.exit')).read_text()=='0\n'
     assert 'Python Exception' not in (root/name/'dosbox.log').read_text()
     prefix=root/name/'sequence';assert validate_endpoint(prefix,600)==600
     assert validate(str(prefix)+'.frames','F')['records']==39
     assert validate(str(prefix)+'.pcm','A')['samples']==27518
    for name in ('frames','pcm','end'):
     assert (root/'baseline'/('sequence.'+name)).read_bytes()==(root/'source'/('sequence.'+name)).read_bytes()
    for name in ('text','bda','vga'):
     assert (root/'baseline'/('start-state.'+name)).read_bytes()==(root/'source'/('start-state.'+name)).read_bytes()
    producers=json.loads((root/'producers.json').read_text())
    assert producers[str(Path(__file__).resolve())]==digest(__file__)
    assert set(producers)=={str(p) for p in producer_paths(repo)}
    for p,h in producers.items():assert digest(p)==h,p
    originals=json.loads((root/'original-hashes.json').read_text())
    assert {str(p.relative_to(repo)) for p in (repo/'armoredfist').rglob('*') if p.is_file()}==set(originals)
    for p,h in originals.items():assert digest(repo/p)==h,p
    rows=[json.loads(x) for x in (root/'source/prefix-fetches.jsonl').read_text().splitlines()];assert len(rows)==242
    bank=json.loads((repo/'tools/oracle/sound_bank_startup_case.json').read_text())
    assert {s:digest(root/'source'/('sequence.'+s)) for s in ('frames','pcm','end')}==bank['capture_sha256']
    gold=json.loads((repo/'tools/oracle/device_reset_case.json').read_text())['fetches']
    # Only observer metadata differs: entry_return_ip names this outer77e2 frame.
    # Every architectural/code/address/time field is compared without narrowing.
    normal=lambda q:{k:v for k,v in q.items() if k!='entry_return_ip'}
    assert [normal(q) for q in rows[14:239]]==[normal(q) for q in gold]
    shared=repo/'tools/oracle/file_error.gdb'
    program=shared.read_text().split('\npython\n',1)[1].rsplit('\nend\nrun',1)[0]
    nodes=[n for n in ast.parse(program).body if isinstance(n,ast.FunctionDef) and n.name=='physical'];assert len(nodes)==1
    namespace={'struct':struct};exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),str(shared),'exec'),namespace);physical=namespace['physical']
    image=(repo/'re_out/fist_image.bin').read_bytes()
    config=json.loads((repo/'tools/oracle/device_config_1280_case.json').read_text())
    assert image[config['code_offset']:config['code_offset']+len(bytes.fromhex(config['code_bytes']))]==bytes.fromhex(config['code_bytes'])
    lazy=(repo/'third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h').read_text()
    import re
    types=re.findall(r'\bt_[A-Za-z0-9_]+\b',lazy.split('//Types of Flag changing instructions',1)[1].split('enum {',1)[1].split('};',1)[0])
    assert types.index('t_CMPw')==23 and types.index('t_ORb')==4
    memory=bytearray((root/'source/77e2.memory').read_bytes());assert len(memory)==16777216
    first=rows[0]
    def address(q,s,a):return physical((q['segments'][s]['base']+a)&0xffffffff,memory,q)
    def read(q,s,a,n):p=address(q,s,a);return int.from_bytes(memory[p:p+n],'little')
    def low(old,v,bits):mask=(1<<bits)-1;return (old&~mask)|(v&mask)
    def clock(q):return q['PIC_Ticks']*q['CPU_CycleMax']+q['CPU_CycleMax']-q['CPU_Cycles']-q['CPU_CycleLeft']
    assert first['cpu_regs.ip.dword[0]']==0x77e2
    assert read(first,2,first['registers'][4],4)==first['entry_return_ip']==0xf57
    boundaries={int(p.stem,16):p for p in (root/'source').glob('*.memory')}
    assert set(boundaries)=={0x77e2,0x77e9,0x1280,0x12ab,0x77ee,0x23c4,0x133a,0x77ff,0x7809}
    assert {int(p.stem,16) for p in (root/'source').glob('*.json')}==set(boundaries)
    expected_prefix=[0x77e2,0x77e9,0x1280,0x1286,0x128d,0x1293,0x129a,0x129f,0x12a6,0x12ab,0x77ee,0x77f6,0x77f8,0x77fa]
    assert [q['cpu_regs.ip.dword[0]'] for q in rows[:14]]==expected_prefix
    assert [q['cpu_regs.ip.dword[0]'] for q in rows[238:]]==[0x77ff,0x7801,0x7803,0x7809]
    calls=[]
    for i,q in enumerate(rows):
     ip=q['cpu_regs.ip.dword[0]'];p=address(q,1,ip)
     assert q['fetched_code_physical']==p and bytes.fromhex(q['fetched_code_hex'])==memory[p:p+32]==image[ip:ip+32]
     assert q['entry_return_ip']==0xf57
     if ip in boundaries:
      assert memory==boundaries[ip].read_bytes(),hex(ip)
      assert q==json.loads(boundaries[ip].with_suffix('.json').read_text())
     if i==len(rows)-1:break
     if 14<=i<238:continue # Existing source owner already proves all224 transitions and unchanged RAM.
     expected=copy.deepcopy(q);r=expected['registers'];next_ip=None
     if ip==0x77e2:
      assert image[ip:ip+7]==bytes.fromhex('c605e077000000');memory[address(q,3,0x77e0)]=0;next_ip=ip+7
     elif ip in (0x77e9,0x77fa):
      assert image[ip]==0xe8
      return_ip=ip+5;next_ip=(return_ip+struct.unpack_from('<i',image,ip+1)[0])&0xffffffff
      r[4]=(r[4]&q['cpu.stack.notmask'])|((r[4]-4)&q['cpu.stack.mask'])
      p=address(q,2,r[4]&q['cpu.stack.mask']);memory[p:p+4]=struct.pack('<I',return_ip)
      calls.append(dict(caller_ip=ip,target_ip=next_ip,return_ip=return_ip,stack_physical=p,esp_before=q['registers'][4],esp_after=r[4]))
     elif ip==0x1280:r[3]=read(q,3,0xc93,4);next_ip=0x1286
     elif ip==0x1286:r[0]=low(r[0],read(q,3,r[3]+0x490,2),16);next_ip=0x128d
     elif ip==0x128d:
      p=address(q,3,0x12cc);memory[p:p+2]=struct.pack('<H',r[0]&65535);next_ip=0x1293
     elif ip==0x1293:r[0]=read(q,3,r[3]+0x492,2);next_ip=0x129a
     elif ip==0x129a:
      p=address(q,3,0x12c4);memory[p:p+4]=struct.pack('<I',r[0]);next_ip=0x129f
     elif ip==0x129f:r[0]=read(q,3,r[3]+0x494,2);next_ip=0x12a6
     elif ip==0x12a6:
      p=address(q,3,0x12c8);memory[p:p+4]=struct.pack('<I',r[0]);next_ip=0x12ab
     elif ip==0x12ab:
      assert image[ip]==0xc3;next_ip=read(q,2,r[4]&q['cpu.stack.mask'],4)
      r[4]=(r[4]&q['cpu.stack.notmask'])|((r[4]+4)&q['cpu.stack.mask'])
     elif ip==0x77ee:
      assert image[ip:ip+8]==bytes.fromhex('66833dcc12000000');a=read(q,3,0x12cc,2)
      expected['lflags.var1.dword[0]']=low(q['lflags.var1.dword[0]'],a,16)
      expected['lflags.var2.dword[0]']=low(q['lflags.var2.dword[0]'],0,16)
      expected['lflags.res.dword[0]']=low(q['lflags.res.dword[0]'],a,16);expected['lflags.type']=23;next_ip=0x77f6
     elif ip in (0x77f6,0x7801):
      assert image[ip]==0x74
      bits=16 if ip==0x77f6 else 8
      zero=(q['lflags.res.dword[0]']&((1<<bits)-1))==0
      next_ip=ip+2+(struct.unpack_from('<b',image,ip+1)[0] if zero else 0)
     elif ip==0x77f8:
      assert image[ip:ip+2]==bytes.fromhex('b001');r[0]=low(r[0],1,8);next_ip=0x77fa
     elif ip==0x77ff:
      assert image[ip:ip+2]==bytes.fromhex('0ac0');a=r[0]&255
      for k in ('lflags.var1.dword[0]','lflags.var2.dword[0]','lflags.res.dword[0]'):expected[k]=low(q[k],a,8)
      expected['lflags.type']=4;next_ip=0x7801
     elif ip==0x7803:
      assert image[ip:ip+6]==bytes.fromhex('8d1598bc0000');r[2]=0xbc98;next_ip=0x7809
     else:raise AssertionError(hex(ip))
     expected['cpu_regs.ip.dword[0]']=next_ip
     expected['CPU_Cycles']-=1
     following=rows[i+1]
     for k in ('fetched_code_physical','fetched_code_hex'):expected[k]=following[k]
     assert expected==following,(hex(ip),{k:(expected[k],following[k]) for k in expected if expected[k]!=following[k]})
     assert clock(following)==clock(q)+1
    case=dict(scope='Original77e2 caller/configuration/reset prefix;242 complete fetch states,241 CPU transitions (224 consumed from the existing reset owner,17 caller/configuration transitions), nine whole16MiB memory boundaries. Does not prove complete77e2 or port integration.',fetches=rows,memory_sha256={p.name:digest(p) for p in boundaries.values()},entry_return_ip=0xf57,first_post_reset_lazy_type=4,first_post_reset_lazy_ip='7801',source_reset_architectural_rows_equal=225,configuration=dict(tcb_operand=read(rows[2],3,0xc93,4),tcb_linear=(rows[2]['segments'][3]['base']+read(rows[2],3,0xc93,4))&0xffffffff,tcb_physical=address(rows[2],3,read(rows[2],3,0xc93,4))),calls=calls,source_frames=39,source_mixed_samples=27518,endpoint_ms=600,all419_originals_unchanged=True,producers_sha256=digest(root/'producers.json'),verification_inputs={str(repo/p):digest(repo/p) for p in ('tools/oracle/device_reset_case.json','tools/oracle/device_config_1280_case.json','tools/oracle/sound_bank_startup_case.json','third_party/dosbox-build/dosbox-0.74-3/src/cpu/lazyflags.h')},verifier_sha256=digest(__file__),complete_original_acceptance=False)
    case.update(producers=json.loads((root/'producers.json').read_text()),reproduction='python3 -B tools/oracle/capture_device_start_prefix.py --repo . --output /tmp/wasm-fist-device-start-prefix-source',source_original_files=len(originals))
    (root/'proof.json').write_text(json.dumps(case,indent=2)+'\n')
    print('PASS: all242 complete original prefix states,17 additional caller/configuration transitions,nine whole-RAM boundaries and39frame/27518PCM/end600 outputs; production integration remains open.')

    return case


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    return verify(args.output.resolve(strict=True),args.repo.resolve(strict=True))


if __name__=='__main__':
    main()
