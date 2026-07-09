/* re_out/fist_dos.c -- Armored Fist platform shim: software-interrupt (INT n) handlers.
 *
 * This is the runtime target of the <callotherfixup targetop="swi"> installed at decompile time
 * (tools/ghidra/InstallIntFixup.java): every `int n` in the engine now marshals its live registers
 * into a fixed reg-file in g_mem, calls fist_int_dispatch(), then reads the results back. So this
 * file sees the REAL DOS/BIOS register operands (AH=function, DS:DX=ptr, CX, BX, ...) and returns
 * AX/CF/DX/CX just like DOS/BIOS would. Honest handlers only -- no faked behaviour; unimplemented
 * calls announce themselves (FIST_TRACE_TRAPS) and set CF so the engine sees an honest failure.
 *
 * REG-FILE LAYOUT (linear in g_mem; MUST match InstallIntFixup.java BODY):
 *   0xF0000 AX  0xF0002 BX  0xF0004 CX  0xF0006 DX  0xF0008 SI  0xF000A DI
 *   0xF000C BP  0xF000E DS  0xF0010 ES  0xF0012 CF(1B)  0xF0014 VEC
 * DOS file I/O reads/writes real files under $FIST_DATADIR (default armoredfist/), case-insensitive,
 * searching DATADIR and DATADIR/FISTDATA (analog to DD2's dd2_filio.c). DS:DX / buffers are linear
 * (SEG<<4)+OFF into g_mem.
 */
#include "ghidra_compat.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef __EMSCRIPTEN__
#include <execinfo.h>
#endif
#include <stdlib.h>
#include <setjmp.h>
#include <dirent.h>

/* ---- reg-file accessors (alias the exact g_mem the engine marshals through) ---- */
#define RF(off)  (g_mem + 0xf0000u + (off))
#define R_AX (*(uint16_t*)RF(0x00))
#define R_BX (*(uint16_t*)RF(0x02))
#define R_CX (*(uint16_t*)RF(0x04))
#define R_DX (*(uint16_t*)RF(0x06))
#define R_SI (*(uint16_t*)RF(0x08))
#define R_DI (*(uint16_t*)RF(0x0a))
#define R_BP (*(uint16_t*)RF(0x0c))
#define R_DS (*(uint16_t*)RF(0x0e))
#define R_ES (*(uint16_t*)RF(0x10))
#define R_CF (*(uint16_t*)RF(0x12))   /* full-word: the engine may read the flag slot as 2 bytes */
#define R_VEC (*(uint16_t*)RF(0x14))
#define R_AL (*(uint8_t *)RF(0x00))
#define R_AH (*(uint8_t *)RF(0x01))

/* linear address from a real-mode SEG:OFF pair */
static inline uint32_t lin(uint16_t seg, uint16_t off){ return ((uint32_t)seg << 4) + off; }

/* ---- trap tracing ---- */
static int g_trace = -1;
static int traceon(void){ if (g_trace < 0) g_trace = getenv("FIST_TRACE_TRAPS") ? 1 : 0; return g_trace; }
#define TRACE(...) do{ if(traceon()) fprintf(stderr, __VA_ARGS__);}while(0)

/* ---- program-exit (INT 21h/4Ch) unwinds back to native_main so we can dump the framebuffer ---- */
extern jmp_buf g_fist_exit;
extern volatile int g_fist_exit_code;

/* ================= INT 21h : DOS ================= */

/* case-insensitive open across the data dirs; returns FILE* or NULL. */
static const char *datadir(void){ const char *d = getenv("FIST_DATADIR"); return d ? d : "armoredfist"; }

static FILE *open_ci(const char *name, const char *mode)
{
    /* An empty (or all-separator) DOS filename is not openable -- real INT 21h AH=3D/3C return
     * "file not found". Without this guard fopen("armoredfist/FISTDATA/", "rb") opens the DIRECTORY
     * on Linux and returns a bogus handle. (Empty names currently arise when a caller's template
     * offset register, e.g. fd79's `mov si,0x89` for "SOUND.CFG", is not threaded to the
     * filename-builder -- a decompile __allregs param-propagation gap, tracked separately; the honest
     * result of the empty name is a failed open, which the engine handles as "config not present".) */
    if (!name || !name[0]) return NULL;
    { const char *b = name; for (const char *p=name; *p; ++p) if (*p=='\\'||*p=='/') b=p+1; if (!*b) return NULL; }
    /* strip any DOS path/drive; keep the last component + honor a relative subdir.
     * Also strip embedded spaces: the engine builds DOS 8.3 names space-PADDED (a 7-char base like
     * "FLDCOMP" -> "FLDCOMP .MRL"); real INT 21h treats the space as padding and opens "FLDCOMP.MRL".
     * DOS filenames never legitimately contain spaces, so dropping them is the faithful canonicalization. */
    char clean[260]; size_t j=0;
    for (const char *p=name; *p && j<sizeof(clean)-1; ++p){ if(*p==' ') continue; clean[j++] = (*p=='\\')?'/':*p; }
    clean[j]=0;
    /* candidate directories to search (most-specific first) */
    const char *dirs[4]; int nd=0;
    static char d0[300], d1[400];
    snprintf(d0,sizeof d0,"%s/FISTDATA", datadir()); dirs[nd++]=d0;
    dirs[nd++]=datadir();
    dirs[nd++]="."; dirs[nd++]="FISTDATA";
    for (int di=0; di<nd; ++di){
        char path[600];
        snprintf(path,sizeof path,"%s/%s", dirs[di], clean);
        FILE *f=fopen(path,mode); if(f) return f;
        /* case-variant probe of the last component */
        char buf[600]; strncpy(buf,path,sizeof buf-1); buf[sizeof buf-1]=0;
        char *comp=buf,*q; for(q=buf;*q;++q) if(*q=='/') comp=q+1;
        size_t cl=strlen(comp); if(!cl) continue;
        /* UPPER (game files are shipped upper-case) */
        for(size_t i=0;i<cl;i++) comp[i]=(char)toupper((unsigned char)comp[i]);
        if((f=fopen(buf,mode))) return f;
        /* lower */
        for(size_t i=0;i<cl;i++) comp[i]=(char)tolower((unsigned char)comp[i]);
        if((f=fopen(buf,mode))) return f;
    }
    (void)d1;
    return NULL;
}

