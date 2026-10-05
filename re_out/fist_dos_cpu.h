#ifndef FIST_DOS_CPU_H
#define FIST_DOS_CPU_H
#include "fist_ram.h"
/* DOSBox dos_inc.h DOS_SDA_SEG/OFS and packed sSDA/sPSP layouts.
 * These are DOS memory addresses, not application DGROUP offsets.
 * dos.cpp DOS_21Handler saves SS:SP-18 before reached AH=1a and
 * stores RealMakeSeg(ds,reg_dx). Neither operation changes GP or flags. */
enum {FIST_DOS_SDA=0xb2u<<4,FIST_DOS_SDA_DTA=0x0c,
      FIST_DOS_SDA_PSP=0x10,FIST_DOS_PSP_STACK=0x2e};
static inline void fist_dos_cpu_save_stack(FistCpuRam *bus) {
 FistCpuState *cpu=bus->cpu;
 unsigned psp=fist_ram_read(bus,FIST_DOS_SDA+FIST_DOS_SDA_PSP,2);
 fist_ram_write(bus,(psp<<4)+FIST_DOS_PSP_STACK,4,
                (cpu->segments[2].value<<16)|(uint16_t)(cpu->esp-18));
}
static inline void fist_dos_cpu_set_dta(FistCpuRam *bus) {
 FistCpuState *cpu=bus->cpu;
 fist_cpu_require(((cpu->eax>>8)&255)==0x1a);
 fist_dos_cpu_save_stack(bus);
 fist_ram_write(bus,FIST_DOS_SDA+FIST_DOS_SDA_DTA,4,
                (cpu->segments[3].value<<16)|(uint16_t)cpu->edx);
}
/* DOS_DTA::sDTA, dos_classes.cpp SetupSearch/SetResult and reached
 * dos.cpp AH4e/localDrive::FindNext success path. Host inputs belong to
 * the mounted drive/cache, not guest GP registers or application snapshots. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
typedef struct __attribute__((packed)) {
 uint8_t sdrive,sname[8],sext[3],sattr;
 uint16_t dirID,dirCluster;
 uint8_t fill[4],attr;
 uint16_t time,date;
 uint32_t size;
 char name[13];
} FistDosDta;
typedef struct {
 unsigned drive,next_free;
 uint8_t occupied[2048],name_prior[13];
 const char *directory;
 void *opaque;
 void (*observe)(void *,const char *);
} FistDosFindHost;
static inline void fist_dos_find_observe(FistDosFindHost *host,const char *kind) {
 if(host->observe)host->observe(host->opaque,kind);
}
static inline uint32_t fist_dos_cpu_dta(FistCpuRam *bus) {
 uint32_t real=fist_ram_read(bus,FIST_DOS_SDA+FIST_DOS_SDA_DTA,4);
 return (real>>16)*16+(real&0xffff);
}
static inline void fist_dos_dta_setup(FistCpuRam *bus,uint32_t pt,unsigned drive,unsigned attr,const char *pattern) {
 fist_ram_write(bus,pt+offsetof(FistDosDta,sdrive),1,drive);
 fist_ram_write(bus,pt+offsetof(FistDosDta,sattr),1,attr);
 for(unsigned i=0;i<11;i++)fist_ram_write(bus,pt+offsetof(FistDosDta,sname)+i,1,' ');
 const char *dot=strchr(pattern,'.');size_t base=dot?(size_t)(dot-pattern):strlen(pattern);
 if(base>8)base=8;
 for(unsigned i=0;i<base;i++)fist_ram_write(bus,pt+offsetof(FistDosDta,sname)+i,1,pattern[i]);
 if(dot){size_t ext=strlen(dot+1);if(ext>3)ext=3;for(unsigned i=0;i<ext;i++)fist_ram_write(bus,pt+offsetof(FistDosDta,sext)+i,1,dot[1+i]);}
}
static inline unsigned fist_dos_find_allocate(FistDosFindHost *host) {
 unsigned scans=0;
 while(scans<2048 && host->occupied[host->next_free]) {host->next_free=(host->next_free+1)%2048;scans++;}
 unsigned id=host->next_free;host->next_free=(host->next_free+1)%2048;
 if(scans==2048){id=0;host->next_free=1;memset(host->occupied,0,sizeof host->occupied);}
 host->occupied[id]=1;return id;
}
static inline void fist_dos_dta_result(FistCpuRam *bus,uint32_t pt,const char *name,uint32_t size,unsigned date,unsigned time,unsigned attr) {
 for(unsigned i=0;i<13;i++)fist_ram_write(bus,pt+offsetof(FistDosDta,name)+i,1,(uint8_t)name[i]);
 fist_ram_write(bus,pt+offsetof(FistDosDta,size),4,size);
 fist_ram_write(bus,pt+offsetof(FistDosDta,date),2,date);
 fist_ram_write(bus,pt+offsetof(FistDosDta,time),2,time);
 fist_ram_write(bus,pt+offsetof(FistDosDta,attr),1,attr);
}
static inline void fist_dos_cpu_find_first(FistCpuRam *bus,FistDosFindHost *host) {
 FistCpuState *cpu=bus->cpu;
 fist_cpu_require(((cpu->eax>>8)&255)==0x4e);
 fist_dos_cpu_save_stack(bus);
 /* MEM_StrCopy advances linearly from SegPhys(ds)+reg_dx and appends NUL
  * after its256-byte bound. It does not wrap each character at WORD DX. */
 char search[257];unsigned n=0;uint32_t address=cpu->segments[3].base+(uint16_t)cpu->edx;
 while(n<256){unsigned v=fist_ram_read(bus,address+n,1);if(!v)break;search[n++]=v;}search[n]=0;
 fist_dos_find_observe(host,"before-FindFirst");
 uint32_t pt=fist_dos_cpu_dta(bus);
 fist_dos_find_observe(host,"before-SetupSearch");
 fist_dos_dta_setup(bus,pt,host->drive,cpu->ecx,search);
 fist_dos_find_observe(host,"after-SetupSearch");
 /* This owner currently implements the reached regular exact-name search.
  * DOS path/device/wildcard/volume/error resolution is still unbound and
  * fails explicitly; no fabricated success/error or directory ID is returned. */
 fist_cpu_require(!strchr(search,'*') && !strchr(search,'?') && !strchr(search,'\\') && !strchr(search,':'));
 fist_dos_find_observe(host,"before-directory");
 unsigned id=fist_dos_find_allocate(host);
 fist_dos_find_observe(host,"after-directory");
 fist_ram_write(bus,pt+offsetof(FistDosDta,dirID),2,id);
 size_t capacity=strlen(host->directory)+strlen(search)+2;char *path=malloc(capacity);
 fist_cpu_require(path!=NULL);snprintf(path,capacity,"%s/%s",host->directory,search);
 struct stat st;int result=stat(path,&st);free(path);fist_cpu_require(result==0 && S_ISREG(st.st_mode));
 struct tm *t=localtime(&st.st_mtime);unsigned date=4,time=6;
 if(t){date=((t->tm_year+1900-1980)<<9)|((t->tm_mon+1)<<5)|t->tm_mday;time=(t->tm_hour<<11)|(t->tm_min<<5)|(t->tm_sec/2);}
 char name[13];memcpy(name,host->name_prior,sizeof name);
 fist_cpu_require(strlen(search)<sizeof name);strcpy(name,search);
 for(unsigned i=0;name[i];i++)name[i]=toupper((unsigned char)name[i]);
 fist_dos_find_observe(host,"before-SetResult");
 fist_dos_dta_result(bus,pt,name,(uint32_t)st.st_size,date,time,32);
 fist_dos_find_observe(host,"after-SetResult");fist_dos_find_observe(host,"after-FindFirst");
 /* CALLBACK_SCF changes the saved WORD flags, not the live flags. */
 uint32_t at=cpu->segments[2].base+(uint16_t)cpu->esp+4;
 uint32_t flags=fist_ram_read(bus,at,2);fist_ram_write(bus,at,2,flags&~1u);
 cpu->eax=fist_cpu_low(cpu->eax,0,16);
}
#endif
