#ifndef FIST_RENDER_MODEL_BITMAP_H
#define FIST_RENDER_MODEL_BITMAP_H

#include "assets/model.h"
#include "assets/palette.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    /* Owned bottom-row-first indices; index zero is transparent. */
    uint8_t *indices;
    uint16_t width;
    uint16_t height;
    /* Authored left/minimum Y coordinates relative to the model origin; Y is up. */
    int16_t left;
    int16_t bottom;
    fist_palette palette;
} fist_model_bitmap;

typedef struct {
    /* Exactly one pose per model part, in part order. Borrowed during the call. */
    const fist_model_pose *poses;
    size_t count;
} fist_model_selection;

/* Compose all parts without perspective scaling or palette interpolation.
 * Source texels are column-major, mirrored horizontally when selected; zero
 * leaves earlier pieces visible. Parts draw by ascending priority, stable for
 * ties. Coordinates retain the original signed X and byte-negated Y anchor.
 * Return 0 on success, -1 on invalid/missing selections/allocation, preserving
 * out. Success owns all indices/palette independently of the model. A valid
 * selection with no pieces yields width/height zero and NULL indices. On
 * success out must not already own a bitmap. */
int fist_model_compose(const fist_model *model, const fist_model_selection *selection,
                       fist_model_bitmap *out);
void fist_model_bitmap_destroy(fist_model_bitmap *bitmap);

#endif
