#ifndef FIST_PROBE_IO_H
#define FIST_PROBE_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Caller frees the complete input, including for an empty file. */
uint8_t *fist_probe_read_file(FILE *file, size_t *size);

#endif
