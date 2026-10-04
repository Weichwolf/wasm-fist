"""Compile verbatim original normal-core control/flag/PIC branches for controlled cases."""
from pathlib import Path
import re
import subprocess

def prepare(repo,directory):
    tree=repo/'third_party/dosbox-build/dosbox-0.74-3'
    text=(repo/'tools/oracle/pic_slice_probe.cpp').read_text()
    text=text.replace('#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/pic.cpp"','#include "'+str(tree/'src/hardware/pic.cpp')+'"')
    text=text.replace('void CPU_Interrupt(Bitu, Bitu, Bitu) { abort(); }','static void observe_irq(Bitu);\nvoid CPU_Interrupt(Bitu vector,Bitu type,Bitu oldeip){assert(type==0 && oldeip==reg_eip);observe_irq(vector);}')
    text=text.replace('int main(int argc, char **argv)','int unused_slice_main(int argc, char **argv)')
    (directory/'original_pic_context.h').write_text(text)
    source=(tree/'src/cpu/cpu.cpp').read_text()
    helpers=[]
    for name in ('CPU_SetFlags','CPU_PrepareException','CPU_CLI','CPU_STI','CPU_Pop16','CPU_Pop32','CPU_POPF'):
        helpers.append(re.search(r'(?:void|bool|Bitu) '+name+r'\(.*?\n\}',source,re.S)[0])
    body=source.split('void CPU_IRET(bool use32,Bitu oldeip) {',1)[1].split('} else {\t/* Protected mode IRET */',1)[0]
    helpers.append('void CPU_IRET(bool use32,Bitu oldeip){'+body+'}else{abort();}}')
    (directory/'original_cpu_control.h').write_text('\n'.join(helpers)+'\n')
    cases=[]
    none=(tree/'src/cpu/core_normal/prefix_none.h').read_text()
    wide=(tree/'src/cpu/core_normal/prefix_66.h').read_text()
    for op,nextop in (('0xfa','0xfb'),('0xfb','0xfc'),('0x9d','0x9e')):
        text=re.search(r'CASE_[BW]\('+op+r'\).*?(?=\s*CASE_[BW]\('+nextop+r'\))',none,re.S)[0]
        cases.append(text.replace('CASE_B('+op+')','case '+op+':').replace('CASE_W('+op+')','case '+op+':'))
    cases.append(re.search(r'CASE_D\(0x9d\).*?(?=\s*CASE_D\(0xa1\))',wide,re.S)[0].replace('CASE_D(0x9d)','case 0x19d:'))
    for text,old,new in ((none,'CASE_W(0xcf)','case 0xcf:'),(wide,'CASE_D(0xcf)','case 0x1cf:')):
        cases.append(re.search(re.escape(old)+r'.*?(?=\s*CASE_[BW]\(0xd0\)|\s*CASE_D\(0xd1\))',text,re.S)[0].replace(old,new).replace('return CBRET_NONE;','return;'))
    (directory/'original_cases.h').write_text('\n'.join(cases)+'\n')
    core=(tree/'src/cpu/core_normal.cpp').read_text()
    (directory/'original_ip.h').write_text('\n'.join(re.search(r'^#define '+name+r'\s+.*$',core,re.M)[0] for name in ('GETIP','SAVEIP','LOADIP','CPU_PIC_CHECK'))+'\n')
    (directory/'original_flags.cpp').write_bytes((tree/'src/cpu/flags.cpp').read_bytes())
    (directory/'original.cpp').write_bytes((repo/'tools/oracle/core_exit_probe.cpp').read_bytes())
    output=directory/'original'
    command=['g++','-O2','-std=gnu++11',*subprocess.check_output(['sdl-config','--cflags'],text=True).split(),'-I'+str(tree/'include'),'-I'+str(tree),'-I'+str(tree/'src/cpu'),'-I'+str(directory),'-ffunction-sections','-fdata-sections',str(directory/'original.cpp'),str(repo/'tools/oracle/io_delay_probe.cpp'),'-Wl,--gc-sections','-lm','-o',str(output)]
    p=subprocess.run(command,capture_output=True,text=True,timeout=120)
    (directory/'original-build.log').write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
    return [str(output)]
