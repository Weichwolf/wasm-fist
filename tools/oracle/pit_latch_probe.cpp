#include "../../third_party/dosbox-build/dosbox-0.74-3/src/hardware/timer.cpp"

Bit32s CPU_Cycles = 0, CPU_CycleLeft = 30000, CPU_CycleMax = 30000;
Bitu PIC_Ticks = 0;

int main(int argc, char **argv) {
    if (argc != 4) return 2;
    unsigned mode = strtoul(argv[1], NULL, 10);
    unsigned period = strtoul(argv[2], NULL, 10);
    char *tail;
    double elapsed = strtod(argv[3], &tail);
    if ((mode != 2 && mode != 3) || !period || period > 65536 || !*argv[3] || *tail ||
        !isfinite(elapsed) || elapsed < 0) return 2;
    pit[0].mode = mode;
    pit[0].cntr = period;
    pit[0].delay = 1000.0f / ((float)PIT_TICK_RATE / (float)period);
    pit[0].start = -(double)elapsed * 1000.0 / PIT_TICK_RATE;
    counter_latch(0);
    printf("%u\n", pit[0].read_latch);
}
