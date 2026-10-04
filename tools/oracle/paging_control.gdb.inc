# Additional read-only CPU_WRITE_CRX observer; loaded by capture_paging_control.py.
crx_events=0

def cache_state(watched=()):
 used=int(gdb.parse_and_eval('paging.links.used'))
 links=[int(gdb.parse_and_eval('paging.links.entries[%d]'%i)) for i in range(used)]
 pages=sorted(set(links)|set(watched))
 cache={}
 for page in pages:
  q={n:int(gdb.parse_and_eval('paging.tlb.%s[%d]'%(n,page))) for n in ('read','write','phys_page','readhandler','writehandler')}
  q['handler_types']={n:str(gdb.parse_and_eval('paging.tlb.%s[%d]'%(n,page)).dynamic_type) for n in ('readhandler','writehandler')}
  q['handler_flags']={n:int(gdb.parse_and_eval('paging.tlb.%s[%d]->flags'%(n,page))) for n in ('readhandler','writehandler')}
  cache[str(page)]=q
 return {'linked_pages':links,'entries':cache,'mem_base':int(gdb.parse_and_eval('MemBase')),'init_handler':int(gdb.parse_and_eval('&init_page_handler'))}

def crx_record(kind,label,args,watched=(),return_value=None):
 q=state();q.update(kind=kind,label=label,arguments=args,context=boot_context,group=boot_group,
    architecture=int(gdb.parse_and_eval('CPU_ArchitectureType')),
    auto_determine=int(gdb.parse_and_eval('CPU_AutoDetermineMode')),
    normal_core=bool(gdb.parse_and_eval('cpudecoder == &CPU_Core_Normal_Run')),
    cache=cache_state(watched),memory_file='crx-%s-%d.memory'%(kind,label))
 if return_value is not None:q['return_value']=int(return_value)
 (root/q['memory_file']).write_bytes(memory())
 with (root/'crx-events.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
 return q

class CrxReturned(gdb.FinishBreakpoint):
 def __init__(self,label,args,watched):
  self.label=label;self.args=args;self.watched=watched;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  crx_record('after-crx',self.label,self.args,self.watched,self.return_value);return False

class Crx(gdb.Breakpoint):
 def stop(self):
  global crx_events
  if not boot_active:return False
  args={k:int(gdb.parse_and_eval(k)) for k in ('cr','value')}
  crx_events+=1
  q=crx_record('before-crx',crx_events,args)
  CrxReturned(crx_events,args,q['cache']['linked_pages']);return False
Crx('*CPU_WRITE_CRX')
