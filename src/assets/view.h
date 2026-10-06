#ifndef FIST_ASSETS_VIEW_H
#define FIST_ASSETS_VIEW_H

#include <stddef.h>
#include <stdint.h>

/* Borrowed immutable bytes; the input owner controls their lifetime. */
typedef struct {
    const uint8_t *data;
    size_t size;
} fist_asset_view;

#endif
