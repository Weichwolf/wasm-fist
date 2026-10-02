# Record original PCM8 DSP setup, every mixer demand/DMA read and IRQ 7.
# Run via gdb -nx -batch -x this-file --args DOSBOX ... in capture_sequence.sh.
# Set FIST_SB_DEMAND_EVENTS to an isolated output JSONL file.
set pagination off
set confirm off
python
import gdb,json,os
out=open(os.environ['FIST_SB_DEMAND_EVENTS'],'w')
fields=['freq','speaker','mode','type','dma.mode','dma.rate','dma.total','dma.left','dma.min','dma.autoinit','dma.stereo','irq.pending_8bit','irq.pending_16bit']
def emit(kind,extra=None):
 row={'event':kind,'tick':int(gdb.parse_and_eval('PIC_Ticks')),'cycles':int(gdb.parse_and_eval('CPU_Cycles')),'left':int(gdb.parse_and_eval('CPU_CycleLeft'))}
 for key in fields:row[key]=int(gdb.parse_and_eval("'sblaster.cpp'::sb."+key))
 if int(gdb.parse_and_eval("'sblaster.cpp'::sb.dma.chan")):
  row['channel']={key:int(gdb.parse_and_eval("'sblaster.cpp'::sb.dma.chan->"+key)) for key in ['baseaddr','curraddr','basecnt','currcnt','pagebase','autoinit','masked','tcount','request']}
 if int(gdb.parse_and_eval("'sblaster.cpp'::sb.chan")):
  row['mixer_channel']={key:int(gdb.parse_and_eval("'sblaster.cpp'::sb.chan->"+key)) for key in ['enabled','needed','done','freq_add','freq_index']}
 if extra:row.update(extra)
 out.write(json.dumps(row)+'\n');out.flush()
class Transfer(gdb.Breakpoint):
 def stop(self):
  emit('transfer-start', {'requested_'+key:int(gdb.parse_and_eval(key)) for key in ['mode','freq','stereo']})
  return False
class Command(gdb.Breakpoint):
 def stop(self):
  command=int(gdb.parse_and_eval("'sblaster.cpp'::sb.dsp.cmd"))
  length=int(gdb.parse_and_eval("'sblaster.cpp'::sb.dsp.cmd_len"))
  emit('command',{'command':command,'arguments':[int(gdb.parse_and_eval("'sblaster.cpp'::sb.dsp.in.data[%d]"%i)) for i in range(length)]})
  return False
class Irq(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('irq'))==7:emit('irq7')
  return False
class Demand(gdb.Breakpoint):
 def stop(self):
  emit('demand', {'size':int(gdb.parse_and_eval('size'))})
  return False
class Read(gdb.Breakpoint):
 def stop(self):
  if int(gdb.parse_and_eval('this->channum'))==1:
   emit('dma-read', {'want':int(gdb.parse_and_eval('want'))})
  return False
demand=Demand('GenerateDMASound')
read=Read('DmaChannel::Read')
transfer=Transfer('DSP_DoDMATransfer')
command=Command('DSP_DoCommand')
irq=Irq('PIC_ActivateIRQ')
end
run
python
out.close()
end
