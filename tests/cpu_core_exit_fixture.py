"""Complete source-observed first IRET/PIC/IRQ composition on both release targets."""
import json
from pathlib import Path
import struct
import subprocess
from device_cpu_fixture import words,clock
from memory_context_fixture import system_words,memory_packet,expected_memory_context
from test_port_io import ROOT,tool

def pic_packet(q):
    data=[]
    for irq in q['pic']['irqs']:data += [irq[k] for k in ('masked','active','inservice','vector')]
    for pic in q['pic']['controllers']:data += [pic[k] for k in ('icw_words','icw_index','special','auto_eoi','rotate_on_auto_eoi','single','request_issr')]
    data += [q['PIC_IRQCheck'],q['PIC_IRQActive']]
    assert len(data)==80
    return struct.pack('<80I',*data)

def observation(row):
    return row['kind']+' '+str(clock(row))+' '+str(row['PIC_Ticks'])+' '+str(row['CPU_Cycles'])+' '+str(row['CPU_CycleLeft'])+' '+' '.join('%08x'%w for w in words(row)+system_words(row))

def build(directory,pic=None,clock_source=None,driver=None,include_dirs=(),extra_flags=()):
    flags=['-O2','-DNDEBUG',*extra_flags,*['-I'+str(p) for p in include_dirs],'-I'+str(ROOT/'tests'),'-I'+str(ROOT/'re_out'),'-ffunction-sections','-fdata-sections','-fno-strict-aliasing','-w']
    sources=[driver or ROOT/'tests/cpu_core_exit.c',clock_source or ROOT/'tests/cpu_core_exit_clock.c',pic or ROOT/'tests/cpu_core_exit_pic.c',ROOT/'re_out/fist_dos.c',ROOT/'re_out/fist_sb.c']
    runs=[]
    for target,compiler,options,output,runner in (
        ('native',['gcc','-m32'],['-Wl,--gc-sections','-lm'],directory/'core-exit',[]),
        ('wasm',[tool('emcc','Git/emsdk/upstream/emscripten/emcc')],['-sNODERAWFS=1','-sEXIT_RUNTIME=1','-sALLOW_MEMORY_GROWTH=1'],directory/'core-exit.js',[tool('node','Git/emsdk/node/*/bin/node')])):
        p=subprocess.run([*compiler,*flags,*map(str,sources),*options,'-o',str(output)],capture_output=True,text=True,timeout=120)
        (directory/(target+'-build.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
        runs.append((target,[*runner,str(output)]))
    return runs

def core_packet(before,folder,clock_packet=None):
    if clock_packet is None:
        extra=[before['PIC_Ticks'],before['CPU_Cycles'],before['CPU_CycleLeft'],len(before['calendar'])]
        for entry in before['calendar']:extra += [entry['index_bits'],entry['value']]
        clock_packet=struct.pack('<%dI'%len(extra),*extra)
    return struct.pack('<58I',*words(before),*system_words(before))+memory_packet(before,folder)+pic_packet(before)+clock_packet

def replay(directory,source,runs,strict=True,fetches=(),rows=None,initial=None,extra_parts=None,compare_failed=False):
    folder=source/'source'
    if rows is None:rows=[json.loads(s) for s in (folder/'events.jsonl').read_text().splitlines()]
    before=rows[0];packet=directory/'initial.input'
    packet.write_bytes(core_packet(before,folder) if initial is None else initial)
    expected=[]
    for row in rows:
        expected.append(observation(row))
        if row['kind']=='handler-fetch':expected.extend(observation(q) for q in fetches)
    expected='\n'.join(expected)+'\n';results=[]
    for target,run in runs:
        output=directory/target
        p=subprocess.run([*run,str(packet),str(folder/before['memory_file']),str(output)],cwd=directory,capture_output=True,text=True,timeout=30)
        (directory/(target+'.log')).write_text(p.stdout+p.stderr)
        same=p.returncode==0 and p.stdout==expected;errors=[]
        try:
            if p.returncode==0 or compare_failed:
                for row in rows:
                    kind=row['kind']
                    parts=[('memory',(folder/row['memory_file']).read_bytes()),('context',expected_memory_context(row,folder)),('pic',pic_packet(row))]
                    if extra_parts:parts.extend(extra_parts(row,folder))
                    for suffix,original in parts:
                        if Path(str(output)+'-'+kind+'.'+suffix).read_bytes()!=original:errors.append((kind,suffix))
            if strict:
                assert p.returncode==0,p.stderr
                assert p.stdout==expected,(target,p.stdout,expected)
                assert not errors,(target,errors)
            results.append(dict(target=target,terminal_exit=p.returncode,complete_CPU_time_equal=same,memory_context_pic_errors=errors))
        finally:
            # Compact observations suffice after complete comparison; never retain
            # duplicate112MiB target RAM per replay.
            for row in rows:
                suffixes=['memory','context','pic']
                if extra_parts:suffixes.extend(suffix for suffix,_ in extra_parts(row,folder))
                for suffix in suffixes:Path(str(output)+'-'+row['kind']+'.'+suffix).unlink(missing_ok=True)
    return results
