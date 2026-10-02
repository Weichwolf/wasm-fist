#include "ghidra_compat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
jmp_buf g_fist_exit;
volatile int g_fist_exit_code;
uint32_t fist_ext_base;
/* Bind only the captured original default RET callback, using its actual
 * production body. Every other dispatcher target fails instead of being stubbed. */
extern void m_ext_FUN_0000_2294(void);
static unsigned g_mixer_callbacks;
code *fist_icall(uint32_t address)
{
    assert(address == fist_ext_base + 0x2294);
    ++g_mixer_callbacks;
    return (code *)m_ext_FUN_0000_2294;
}
/* Unrelated interrupt routes must never be reached by the file-read regression. */
void fist_set_int8_handler(uint32_t p) { (void)p; abort(); }
void fist_input_set_mouse_handler(uint32_t p, unsigned mask) { (void)p; (void)mask; abort(); }
void fist_input_mouse_state(unsigned *x, unsigned *y, unsigned *b) { (void)x; (void)y; (void)b; abort(); }
void fist_input_mouse_setpos(unsigned x, unsigned y) { (void)x; (void)y; abort(); }

uint8_t g_mem[FIST_MEM_SIZE];
void fist_timer_pump(void) { extern void fist_clock_advance(unsigned); fist_clock_advance(1); }
static unsigned g_irqs;
static uint32_t g_loaded_size;
static unsigned g_registered;
int fist_ovl_register(const char *name, uint32_t base, uint32_t size)
{
    (void)name; (void)base;
    g_loaded_size = size;
    ++g_registered;
    return 0;
}
void fist_int8_fire(void) { ++g_irqs; }
int fist_opl_owns(int port) { (void)port; return 0; }
int fist_opl_in(int port) { (void)port; return 0; }
void fist_opl_out(int port, int value) { (void)port; (void)value; }
int fist_sb_owns(int port) { (void)port; return 0; }
int fist_sb_in(int port) { (void)port; return 0; }
void fist_sb_out(int port, int value) { (void)port; (void)value; }
extern unsigned long long fist_clock_now(void);
static unsigned long long g_end_clock;
static void check_endpoint(void)
{
    assert(fist_clock_now() == g_end_clock);
    assert(g_irqs == (g_end_clock - 1) / 0x10000);
}

