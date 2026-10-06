#include "assets/model.h"
#include "assets/bytes.h"
#include "assets/palette.h"
#include "assets/source.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    HEADER_SIZE = 16,
    WORD_SIZE = 2,
    FACE_OFFSET = 2,
    PART_TABLE_OFFSET = 6,
    ATLAS_OFFSET = 14,
    SPRITE_DESCRIPTOR_SIZE = 4,
    PIECE_SIZE = 4,
    FACING_BYTES = FIST_MODEL_DIRECTIONS * WORD_SIZE,
    HALF_FACING_BYTES = FACING_BYTES / 2,
    HALF_DIRECTIONS = FIST_MODEL_DIRECTIONS / 2,
    BYTE_SIGNED_MAX = 127,
    EXTENSION_SIZE = 3,
    FILE_NAME_SIZE = FIST_MODEL_NAME_SIZE + 1 + EXTENSION_SIZE + 1
};

typedef struct {
    size_t first;
    size_t end;
} model_list_bounds;

static size_t variant_table_end(fist_asset_view offsets, size_t start,
                                const model_list_bounds *area) {
    size_t end = area->first;
    for (size_t offset = 0; offset < offsets.size; offset += WORD_SIZE) {
        const size_t candidate = fist_read_u16le(offsets.data + offset);
        if (candidate > start && candidate < end) {
            end = candidate;
        }
    }
    return end;
}

static int stream_count(fist_asset_view input, size_t *out) {
    size_t offset = 0;
    size_t count = 0;
    while (input.size - offset >= WORD_SIZE) {
        const size_t length = fist_read_u16le(input.data + offset);
        if (length == 0) {
            if (offset + WORD_SIZE != input.size) {
                return -1;
            }
            *out = count;
            return 0;
        }
        if (length < HEADER_SIZE || length > input.size - offset) {
            return -1;
        }
        offset += length;
        ++count;
    }
    return -1;
}

static int decode_sprites(fist_model_record *record, size_t atlas) {
    const fist_asset_view bytes = record->bytes;
    if (atlas < HEADER_SIZE || atlas > bytes.size - SPRITE_DESCRIPTOR_SIZE) {
        return -1;
    }
    const size_t texels = fist_read_u16le(bytes.data + atlas);
    if (texels < atlas + SPRITE_DESCRIPTOR_SIZE || texels > bytes.size ||
        (texels - atlas) % SPRITE_DESCRIPTOR_SIZE != 0) {
        return -1;
    }
    const size_t count = ((texels - atlas) / SPRITE_DESCRIPTOR_SIZE) - 1;
    const size_t sentinel = atlas + (count * SPRITE_DESCRIPTOR_SIZE);
    if (fist_read_u16le(bytes.data + sentinel) != bytes.size ||
        fist_read_u16le(bytes.data + sentinel + WORD_SIZE) != 0) {
        return -1;
    }
    if (count != 0) {
        record->sprites = calloc(count, sizeof(*record->sprites));
        if (record->sprites == NULL) {
            return -1;
        }
    }
    record->sprite_count = count;
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *descriptor = bytes.data + atlas + (index * SPRITE_DESCRIPTOR_SIZE);
        const size_t start = fist_read_u16le(descriptor);
        const size_t end = fist_read_u16le(descriptor + SPRITE_DESCRIPTOR_SIZE);
        const uint8_t width = descriptor[WORD_SIZE];
        const uint8_t height = descriptor[WORD_SIZE + 1];
        if (start < texels || end < start || end > bytes.size ||
            end - start != (size_t)width * height) {
            return -1;
        }
        record->sprites[index] =
            (fist_model_sprite){width, height, {bytes.data + start, end - start}};
    }
    return 0;
}

static int validate_list(const fist_model_record *record, size_t base_sprite_count,
                         const model_list_bounds *area, size_t offset) {
    if (offset < area->first || offset > record->bytes.size - WORD_SIZE || offset > area->end) {
        return -1;
    }
    const uint8_t *list = record->bytes.data + offset;
    const size_t length = WORD_SIZE + ((size_t)list[0] * PIECE_SIZE);
    if (length > area->end - offset) {
        return -1;
    }
    for (size_t index = 0; index < list[0]; ++index) {
        const uint16_t reference = fist_read_u16le(list + WORD_SIZE + (index * PIECE_SIZE));
        const size_t count =
            (reference & FIST_MODEL_BASE_SPRITE) != 0 ? base_sprite_count : record->sprite_count;
        if ((reference & FIST_MODEL_SPRITE_INDEX_MASK) >= count) {
            return -1;
        }
    }
    return 0;
}

