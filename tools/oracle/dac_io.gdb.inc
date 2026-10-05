# Full observations around actual original IO_WriteB calls.
original_dac_save=save
def save(kind,extra=None):
 return original_dac_save(kind,dict(extra or {},dac=dac_state()))
io_serial=0
class IoReturned(gdb.FinishBreakpoint):
 def __init__(self,q):self.q=q;super().__init__(gdb.newest_frame(),internal=True)
 def stop(self):
  q=dict(self.q,phase='after',cpu=state(),dac=dac_state())
  with (root/'io-writes.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  return False
class IoWritten(gdb.Breakpoint):
 def stop(self):
  global io_serial
  if not started or not fetch.enabled:return False
  assert int(gdb.parse_and_eval('$pc'))==int(gdb.parse_and_eval('(void *)IO_WriteB'))
  assert gdb.lookup_type('Bitu').sizeof==8
  q=dict(serial=io_serial,phase='before',port=int(gdb.parse_and_eval('$rdi')),value=int(gdb.parse_and_eval('$rsi')),cpu=state(),dac=dac_state())
  io_serial+=1
  with (root/'io-writes.jsonl').open('a') as f:f.write(json.dumps(q)+'\n')
  IoReturned(q);return False
IoWritten('*IO_WriteB')
