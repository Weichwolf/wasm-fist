#include "assets/bytes.h"
#include "assets/owned_terrain.h"
#include "probe_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    BYTE_BITS = 8,
    DOUBLE_BYTES = 8,
    COORDINATE_BYTES = 16,
    COLOR_BYTES = 3,
    HEIGHT_LENGTH_OFFSET = 20
};

static int write_integer(uint64_t value, FILE *output, size_t size) {
    uint8_t bytes[DOUBLE_BYTES] = {0};
    for (size_t index = 0; index < size; ++index) {
        bytes[index] = (uint8_t)(value >> (index * BYTE_BITS));
    }
    return fwrite(bytes, 1, size, output) == size ? 0 : -1;
}

static int write_double(double value) {
    uint64_t bits = 0;
    fist_probe_capture(&value, sizeof(bits), &bits);
    return write_integer(bits, stdout, sizeof(bits));
}

static double read_double(const uint8_t *data) {
    uint64_t bits = 0;
    for (size_t index = 0; index < sizeof(bits); ++index) {
        bits |= (uint64_t)data[index] << (index * BYTE_BITS);
    }
    double value = 0;
    fist_probe_capture(&bits, sizeof(value), &value);
    return value;
}

static uint8_t *read_path(const char *path, size_t *size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        (void)fclose(file);
        return NULL;
    }
    const long length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        (void)fclose(file);
        return NULL;
    }
    *size = (size_t)length;
    uint8_t *data = malloc(*size != 0 ? *size : 1);
    if (data == NULL) {
        (void)fclose(file);
        return NULL;
    }
    const size_t read = fread(data, 1, *size, file);
    if (read != *size || ferror(file) != 0) {
        free(data);
        (void)fclose(file);
        return NULL;
    }
    const int end = feof(file) == 0 ? fgetc(file) : EOF;
    const int error = ferror(file);
    const int closed = fclose(file);
    if (end != EOF || error != 0 || closed != 0) {
        free(data);
        return NULL;
    }
    return data;
}

static int sample_coordinates(const fist_owned_terrain *terrain, const uint8_t *data, size_t size,
                              double **out, size_t *out_count) {
    if (size < sizeof(uint32_t)) {
        return -1;
    }
    const uint32_t count = fist_read_u32le(data);
    if (count == 0 || (size - sizeof(uint32_t)) / COORDINATE_BYTES != count ||
        (size - sizeof(uint32_t)) % COORDINATE_BYTES != 0) {
        return -1;
    }
    double *samples = malloc((size_t)count * sizeof(*samples));
    if (samples == NULL) {
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *pair = data + sizeof(uint32_t) + (index * COORDINATE_BYTES);
        double value = 1;
        unsigned char before[sizeof(value)] = {0};
        fist_probe_capture(&value, sizeof(value), before);
        if (fist_owned_terrain_surface(terrain, read_double(pair), read_double(pair + DOUBLE_BYTES),
                                       &value) != 0) {
            const int unchanged = fist_probe_unchanged(&value, sizeof(value), before);
            free(samples);
            return unchanged != 0 ? -1 : -2;
        }
        samples[index] = value;
    }
    *out = samples;
    *out_count = count;
    return 0;
}

static int write_terrain(const fist_owned_terrain *terrain, const double *samples,
                         size_t sample_count) {
    if (write_integer(terrain->side, stdout, sizeof(terrain->side)) != 0 ||
        write_double(terrain->minimum) != 0 || write_double(terrain->maximum) != 0 ||
        write_double(terrain->water_level) != 0) {
        return -1;
    }
    const size_t count = (size_t)terrain->side * terrain->side;
    for (size_t index = 0; index < count; ++index) {
        if (write_integer(terrain->heights[index], stdout, sizeof(*terrain->heights)) != 0) {
            return -1;
        }
    }
    if (fwrite(terrain->colors, COLOR_BYTES, count, stdout) != count) {
        return -1;
    }
    for (size_t index = 0; index < sample_count; ++index) {
        if (write_double(samples[index]) != 0) {
            return -1;
        }
    }
    return 0;
}

