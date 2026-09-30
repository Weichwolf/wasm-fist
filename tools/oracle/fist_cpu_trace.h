#include <stdint.h>

static void fist_cpu_trace(void) {
    static bool initialized = false;
    static FILE *file = NULL;
    static uint32_t first, last;
    static uint64_t count;
    if (!initialized) {
        initialized = true;
        const char *path = getenv("FIST_CPU_TRACE"), *window = getenv("FIST_CPU_TRACE_WINDOW");
        if (!path && !window) return;
        if (!path || !window || *window < '0' || *window > '9') E_Exit("Invalid CPU trace window");
        char *tail;
        uint64_t from = strtoull(window, &tail, 10);
        if (*tail != ':' || tail[1] < '0' || tail[1] > '9') E_Exit("Invalid CPU trace window");
        uint64_t to = strtoull(tail + 1, &tail, 10);
        if (*tail || from > UINT32_MAX || to > UINT32_MAX || from >= to) E_Exit("Invalid CPU trace window");
        first = (uint32_t)from;
        last = (uint32_t)to;
        file = fopen(path, "w");
        if (!file) E_Exit("Cannot open CPU trace");
        fprintf(file, "FISTCPU1 %u %u\n", first, last);
    }
    if (!file || PIC_Ticks < first) return;
    if (PIC_Ticks >= last) {
        fprintf(file, "E %llu %llu\n", (unsigned long long)count, (unsigned long long)PIC_Ticks);
        if (fclose(file)) E_Exit("CPU trace close failed");
        file = NULL;
        return;
    }
    fprintf(file, "I %llu %d %d %d %04x %08x %08x",
        (unsigned long long)PIC_Ticks, CPU_CycleMax, CPU_CycleLeft, CPU_Cycles,
        (unsigned)SegValue(cs), (unsigned)reg_eip, (unsigned)(SegPhys(cs) + reg_eip));
    for (unsigned i = 0; i < 8; ++i) fprintf(file, " %08x", (unsigned)cpu_regs.regs[i].dword[0]);
    for (unsigned i = 0; i < 6; ++i) fprintf(file, " %04x:%08x", (unsigned)Segs.val[i], (unsigned)Segs.phys[i]);
    fputc('\n', file);
    if (ferror(file)) E_Exit("CPU trace write failed");
    ++count;
}
