#!/usr/bin/env python3
"""Compare complete original VGA DAC ports and APIs on both release targets."""
import argparse,hashlib,json,re,struct
from pathlib import Path
from palette_control_fixture import build,compare,digest,run


def fixtures(repo,root):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    source=(tree/'src/gui/render.cpp').read_text()
    set_pal=re.search(r'void RENDER_SetPal\(.*?\n\}',source,re.S)[0]
    # Observe after the unchanged original function body has committed its state.
    set_pal=set_pal[:-1]+' event(entry,red,green,blue);\n}'
    original=(repo/'tools/oracle/dac_probe.cpp').read_text()
    original=original.replace('@DOSBOX_DAC@',str(tree/'src/hardware/vga_dac.cpp'))
    original=original.replace('@RENDER_SET_PAL@',set_pal)
    (root/'original.cpp').write_text(original)
    (root/'portable.c').write_bytes((repo/'tests/dac_controlled.c').read_bytes())
    (root/'cases.inc').write_bytes((repo/'tools/oracle/dac_cases.inc').read_bytes())


def rows(data):
 result=[];offset=0
 while offset<len(data):
  assert offset+3815<=len(data)
  count=struct.unpack_from('<I',data,offset+3811)[0];assert count<=256
  end=offset+3815+count*16;assert end<=len(data)
  result.append(data[offset:end]);offset=end
 assert len(result)==4
 return result


def causal(repo,root):
    work=root/'causal';work.mkdir()
    common=(root/'cases.inc').read_text();common=common[:common.index('int main(int argc,char **argv) {')]+r'''int main(int argc,char **argv) {
     REQUIRE(argc==2);output=fopen(argv[1],"wb");REQUIRE(output!=NULL);
     reset(3,255,2,127,1,1);set_entry(255,255,128,64);record(0);
     reset(3,71,2,127,1,1);SET_CARD(1);combine(7,255);record(0);
     reset(3,253,255,127,255,1);SET_MACHINE(4);setup();record(0);
     reset(3,253,255,127,255,1);setup();record(0);
     REQUIRE(programs==4);REQUIRE(!ferror(output) && !fclose(output));return 0;
    }
    '''
    (work/'cases.inc').write_text(common)
    (work/'original.cpp').write_bytes((root/'original.cpp').read_bytes())
    command=build(repo,work,original=True)[0][1]
    run(command,work/'original.raw');expected=(work/'original.raw').read_bytes()
    wanted=rows(expected);header=(repo/'re_out/fist_dac.h').read_text()
    def compile_mutant(folder,content):
        folder.mkdir();(folder/'fist_dac.h').write_text(content)
        (folder/'portable.c').write_bytes((root/'portable.c').read_bytes())
        (folder/'cases.inc').write_text(common)
        return build(repo,folder)
    for target,command in compile_mutant(work/'positive',header):
        output=work/(target+'-positive.raw');run(command,output)
        assert output.read_bytes()==expected,(target,'Complete source positive')
        output.unlink()
    mutations=[
     ('set-entry-sends-new-entry',header.replace('fist_dac_send(d,opaque,set_pal,i,i);','fist_dac_send(d,opaque,set_pal,i,entry);'),0,'dac.xlat16[0]'),
     ('combine-ignores-svga',header.replace('if(mode==3 && (!is_vga || !svga_none))return;','if(mode==3 && !is_vga)return;'),1,'dac.xlat16[7]'),
     ('ega-setup-overwrites256',header.replace('else if(is_ega)for(unsigned i=0;i<64;i++)','else if(is_ega)for(unsigned i=0;i<256;i++)'),2,'dac.rgb[64].red'),
     ('vga-setup-clears-palette',header.replace('if(is_vga)for(unsigned port=0x3c6;', 'if(is_vga)for(unsigned i=0;i<256;i++)d->rgb[i]=(FistDacRgb){0,0,0};\n if(is_vga)for(unsigned port=0x3c6;'),3,'dac.rgb[0].red'),
    ]
    results=[]
    for name,mutant,index,field in mutations:
     assert mutant!=header
     folder=work/name
     for target,command in compile_mutant(folder,mutant):
      run(command,folder/(target+'.raw'));data=(folder/(target+'.raw')).read_bytes();actual=rows(data)
      first=next(i for i,(a,b) in enumerate(zip(actual,wanted)) if a!=b);assert first==index,(name,target,first,index)
      offset=next(i for i,(a,b) in enumerate(zip(actual[first],wanted[first])) if a!=b)
      expected_offsets={'dac.xlat16[0]':802,'dac.xlat16[7]':816,'dac.rgb[64].red':226,'dac.rgb[0].red':34}
      assert offset==expected_offsets[field],(name,target,offset,field)
      results.append(dict(fault=name,target=target,terminal_exit=0,first_differing_program=first,first_differing_byte=offset,first_differing_named_field=field,all_preceding_complete_records_equal=True,unmodified_complete_source_positive_equal=True,original_sha256=hashlib.sha256(expected).hexdigest(),mutant_sha256=hashlib.sha256(data).hexdigest()))
      (folder/(target+'.raw')).unlink()
     print('PASS full original DAC APIs distinguish',name,'on both release targets',flush=True)
    (work/'original.raw').unlink()
    proof=dict(scope='Four original DAC API substitutions fail both targets after complete source positives.',results=results)
    (work/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    return results


def capture(repo,root):
    assert root.is_relative_to(Path('/tmp'))
    root.mkdir(parents=True,exist_ok=False);fixtures(repo,root)
    results=compare(repo,root,671224)
    negatives=causal(repo,root)
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    paths=[Path(__file__),repo/'tools/oracle/palette_control_fixture.py',
           repo/'tools/oracle/dac_probe.cpp',repo/'tests/dac_controlled.c',
           repo/'tools/oracle/dac_cases.inc',root/'original.cpp',root/'portable.c',root/'cases.inc',
           repo/'re_out/fist_dac.h',repo/'re_out/fist_render_palette.h',
           tree/'src/hardware/vga_dac.cpp',tree/'src/gui/render.cpp',tree/'config.h',
           *sorted((tree/'include').glob('*.h'))]
    proof=dict(scope='Complete original DAC ports3c6..3c9,CombineColor,SetEntry,SetupDAC and RENDER_SetPal compare671224 programs. Every named DAC/renderer field,ordered notification and handler registration matches Native32/WASM. Actual production callers,reset/consumer timing,protected faults and full frame/audio parity remain separate.',
               results=results,causal_negatives=negatives,
               inputs_sha256={str(p):digest(p) for p in paths},complete_original_acceptance=False)
    (root/'proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    return proof


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();capture(args.repo.resolve(),args.output.resolve())
