def memory_context(prefix):
 q={'provider':physical_provider_state(),'cache':cache_state(),
    'architecture':int(gdb.parse_and_eval('CPU_ArchitectureType')),
    'normal_core':bool(gdb.parse_and_eval('cpudecoder == &CPU_Core_Normal_Run')),
    'auto_determine':int(gdb.parse_and_eval('CPU_AutoDetermineMode'))}
 v={key:int(gdb.parse_and_eval('vga.'+key)) for key in ('vmemsize','vmemwrap','svga.bank_read_full','svga.bank_write_full')}
 v['pages']={key:int(gdb.parse_and_eval("'vga_memory.cpp'::vgapages."+key)) for key in ('base','mask')}
 for key,size in (('linear',v['vmemsize']),('fastmem',2*v['vmemsize'])):
  pointer=int(gdb.parse_and_eval('vga.mem.linear' if key=='linear' else 'vga.fastmem'))
  name=prefix+'.'+key
  (root/name).write_bytes(bytes(gdb.selected_inferior().read_memory(pointer,size)))
  v[key]={'pointer':pointer,'file':name,'bytes':size}
 q['vga']=v
 return q
