#include "probe_source.h"
#include "assets/source.h"
#include "assets/view.h"
#include "probe_io.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *asset_path(const char *directory, const char *name) {
    const size_t prefix = strlen(directory);
    const size_t length = strlen(name);
    if (prefix > SIZE_MAX - 2 || length > SIZE_MAX - prefix - 2) {
        return NULL;
    }
    char *path = malloc(prefix + length + 2);
    if (path == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < prefix; ++index) {
        path[index] = directory[index];
    }
    path[prefix] = '/';
    for (size_t index = 0; index < length; ++index) {
        const char value = name[index];
        if (value == '/' || value == '\\' || value == ':' || value <= ' ') {
            free(path);
            return NULL;
        }
        path[prefix + 1 + index] = value;
        if (value >= 'a' && value <= 'z') {
            path[prefix + 1 + index] = (char)(value - ('a' - 'A'));
        }
    }
    path[prefix + length + 1] = '\0';
    return path;
}

fist_asset_read_result fist_probe_source_read(void *context, const char *name,
                                              fist_asset_view *out) {
    fist_probe_source *source = context;
    char *path = asset_path(source->directory, name);
    free(source->buffer);
    source->buffer = NULL;
    if (path == NULL) {
        return FIST_ASSET_READ_ERROR;
    }
    FILE *file = fopen(path, "rb");
    const int open_error = errno;
    free(path);
    if (file == NULL) {
        return open_error == ENOENT ? FIST_ASSET_READ_NOT_FOUND : FIST_ASSET_READ_ERROR;
    }
    size_t size = 0;
    source->buffer = fist_probe_read_file(file, &size);
    const int closed = fclose(file);
    if (source->buffer == NULL || closed != 0) {
        fist_probe_source_close(source);
        return FIST_ASSET_READ_ERROR;
    }
    *out = (fist_asset_view){source->buffer, size};
    return FIST_ASSET_READ_OK;
}

void fist_probe_source_close(fist_probe_source *source) {
    free(source->buffer);
    source->buffer = NULL;
}
