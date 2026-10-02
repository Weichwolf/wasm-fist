#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/pic.cpp"
#include <assert.h>

Bit32s CPU_Cycles = 0, CPU_CycleLeft = 30000, CPU_CycleMax = 30000;
Bit64s CPU_IODelayRemoved = 0;
MachineType machine = MCH_VGA;
Segments Segs;
void source_io_read_delay(void);
void source_io_write_delay(void);
void source_dos_transfer(unsigned value);
void source_rep_setup(unsigned,unsigned,int,int);
bool source_rep_step(unsigned);
void source_rep_dump(const char *);
CPU_Regs cpu_regs;
CPU_Decoder *cpudecoder;
Bits CPU_Core_Normal_Trap_Run(void) { abort(); }
void CPU_Interrupt(Bitu, Bitu, Bitu) { abort(); }
void E_Exit(const char *, ...) { abort(); }
void GFX_ShowMsg(const char *, ...) { abort(); }

/* Original PIC queue/masks and I/O delays; retirement is no-op, VGA effects and IRQs are excluded. */
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
        (argc != 6 || (strcmp(argv[4], "retire") && strcmp(argv[4], "file-read") &&
                       strcmp(argv[4], "masked-read"))) &&
        (argc != 7 || (strcmp(argv[4], "dos-cap") && strcmp(argv[4], "dos-read"))) &&
        (argc != 8 || strcmp(argv[4], "dos-cap")) &&
        (argc != 10 || strcmp(argv[4], "rep"))) return 2;
    PIC_8259A controller(NULL);
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
    if (argc == 5 || (argc == 6 && strcmp(argv[4], "retire")) ||
        (argc == 7 && !strcmp(argv[4], "dos-read")))
        pixel_clock = 28322000u / 9u;
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
    if (argc == 10) {
        unsigned count = strtoul(argv[5], NULL, 0), width = strtoul(argv[6], NULL, 0);
        int direction = strtol(argv[7], NULL, 0), displacement = strtol(argv[8], NULL, 0);
        source_rep_setup(count, width, direction, displacement);
        do {
            while (CPU_Cycles-- <= 0) while (!PIC_RunQueue()) TIMER_AddTick();
        } while (source_rep_step(width));
        source_rep_dump(argv[9]);
        printf("%llu %d\n", (unsigned long long)(PIC_Ticks * CPU_CycleMax + PIC_TickIndexND()), CPU_Cycles);
        return 0;
    }
    if (argc == 7 || argc == 8) {
        unsigned value = strtoul(argv[5], &tail, 10);
        if (*tail || value > 65535) return 2;
        unsigned count = 0;
        if (strcmp(argv[6], "invalid")) {
            count = strtoul(argv[6], &tail, 10);
            if (*tail) return 2;
        }
        while (count) {
            while (CPU_Cycles-- > 0) if (!--count) break;
            if (!count) break;
            while (!PIC_RunQueue()) TIMER_AddTick();
        }
        if (!strcmp(argv[4], "dos-read") && count == 0 && strcmp(argv[6], "invalid")) {
            source_io_read_delay();
            unsigned mask = read_data(0x21, 1);
            if (mask & 4) { source_io_write_delay(); write_data(0x21, mask & 0xfb, 1); }
        }
        source_dos_transfer(value);
        if (argc == 8) {
            count = strtoul(argv[7], &tail, 10);
            if (*tail) return 2;
            while (count) {
                while (CPU_Cycles-- > 0) if (!--count) break;
                if (!count) break;
                while (!PIC_RunQueue()) TIMER_AddTick();
            }
        }
        printf("%llu %d\n", (unsigned long long)(PIC_Ticks * CPU_CycleMax + PIC_TickIndexND()), CPU_Cycles);
        return 0;
    }
    if (argc == 6) {
        if (!strcmp(argv[4], "masked-read")) {
            source_io_write_delay();
            write_data(0x21, 0xfc, 1);
        }
        const char *input = argv[5];
        do {
            char *end;
            unsigned long count = strtoul(input, &end, 10);
            if (!*input || end == input || !count || count > 0xfffffffful || (*end && *end != ',')) return 2;
            if (strcmp(argv[4], "retire")) {
                while (count--) {
                    source_io_read_delay();
                    unsigned mask = read_data(0x21, 1);
                    if (mask & 4) {
                        source_io_write_delay();
                        write_data(0x21, mask & 0xfb, 1);
                    }
                }
                printf("%llu %d %u\n", (unsigned long long)(PIC_Ticks * CPU_CycleMax + PIC_TickIndexND()),
                       CPU_Cycles, (unsigned)read_data(0x21, 1));
                return 0;
            }
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
