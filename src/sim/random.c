#include "sim/random.h"

#include <stddef.h>
#include <stdint.h>

int fist_random_next(fist_random *random, uint16_t *out) {
    enum { POLYNOMIAL = 0xb400 };
    if (random == NULL || out == NULL || random->next_stream >= FIST_RANDOM_STREAMS) {
        return -1;
    }
    const size_t index = random->next_stream;
    const uint16_t previous = random->words[index];
    uint16_t next = (uint16_t)(previous >> 1);
    if (previous % 2 != 0) {
        next ^= POLYNOMIAL;
    }
    random->words[index] = next;
    random->next_stream = (uint8_t)((index + 1) % FIST_RANDOM_STREAMS);
    *out = (uint16_t)(next - 1);
    return 0;
}
