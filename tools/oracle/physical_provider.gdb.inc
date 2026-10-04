def physical_provider_state():
 count=int(gdb.parse_and_eval("'memory.cpp'::memory.pages"))
 pointers=int(gdb.parse_and_eval("'memory.cpp'::memory.phandlers"))
 width=int(gdb.lookup_type('PageHandler').pointer().sizeof)
 raw=bytes(gdb.selected_inferior().read_memory(pointers,count*width))
 slots=[int.from_bytes(raw[i*width:(i+1)*width],'little') for i in range(count)]
 def handler(pointer):
  if not pointer:return None
  obj=gdb.parse_and_eval('(PageHandler *)%d'%pointer)
  return {'type':str(obj.dynamic_type),'flags':int(obj.dereference()['flags'])}
 firstmb=[int(gdb.parse_and_eval('paging.firstmb[%d]'%i)) for i in range(int(gdb.parse_and_eval('sizeof(paging.firstmb)/sizeof(paging.firstmb[0])')))]
 lfb={key:int(gdb.parse_and_eval("'memory.cpp'::memory.lfb.%s"%key)) for key in ('start_page','end_page','pages','handler','mmiohandler')}
 lfb['handlers']={key:handler(lfb[key]) for key in ('handler','mmiohandler')}
 return {'pages':count,'slots':slots,'handlers':{str(p):handler(p) for p in sorted(set(slots))},
         'firstmb':firstmb,'a20_enabled':bool(gdb.parse_and_eval("'memory.cpp'::memory.a20.enabled")),
         'a20_controlport':int(gdb.parse_and_eval("'memory.cpp'::memory.a20.controlport")),
         'lfb':lfb}
