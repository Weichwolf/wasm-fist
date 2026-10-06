#ifndef FIST_ASSETS_SOURCE_H
#define FIST_ASSETS_SOURCE_H

#include "assets/view.h"

typedef enum {
    FIST_ASSET_READ_ERROR = -1,
    FIST_ASSET_READ_OK = 0,
    FIST_ASSET_READ_NOT_FOUND = 1
} fist_asset_read_result;

/* Storage providers resolve DOS basenames case-insensitively. An OK view must
 * remain immutable and valid until the next read call or source destruction.
 * A missing file is distinct from a read error; only missing palettes may fall
 * back to PAL.RES. The shared asset layer owns archive/format interpretation. */
typedef fist_asset_read_result (*fist_asset_read)(void *context, const char *name,
                                                  fist_asset_view *out);

typedef struct {
    fist_asset_read read;
    void *context;
} fist_asset_source;

#endif
