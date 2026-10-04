# Complete reached startup/DOS observation; read-only host and guest reads.
detailed_root=root/'handler';detailed_root.mkdir()
detailed_serial=0
def detailed_record(kind):
 global detailed_serial
 detailed_serial+=1
 q=state();q.update(kind=kind,serial=detailed_serial)
 label='handler/%02d-%s.memory'%(detailed_serial,kind)
 q['memory_file']=label;q['memory_context']=memory_context(label)
 (root/label).write_bytes(memory())
 with (detailed_root/'events.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
class DetailedCaller(SoftwareCaller):
 def stop(self):
  if software_active:
   cs=int(gdb.parse_and_eval('Segs.val[cs]'));ip=int(gdb.parse_and_eval('cpu_regs.ip.dword[0]'))
   if (cs,ip) in ((8,0x1abb),(8,0x1abf),(8,0x142f),(8,0x1432),
                 (0x2dd,0x1437),(0x2dd,0x143a),(0x2dd,0x143d),(0x2dd,0x143f),
                 (0x2dd,0x1548),(0x2dd,0x154b),(0x2dd,0x1550),(0x2dd,0x1553),
                 (8,0x1558),(8,0x155d),(8,0x1563),(8,0x1569)):
    detailed_record('fetch-%04x-%04x'%(cs,ip))
  return super().stop()
software_caller.delete();software_caller=DetailedCaller('fist_cpu_trace');software_caller.enabled=False
class DosReturned(gdb.FinishBreakpoint):
 def __init__(self):super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):detailed_record('after-DOS21');return False
class Dos(gdb.Breakpoint):
 def stop(self):
  if software_active:detailed_record('before-DOS21');DosReturned()
  return False
Dos('DOS_21Handler')

following_active=False;following_done=False
following_root=root/'following';following_root.mkdir()
class FollowingCaller(DetailedCaller):
 def stop(self):
  global following_active,following_done
  if following_active:
   q=software_fetch_state();raw=bytes.fromhex(q['fetched_code_hex'])
   with (following_root/'fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
   if q['segments'][1]['value']==ext_cs and raw[:2]==b'\xcd\x21':
    q['memory_file']='following/before-next-DOS.memory'
    q['memory_context']=memory_context(q['memory_file'])
    (root/q['memory_file']).write_bytes(memory())
    (following_root/'before-next-DOS.json').write_text(json.dumps(q)+'\n')
    following_active=False;following_done=True;self.enabled=False
   return False
  result=super().stop()
  if software_done and not software_active and not following_done:
   q=state()
   if q['segments'][1]['value']==ext_cs and q['cpu_regs.ip.dword[0]']==0x5cdf:
    following_active=True;self.enabled=True
  return result
software_caller.delete();software_caller=FollowingCaller('fist_cpu_trace');software_caller.enabled=False

next_active=False;next_done=False;next_serial=0
next_root=root/'findfirst';next_root.mkdir()
def next_record(kind,arguments=None):
 global next_serial
 next_serial+=1;q=state();q.update(kind=kind,serial=next_serial)
 if arguments is not None:q['arguments']=arguments
 label='findfirst/%02d-%s.memory'%(next_serial,kind)
 q['memory_file']=label;q['memory_context']=memory_context(label)
 (root/label).write_bytes(memory())
 with (next_root/'events.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 return q
class NextReturned(gdb.FinishBreakpoint):
 def __init__(self,kind):self.kind=kind;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):next_record(self.kind);return False
class NextSoftware(gdb.Breakpoint):
 def stop(self):
  global next_active,next_done
  if next_done:return False
  q=state()
  if q['segments'][1]['value']!=ext_cs or q['cpu_regs.ip.dword[0]']!=0x5df4:return False
  args={k:int(gdb.parse_and_eval(k)) for k in ('num','type','oldeip')}
  next_active=True;next_done=True;next_trace.enabled=True
  next_record('before-software',args);NextReturned('after-software');return False
class NextFar(gdb.Breakpoint):
 def stop(self):
  if next_active:
   args={k:int(gdb.parse_and_eval(k)) for k in ('use32','bytes','oldeip')}
   next_record('before-far-ret',args);NextReturned('after-far-ret')
  return False
class NextTrace(gdb.Breakpoint):
 def stop(self):
  global next_active,host_armed
  if not next_active:return False
  q=software_fetch_state()
  with (next_root/'fetches.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
  if q['segments'][1]['value']==ext_cs:
   next_record('caller');next_active=False;host_armed=False;self.enabled=False
  return False
class NextDos(gdb.Breakpoint):
 def stop(self):
  if next_active:next_record('before-DOS21');NextReturned('after-DOS21')
  return False
class NextFind(gdb.Breakpoint):
 def stop(self):
  if next_active:
   args=dict(search=gdb.parse_and_eval('search').string(),attr=int(gdb.parse_and_eval('attr')),fcb_findfirst=int(gdb.parse_and_eval('fcb_findfirst')))
   next_record('before-FindFirst',args);NextReturned('after-FindFirst')
  return False
class NextSetup(gdb.Breakpoint):
 def stop(self):
  if next_active:
   args=dict(drive=int(gdb.parse_and_eval('_sdrive')),attr=int(gdb.parse_and_eval('_sattr')),pattern=gdb.parse_and_eval('pattern').string(),pt=int(gdb.parse_and_eval('this->pt')))
   next_record('before-SetupSearch',args);NextReturned('after-SetupSearch')
  return False
class NextResult(gdb.Breakpoint):
 def stop(self):
  if next_active:
   args={k:int(gdb.parse_and_eval('_'+k)) for k in ('size','date','time','attr')}
   args.update(name=gdb.parse_and_eval('_name').string(),name_raw_hex=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('_name')),13)).hex(),pt=int(gdb.parse_and_eval('this->pt')))
   next_record('before-SetResult',args);NextReturned('after-SetResult')
  return False
