#ifndef FIST_PROBE_IO_H
#define FIST_PROBE_IO_H

#include "assets/orders.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Caller frees the complete input, including for an empty file. */
uint8_t *fist_probe_read_file(FILE *file, size_t *size);

/* Preserve exact object bytes, including initialized padding, for probe-only
 * transactional failure and unrelated-state mutation checks. */
void fist_probe_capture(const void *object, size_t size, void *out);
int fist_probe_unchanged(const void *object, size_t size, const void *before);

/* Complete owned order observation; NULL explicitly denotes unloaded orders. */
void fist_probe_write_orders(const fist_mission_orders *orders);

void fist_probe_write_route(size_t platoon, const fist_order_route *route);

#endif
