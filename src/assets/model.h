#ifndef FIST_ASSETS_MODEL_H
#define FIST_ASSETS_MODEL_H

#include "assets/palette.h"
#include "assets/source.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_MODEL_DIRECTIONS = 32,
    FIST_MODEL_ORIENTATIONS = 2,
    FIST_MODEL_PART_VARIANTS = 128,
    FIST_MODEL_FILE_COUNT = 4,
    FIST_MODEL_PIECE_BYTES = 4,
    FIST_MODEL_BASE_FACING = 65534,
    FIST_MODEL_NAME_SIZE = 8,
    FIST_MODEL_BASE_SPRITE = 16384,
    FIST_MODEL_MIRRORED = 32768,
    FIST_MODEL_SPRITE_INDEX_MASK = 16383
};

typedef struct {
    uint8_t width;
    uint8_t height;
    fist_asset_view pixels;
} fist_model_sprite;

typedef struct {
    /* Validated immutable little-endian offsets into the owning record. */
    fist_asset_view variants;
} fist_model_part;

typedef struct {
    fist_asset_view bytes;
    uint16_t facing[FIST_MODEL_ORIENTATIONS];
    size_t sprite_count;
    fist_model_sprite *sprites;
    size_t part_count;
    /* Original header +6 and +8 tables. Facing 0..15 uses +8, 16..31 uses +6. */
    fist_model_part *parts[FIST_MODEL_ORIENTATIONS];
} fist_model_record;

typedef struct {
    uint8_t priority;
    fist_asset_view pieces;
} fist_model_variant;

typedef struct {
    uint16_t sprite_index;
    uint8_t use_base;
    uint8_t mirrored;
    int8_t offset_x;
    int8_t offset_y;
} fist_model_piece;

typedef struct {
    uint8_t facing;
    size_t part;
    size_t variant;
} fist_model_pose;

typedef struct {
    uint8_t *storage;
    size_t count;
    fist_model_record *records;
} fist_model_file;

typedef struct {
    fist_palette palette;
    /* M00, M08, M16, M32, all owned. */
    fist_model_file files[FIST_MODEL_FILE_COUNT];
    size_t part_count;
    const fist_model_record *faces[FIST_MODEL_DIRECTIONS];
} fist_model;

/* Decode one complete model stream, including its final zero word. Copies all
 * input bytes. Base references are validated against the supplied M00 sprite
 * count. Return 0 on success, -1 on malformed input/allocation; failure leaves
 * out unchanged. On success out must not already own a file. */
int fist_model_file_decode(fist_asset_view input, size_t base_sprite_count, fist_model_file *out);
void fist_model_file_destroy(fist_model_file *file);
/* Get validated part variants and pieces without mutation/allocation. Return
 * 0 on success, -1 on absent/out-of-range arguments, leaving out unchanged.
 * Views borrow the owning model file, not source storage. */
int fist_model_variant_get(const fist_model_record *record, const fist_model_pose *pose,
                           fist_model_variant *out);
int fist_model_piece_get(const fist_model_variant *variant, size_t index, fist_model_piece *out);
/* Load all five files by DOS basename through an ephemeral source. No source
 * views survive. All 32 faces must exist and part counts must agree. Return 0
 * on success, -1 on missing/invalid input/I/O/allocation, preserving out. */
int fist_model_load(const char *name, const fist_asset_source *source, fist_model *out);
void fist_model_destroy(fist_model *model);

#endif
