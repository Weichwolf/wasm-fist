#include "render/model_bitmap.h"

#include "assets/model.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    const fist_model_sprite *sprite;
    int left;
    int lower_y;
    uint8_t mirrored;
} selected_piece;

typedef struct {
    uint8_t priority;
    size_t count;
    selected_piece *pieces;
} selected_part;

typedef struct {
    int left;
    int lower_y;
    int right;
    int upper_y;
    int populated;
} bitmap_bounds;

/* ad47 negates DH as a byte before MOVSX, including -128 wrapping to itself. */
static int piece_bottom(int8_t offset) {
    return offset == INT8_MIN ? INT8_MIN : -(int)offset;
}

static int select_piece(const fist_model *model, const fist_model_record *record,
                        const fist_model_variant *variant, size_t index, selected_piece *out) {
    fist_model_piece piece = {0};
    if (fist_model_piece_get(variant, index, &piece) != 0) {
        return -1;
    }
    if (piece.use_base != 0) {
        record = &model->files[0].records[0];
    }
    if (piece.sprite_index >= record->sprite_count || record->sprites == NULL) {
        return -1;
    }
    const fist_model_sprite *sprite = &record->sprites[piece.sprite_index];
    if (sprite->width == 0 || sprite->height == 0 || sprite->pixels.data == NULL ||
        sprite->pixels.size != (size_t)sprite->width * sprite->height) {
        return -1;
    }
    *out = (selected_piece){sprite, piece.offset_x, piece_bottom(piece.offset_y), piece.mirrored};
    return 0;
}

static void include_piece(bitmap_bounds *bounds, const selected_piece *piece) {
    const int right = piece->left + piece->sprite->width;
    const int upper_y = piece->lower_y + piece->sprite->height;
    if (bounds->populated == 0) {
        *bounds = (bitmap_bounds){piece->left, piece->lower_y, right, upper_y, 1};
        return;
    }
    if (piece->left < bounds->left) {
        bounds->left = piece->left;
    }
    if (piece->lower_y < bounds->lower_y) {
        bounds->lower_y = piece->lower_y;
    }
    if (right > bounds->right) {
        bounds->right = right;
    }
    if (upper_y > bounds->upper_y) {
        bounds->upper_y = upper_y;
    }
}

static int select_parts(const fist_model *model, const fist_model_selection *selection,
                        selected_part *parts, bitmap_bounds *bounds) {
    for (size_t index = 0; index < selection->count; ++index) {
        const fist_model_pose *pose = &selection->poses[index];
        if (pose->part != index || pose->facing >= FIST_MODEL_DIRECTIONS) {
            return -1;
        }
        const fist_model_record *record = model->faces[pose->facing];
        fist_model_variant variant = {0};
        if (record == NULL || fist_model_variant_get(record, pose, &variant) != 0 ||
            variant.pieces.size % FIST_MODEL_PIECE_BYTES != 0) {
            return -1;
        }
        selected_part *part = &parts[index];
        part->priority = variant.priority;
        part->count = variant.pieces.size / FIST_MODEL_PIECE_BYTES;
        part->pieces = calloc(part->count + 1, sizeof(*part->pieces));
        if (part->pieces == NULL) {
            return -1;
        }
        for (size_t piece = 0; piece < part->count; ++piece) {
            if (select_piece(model, record, &variant, piece, &part->pieces[piece]) != 0) {
                return -1;
            }
            include_piece(bounds, &part->pieces[piece]);
        }
    }
    /* Stable insertion: original 2fd4 inserts after every equal priority. */
    for (size_t index = 1; index < selection->count; ++index) {
        const selected_part part = parts[index];
        size_t position = index;
        while (position > 0 && parts[position - 1].priority > part.priority) {
            parts[position] = parts[position - 1];
            --position;
        }
        parts[position] = part;
    }
    return 0;
}

static void draw_piece(const selected_piece *piece, fist_model_bitmap *bitmap) {
    const fist_model_sprite *sprite = piece->sprite;
    const size_t left = (size_t)(piece->left - bitmap->left);
    const size_t lower_y = (size_t)(piece->lower_y - bitmap->bottom);
    for (size_t column = 0; column < sprite->width; ++column) {
        const size_t source = piece->mirrored != 0 ? sprite->width - column - 1 : column;
        for (size_t row = 0; row < sprite->height; ++row) {
            const uint8_t value = sprite->pixels.data[(source * sprite->height) + row];
            if (value != 0) {
                bitmap->indices[((lower_y + row) * bitmap->width) + left + column] = value;
            }
        }
    }
}

static void destroy_parts(selected_part *parts, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        free(parts[index].pieces);
    }
    free(parts);
}

static void draw_parts(const selected_part *parts, size_t count, fist_model_bitmap *bitmap) {
    for (size_t index = 0; index < count; ++index) {
        for (size_t piece = 0; piece < parts[index].count; ++piece) {
            draw_piece(&parts[index].pieces[piece], bitmap);
        }
    }
}

void fist_model_bitmap_destroy(fist_model_bitmap *bitmap) {
    if (bitmap != NULL) {
        free(bitmap->indices);
        *bitmap = (fist_model_bitmap){0};
    }
}

int fist_model_compose(const fist_model *model, const fist_model_selection *selection,
                       fist_model_bitmap *out) {
    if (model == NULL || selection == NULL || out == NULL ||
        selection->count != model->part_count || selection->poses == NULL ||
        model->files[0].count != 1 || model->files[0].records == NULL ||
        model->part_count > SIZE_MAX / sizeof(selected_part)) {
        return -1;
    }
    selected_part *parts = calloc(model->part_count + 1, sizeof(*parts));
    if (parts == NULL) {
        return -1;
    }
    bitmap_bounds bounds = {0};
    if (select_parts(model, selection, parts, &bounds) != 0) {
        destroy_parts(parts, selection->count);
        return -1;
    }
    fist_model_bitmap bitmap = {.palette = model->palette};
    if (bounds.populated != 0) {
        bitmap.left = (int16_t)bounds.left;
        bitmap.bottom = (int16_t)bounds.lower_y;
        bitmap.width = (uint16_t)(bounds.right - bounds.left);
        bitmap.height = (uint16_t)(bounds.upper_y - bounds.lower_y);
        bitmap.indices = calloc((size_t)bitmap.width * bitmap.height, sizeof(*bitmap.indices));
        if (bitmap.indices == NULL) {
            destroy_parts(parts, selection->count);
            return -1;
        }
        draw_parts(parts, selection->count, &bitmap);
    }
    destroy_parts(parts, selection->count);
    *out = bitmap;
    return 0;
}