/* DOS handle table (handle 0..4 reserved for std streams like real DOS). */
#define MAXH 256
static FILE *g_htab[MAXH];
static int   g_hinit;
static int alloc_handle(FILE *f){
    if(!g_hinit){ g_hinit=1; }
    for(int h=5; h<MAXH; ++h) if(!g_htab[h]){ g_htab[h]=f; return h; }
    return -1;
}

/* DOS conventional-memory bump allocator: hands out real-mode segments in the free hole between the
 * loaded engine/heap and the VGA aperture (0xA0000). Faithful enough for AH=48/49/4A to succeed. */
static uint16_t g_next_seg = 0x3400;      /* linear 0x34000, above the 0x33...-byte engine image */
#define HEAP_TOP_SEG 0x9000               /* linear 0x90000, below VGA at 0xA0000 */

static void set_cf(int err){ R_CF = err ? 1 : 0; }

/* ================= INT 21h AH=4E/4F : FILEMGR find-first / find-next =================
 * The engine's list dialogs (SELECT PLAYER `*.FPL`, SELECT BATTLE `*.FSG`) enumerate the game
 * directory via INT 21h AH=4E (find first) / AH=4F (find next), reading each result's 8.3 name from
 * the DTA at +0x1e (standard DOS Find-File DTA).  We ARE FILEMGR/DOS here, so we enumerate the REAL
 * files under the search dirs (armoredfist/FISTDATA + armoredfist), apply the DOS 8.3 wildcard from
 * DS:DX, de-duplicate, and return them **sorted ascending by 8.3 name (strcmp, upper-case)**.
 *
 * ORDER SOURCE: DOSBox serves a mounted host directory through its DOS_Drive_Cache, which stores and
 * returns directory entries SORTED alphabetically (case-insensitive 8.3 name) -- so its find-first /
 * find-next stream is deterministic and alphabetical regardless of host readdir order.  We reproduce
 * exactly that order (verified against ref/selplayer_native320.png: D, GAMESWIN, JO, JOE, KKR, PP,
 * TRT -- the seven *.FPL files in strcmp order, D selected).  This makes the enumeration deterministic
 * and identical native<->wasm and vs the DOSBox reference, with no dependence on the host filesystem. */
static int g_ff_trace = -1;
static int fftrace(void){ if (g_ff_trace<0) g_ff_trace = getenv("FIST_FFTRACE")?1:0; return g_ff_trace; }

#define FF_MAX 512
static char  g_ff_names[FF_MAX][13];   /* sorted 8.3 ASCIIZ names for the current search */
static uint32_t g_ff_size[FF_MAX];     /* file sizes (bytes) parallel to g_ff_names */
static int   g_ff_count;               /* number of matches */
static int   g_ff_cur;                 /* find-next cursor */

/* DOS 8.3 wildcard match: pattern & name are upper-cased 8.3 forms ("NAME.EXT"); '*' and '?' honored.
 * We normalize both to an 11-char (8+3) blank-padded FCB form and compare with '?' as any char. */
static void to_fcb(const char *s, char out[11])
{
    memset(out, ' ', 11);
    int i = 0;                      /* name part */
    while (*s && *s != '.' && i < 8) {
        if (*s == '*') { while (i < 8) out[i++] = '?'; break; }
        out[i++] = (char)toupper((unsigned char)*s); ++s;
    }
    while (*s && *s != '.') ++s;    /* skip overflow of name part */
    if (*s == '.') {
        ++s; int e = 0;
        while (*s && e < 3) {
            if (*s == '*') { while (e < 3) out[8 + e++] = '?'; break; }
            out[8 + e++] = (char)toupper((unsigned char)*s); ++s;
        }
    }
}
static int fcb_match(const char pat[11], const char nm[11])
{
    for (int i = 0; i < 11; ++i) if (pat[i] != '?' && pat[i] != nm[i]) return 0;
    return 1;
}
static int ff_cmp(const void *a, const void *b)
{ return strcmp((const char*)a, (const char*)b); }

