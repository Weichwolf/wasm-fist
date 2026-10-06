#ifndef FIST_SIM_RANDOM_H
#define FIST_SIM_RANDOM_H

#include <stdint.h>

enum { FIST_RANDOM_STREAMS = 4 };

typedef struct {
    /* Caller supplies all four original 16-bit LFSR states. No wall clock or
     * platform random source enters deterministic simulation. */
    uint16_t words[FIST_RANDOM_STREAMS];
    uint8_t next_stream;
} fist_random;

/* Original 0291: advance one of four cyclic streams, apply the right-shift
 * polynomial, return the new word minus one. Zero seeds remain valid.
 * Return 0 on success, -1 on invalid pointers/index without changing outputs. */
int fist_random_next(fist_random *random, uint16_t *out);

#endif
