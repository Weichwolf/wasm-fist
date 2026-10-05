"""Source-owned bootstrap boundaries with complete CPU/RAM/device comparisons."""
from pathlib import Path
import hashlib,json,struct,subprocess
from cpu_core_exit_fixture import build,core_packet,observation,pic_packet
from device_cpu_fixture import clock
from memory_context_fixture import expected_memory_context
from cpu_trace import records
from capture_cpu_task_gate import trace_key

def replay(work,source,runs,rows,*,complete=True,expected_rows=None,compare_context=True,strict=True):
    work.mkdir(parents=True,exist_ok=True)
    folder=source/'source';packet=work/'initial.input';packet.write_bytes(core_packet(rows[0],folder))
    boundary=work/'boundaries.input'
    boundary.write_text(''.join('%x %x %s\n'%(q['segments'][1]['value'],q['cpu_regs.ip.dword[0]'],q['kind']) for q in rows))
    trace=list(records(source/'cpu.trace'))
    def match(row):
        key=trace_key(row)
        matches=[i for i,r in enumerate(trace) if r==key]
        assert len(matches)==1,(row['kind'],matches)
        return matches[0]
    begin=match(rows[0]);end=match(rows[-1]);assert end>begin
    expected=trace[begin:end+1]
    results=[]
    for target,command in runs:
        output=work/target
        p=subprocess.run([*command,str(packet),str(folder/rows[0]['memory_file']),str(output),str(boundary)],capture_output=True,text=True,timeout=30)
        (work/(target+'.log')).write_text(p.stdout+p.stderr)
        lines=p.stdout.splitlines();states=[s for s in lines if not s.startswith('fetch ')]
        seen=[]
        for line in lines:
            if not line.startswith('fetch '):continue
            fields=line.split();assert len(fields)==63
            raw=[int(v,16) for v in fields[5:]]
            seen.append((int(fields[1]),(raw[11],raw[8]),tuple(raw[:8]),tuple((raw[i],raw[i+1]) for i in range(9,21,2))))
        if complete:
            assert p.returncode==0,(target,p.returncode,p.stderr)
            if strict:
                assert states==[observation(q) for q in rows],(target,'Complete CPU/system/time states differ')
                assert seen==expected,(target,'Complete instruction trace prefix differs')
            selected=rows
        else:
            selected=rows[:1] if expected_rows is None else expected_rows
            assert p.returncode!=0 and states==[observation(q) for q in selected],(target,p.returncode,states)
            failing=match(selected[-1]);assert seen==trace[begin:failing+1],(target,'Prior decoder must reach and fail on the first missing opcode fetch')
        artifacts=[]
        for row in selected:
            for suffix,data in [('memory',(folder/row['memory_file']).read_bytes()),('context',expected_memory_context(row,folder)),('pic',pic_packet(row))]:
                path=Path(str(output)+'-'+row['kind']+'.'+suffix)
                actual=path.read_bytes()
                if compare_context or suffix!='context':assert actual==data,(target,row['kind'],suffix)
                else:artifacts.append(dict(boundary=row['kind'],kind='context-diagnostic',equal=actual==data))
                artifacts.append(dict(boundary=row['kind'],kind=suffix,bytes=len(actual),sha256=hashlib.sha256(actual).hexdigest()))
                path.unlink()
        results.append(dict(target=target,terminal_exit=p.returncode,boundaries=len(selected),fetches=len(seen),artifacts=artifacts,complete_context_equal=compare_context,
            CPU_time_equal=states==[observation(q) for q in selected],trace_equal=seen==expected,
            observations=states,fetch_records=seen,complete_original_acceptance=False))
    return results