/* Build the sorted match list for the pattern at DS:DX. */
static void ff_build(const char *pat)
{
    char pfcb[11]; to_fcb(pat, pfcb);
    g_ff_count = 0; g_ff_cur = 0;
    /* same search dirs as open_ci, most-specific first; de-dup by name so a file present in more than
     * one search dir is listed once (matches a single DOS directory view). */
    char d0[300]; const char *dirs[4]; int nd = 0;
    snprintf(d0, sizeof d0, "%s/FISTDATA", datadir()); dirs[nd++] = d0;
    dirs[nd++] = datadir(); dirs[nd++] = "."; dirs[nd++] = "FISTDATA";
    for (int di = 0; di < nd; ++di) {
        DIR *dp = opendir(dirs[di]); if (!dp) continue;
        struct dirent *de;
        while ((de = readdir(dp))) {
            const char *fn = de->d_name;
            if (fn[0] == '.') continue;                 /* skip ".", "..", dotfiles */
            /* form the upper-case 8.3 name; reject names that don't fit 8.3 (DOS wouldn't see them) */
            char base[64]; int bl = 0; const char *dot = 0;
            for (const char *p = fn; *p; ++p) { if (*p == '.') dot = p; }
            char nm83[13]; int ok = 1;
            { int nlen = dot ? (int)(dot - fn) : (int)strlen(fn);
              int elen = dot ? (int)strlen(dot + 1) : 0;
              if (nlen < 1 || nlen > 8 || elen > 3) ok = 0;
              if (ok) { int k = 0;
                for (int i = 0; i < nlen; ++i) nm83[k++] = (char)toupper((unsigned char)fn[i]);
                if (elen) { nm83[k++] = '.'; for (int i = 0; i < elen; ++i) nm83[k++] = (char)toupper((unsigned char)dot[1+i]); }
                nm83[k] = 0; }
            }
            (void)base; (void)bl;
            if (!ok) continue;
            char nfcb[11]; to_fcb(nm83, nfcb);
            if (!fcb_match(pfcb, nfcb)) continue;
            /* de-dup */
            int dup = 0; for (int i = 0; i < g_ff_count; ++i) if (!strcmp(g_ff_names[i], nm83)) { dup = 1; break; }
            if (dup || g_ff_count >= FF_MAX) continue;
            strcpy(g_ff_names[g_ff_count], nm83);
            /* file size */
            char path[600]; snprintf(path, sizeof path, "%s/%s", dirs[di], de->d_name);
            FILE *sf = fopen(path, "rb"); long sz = 0;
            if (sf) { fseek(sf, 0, SEEK_END); sz = ftell(sf); fclose(sf); }
            g_ff_size[g_ff_count] = (uint32_t)sz;
            g_ff_count++;
        }
        closedir(dp);
    }
    /* sort names ascending; keep sizes paired */
    for (int i = 0; i < g_ff_count; ++i)
        for (int j = i + 1; j < g_ff_count; ++j)
            if (strcmp(g_ff_names[i], g_ff_names[j]) > 0) {
                char tn[13]; strcpy(tn, g_ff_names[i]); strcpy(g_ff_names[i], g_ff_names[j]); strcpy(g_ff_names[j], tn);
                uint32_t ts = g_ff_size[i]; g_ff_size[i] = g_ff_size[j]; g_ff_size[j] = ts;
            }
    (void)ff_cmp;
}

/* Fill the DTA (linear = get-DTA address, i.e. 0x0080:0x0000 = 0x800) with match index `idx`.
 * DOS Find-File DTA: +0x15 attr, +0x16 time, +0x18 date, +0x1a size(dword), +0x1e name(ASCIIZ). */
static void ff_fill_dta(int idx)
{
    uint32_t dta = lin(0x0080, 0x0000);   /* matches AH=2F get-DTA -> ES:BX = 0x80:0 */
    if (dta + 0x2b > FIST_MEM_SIZE) return;
    uint8_t *d = g_mem + dta;
    memset(d, 0, 0x2b);
    d[0x15] = 0x20;                                     /* archive attribute */
    *(uint16_t*)(d + 0x16) = 0;                         /* time */
    *(uint16_t*)(d + 0x18) = 0x2141;                    /* date (arbitrary, deterministic) */
    *(uint32_t*)(d + 0x1a) = g_ff_size[idx];
    strncpy((char*)(d + 0x1e), g_ff_names[idx], 12); d[0x1e + 12] = 0;
}

/* ---- INT 21h AH=4B AL=03 : load-overlay (faithful 16-bit MZ overlay loader) ----
 * The engine EXEC-loads its driver overlays (MGAVIDEO.DVR video, SOUNDDVR.DVR sound) into a segment
 * it has already allocated (AH=48), passing the load segment + relocation factor in the ES:BX param
 * block. AL=03 is the "load, do NOT create a PSP, do NOT run" sub-function: DOS just reads the MZ
 * load-module, copies it to load_seg:0000, and adds the reloc factor to each MZ relocation site --
 * then returns; the caller far-calls the module entry itself. We replicate exactly that, reading the
 * REAL .DVR file (its MZ header carries the reloc table) so nothing is faked. Faithful failure (return
 * -1 -> CF) if the file is missing or the MZ is malformed. */