NextSoftware('CPU_Interrupt');NextFar('CPU_RET');NextDos('DOS_21Handler')
NextFind('DOS_FindFirst');NextSetup('DOS_DTA::SetupSearch');NextResult('DOS_DTA::SetResult')
next_trace=NextTrace('fist_cpu_trace');next_trace.enabled=False

class NextDirectory(gdb.Breakpoint):
 def stop(self):
  if next_active:
   args=dict(path=gdb.parse_and_eval('path').string(),next_free=int(gdb.parse_and_eval('this->nextFreeFindFirst')),occupied=directory_slots('this->dirFindFirst'))
   next_record('before-directory',args);NextReturned('after-directory')
  return False
NextDirectory('DOS_Drive_Cache::FindFirst')


def directory_slots(expression):
 pointer_size=gdb.lookup_type('void').pointer().sizeof
 address=int(gdb.parse_and_eval('&'+expression+'[0]'))
 byte_size=int(gdb.parse_and_eval('sizeof('+expression+')'))
 raw=bytes(gdb.selected_inferior().read_memory(address,byte_size))
 return [bool(int.from_bytes(raw[i:i+pointer_size],'little')) for i in range(0,byte_size,pointer_size)]

host_armed=False;host_serial=0
original_observe=observe
def host_snapshot(kind,arguments=None):
 global host_serial
 drive=int(gdb.parse_and_eval('dos.current_drive'));cache='Drives[%d]->dirCache'%drive
 slots=directory_slots(cache+'.dirFindFirst')
 host_serial+=1
 q=dict(kind=kind,serial=host_serial,current_drive=drive,current_directory=gdb.parse_and_eval('Drives[%d]->curdir'%drive).string(),next_free=int(gdb.parse_and_eval(cache+'.nextFreeFindFirst')),occupied=slots,CPU=state())
 if arguments is not None:q['arguments']=arguments
 with (root/'host-events.jsonl').open('a') as out:out.write(json.dumps(q)+'\n')
 return q
def observe(q):
 global host_armed
 if q['cpu_regs.ip.dword[0]']==0x77e2:
  host_armed=True;host_snapshot('initial77e2')
 original_observe(q)
class HostDirectoryReturned(gdb.FinishBreakpoint):
 def __init__(self,id_address):self.id_address=id_address;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  directory_id=int.from_bytes(bytes(gdb.selected_inferior().read_memory(self.id_address,2)),'little')
  host_snapshot('after-directory',dict(id=directory_id));return False
class HostDirectory(gdb.Breakpoint):
 def stop(self):
  if host_armed:
   drive=int(gdb.parse_and_eval('dos.current_drive'));assert int(gdb.parse_and_eval('this'))==int(gdb.parse_and_eval('&Drives[%d]->dirCache'%drive))
   host_snapshot('before-directory',dict(path=gdb.parse_and_eval('path').string()))
   HostDirectoryReturned(int(gdb.parse_and_eval('&id')))
  return False
class HostResult(gdb.Breakpoint):
 def stop(self):
  if host_armed:
   args={k:int(gdb.parse_and_eval('_'+k)) for k in ('size','date','time','attr')}
   args.update(name=gdb.parse_and_eval('_name').string(),name_raw_hex=bytes(gdb.selected_inferior().read_memory(int(gdb.parse_and_eval('_name')),13)).hex(),pt=int(gdb.parse_and_eval('this->pt')))
   host_snapshot('before-SetResult',args)
  return False
HostDirectory('DOS_Drive_Cache::FindFirst');HostResult('DOS_DTA::SetResult')
