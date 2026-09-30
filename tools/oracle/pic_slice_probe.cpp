#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/pic.cpp"
#include <assert.h>

Bit32s CPU_Cycles = 0, CPU_CycleLeft = 30000, CPU_CycleMax = 30000;
CPU_Regs cpu_regs;
CPU_Decoder *cpudecoder;
Bits CPU_Core_Normal_Trap_Run(void) { abort(); }
void CPU_Interrupt(Bitu, Bitu, Bitu) { abort(); }
void E_Exit(const char *, ...) { abort(); }

/* PIC and no-op normal-loop retirement only; guest instructions, VGA effects and IRQs are excluded. */
static void latch(Bitu) {}
static unsigned pixel_clock = 25175000u / 8u;
static void part(Bitu count) {
    if (count < 4) PIC_AddEvent(part, (float)(100.0 * 100 * 1000 / pixel_clock), count + 1);
}
static void vertical(Bitu) {
    PIC_AddEvent(vertical, (float)(100.0 * 449 * 1000 / (28322000u / 9u)));
    PIC_AddEvent(latch, (float)(100.0 * 412 * 1000 / pixel_clock));
    PIC_AddEvent(latch, (float)(100.0 * 414 * 1000 / pixel_clock));
    PIC_AddEvent(latch, (float)(100.0 * 400 * 1000 / pixel_clock + 0.005));
    PIC_AddEvent(part, (float)(100.0 * 100 * 1000 / pixel_clock), 1);
}
static void resize(Bitu) { pixel_clock = 25175000u / 8u; PIC_RemoveEvents(part); }

int main(int argc, char **argv) {
    if (argc != 4 && (argc != 5 || strcmp(argv[4], "transition")) &&
        (argc != 6 || strcmp(argv[4], "retire"))) return 2;
    unsigned long long start, scanout, tick;
    float lag;
    FILE *fixture = fopen(argv[1], "rb");
    if (!fixture || fscanf(fixture, "FISTVGA1\n%llu %llu\n%llu %a", &start, &scanout, &tick, &lag) != 4 ||
        fgetc(fixture) != '\n' || fgetc(fixture) != EOF || fclose(fixture) ||
        scanout > start || tick > 10000 || !(lag >= 0 && lag < 1)) return 2;
    char *tail;
    unsigned long target = strtoul(argv[2], &tail, 10);
    if (!*argv[2] || *tail || target > 10000 || target < tick) return 2;
    unsigned long index = strtoul(argv[3], &tail, 10);
    if (!*argv[3] || *tail || index >= 30000) return 2;
    for (unsigned i = 0; i < PIC_QUEUESIZE - 1; ++i) pic_queue.entries[i].next = &pic_queue.entries[i + 1];
    pic_queue.free_entry = pic_queue.entries;
    PIC_Ticks = tick;
    if (argc == 5) pixel_clock = 28322000u / 9u;
    PIC_AddEvent(vertical, lag);
    /* Post-resize queue snapshot; the resize timestamp itself is outside this proof. */
    if (argc == 5) PIC_AddEvent(resize, 51.0f);
    while (PIC_Ticks < target) {
        while (PIC_RunQueue()) CPU_Cycles = -1;
        TIMER_AddTick();
    }
    while (pic_queue.next_entry && pic_queue.next_entry->index * CPU_CycleMax <= index) {
        CPU_Cycles = 0;
        CPU_CycleLeft = 30000 - index;
        assert(PIC_RunQueue());
    }
    CPU_Cycles = 0;
    CPU_CycleLeft = 30000 - index;
    assert(PIC_RunQueue());
    if (argc == 6) {
        const char *input = argv[5];
        do {
            char *end;
            unsigned long count = strtoul(input, &end, 10);
            if (!*input || end == input || !count || count > 0xfffffffful || (*end && *end != ',')) return 2;
            while (count) {
                while (CPU_Cycles-- > 0) if (!--count) break;
                if (!count) break;
                while (!PIC_RunQueue()) TIMER_AddTick();
            }
            if (!*end) break;
            input = end + 1;
        } while (1);
    }
    printf("%llu %d\n", (unsigned long long)(PIC_Ticks * CPU_CycleMax + PIC_TickIndexND()), CPU_Cycles);
}