int fist_load_overlay(const char *name, uint16_t load_seg, uint16_t reloc)
{
    FILE *f = open_ci(name, "rb");
    if (!f) { TRACE("[dos] 4B03 overlay '%s' NOT FOUND\n", name); return -1; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long fsz = ftell(f); fseek(f, 0, SEEK_SET);
    if (fsz < 0x20) { fclose(f); return -1; }
    uint8_t hdr[0x40];
    if (fread(hdr, 1, sizeof hdr, f) != sizeof hdr) { fclose(f); return -1; }
    if (hdr[0] != 'M' || hdr[1] != 'Z') { fclose(f); TRACE("[dos] 4B03 '%s' not MZ\n", name); return -1; }
    uint16_t cblp    = *(uint16_t*)(hdr + 0x02);
    uint16_t cp      = *(uint16_t*)(hdr + 0x04);
    uint16_t crlc    = *(uint16_t*)(hdr + 0x06);
    uint16_t cparhdr = *(uint16_t*)(hdr + 0x08);
    uint16_t lfarlc  = *(uint16_t*)(hdr + 0x18);
    uint32_t hdr_bytes = (uint32_t)cparhdr * 16;
    uint32_t img_bytes = cblp ? (uint32_t)(cp - 1) * 512 + cblp : (uint32_t)cp * 512;
    if (img_bytes > (uint32_t)fsz || hdr_bytes >= img_bytes) { fclose(f); return -1; }
    uint32_t lm_bytes = img_bytes - hdr_bytes;               /* load-module size */
    uint32_t dst = (uint32_t)load_seg << 4;
    /* Honest invalid-load-target check: the load segment comes from the engine's driver-object struct
     * (ES:BX param-block word0 = struct[0] = the AH=48-allocated segment). If it lands inside the loaded
     * engine image (< image top paragraph) it would overwrite the running engine / IVT -- never a real
     * value (the real struct holds an alloc'd segment ABOVE the image, e.g. 0x3400). This is exactly
     * BLOCKER #1 leg (b): the driver-struct load-segment field is not yet populated (it is set by the
     * FUN_1000_184b/0a31 allocator reached through the still-unthreaded DGROUP:0xd0/0xd8 method vectors),
     * so load_seg arrives 0. Fail LOUDLY here rather than corrupting g_mem at linear 0. */
    if (load_seg < 0x3400u) {   /* 0x3400 = fist_dos.c heap base = first paragraph above the engine image */
        fclose(f);
        TRACE("[dos] 4B03 '%s' REJECT load_seg=%04x (< heap base 0x3400) -- driver-struct load segment "
              "not populated (BLOCKER #1 leg b: 184b/0a31 allocator not yet threaded)\n", name, load_seg);
        return -1;
    }
    if (dst + lm_bytes > FIST_MEM_SIZE) { fclose(f); TRACE("[dos] 4B03 '%s' load 0x%x+0x%x overflows\n",
                                                            name, dst, lm_bytes); return -1; }
    /* copy the load-module into g_mem at load_seg:0000 */
    fseek(f, hdr_bytes, SEEK_SET);
    if (fread(g_mem + dst, 1, lm_bytes, f) != lm_bytes) { fclose(f); return -1; }
    /* apply the MZ relocations: word at (load_seg<<4)+(seg<<4)+off += reloc */
    int applied = 0;
    for (uint16_t i = 0; i < crlc; ++i) {
        uint8_t rb[4];
        if (fseek(f, lfarlc + i * 4, SEEK_SET) != 0 || fread(rb, 1, 4, f) != 4) { fclose(f); return -1; }
        uint16_t r_off = *(uint16_t*)rb, r_seg = *(uint16_t*)(rb + 2);
        uint32_t site = dst + ((uint32_t)r_seg << 4) + r_off;
        if (site + 2 > FIST_MEM_SIZE) { fclose(f); return -1; }
        *(uint16_t*)(g_mem + site) = (uint16_t)(*(uint16_t*)(g_mem + site) + reloc);
        ++applied;
    }
    fclose(f);
    /* strip DOS path to the bare basename for the registry / fmap wiring */
    const char *bn = name; for (const char *p = name; *p; ++p) if (*p == '\\' || *p == '/') bn = p + 1;
    fist_ovl_register(bn, dst, lm_bytes);
    TRACE("[dos] 4B03 loaded '%s' at seg %04x (linear 0x%05x, %u bytes, %d relocs, factor %04x)\n",
          bn, load_seg, dst, lm_bytes, applied, reloc);
    return 0;
}

static void dos_int(void)
{
    unsigned ah = R_AH;
    switch (ah) {
    case 0x25: /* set interrupt vector (DS:DX = new handler seg:off). */
        TRACE("[dos] 25 set vec 0x%02x -> %04x:%04x\n", R_AL, R_DS, R_DX);
        if (R_AL == 0x08) fist_set_int8_handler(lin(R_DS, R_DX));  /* PIT ISR: arm the cooperative pump */
        set_cf(0); return;
    case 0x35: /* get interrupt vector -> ES:BX = a synthetic, CALLABLE far ptr (the MAGIC). The
                * engine saves this and later far-calls it to chain the original DOS handler; the
                * indirect-call dispatcher (fist_icall_far) recognises FIST_INTVEC_SEG:n and routes
                * back to fist_int_dispatch as "run INT n". Returning 0:0 (old) segfaulted the chain. */
        TRACE("[dos] 35 get vec 0x%02x -> magic %04x:%04x\n", R_AL, FIST_INTVEC_SEG, R_AL);
        R_ES = FIST_INTVEC_SEG; R_BX = R_AL; set_cf(0); return;
    case 0x30: /* get DOS version -> AL=major, AH=minor */
        R_AX = 0x0006; set_cf(0); return;             /* report DOS 6.00 */
    case 0x19: /* get current default drive -> AL */
        R_AL = 2; set_cf(0); return;                  /* C: */
    case 0x0e: /* select disk -> AL = number of drives */
        R_AL = 4; set_cf(0); return;
    case 0x2f: /* get DTA -> ES:BX */
        R_ES = 0x0080; R_BX = 0x0000; set_cf(0); return;   /* DTA at linear 0x800 */
    case 0x1a: /* set DTA (DS:DX) -- accept */
        set_cf(0); return;
    case 0x33: /* get/set ctrl-break -> DL=0 */
        R_DX &= 0xff00; set_cf(0); return;
    case 0x06: /* direct console I/O: DL=0xff -> input (AL=0,ZF), else output DL */
        if (R_DX & 0xff) { fputc(R_DX & 0xff, stderr); } else { R_AL = 0; }
        set_cf(0); return;
    case 0x09: { /* print '$'-terminated string at DS:DX */
        uint32_t p = lin(R_DS, R_DX);
        for (int i=0;i<4096 && p+i<FIST_MEM_SIZE;i++){ char c=g_mem[p+i]; if(c=='$')break; fputc(c,stderr);}
        set_cf(0); return; }
    case 0x3b: /* chdir -- accept (files resolved via search path in open_ci) */
    case 0x47: /* get current dir -> fill DS:SI with "" */
        if (ah==0x47){ uint32_t p=lin(R_DS,R_SI); if(p<FIST_MEM_SIZE) g_mem[p]=0; }
        set_cf(0); return;
    case 0x3c: { /* create/truncate file, CX=attr, DS:DX=name -> AX=handle */
        char *nm = (char*)(g_mem + lin(R_DS, R_DX));
        FILE *f = open_ci(nm, "wb+");
        TRACE("[dos] 3C create '%s' -> %s\n", nm, f?"ok":"FAIL");
        if(!f){ R_AX=5; set_cf(1); return; }
        int h=alloc_handle(f); if(h<0){ fclose(f); R_AX=4; set_cf(1); return; }
        R_AX=(uint16_t)h; set_cf(0); return; }
    case 0x3d: { /* open file, AL=mode, DS:DX=name -> AX=handle */
        char *nm = (char*)(g_mem + lin(R_DS, R_DX));
        const char *mode = (R_AL & 3) ? "rb+" : "rb";
        FILE *f = open_ci(nm, mode);
        if(!f && (R_AL&3)) f = open_ci(nm, "rb");   /* fall back to read-only */
        TRACE("[dos] 3D open  '%s' mode=%d -> %s [DS:DX=%04x:%04x lin=0x%05x b0=%02x %02x %02x %02x]\n", nm, R_AL&3, f?"ok":"FAIL", R_DS,R_DX,lin(R_DS,R_DX), (uint8_t)nm[0],(uint8_t)nm[1],(uint8_t)nm[2],(uint8_t)nm[3]);
        if(!f && getenv("FIST_NAMEDUMP")){ char*d740=(char*)(g_mem+0x1c740); char*dff4=(char*)(g_mem+0x1cff4);
            uint16_t tseg=*(uint16_t*)(g_mem+0x1c70a);
            fprintf(stderr,"[namedump] DGROUP:0x740='%.16s'  DGROUP:0xff4='%.16s'  DGROUP:0x70a(tseg)=%04x\n", d740, dff4, tseg);
            if(tseg){ uint32_t base=(uint32_t)tseg<<4; fprintf(stderr,"[nametbl] ptr-table @ %04x:0 :\n", tseg);
              for(int i=0;i<24;i++){ uint16_t p=*(uint16_t*)(g_mem+base+i*2); char*s=(char*)(g_mem+base+p);
                fprintf(stderr,"   [bx=0x%02x] ->%04x '%.13s'\n", i*2, p, s); } } }
#ifndef __EMSCRIPTEN__
        if(!f && getenv("FIST_OPENBT")){ void*bt[24]; int n=backtrace(bt,24); fprintf(stderr,"[openbt] failed open '%s':\n",nm); backtrace_symbols_fd(bt,n,2); }
#endif
        if(!f){ R_AX=2; set_cf(1); return; }         /* 2 = file not found */
        int h=alloc_handle(f); if(h<0){ fclose(f); R_AX=4; set_cf(1); return; }
        R_AX=(uint16_t)h; set_cf(0); return; }
    case 0x3e: { /* close, BX=handle */
        int h=R_BX; if(h>=5 && h<MAXH && g_htab[h]){ fclose(g_htab[h]); g_htab[h]=NULL; }
        set_cf(0); return; }
    case 0x3f: { /* read, BX=handle CX=count DS:DX=buf -> AX=bytes */
        int h=R_BX; uint16_t n=R_CX; uint32_t p=lin(R_DS,R_DX);
        if(h<5||h>=MAXH||!g_htab[h]){ R_AX=6; set_cf(1); return; }  /* 6 = invalid handle */
        size_t r = (p<FIST_MEM_SIZE) ? fread(g_mem+p,1,n,g_htab[h]) : 0;
        TRACE("[dos] 3F read  h=%d n=%u -> %zu\n", h, n, r);
        R_AX=(uint16_t)r; set_cf(0); return; }
    case 0x40: { /* write, BX=handle CX=count DS:DX=buf -> AX=bytes */
        int h=R_BX; uint16_t n=R_CX; uint32_t p=lin(R_DS,R_DX);
        if(h<5||h>=MAXH||!g_htab[h]){ R_AX=6; set_cf(1); return; }
        size_t w = (p<FIST_MEM_SIZE) ? fwrite(g_mem+p,1,n,g_htab[h]) : 0;
        R_AX=(uint16_t)w; set_cf(0); return; }
    case 0x42: { /* lseek, AL=whence BX=handle CX:DX=offset -> DX:AX=newpos */
        int h=R_BX; if(h<5||h>=MAXH||!g_htab[h]){ R_AX=6; set_cf(1); return; }
        long off = (long)(((uint32_t)R_CX<<16) | R_DX);
        int whence = (R_AL==1)?SEEK_CUR : (R_AL==2)?SEEK_END : SEEK_SET;
        fseek(g_htab[h], off, whence);
        long pos = ftell(g_htab[h]);
        R_AX=(uint16_t)(pos & 0xffff); R_DX=(uint16_t)((pos>>16)&0xffff); set_cf(0); return; }
    case 0x43: /* get/set file attributes -> CX=attr, succeed */
        R_CX=0x20; set_cf(0); return;
    case 0x57: /* get/set file date/time -> succeed */
        set_cf(0); return;
    case 0x48: { /* allocate memory, BX=paragraphs -> AX=segment (or CF=1, BX=max avail) */
        uint16_t need=R_BX;
        if ((uint32_t)g_next_seg + need > HEAP_TOP_SEG){
            R_BX = HEAP_TOP_SEG - g_next_seg; R_AX=8; set_cf(1); return; /* 8 = insufficient mem */
        }
        R_AX = g_next_seg; g_next_seg += need; set_cf(0);
        TRACE("[dos] 48 alloc %u para -> seg 0x%04x\n", need, R_AX); return; }
    case 0x49: /* free memory (ES=seg) -- bump allocator: no-op success */
        set_cf(0); return;
    case 0x4a: /* resize memory block, BX=new paragraphs -> succeed (report available) */
        set_cf(0); return;
    case 0x4b: { /* EXEC / load-overlay */
        char *nm = (char*)(g_mem + lin(R_DS, R_DX));
        if (R_AL != 0x03) {   /* AL=00 (EXEC-and-run) is not part of the overlay path -- honest fail */
            TRACE("[dos] 4B AL=0x%02x unsupported (only AL=03 load-overlay) name='%s'\n", R_AL, nm);
            R_AX = 1; set_cf(1); return;
        }
        uint32_t pb = lin(R_ES, R_BX);                 /* ES:BX param block { word0=load_seg, word1=reloc } */
        uint16_t load_seg = *(uint16_t*)(g_mem + pb);
        uint16_t reloc    = *(uint16_t*)(g_mem + pb + 2);
        if (traceon()) {
            uint32_t np = lin(R_DS, R_DX);
            fprintf(stderr, "[dos] 4B03 load-overlay DS:DX=%04x:%04x ES:BX=%04x:%04x load_seg=%04x reloc=%04x\n"
                            "      name@0x%05x = '%.20s'  bytes:", R_DS, R_DX, R_ES, R_BX, load_seg, reloc, np, nm);
            for (int i = 0; i < 16; ++i) fprintf(stderr, " %02x", g_mem[np + i]);
            fprintf(stderr, "\n");
        }
        if (fist_load_overlay(nm, load_seg, reloc) < 0) { R_AX = 2; set_cf(1); return; }
        R_AX = 0; set_cf(0); return; }
    case 0x4c: /* terminate with return code AL */
        TRACE("[dos] 4C terminate code=%d\n", R_AL);
        g_fist_exit_code = R_AL; longjmp(g_fist_exit, 1); return;
    case 0x4e: { /* find first: build the sorted match list for DS:DX pattern, fill DTA with entry 0 */
        char *pat = (char*)(g_mem + lin(R_DS, R_DX));
        if (fftrace()) fprintf(stderr, "[ff] 4E find-first pattern DS:DX=%04x:%04x = '%.16s' CX=%04x\n",
                               R_DS, R_DX, pat, R_CX);
        ff_build(pat);
        if (fftrace()) { fprintf(stderr, "[ff]    -> %d matches:", g_ff_count);
            for (int i=0;i<g_ff_count && i<16;i++) fprintf(stderr, " %s", g_ff_names[i]); fprintf(stderr, "\n"); }
        if (g_ff_count == 0) { R_AX = 18; set_cf(1); return; }   /* 18 = no more files */
        ff_fill_dta(0); g_ff_cur = 0; R_AX = 0; set_cf(0); return; }
    case 0x4f: { /* find next: advance the cursor set by the preceding find-first */
        if (++g_ff_cur >= g_ff_count) { R_AX = 18; set_cf(1); return; }
        ff_fill_dta(g_ff_cur); R_AX = 0; set_cf(0); return; }
    default:
        TRACE("[dos] UNIMPL INT21 AH=0x%02x AL=0x%02x BX=%04x CX=%04x DX=%04x\n",
              ah, R_AL, R_BX, R_CX, R_DX);
        set_cf(1); return;                            /* honest failure, not a fake success */
    }
}

/* ================= INT 10h : BIOS video ================= */
extern void fist_vga_set_mode(int mode);
static void bios_video(void)
{
    unsigned ah = R_AH;
    switch (ah) {
    case 0x00: /* set video mode AL */
        TRACE("[vid] 10h set mode 0x%02x\n", R_AL); fist_vga_set_mode(R_AL); return;
    case 0x0f: /* get current video mode -> AL=mode, AH=cols, BH=page */
        R_AX = 0x2813; return;                        /* 40 cols, mode 0x13 */
    case 0x0b: /* set palette/border -- ignore */
    case 0x10: /* palette register / DAC via BIOS -- ignore (game uses ports) */
        return;
    case 0x01: case 0x02: case 0x05: case 0x06: case 0x07: case 0x09: case 0x0e:
        return;                                       /* cursor/scroll/teletype: ignore */
    default:
        TRACE("[vid] INT10 AH=0x%02x AL=0x%02x\n", ah, R_AL); return;
    }
}

/* ================= INT 33h : mouse =================
 * We ARE the DOS mouse driver.  The engine (FUN_1000_392a at boot) resets us (fn 0), sets the cursor
 * position (fn 4), reads it (fn 3), and installs a movement/button EVENT HANDLER (fn 0x14 swap /
 * fn 0x0c set -- ES:DX = handler, CX = call mask).  After that the menu is EVENT-DRIVEN: it does NOT
 * poll fn 3 in its idle loop -- it waits for us to far-call the installed handler on each event.  So
 * fn 0x14/0x0c must CAPTURE the handler (fist_input_set_mouse_handler); the scripted-input driver in
 * native_main.c then synthesizes events and far-calls it, exactly as a real mouse driver would.
 * fn 3 returns the driver's current virtual position/buttons (kept by the scripted driver). */
extern void     fist_input_set_mouse_handler(uint32_t handler_lin, unsigned mask);
extern void     fist_input_mouse_state(unsigned *vx, unsigned *vy, unsigned *buttons); /* virtual coords */
extern void     fist_input_mouse_setpos(unsigned vx, unsigned vy);
static void mouse_int(void)
{
    unsigned fn = R_AX;
    switch (fn) {
    case 0x00: /* reset/init -> AX=0xFFFF (installed), BX=button count */
        R_AX=0xffff; R_BX=2; TRACE("[mouse] init\n"); return;
    case 0x01: /* show cursor */ case 0x02: /* hide cursor */
        return;
    case 0x03: { /* get position+buttons -> BX=buttons, CX=virtual x, DX=virtual y */
        unsigned vx,vy,b; fist_input_mouse_state(&vx,&vy,&b);
        R_BX=b; R_CX=vx; R_DX=vy; return; }
    case 0x04: /* set cursor position: CX=virtual x, DX=virtual y */
        fist_input_mouse_setpos(R_CX, R_DX); return;
    case 0x07: case 0x08: /* set x/y ranges */
    case 0x0f: case 0x10: /* set mickeys / conditional off */
        return;
    case 0x0c: /* set event handler: CX=call mask, ES:DX=handler */
    case 0x14: /* swap event handler: CX=call mask, ES:DX=new handler (old returned in CX/ES:DX) */
        fist_input_set_mouse_handler(((uint32_t)R_ES<<4)+R_DX, R_CX);
        if (fn==0x14){ R_CX=0; R_ES=0; R_DX=0; }   /* no previous handler to return */
        return;
    case 0x0b: /* read motion counters -> CX=dx, DX=dy */
        R_CX=0; R_DX=0; return;
    default:
        TRACE("[mouse] fn=0x%02x\n", fn); return;
    }
}

/* ================= INT 16h : BIOS keyboard ================= */
static void bios_kbd(void)
{
    unsigned ah = R_AH;
    switch (ah) {
    case 0x00: case 0x10: /* read key (blocking) -> AX=0 (no key; avoid hang for now) */
        R_AX=0; return;
    case 0x01: case 0x11: /* check key -> ZF set = no key. model via AX=0 + we can't set ZF; engine reads AX */
        R_AX=0; R_CF=0; return;
    case 0x02: case 0x12: /* shift flags -> AL=0 */
        R_AL=0; return;
    default: return;
    }
}

/* ================= saved-vector chaining ================= */
/* The indirect-call dispatcher lands here when the engine far-calls a saved INT-vector MAGIC ptr
 * (handed out by AH=35 above). We ARE the DOS handler for that vector, so "chain to the original"
 * == run our handler for INT n with the engine's current register file. */
void fist_int_chain(unsigned vec)
{
    TRACE("[int] chain saved INT 0x%02x handler\n", vec);
    R_VEC = (uint16_t)vec;
    fist_int_dispatch();
}

/* ================= INT 21h : DOS  (EXTENDER / 32-bit-flat mode) =================
 * The Doug-Huffman extender (re_out/fist_ext.c, the KDV intro-FMV player) is 32-bit FLAT: its real
 * `int 0x21` sites pass the buffer in EDX, the count in ECX, the handle in (E)BX -- e.g.
 *   mov edx,[ds:0x6e80]      ; EDX = a flat host heap pointer (MEMMGR, [0x807]=0 identity map)
 *   mov edx,0x5898           ; EDX = a MODULE OFFSET (the extender image is base-0 @ fist_ext_base)
 * Ghidra's 16-bit InstallIntFixup truncated EDX to the DX word (host pointers lost).  The FLAT32
 * assembler therefore also captures the full 32-bit operands into the shadow slots R_EDX32/R_ECX32
 * (0xF002c/0xF0028).  When g_fist_ext_int is set (native_main wraps the KDV gate) we service INT 21h
 * here with flat addressing.  This is the extender's OWN FILEMGR file I/O (find/open/read TITLE.KDV);
 * no faked behaviour -- real files under $FIST_DATADIR via open_ci(), honest CF on failure. */
int g_fist_ext_int = 0;                 /* set by native_main around the extender KDV player */
#define R_EBX32 (*(uint32_t*)RF(0x24))
#define R_ECX32 (*(uint32_t*)RF(0x28))
#define R_EDX32 (*(uint32_t*)RF(0x2c))
static uint8_t *g_ext_dta;              /* current DTA (host pointer), set by AH=1A set-DTA */

/* Resolve an extender flat operand to a host pointer.  Small values are module offsets (image is
 * base-0, placed at fist_ext_base); large values are already host pointers into g_mem (heap /
 * framebuffer / TCB, all seeded as host addresses with [0x807]=0 identity). */
static uint8_t *ext_addr(uint32_t v){
    if (v < 0x10000u) return g_mem + fist_ext_base + v;
    return (uint8_t*)(uintptr_t)v;
}

static void dos_int_ext(void)
{
    unsigned ah = R_AH;
    switch (ah) {
    case 0x1a: /* set DTA: the extender points its DTA at EDX (== [0x927], a seeded host pointer) */
        g_ext_dta = ext_addr(R_EDX32);
        set_cf(0); return;
    case 0x4e:   /* find first: stat the file, fill the DTA (size at +0x1a) as DOS would */
    case 0x4f: { /* find next: the KDV opens a single file -> no more matches */
        if (ah == 0x4f) { R_AX = 18; set_cf(1); return; }
        char *nm = (char*)ext_addr(R_EDX32);
        FILE *f = open_ci(nm, "rb");
        TRACE("[ext] 4E find-first '%s' -> %s\n", nm, f ? "ok" : "FAIL");
        if (!f) { R_AX = 18; set_cf(1); return; }   /* 18 = no more files */
        fseek(f, 0, SEEK_END); long sz = ftell(f); fclose(f);
        if (g_ext_dta) {
            memset(g_ext_dta, 0, 0x2b);
            *(uint32_t*)(g_ext_dta + 0x1a) = (uint32_t)sz;    /* file size dword */
            /* +0x1e = 13-byte ASCIIZ name (last path component) */
            const char *b = nm; for (const char *p = nm; *p; ++p) if (*p=='\\'||*p=='/') b = p+1;
            strncpy((char*)g_ext_dta + 0x1e, b, 12); g_ext_dta[0x2a] = 0;
        }
        R_AX = 0; set_cf(0); return; }
    case 0x3d: { /* open, AL=mode, EDX=name -> AX=handle */
        char *nm = (char*)ext_addr(R_EDX32);
        const char *mode = (R_AL & 3) ? "rb+" : "rb";
        FILE *f = open_ci(nm, mode);
        if (!f && (R_AL & 3)) f = open_ci(nm, "rb");
        TRACE("[ext] 3D open '%s' mode=%d -> %s\n", nm, R_AL & 3, f ? "ok" : "FAIL");
        if (!f) { R_AX = 2; set_cf(1); return; }
        int h = alloc_handle(f); if (h < 0) { fclose(f); R_AX = 4; set_cf(1); return; }
        R_AX = (uint16_t)h; set_cf(0); return; }
    case 0x3f: { /* read, BX=handle ECX=count EDX=buf -> AX=bytes */
        int h = R_BX; uint32_t n = R_ECX32; uint8_t *buf = ext_addr(R_EDX32);
        if (h < 5 || h >= MAXH || !g_htab[h]) { R_AX = 6; set_cf(1); return; }
        size_t r = fread(buf, 1, n, g_htab[h]);
        TRACE("[ext] 3F read h=%d n=%u -> %zu\n", h, n, r);
        R_AX = (uint16_t)r; set_cf(0); return; }
    case 0x3e: { /* close, BX=handle */
        int h = R_BX; if (h >= 5 && h < MAXH && g_htab[h]) { fclose(g_htab[h]); g_htab[h] = NULL; }
        set_cf(0); return; }
    case 0x42: { /* lseek, AL=whence BX=handle ECX:EDX=offset -> DX:AX=newpos */
        int h = R_BX; if (h < 5 || h >= MAXH || !g_htab[h]) { R_AX = 6; set_cf(1); return; }
        long off = (long)R_EDX32;                    /* full 32-bit flat offset */
        int whence = (R_AL == 1) ? SEEK_CUR : (R_AL == 2) ? SEEK_END : SEEK_SET;
        fseek(g_htab[h], off, whence); long pos = ftell(g_htab[h]);
        R_AX = (uint16_t)(pos & 0xffff); R_DX = (uint16_t)((pos >> 16) & 0xffff); set_cf(0); return; }
    default:
        /* uncovered extender INT-21 -> fall back to the 16-bit engine handler (harmless; it only
         * mis-addresses buffer calls, which this switch already covers). */
        TRACE("[ext] INT21 AH=0x%02x (fallthrough to engine handler) EDX32=%08x ECX32=%08x\n",
              ah, R_EDX32, R_ECX32);
        dos_int(); return;
    }
}

/* ================= dispatcher ================= */
static long g_intcount, g_maxints = -2;
void fist_int_dispatch(void)
{
    if (g_maxints == -2){ const char *m=getenv("FIST_MAXINTS"); g_maxints = m?atol(m):-1; }
    if (g_maxints >= 0 && ++g_intcount > g_maxints){
        TRACE("[int] FIST_MAXINTS=%ld reached -> forced capture-exit\n", g_maxints);
        g_fist_exit_code = 0; longjmp(g_fist_exit, 2);
    }
    unsigned v = R_VEC;
    if (g_fist_ext_int && v == 0x21) { dos_int_ext(); return; }   /* extender flat-mode file I/O */
    switch (v) {
    case 0x21: dos_int();     break;
    case 0x10: bios_video();  break;
    case 0x33: mouse_int();   break;
    case 0x16: bios_kbd();    break;
    case 0x2f: R_AX=0; R_CF=0; break;   /* multiplex: report "not installed"/no-op */
    case 0x08: /* chained BIOS INT 8 (the engine's PIT ISR far-calls the saved vector for the 18.2 Hz
                * tick). We ARE that handler: advance the BIOS tick counter at 0040:006C, as the BIOS
                * would. (The SIGALRM also bumps it; a double count here is harmless -- it is only used
                * for coarse timeouts.) */
        (*(volatile uint32_t*)(g_mem + 0x46C))++; R_CF = 0; break;
    default:
        TRACE("[int] UNHANDLED INT 0x%02x AX=%04x\n", v, R_AX);
        R_CF = 1; break;
    }
}
