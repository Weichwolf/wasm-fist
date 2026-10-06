#ifndef FIST_ASSETS_RESOURCE_H
#define FIST_ASSETS_RESOURCE_H

#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

/* Validate a complete RESOURCE1 archive and find the first matching member.
 * name is a NUL-terminated ASCII basename of at most 12 bytes, without spaces,
 * separators or drive prefixes. Names use the original DOS case folding.
 * Return 0 on success, -1 for invalid input or a missing name. Failure leaves
 * out unchanged. The returned view borrows the immutable archive buffer.
 * The first offset must follow the directory and the sentinel must end the file. */
int fist_resource_find(const uint8_t *data, size_t size, const char *name, fist_asset_view *out);

#endif