static int contracts(void) {
    fist_owned_terrain terrain = {0};
    unsigned char before[sizeof(terrain)] = {0};
    fist_probe_capture(&terrain, sizeof(terrain), before);
    double height = 1;
    if (fist_owned_terrain_decode(NULL, 0, &terrain) != -1 ||
        fist_owned_terrain_decode(NULL, 0, NULL) != -1 ||
        fist_owned_terrain_surface(NULL, 0, 0, &height) != -1 ||
        fist_owned_terrain_surface(&terrain, 0, 0, &height) != -1 ||
        fist_owned_terrain_surface(&terrain, 0, 0, NULL) != -1 || height != 1 ||
        fist_probe_unchanged(&terrain, sizeof(terrain), before) == 0) {
        return EXIT_FAILURE;
    }
    fist_owned_terrain_destroy(NULL);
    fist_owned_terrain_destroy(&terrain);
    fist_owned_terrain_destroy(&terrain);
    return terrain.side == 0 && terrain.heights == NULL && terrain.colors == NULL ? EXIT_SUCCESS
                                                                                  : EXIT_FAILURE;
}

static int rejected_decode(const fist_owned_terrain *terrain, const void *before,
                           size_t recovery_size) {
    if (fist_probe_unchanged(terrain, sizeof(*terrain), before) == 0) {
        (void)fputs("decode changed output on failure\n", stderr);
        return 2;
    }
    if (recovery_size != 0) {
        /* With the fixture memory limit, this only fits if the partially
         * allocated height plane was freed before returning failure. */
        void *recovered = malloc(recovery_size);
        if (recovered == NULL) {
            (void)fputs("allocation not recovered\n", stderr);
            return 2;
        }
        free(recovered);
    }
    (void)fputs("decode rejected\n", stderr);
    return EXIT_FAILURE;
}

static int observe_terrain(const fist_owned_terrain *terrain, const char *sample_path) {
    double *samples = NULL;
    size_t sample_count = 0;
    if (sample_path != NULL) {
        size_t size = 0;
        uint8_t *data = read_path(sample_path, &size);
        const int sampled =
            data != NULL ? sample_coordinates(terrain, data, size, &samples, &sample_count) : -1;
        free(data);
        if (sampled != 0) {
            (void)fputs(sampled == -2 ? "sample changed output on failure\n" : "sample rejected\n",
                        stderr);
            return sampled == -2 ? 2 : EXIT_FAILURE;
        }
    }
    const int result = write_terrain(terrain, samples, sample_count);
    free(samples);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "contracts") == 0) {
        return contracts();
    }
    if (argc < 2 || argc > 3) {
        return EXIT_FAILURE;
    }
    size_t size = 0;
    uint8_t *data = read_path(argv[1], &size);
    if (data == NULL) {
        (void)fputs("read failed\n", stderr);
        return EXIT_FAILURE;
    }
    fist_owned_terrain terrain = {0};
    unsigned char *terrain_bytes = (unsigned char *)&terrain;
    for (size_t index = 0; index < sizeof(terrain); ++index) {
        terrain_bytes[index] = 0;
    }
    terrain.side = UINT32_MAX;
    terrain.water_level = 1;
    unsigned char before[sizeof(terrain)] = {0};
    fist_probe_capture(&terrain, sizeof(terrain), before);
    const int decoded = fist_owned_terrain_decode(data, size, &terrain);
    const int allocation_check = argc == 3 && strcmp(argv[2], "allocation") == 0;
    size_t recovery_size = 0;
    if (allocation_check != 0 && size >= HEIGHT_LENGTH_OFFSET + sizeof(uint32_t)) {
        recovery_size = size + fist_read_u32le(data + HEIGHT_LENGTH_OFFSET);
    }
    /* Mutate and release the complete source before observing any owned data. */
    volatile uint8_t *source = data;
    for (size_t index = 0; index < size; ++index) {
        source[index] = UINT8_MAX;
    }
    free(data);
    if (decoded != 0) {
        return rejected_decode(&terrain, before, recovery_size);
    }
    if (allocation_check != 0) {
        fist_owned_terrain_destroy(&terrain);
        return EXIT_FAILURE;
    }
    const int result = observe_terrain(&terrain, argc == 3 ? argv[2] : NULL);
    fist_owned_terrain_destroy(&terrain);
    fist_owned_terrain_destroy(&terrain);
    return result == 0 && terrain.heights == NULL && terrain.colors == NULL && terrain.side == 0
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
