#include "assets/model.h"
#include "assets/palette.h"
#include "assets/source.h"
#include "assets/view.h"
#include "probe_io.h"
#include "probe_source.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CONTRACT_FAILURE = 2,
    MARKER_COUNT = 137,
    MARKER_COMPONENT = 165,
    PIECE_SIZE = FIST_MODEL_PIECE_BYTES
};

static void write_bytes(fist_asset_view bytes) {
    for (size_t index = 0; index < bytes.size; ++index) {
        printf(" %02x", (unsigned)bytes.data[index]);
    }
    printf("\n");
}

static int variant_marker_intact(const fist_model_variant *variant, fist_asset_view bytes) {
    return variant->priority == MARKER_COUNT && variant->pieces.data == bytes.data &&
           variant->pieces.size == bytes.size;
}

static int check_variant_errors(const fist_model_record *record) {
    fist_model_variant variant = {MARKER_COUNT, record->bytes};
    fist_model_pose pose = {FIST_MODEL_DIRECTIONS, 0, 0};
    if (fist_model_variant_get(record, &pose, &variant) != -1 ||
        fist_model_variant_get(NULL, &pose, &variant) != -1 ||
        fist_model_variant_get(record, NULL, &variant) != -1 ||
        fist_model_variant_get(record, &pose, NULL) != -1 ||
        variant_marker_intact(&variant, record->bytes) == 0) {
        return CONTRACT_FAILURE;
    }
    pose = (fist_model_pose){0, record->part_count, 0};
    if (fist_model_variant_get(record, &pose, &variant) != -1 ||
        variant_marker_intact(&variant, record->bytes) == 0) {
        return CONTRACT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int write_variant(const fist_model_record *record, const fist_model_pose *pose,
                         size_t record_index) {
    fist_model_variant variant = {0};
    if (fist_model_variant_get(record, pose, &variant) != 0) {
        return CONTRACT_FAILURE;
    }
    const size_t orientation = pose->facing == 0 ? 1 : 0;
    const size_t pieces = variant.pieces.size / PIECE_SIZE;
    printf("variant %zu %zu %zu %zu %u %zu\n", record_index, orientation, pose->part, pose->variant,
           (unsigned)variant.priority, pieces);
    fist_model_piece piece = {.sprite_index = MARKER_COUNT};
    if (fist_model_piece_get(&variant, pieces, &piece) != -1 ||
        fist_model_piece_get(NULL, 0, &piece) != -1 ||
        fist_model_piece_get(&variant, 0, NULL) != -1 || piece.sprite_index != MARKER_COUNT ||
        piece.use_base != 0 || piece.mirrored != 0 || piece.offset_x != 0 || piece.offset_y != 0) {
        return CONTRACT_FAILURE;
    }
    for (size_t index = 0; index < pieces; ++index) {
        if (fist_model_piece_get(&variant, index, &piece) != 0) {
            return CONTRACT_FAILURE;
        }
        printf("piece %zu %zu %zu %zu %zu %u %u %u %d %d\n", record_index, orientation, pose->part,
               pose->variant, index, (unsigned)piece.sprite_index, (unsigned)piece.use_base,
               (unsigned)piece.mirrored, (int)piece.offset_x, (int)piece.offset_y);
    }
    return EXIT_SUCCESS;
}

static int write_record(size_t index, const fist_model_record *record) {
    if (check_variant_errors(record) != EXIT_SUCCESS) {
        return CONTRACT_FAILURE;
    }
    printf("record %zu %u %u %zu %zu %zu", index, (unsigned)record->facing[0],
           (unsigned)record->facing[1], record->part_count, record->sprite_count,
           record->bytes.size);
    write_bytes(record->bytes);
    for (size_t sprite_index = 0; sprite_index < record->sprite_count; ++sprite_index) {
        const fist_model_sprite *sprite = &record->sprites[sprite_index];
        printf("sprite %zu %zu %u %u", index, sprite_index, (unsigned)sprite->width,
               (unsigned)sprite->height);
        write_bytes(sprite->pixels);
    }
    for (size_t orientation = 0; orientation < FIST_MODEL_ORIENTATIONS; ++orientation) {
        for (size_t part = 0; part < record->part_count; ++part) {
            const size_t count = record->parts[orientation][part].variants.size / sizeof(uint16_t);
            fist_model_pose pose = {(uint8_t)(orientation == 0 ? FIST_MODEL_DIRECTIONS / 2 : 0),
                                    part, count};
            fist_model_variant variant = {MARKER_COUNT, record->bytes};
            if (fist_model_variant_get(record, &pose, &variant) != -1 ||
                variant_marker_intact(&variant, record->bytes) == 0) {
                return CONTRACT_FAILURE;
            }
            for (size_t variant_index = 0; variant_index < count; ++variant_index) {
                pose.variant = variant_index;
                if (write_variant(record, &pose, index) != EXIT_SUCCESS) {
                    return CONTRACT_FAILURE;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

static int write_file(const fist_model_file *file) {
    printf("records %zu\n", file->count);
    for (size_t index = 0; index < file->count; ++index) {
        if (write_record(index, &file->records[index]) != EXIT_SUCCESS) {
            return CONTRACT_FAILURE;
        }
    }
    return ferror(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int probe_file(int prefixes, const char *path, size_t base_count) {
    FILE *input = fopen(path, "rb");
    if (input == NULL) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *bytes = fist_probe_read_file(input, &size);
    const int closed = fclose(input);
    fist_model_file file = {.count = MARKER_COUNT};
    if (bytes == NULL || closed != 0) {
        free(bytes);
        return EXIT_FAILURE;
    }
    if (fist_model_file_decode((fist_asset_view){NULL, 0}, base_count, &file) != -1 ||
        fist_model_file_decode((fist_asset_view){bytes, size}, base_count, NULL) != -1 ||
        file.count != MARKER_COUNT || file.records != NULL || file.storage != NULL) {
        free(bytes);
        return CONTRACT_FAILURE;
    }
    if (prefixes != 0) {
        for (size_t length = 0; length < size; ++length) {
            if (fist_model_file_decode((fist_asset_view){bytes, length}, base_count, &file) != -1 ||
                file.count != MARKER_COUNT || file.records != NULL || file.storage != NULL) {
                free(bytes);
                return CONTRACT_FAILURE;
            }
        }
        free(bytes);
        return EXIT_SUCCESS;
    }
    if (fist_model_file_decode((fist_asset_view){bytes, size}, base_count, &file) != 0) {
        free(bytes);
        return file.count == MARKER_COUNT && file.records == NULL && file.storage == NULL
                   ? EXIT_FAILURE
                   : CONTRACT_FAILURE;
    }
    for (size_t index = 0; index < size; ++index) {
        bytes[index] = 0;
    }
    free(bytes);
    const int result = write_file(&file);
    fist_model_file_destroy(&file);
    fist_model_file_destroy(&file);
    return file.count == 0 && file.records == NULL && file.storage == NULL ? result
                                                                           : CONTRACT_FAILURE;
}

static int model_marker_intact(const fist_model *model) {
    if (model->part_count != MARKER_COUNT) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (model->palette.rgb6[index] != MARKER_COMPONENT) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_MODEL_FILE_COUNT; ++index) {
        const fist_model_file *file = &model->files[index];
        if (file->count != 0 || file->storage != NULL || file->records != NULL) {
            return 0;
        }
    }
    for (size_t index = 0; index < FIST_MODEL_DIRECTIONS; ++index) {
        if (model->faces[index] != NULL) {
            return 0;
        }
    }
    return 1;
}

static int write_faces(const fist_model *model) {
    for (size_t facing = 0; facing < FIST_MODEL_DIRECTIONS; ++facing) {
        size_t matches = 0;
        for (size_t index = 1; index < FIST_MODEL_FILE_COUNT; ++index) {
            for (size_t record = 0; record < model->files[index].count; ++record) {
                if (model->faces[facing] == &model->files[index].records[record]) {
                    printf("face %zu %zu %zu\n", facing, index, record);
                    ++matches;
                }
            }
        }
        if (matches != 1) {
            return CONTRACT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static int probe_model(const char *name, fist_probe_source *storage) {
    const fist_asset_source source = {fist_probe_source_read, storage};
    fist_model model = {.part_count = MARKER_COUNT};
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        model.palette.rgb6[index] = MARKER_COMPONENT;
    }
    if (fist_model_load(NULL, &source, &model) != -1 || fist_model_load(name, NULL, &model) != -1 ||
        fist_model_load(name, &source, NULL) != -1 || model_marker_intact(&model) == 0) {
        return CONTRACT_FAILURE;
    }
    const int loaded = fist_model_load(name, &source, &model);
    fist_probe_source_close(storage);
    if (loaded != 0) {
        return model_marker_intact(&model) != 0 ? EXIT_FAILURE : CONTRACT_FAILURE;
    }
    printf("model %zu", model.part_count);
    write_bytes((fist_asset_view){model.palette.rgb6, FIST_PALETTE_SIZE});
    int result = EXIT_SUCCESS;
    for (size_t index = 0; index < FIST_MODEL_FILE_COUNT && result == EXIT_SUCCESS; ++index) {
        printf("file %zu\n", index);
        result = write_file(&model.files[index]);
    }
    if (write_faces(&model) != EXIT_SUCCESS) {
        result = CONTRACT_FAILURE;
    }
    fist_model_destroy(&model);
    fist_model_destroy(&model);
    if (model.part_count != 0) {
        return CONTRACT_FAILURE;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (model.palette.rgb6[index] != 0) {
            return CONTRACT_FAILURE;
        }
    }
    for (size_t index = 0; index < FIST_MODEL_DIRECTIONS; ++index) {
        if (model.faces[index] != NULL) {
            return CONTRACT_FAILURE;
        }
    }
    return result;
}

int main(int argc, char **argv) {
    fist_model_file_destroy(NULL);
    fist_model_destroy(NULL);
    if (argc != 4) {
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "model") == 0) {
        fist_probe_source storage = {.directory = argv[2]};
        return probe_model(argv[3], &storage);
    }
    if (strcmp(argv[1], "file") != 0 && strcmp(argv[1], "prefixes") != 0) {
        return EXIT_FAILURE;
    }
    errno = 0;
    char *end = NULL;
    const unsigned long count = strtoul(argv[3], &end, 10);
    if (errno != 0 || end == argv[3] || *end != '\0' || argv[3][0] == '-' || count > SIZE_MAX) {
        return EXIT_FAILURE;
    }
    return probe_file(strcmp(argv[1], "prefixes") == 0, argv[2], (size_t)count);
}
