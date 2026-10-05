"""Observe the first actual IRQ0 handler prefix after the reaching IRET/PIC chain."""
import argparse
import json
from pathlib import Path
from capture_cpu_core_exit import capture as capture_core,observer as core_observer,verify as verify_core,digest

def observer(repo):
    text=core_observer(repo)
    old="class Fetch(gdb.Breakpoint):\n def stop(self):save('handler-fetch');self.enabled=False;return False"
    assert text.count(old)==1
    return text.replace(old,'''prefix_count=0
class Fetch(gdb.Breakpoint):
 def stop(self):
  global prefix_count
  if not prefix_count:save('handler-fetch')
  q=state();assert not q['paging.enabled']
  address=(q['segments'][1]['base']+q['cpu_regs.ip.dword[0]'])&0xffffffff
  code=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('MemBase'))+address,16))
  q.update(kind='fetch',ordinal=prefix_count+1,code_physical=address,code_hex=code.hex())
  with (root/'prefix-fetches.jsonl').open('a') as f:f.write(json.dumps(q)+'\\n')
  prefix_count+=1;assert prefix_count<100
  if code[0]==0xec:save('first-io');self.enabled=False
  return False''')

def verify(repo,root):
    result=verify_core(repo,root,('first-io',))
    folder=root/'source'
    fetches=[json.loads(s) for s in (folder/'prefix-fetches.jsonl').read_text().splitlines()]
    assert len(fetches)==21 and [q['ordinal'] for q in fetches]==list(range(1,22))
    assert fetches[0]['cpu_regs.ip.dword[0]']==0x3a68
    assert fetches[-1]['code_hex'].startswith('ec')
    raw=(folder/result['events'][6]['memory_file']).read_bytes()
    for q in fetches:
        assert q['CPU_CycleMax']==30000 and not q['paging.enabled']
        address=q['code_physical']
        assert raw[address:address+16].hex()==q['code_hex']
    result.update(scope='One actual original initial pending-IRQ IRET through core/PIC/hardware frame and21 complete first-IRQ0-handler fetches, stopping before first I/O. Eight full58-word/RAM/provider/cache/VGA/PIC/calendar boundaries. Complete39frames/27518PCM/end600 equals unprobed original. No I/O/device-event/whole-handler/runtime adoption.',fetches=fetches)
    (root/'proof.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS complete original IRET/PIC/frame and21-fetch first IRQ0 prefix; original full output unchanged',flush=True)
    return result

def capture(repo,root,baseline=None):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    return capture_core(repo,root,baseline,make_observer=observer,check_capture=verify,
                        additional_inputs=(Path(__file__),tree/'src/cpu/instructions.h',tree/'src/cpu/lazyflags.h'))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--baseline',type=Path);parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args();repo=args.repo.resolve();root=args.output.resolve()
    if args.verify_only:verify(repo,root)
    else:capture(repo,root,args.baseline.resolve() if args.baseline else None)