int main(int argc, char **argv)
{
    if (argc == 5 && !strcmp(argv[1], "mixer")) {
        extern void m_ext_FUN_0000_2630(void);
        const unsigned size = 0x100000, dma = 0x2de0;
        const unsigned pointers[] = {0x15d7,0x15db,0x15df,0x15fb,0x15ff,0x1603,0x23dc,0x23e0,0x2716};
        fist_ext_base = 0x100000;
        FILE *input = fopen(argv[2], "rb");
        assert(input && fread(g_mem+fist_ext_base,1,size,input)==size && fread(g_mem+dma,1,0x800,input)==0x800);
        assert(fgetc(input)==EOF && !fclose(input));
        for (unsigned i=0;i<sizeof pointers/sizeof *pointers;++i) {
            uint32_t *field=(uint32_t *)(g_mem+fist_ext_base+pointers[i]), offset=*field;
            uint32_t linear=0x10000000u+offset;
            uint8_t *pointer;
            if (linear>=0x10000000u && linear-0x10000000u<size) pointer=g_mem+fist_ext_base+offset;
            else { assert(linear<FIST_MEM_SIZE); pointer=g_mem+linear; }
            *field=(uint32_t)(uintptr_t)pointer;
        }
        m_ext_FUN_0000_2630();
        assert(g_mixer_callbacks == strtoul(argv[4], NULL, 0));
        /* Report logical guest offsets; verify each actual host pointer before
         * translating it back. Representational rebasing never masks a wrong value. */
        for (unsigned i=0;i<sizeof pointers/sizeof *pointers;++i) {
            uint32_t *field=(uint32_t *)(g_mem+fist_ext_base+pointers[i]);
            uint32_t relative=*field-(uint32_t)(uintptr_t)g_mem;
            assert(relative<FIST_MEM_SIZE);
            *field=relative>=fist_ext_base && relative-fist_ext_base<size ? relative-fist_ext_base : relative-0x10000000u;
        }
        FILE *output=fopen(argv[3], "wb");
        assert(output && fwrite(g_mem+fist_ext_base,1,size,output)==size && fwrite(g_mem+dma,1,0x800,output)==0x800 && !fclose(output));
        return 0;
    }
    if (argc == 5 && !strcmp(argv[1], "blit")) {
        extern void fist_text_init(void), fist_vga_set_mode(int), fist_clock_advance_cpu_cycles(unsigned);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        extern void m_ext_FUN_0000_7120(void);
        unsigned tick = strtoul(argv[2], NULL, 0), index = strtoul(argv[3], NULL, 0);
        fist_text_init(); fist_vga_set_mode(0x13);
        uint64_t current, target = (uint64_t)tick * 30000 + index;
        fist_clock_cpu_slice(&current);
        assert(index < 30000 && target >= current && target-current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target-current));
        uint8_t *memory = g_mem + 0x88000;
        for (unsigned i = 0; i < 0x40000; ++i) memory[i] = (i*37+(i>>8)+11)&255;
        fist_ext_base = 0x100000;
        *(uint32_t *)(g_mem + fist_ext_base + 0x6e88) = (uint32_t)(uintptr_t)(memory + 0x8000);
        *(uint32_t *)(g_mem + fist_ext_base + 0x917) = (uint32_t)(uintptr_t)(memory + 0x18000);
        m_ext_FUN_0000_7120();
        uint64_t cycle; unsigned remaining = fist_clock_cpu_slice(&cycle);
        FILE *output = fopen(argv[4], "wb");
        assert(output && fwrite(memory, 1, 0x40000, output) == 0x40000 && !fclose(output));
        printf("%llu %u\n", (unsigned long long)cycle, remaining);
        return 0;
    }
    if (argc == 9 && !strcmp(argv[1], "rep")) {
        extern void fist_text_init(void), fist_vga_set_mode(int), fist_clock_advance_cpu_cycles(unsigned);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        unsigned tick = strtoul(argv[2], NULL, 0), index = strtoul(argv[3], NULL, 0);
        unsigned count = strtoul(argv[4], NULL, 0), width = strtoul(argv[5], NULL, 0);
        int direction = strtol(argv[6], NULL, 0), displacement = strtol(argv[7], NULL, 0);
        assert((width==1 || width==2 || width==4) && (direction==1 || direction==-1));
        fist_text_init(); fist_vga_set_mode(0x13);
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target = (uint64_t)tick*30000+index;
        assert(target >= current && target-current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target-current));
        uint8_t *memory = g_mem + 0x100000;
        for (unsigned i=0;i<0x40000;++i) memory[i]=(i*37+(i>>8)+11)&255;
        unsigned start = 0x8000 + (direction < 0 && count ? (count-1)*width : 0);
        fist_clock_rep_movs(memory+start+displacement,memory+start,width,count,direction);
        uint64_t cycle; unsigned remaining = fist_clock_cpu_slice(&cycle);
        FILE *output = fopen(argv[8],"wb");
        assert(output && fwrite(memory,1,0x40000,output)==0x40000 && !fclose(output));
        printf("%llu %u\n",(unsigned long long)cycle,remaining);
        return 0;
    }
    if (argc == 7 && !strcmp(argv[1], "ext-read")) {
        extern void fist_text_init(void), fist_vga_set_mode(int), fist_clock_advance_cpu_cycles(unsigned);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        extern int g_fist_ext_int;
        unsigned tick = strtoul(argv[2],NULL,0), index = strtoul(argv[3],NULL,0);
        unsigned requested = strtoul(argv[4],NULL,0);
        assert(tick>=76 && tick<=10000 && index<30000 && requested<=0x20000);
        fist_text_init(); fist_vga_set_mode(0x13);
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target=(uint64_t)tick*30000+index;
        assert(target>=current && target-current<=UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target-current));
        uint16_t *rf=(uint16_t*)(g_mem+0xf0000);
        uint32_t *shadow=(uint32_t*)(g_mem+0xf0020);
        fist_ext_base=0x100000; g_fist_ext_int=1;
        strcpy((char*)g_mem+fist_ext_base+0x2000,"READ.BIN");
        rf[0]=0x3d00; rf[10]=0x21; shadow[3]=0x2000;
        fist_int_dispatch(); assert(!rf[9]);
        unsigned handle=!strcmp(argv[5],"invalid")?99:rf[0];
        uint32_t pointer=(uint32_t)(uintptr_t)(g_mem+fist_ext_base+0x30000);
        rf[0]=0x3f00; rf[1]=handle; rf[2]=(uint16_t)requested; rf[3]=(uint16_t)pointer;
        shadow[0]=0xaa003f00; shadow[1]=0x20000|handle; shadow[2]=requested; shadow[3]=pointer;
        memset(g_mem+fist_ext_base+0x30000,0xa5,requested+16);
        fist_int_dispatch();
        assert(shadow[1]==(0x20000|handle) && rf[1]==handle);
        assert(rf[0]==(uint16_t)shadow[0] && rf[2]==(uint16_t)shadow[2] && rf[3]==(uint16_t)shadow[3]);
        if (rf[9]) assert(shadow[3]==(pointer&0xffff0000));
        uint64_t cycle; unsigned remaining=fist_clock_cpu_slice(&cycle);
        FILE *output=fopen(argv[6],"wb");
        assert(output && fwrite(g_mem+fist_ext_base+0x30000,1,requested+16,output)==requested+16 && !fclose(output));
        printf("%llu %u %u %u %u\n",(unsigned long long)cycle,remaining,shadow[0],shadow[2],rf[9]);
        return 0;
    }
    if (argc == 7 && !strcmp(argv[1], "dos-cap")) {
        extern void fist_text_init(void), fist_vga_set_mode(int), fist_clock_advance_cpu_cycles(unsigned);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        unsigned tick = strtoul(argv[2], NULL, 10), index = strtoul(argv[3], NULL, 10);
        unsigned value = strtoul(argv[4], NULL, 10), retire = strtoul(argv[5], NULL, 10);
        unsigned after = strtoul(argv[6], NULL, 10);
        assert(tick >= 76 && tick <= 10000 && index < 30000 && value <= 65535);
        fist_text_init();
        fist_vga_set_mode(0x13);
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target = (uint64_t)tick * 30000 + index;
        assert(target >= current && target - current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target - current));
        fist_clock_charge_cpu_instructions(retire);
        fist_clock_charge_dos_transfer(value);
        fist_clock_charge_cpu_instructions(after);
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        printf("%llu %u\n", (unsigned long long)cycle, remaining);
        return 0;
    }
    if (argc == 7 && !strcmp(argv[1], "dos-read")) {
        extern void fist_text_init(void), fist_clock_advance_cpu_cycles(unsigned);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        unsigned tick = strtoul(argv[2], NULL, 10), index = strtoul(argv[3], NULL, 10);
        unsigned requested = strtoul(argv[4], NULL, 10);
        assert(tick >= 20 && tick <= 10000 && index < 30000 && requested <= 65535);
        fist_text_init();
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target = (uint64_t)tick * 30000 + index;
        assert(target >= current && target - current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target - current));
        uint16_t *rf = (uint16_t*)(g_mem + 0xf0000);
        strcpy((char*)g_mem + 0x20000, "READ.BIN");
        rf[0] = 0x3d00; rf[7] = 0x2000; rf[3] = 0; rf[10] = 0x21;
        fist_int_dispatch();
        assert(!rf[9]);
        rf[1] = !strcmp(argv[5], "invalid") ? 99 : rf[0];
        rf[0] = 0x3f00; rf[2] = requested; rf[7] = 0x3000;
        memset(g_mem + 0x30000, 0xa5, requested + 16);
        fist_int_dispatch();
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        FILE *output = fopen(argv[6], "wb");
        assert(output && fwrite(g_mem + 0x30000, 1, requested + 16, output) == requested + 16 && !fclose(output));
        printf("%llu %u %u %u\n", (unsigned long long)cycle, remaining, rf[0], rf[9]);
        return 0;
    }
    if (argc == 4 && !strcmp(argv[1], "mz-start")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_clock_init();
        uint32_t loaded;
        assert(!fist_load_mz(argv[2], 0, 0, &loaded));
        for (unsigned stage = 0; stage < 3; ++stage) {
            if (stage == 1) fist_text_init();
            if (stage == 2) fist_clock_charge_cpu_instructions(2);
            uint64_t cycle;
            unsigned remaining = fist_clock_cpu_slice(&cycle);
            printf("%llu %u\n", (unsigned long long)cycle, remaining);
        }
        FILE *output = fopen(argv[3], "wb");
        assert(output && fwrite(g_mem, 1, loaded, output) == loaded && !fclose(output));
        return 0;
    }
    if (argc == 7 && !strcmp(argv[1], "mz-overlay")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_init();
        memset(g_mem, 0xa5, sizeof g_mem);
        unsigned segment = strtoul(argv[3], NULL, 0), relocation = strtoul(argv[4], NULL, 0);
        unsigned length = strtoul(argv[5], NULL, 0);
        assert(length <= FIST_MEM_SIZE);
        int result = fist_load_overlay(argv[2], segment, relocation);
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        FILE *output = fopen(argv[6], "wb");
        assert(output && fwrite(g_mem, 1, length, output) == length && !fclose(output));
        printf("%d %llu %u %u %u\n", result, (unsigned long long)cycle, remaining,
               g_loaded_size, g_registered);
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "start-cpu")) {
        extern void fist_text_init(void);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        fist_text_init();
        uint64_t cycle;
        unsigned remaining = fist_clock_cpu_slice(&cycle);
        printf("%llu %u\n", (unsigned long long)cycle, remaining);
        return 0;
    }
    if ((argc == 4 && !strcmp(argv[1], "pic-slice")) ||
        (argc == 5 && (!strcmp(argv[1], "pic-retire") || !strcmp(argv[1], "pic-file-read") ||
                       !strcmp(argv[1], "pic-masked-read")))) {
        extern void fist_text_init(void), fist_clock_charge_cpu_instructions(unsigned);
        extern void fist_clock_advance_cpu_cycles(unsigned);
        extern void fist_vga_set_mode(int);
        extern unsigned fist_clock_cpu_slice(uint64_t *);
        unsigned tick = strtoul(argv[2], NULL, 10), index = strtoul(argv[3], NULL, 10);
        int file_read = strcmp(argv[1], "pic-slice") && strcmp(argv[1], "pic-retire");
        assert(tick >= (file_read ? 20u : 76u) && tick <= 10000 && index < 30000);
        fist_text_init();
        if (!file_read) fist_vga_set_mode(0x13);
        uint64_t current;
        fist_clock_cpu_slice(&current);
        uint64_t target = (uint64_t)tick * 30000u + index;
        assert(target >= current && target - current <= UINT32_MAX);
        fist_clock_advance_cpu_cycles((unsigned)(target - current));
        if (file_read) {
            if (!strcmp(argv[1], "pic-masked-read")) out(0x21, 0xfc);
            unsigned count = strtoul(argv[4], NULL, 10), mask = 0;
            while (count--) {
                mask = in(0x21);
                if (mask & 4) { mask &= 0xfb; out(0x21, mask); }
            }
            uint64_t cycle;
            unsigned slice = fist_clock_cpu_slice(&cycle);
            printf("%llu %u %u\n", (unsigned long long)cycle, slice, mask);
            return 0;
        }
        if (argc == 5) {
            const char *input = argv[4];
            do {
                char *tail;
                unsigned long count = strtoul(input, &tail, 10);
                assert(*input && tail != input && count && count <= UINT32_MAX && (!*tail || *tail == ','));
                fist_clock_charge_cpu_instructions((unsigned)count);
                if (!*tail) break;
                input = tail + 1;
            } while (1);
        }
        uint64_t cycle;
        unsigned slice = fist_clock_cpu_slice(&cycle);
        printf("%llu %u\n", (unsigned long long)cycle, slice);
        return 0;
    }
    if (argc == 5 && (!strcmp(argv[1], "pit") || !strcmp(argv[1], "pit-cpu") || !strcmp(argv[1], "pit-cpu-base"))) {
        extern void fist_clock_advance(unsigned), fist_clock_charge_cpu_instructions(unsigned);
        unsigned mode = strtoul(argv[2], NULL, 10), period = strtoul(argv[3], NULL, 10);
        unsigned elapsed = strtoul(argv[4], NULL, 10);
        assert((mode == 2 || mode == 3) && period && period <= 65536 && elapsed);
        if (!strcmp(argv[1], "pit-cpu-base")) fist_clock_charge_cpu_instructions(123);
        out(0x43, 0x30 | (mode << 1));
        out(0x40, period & 0xff);
        out(0x40, period >> 8);
        if (!strcmp(argv[1], "pit")) fist_clock_advance(elapsed - 1);
        else fist_clock_charge_cpu_instructions(elapsed);
        out(0x43, 0);
        unsigned low = in(0x40), high = in(0x40);
        printf("%u\n", low | (high << 8));
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "sequence")) {
        extern void fist_text_init(void), fist_clock_advance(unsigned);
        g_end_clock = (strtoull(getenv("FIST_SEQUENCE_END_MS"), NULL, 10) * 1193182u + 999) / 1000;
        atexit(check_endpoint);
        fist_text_init();
        fist_clock_advance(4 * 1193182u);
        return 42;
    }
    const int ports[] = {0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3ce, 0x3cf,
                         0x3d4, 0x3d5, 0x3d8, 0x3d9, 0x20, 0xa0, 0x21, 0xa1, 0x64};
    for (unsigned i = 0; i < sizeof ports / sizeof *ports; ++i) {
        for (int mode = 0; mode < 4; ++mode) {
            out(0x61, mode);
            assert((in(0x61) & 3) == mode);
            out(0x43, 0xb4);   /* channel 2, low/high bytes, mode 2 */
            out(0x42, 50000 & 0xff);
            out(0x42, 50000 >> 8);
            out(0x43, 0x80);
            int before_low = in(0x42), before_high = in(0x42);
            out(ports[i], mode ^ 3);
            out(0x43, 0x80);
            int after_low = in(0x42), after_high = in(0x42);
            if (mode & 1) printf("%u %d %u %u\n", ports[i], mode,
                                before_low | (before_high << 8), after_low | (after_high << 8));
            assert((in(0x61) & 3) == mode);
        }
    }
    return 0;
}
