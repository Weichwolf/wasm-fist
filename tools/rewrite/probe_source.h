#ifndef FIST_PROBE_SOURCE_H
#define FIST_PROBE_SOURCE_H

#include "assets/source.h"
#include "assets/view.h"

#include <stdint.h>

/* Probe storage only. Runtime platform providers implement the same shared
 * source contract; archive lookup and decoding remain in the asset library. */
typedef struct {
    const char *directory;
    uint8_t *buffer;
} fist_probe_source;

fist_asset_read_result fist_probe_source_read(void *context, const char *name,
                                              fist_asset_view *out);
void fist_probe_source_close(fist_probe_source *source);

#endif
