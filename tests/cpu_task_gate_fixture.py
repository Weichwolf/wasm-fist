"""Source-owned bootstrap boundaries with complete CPU/RAM/device comparisons."""
from pathlib import Path
import hashlib,json,struct,subprocess
from cpu_core_exit_fixture import build,core_packet,observation,pic_packet
from device_cpu_fixture import clock
from memory_context_fixture import expected_memory_context
from cpu_trace import records
from capture_cpu_task_gate import trace_key

def replay(work,source,runs,rows,*,complete=True,expected_rows=None,compare_context=True,strict=True,dos=False,dos_events=(),compare_artifacts=True):
    work.mkdir(parents=True,exist_ok=True)
    folder=source/'source';packet=work/'initial.input';packet.write_bytes(core_packet(rows[0],folder))
    boundary=work/'boundaries.input'
    boundary.write_text(''.join('%x %x %s\n'%(q['segments'][1]['value'],q['cpu_regs.ip.dword[0]'],q['kind']) for q in rows))
    observed = rows
    arguments=[]
    if dos:
        callback=next(q for q in rows if q['kind']=='before-dos-callback')
        code=bytes.fromhex(callback['opcode_hex']);assert code.startswith(bytes.fromhex('fe38'))
        h=rows[0]['HOST'];case=json.loads((source/'proof.json').read_text())
        result=next(q for q in case['find_events'] if q['kind']=='before-SetResult')['arguments']
        host=work/'host.input'
        host.write_bytes(struct.pack('<III',h['drive'],h['next_free'],int.from_bytes(code[2:4],'little'))+
                         bytes(h['occupied'])+bytes.fromhex(result['name_raw_hex']))
        arguments=[str(host),str(folder/'game/FISTDATA')]
        if dos_events:
            at=next(i for i,q in enumerate(rows) if q['kind']=='before-find-first-callback')+1
            observed=rows[:at]+[dict(q,kind='find-'+q['kind']) for q in dos_events]+rows[at:]
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
        p=subprocess.run([*command,str(packet),str(folder/rows[0]['memory_file']),str(output),str(boundary),*arguments],capture_output=True,text=True,timeout=30)
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
                assert states==[observation(q) for q in observed],(target,'Complete CPU/system/time states differ')
                assert seen==expected,(target,'Complete instruction trace prefix differs')
            selected=observed
        else:
            selected=rows[:1] if expected_rows is None else expected_rows
            assert p.returncode!=0 and states==[observation(q) for q in selected],(target,p.returncode,states)
            failing=match(selected[-1]);assert seen==trace[begin:failing+1],(target,'Prior decoder must reach and fail on the first missing opcode fetch')
        artifacts=[]
        for row in selected:
            for suffix,data in [('memory',(folder/row['memory_file']).read_bytes()),('context',expected_memory_context(row,folder)),('pic',pic_packet(row))]:
                path=Path(str(output)+'-'+row['kind']+'.'+suffix)
                actual=path.read_bytes()
                if compare_artifacts and (compare_context or suffix!='context'):assert actual==data,(target,row['kind'],suffix)
                if not compare_context and suffix=='context':artifacts.append(dict(boundary=row['kind'],kind='context-diagnostic',equal=actual==data))
                artifacts.append(dict(boundary=row['kind'],kind=suffix,equal=actual==data,bytes=len(actual),sha256=hashlib.sha256(actual).hexdigest()))
                path.unlink()
        host_errors=[]
        if dos:
            for label,q in [('initial-host',rows[0]),*[(q['kind'],q) for q in selected]]:
                path=Path(str(output)+'-'+label+'.host');h=q['HOST']
                expected_host=struct.pack('<II',h['drive'],h['next_free'])+bytes(h['occupied'])
                if not path.exists():host_errors.append(dict(boundary=label,missing=True))
                elif path.read_bytes()!=expected_host:host_errors.append(dict(boundary=label,missing=False))
                path.unlink(missing_ok=True)
            if strict:assert not host_errors,(target,host_errors)
        first=next((dict(index=i,original=a,port=b) for i,(a,b) in enumerate(zip(expected,seen)) if a!=b),None)
        state_errors=[]
        for i,(actual,q) in enumerate(zip(states,selected)):
            original=observation(q)
            if actual!=original:state_errors.append(dict(boundary=q['kind'],words=[k for k,(a,b) in enumerate(zip(original.split(),actual.split())) if a!=b]))
        results.append(dict(target=target,terminal_exit=p.returncode,boundaries=len(selected),fetches=len(seen),artifacts=artifacts,complete_context_equal=compare_context,
            CPU_time_equal=states==[observation(q) for q in selected],trace_equal=seen==expected,
            host_errors=host_errors,state_errors=state_errors,first_trace_difference=first,observations=states,fetch_records=seen,complete_original_acceptance=False))
    return results
