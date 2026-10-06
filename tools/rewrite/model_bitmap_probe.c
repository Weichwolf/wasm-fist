#include "assets/model.h"
#include "assets/palette.h"
#include "assets/source.h"
#include "probe_source.h"
#include "render/model_bitmap.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CONTRACT_FAILURE = 2, MARKER_WIDTH = 137, WORD_SIZE = 2, SECONDARY_FACE_OFFSET = 16 };

static int marker_intact(const fist_model_bitmap *bitmap) {
    if (bitmap->indices != NULL || bitmap->width != MARKER_WIDTH || bitmap->height != 0 ||
        bitmap->left != 0 || bitmap->top != 0) {
        return 0;
    }
    for (size_t index = 0; index < FIST_PALETTE_SIZE; ++index) {
        if (bitmap->palette.rgb6[index] != 0) {
            return 0;
        }
    }
    return 1;
}

static void write_bytes(const uint8_t *data, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        printf(" %02x", (unsigned)data[index]);
    }
    printf("\n");
}

static void set_poses(size_t count, fist_model_pose *poses, uint8_t facing) {
    for (size_t part = 0; part < count; ++part) {
        poses[part] = (fist_model_pose){.facing = facing, .part = part};
    }
}

static int failed_selection(const fist_model *model, const fist_model_selection *selection) {
    fist_model_bitmap marker = {.width = MARKER_WIDTH};
    return fist_model_compose(model, selection, &marker) == -1 && marker_intact(&marker) != 0;
}

static int check_invalid(fist_model *model, fist_model_pose *poses) {
    fist_model_selection selection = {.poses = poses, .count = model->part_count};
    if (failed_selection(NULL, &selection) == 0 || failed_selection(model, NULL) == 0 ||
        fist_model_compose(model, &selection, NULL) != -1) {
        return -1;
    }
    selection.count = model->part_count + 1;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    selection.count = model->part_count;
    selection.poses = NULL;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    selection.poses = poses;
    poses[0].part = model->part_count;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    poses[0].part = 0;
    poses[0].facing = FIST_MODEL_DIRECTIONS;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    poses[0].facing = 0;
    poses[0].variant = FIST_MODEL_PART_VARIANTS;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    poses[0].variant = 0;
    const size_t last = model->part_count - 1;
    poses[last].variant = FIST_MODEL_PART_VARIANTS;
    if (failed_selection(model, &selection) == 0) {
        return -1;
    }
    poses[last].variant = 0;
    const fist_model_record *record = model->faces[0];
    model->faces[0] = NULL;
    const int failed = failed_selection(model, &selection);
    model->faces[0] = record;
    return failed != 0 ? 0 : -1;
}

static int write_bitmap(const fist_model_bitmap *bitmap) {
    printf("bitmap %d %d %u %u", (int)bitmap->left, (int)bitmap->top, (unsigned)bitmap->width,
           (unsigned)bitmap->height);
    write_bytes(bitmap->indices, (size_t)bitmap->width * bitmap->height);
    return ferror(stdout) == 0 ? 0 : -1;
}

static int compose_case(const fist_model *model, const fist_model_pose *poses) {
    const fist_model_selection selection = {.poses = poses, .count = model->part_count};
    fist_model_bitmap bitmap = {0};
    if (fist_model_compose(model, &selection, &bitmap) != 0) {
        return -1;
    }
    const int result = write_bitmap(&bitmap);
    fist_model_bitmap_destroy(&bitmap);
    return result;
}

static int write_cases(const fist_model *model, fist_model_pose *poses) {
    for (unsigned facing = 0; facing < FIST_MODEL_DIRECTIONS; ++facing) {
        set_poses(model->part_count, poses, (uint8_t)facing);
        printf("case %u -1 0\n", facing);
        if (compose_case(model, poses) != 0) {
            return -1;
        }
        const uint8_t secondary =
            (uint8_t)((facing + SECONDARY_FACE_OFFSET) % FIST_MODEL_DIRECTIONS);
        const fist_model_record *record = model->faces[secondary];
        const size_t orientation = secondary < SECONDARY_FACE_OFFSET ? 1 : 0;
        for (size_t part = 0; part < model->part_count; ++part) {
            const size_t count = record->parts[orientation][part].variants.size / WORD_SIZE;
            for (size_t variant = 0; variant < count; ++variant) {
                set_poses(model->part_count, poses, (uint8_t)facing);
                poses[part].facing = secondary;
                poses[part].variant = variant;
                printf("case %u %zu %zu\n", facing, part, variant);
                if (compose_case(model, poses) != 0) {
                    return -1;
                }
            }
        }
    }
    return 0;
}

static int run_model(const char *name, fist_probe_source *storage) {
    const fist_asset_source source = {fist_probe_source_read, storage};
    fist_model model = {0};
    const int loaded = fist_model_load(name, &source, &model);
    fist_probe_source_close(storage);
    if (loaded != 0) {
        return EXIT_FAILURE;
    }
    fist_model_pose *poses = calloc(model.part_count + 1, sizeof(*poses));
    if (poses == NULL) {
        fist_model_destroy(&model);
        return EXIT_FAILURE;
    }
    set_poses(model.part_count, poses, 0);
    if (model.part_count == 0 || check_invalid(&model, poses) != 0) {
        free(poses);
        fist_model_destroy(&model);
        return CONTRACT_FAILURE;
    }
    fist_model_bitmap retained = {0};
    const fist_model_selection selection = {.poses = poses, .count = model.part_count};
    if (fist_model_compose(&model, &selection, &retained) != 0) {
        free(poses);
        fist_model_destroy(&model);
        return CONTRACT_FAILURE;
    }
    printf("parts %zu\n", model.part_count);
    int result = write_cases(&model, poses) == 0 ? EXIT_SUCCESS : CONTRACT_FAILURE;
    fist_model_destroy(&model);
    free(poses);
    /* Observe complete independent indices/palette after source/model destruction. */
    printf("retained\n");
    write_bytes(retained.palette.rgb6, FIST_PALETTE_SIZE);
    if (write_bitmap(&retained) != 0) {
        result = CONTRACT_FAILURE;
    }
    fist_model_bitmap_destroy(&retained);
    fist_model_bitmap_destroy(&retained);
    fist_model_bitmap_destroy(NULL);
    return retained.indices == NULL && retained.width == 0 ? result : CONTRACT_FAILURE;
}

int main(int argc, char **argv) {
    if (argc != 4 || strcmp(argv[1], "all") != 0) {
        return EXIT_FAILURE;
    }
    fist_probe_source source = {.directory = argv[2]};
    return run_model(argv[3], &source);
}