static int decode_parts(fist_model_record *record, size_t base_sprite_count, fist_asset_view region,
                        fist_model_part **out) {
    /* region bounds the part table's start through the start of the atlas. */
    const uint8_t *bytes = record->bytes.data;
    const size_t table = (size_t)(region.data - bytes);
    const size_t atlas = table + region.size;
    if (table < HEADER_SIZE || region.size < WORD_SIZE) {
        return -1;
    }
    const size_t variants_start = fist_read_u16le(region.data);
    if (variants_start <= table || variants_start > atlas ||
        (variants_start - table) % WORD_SIZE != 0) {
        return -1;
    }
    const size_t count = (variants_start - table) / WORD_SIZE;
    if (record->part_count != 0 && record->part_count != count) {
        return -1;
    }
    record->part_count = count;
    fist_model_part *parts = calloc(count, sizeof(*parts));
    if (parts == NULL) {
        return -1;
    }
    *out = parts;
    size_t lists_start = atlas;
    for (size_t index = 0; index < count; ++index) {
        const size_t variant_table = fist_read_u16le(bytes + table + (index * WORD_SIZE));
        if (variant_table < variants_start || variant_table > atlas - WORD_SIZE) {
            return -1;
        }
        const size_t first_list = fist_read_u16le(bytes + variant_table);
        if (first_list < lists_start) {
            lists_start = first_list;
        }
    }
    const model_list_bounds area = {lists_start, atlas};
    for (size_t index = 0; index < count; ++index) {
        const size_t start = fist_read_u16le(bytes + table + (index * WORD_SIZE));
        const size_t end =
            variant_table_end((fist_asset_view){bytes + table, count * WORD_SIZE}, start, &area);
        if (end <= start || end > atlas || (end - start) % WORD_SIZE != 0 ||
            (end - start) / WORD_SIZE > FIST_MODEL_PART_VARIANTS) {
            return -1;
        }
        parts[index].variants = (fist_asset_view){bytes + start, end - start};
        for (size_t offset = start; offset < end; offset += WORD_SIZE) {
            const size_t list = fist_read_u16le(bytes + offset);
            if (validate_list(record, base_sprite_count, &area, list) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int decode_record(fist_model_record *record, size_t base_sprite_count) {
    const uint8_t *bytes = record->bytes.data;
    const size_t atlas = fist_read_u16le(bytes + ATLAS_OFFSET);
    if (decode_sprites(record, atlas) != 0) {
        return -1;
    }
    if (fist_read_u16le(bytes + FACE_OFFSET) == FIST_MODEL_BASE_FACING) {
        for (size_t index = 0; index < FIST_MODEL_ORIENTATIONS; ++index) {
            record->facing[index] = FIST_MODEL_BASE_FACING;
        }
        return 0;
    }
    for (size_t index = 0; index < FIST_MODEL_ORIENTATIONS; ++index) {
        const uint16_t facing = fist_read_u16le(bytes + FACE_OFFSET + (index * WORD_SIZE));
        const size_t table = fist_read_u16le(bytes + PART_TABLE_OFFSET + (index * WORD_SIZE));
        if (facing >= FACING_BYTES || facing % WORD_SIZE != 0 || table > atlas) {
            return -1;
        }
        record->facing[index] =
            (uint16_t)(((facing + HALF_FACING_BYTES) % FACING_BYTES) / WORD_SIZE);
        if (index != 0 && table == fist_read_u16le(bytes + PART_TABLE_OFFSET)) {
            record->parts[index] = record->parts[0];
        } else if (decode_parts(record, base_sprite_count,
                                (fist_asset_view){bytes + table, atlas - table},
                                &record->parts[index]) != 0) {
            return -1;
        }
    }
    return 0;
}

void fist_model_file_destroy(fist_model_file *file) {
    if (file != NULL) {
        for (size_t index = 0; index < file->count; ++index) {
            fist_model_record *record = &file->records[index];
            free(record->sprites);
            if (record->parts[1] != record->parts[0]) {
                free(record->parts[1]);
            }
            free(record->parts[0]);
        }
        free(file->records);
        free(file->storage);
        *file = (fist_model_file){0};
    }
}

int fist_model_file_decode(fist_asset_view input, size_t base_sprite_count, fist_model_file *out) {
    if (input.data == NULL || out == NULL) {
        return -1;
    }
    fist_model_file file = {0};
    if (stream_count(input, &file.count) != 0) {
        return -1;
    }
    file.storage = calloc(input.size, sizeof(*file.storage));
    if (file.count != 0) {
        file.records = calloc(file.count, sizeof(*file.records));
    }
    if (file.storage == NULL || (file.count != 0 && file.records == NULL)) {
        /* No record array exists yet on allocation failure. */
        free(file.storage);
        free(file.records);
        return -1;
    }
    for (size_t index = 0; index < input.size; ++index) {
        file.storage[index] = input.data[index];
    }
    size_t offset = 0;
    for (size_t index = 0; index < file.count; ++index) {
        fist_model_record *record = &file.records[index];
        record->bytes =
            (fist_asset_view){file.storage + offset, fist_read_u16le(file.storage + offset)};
        if (decode_record(record, base_sprite_count) != 0) {
            fist_model_file_destroy(&file);
            return -1;
        }
        offset += record->bytes.size;
    }
    *out = file;
    return 0;
}

int fist_model_variant_get(const fist_model_record *record, const fist_model_pose *pose,
                           fist_model_variant *out) {
    if (record == NULL || pose == NULL || out == NULL || pose->facing >= FIST_MODEL_DIRECTIONS ||
        pose->part >= record->part_count) {
        return -1;
    }
    const size_t orientation = pose->facing < HALF_DIRECTIONS ? 1 : 0;
    const fist_model_part *parts = record->parts[orientation];
    if (parts == NULL || pose->variant >= parts[pose->part].variants.size / WORD_SIZE) {
        return -1;
    }
    const uint8_t *offset = parts[pose->part].variants.data + (pose->variant * WORD_SIZE);
    const uint8_t *list = record->bytes.data + fist_read_u16le(offset);
    *out = (fist_model_variant){list[1], {list + WORD_SIZE, (size_t)list[0] * PIECE_SIZE}};
    return 0;
}

static int8_t signed_byte(uint8_t value) {
    if (value <= BYTE_SIGNED_MAX) {
        return (int8_t)value;
    }
    return (int8_t)(-1 - (int8_t)(UINT8_MAX - value));
}

int fist_model_piece_get(const fist_model_variant *variant, size_t index, fist_model_piece *out) {
    if (variant == NULL || out == NULL || variant->pieces.data == NULL ||
        index >= variant->pieces.size / PIECE_SIZE) {
        return -1;
    }
    const uint8_t *piece = variant->pieces.data + (index * PIECE_SIZE);
    const uint16_t reference = fist_read_u16le(piece);
    *out = (fist_model_piece){(uint16_t)(reference & FIST_MODEL_SPRITE_INDEX_MASK),
                              (uint8_t)((reference & FIST_MODEL_BASE_SPRITE) != 0),
                              (uint8_t)((reference & FIST_MODEL_MIRRORED) != 0),
                              signed_byte(piece[WORD_SIZE]), signed_byte(piece[WORD_SIZE + 1])};
    return 0;
}

static int model_name(const char *name, char out[FILE_NAME_SIZE]) {
    size_t length = 0;
    while (name[length] != '\0' && length < FIST_MODEL_NAME_SIZE) {
        const char value = name[length];
        if (value <= ' ' || value == '.' || value == '/' || value == '\\' || value == ':') {
            return -1;
        }
        out[length] = value;
        ++length;
    }
    if (length == 0 || name[length] != '\0') {
        return -1;
    }
    out[length] = '.';
    return (int)length + 1;
}

void fist_model_destroy(fist_model *model) {
    if (model != NULL) {
        for (size_t index = 0; index < FIST_MODEL_FILE_COUNT; ++index) {
            fist_model_file_destroy(&model->files[index]);
        }
        *model = (fist_model){0};
    }
}

static int install_file(fist_model *model, size_t file_index) {
    const fist_model_file *file = &model->files[file_index];
    if (file->count == 0) {
        return -1;
    }
    if (file_index == 0) {
        return file->count == 1 && file->records[0].facing[0] == FIST_MODEL_BASE_FACING ? 0 : -1;
    }
    for (size_t index = 0; index < file->count; ++index) {
        const fist_model_record *record = &file->records[index];
        if (record->facing[0] == FIST_MODEL_BASE_FACING ||
            (model->part_count != 0 && model->part_count != record->part_count)) {
            return -1;
        }
        model->part_count = record->part_count;
        for (size_t facing = 0; facing < FIST_MODEL_ORIENTATIONS; ++facing) {
            model->faces[record->facing[facing]] = record;
        }
    }
    return 0;
}

int fist_model_load(const char *name, const fist_asset_source *source, fist_model *out) {
    if (name == NULL || source == NULL || source->read == NULL || out == NULL) {
        return -1;
    }
    char filename[FILE_NAME_SIZE] = {0};
    const int extension = model_name(name, filename);
    if (extension < 0) {
        return -1;
    }
    static const char suffixes[FIST_MODEL_FILE_COUNT + 1][EXTENSION_SIZE + 1] = {
        "MAL", "M00", "M08", "M16", "M32"};
    fist_model model = {0};
    for (size_t index = 0; index <= FIST_MODEL_FILE_COUNT; ++index) {
        for (size_t letter = 0; letter <= EXTENSION_SIZE; ++letter) {
            filename[(size_t)extension + letter] = suffixes[index][letter];
        }
        fist_asset_view input = {0};
        if (source->read(source->context, filename, &input) != FIST_ASSET_READ_OK) {
            fist_model_destroy(&model);
            return -1;
        }
        if (index == 0) {
            if (fist_palette_decode(input.data, input.size, &model.palette) != 0) {
                fist_model_destroy(&model);
                return -1;
            }
            continue;
        }
        const size_t base_count = index > 1 ? model.files[0].records[0].sprite_count : 0;
        fist_model_file *file = &model.files[index - 1];
        if (fist_model_file_decode(input, base_count, file) != 0 ||
            install_file(&model, index - 1) != 0) {
            fist_model_destroy(&model);
            return -1;
        }
    }
    for (size_t index = 0; index < FIST_MODEL_DIRECTIONS; ++index) {
        if (model.faces[index] == NULL) {
            fist_model_destroy(&model);
            return -1;
        }
    }
    *out = model;
    return 0;
}
