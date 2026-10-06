#include "assets/resource.h"
#include "assets/bytes.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    RESOURCE_TAG_SIZE = 12,
    HEADER_SIZE = 16,
    COUNT_OFFSET = 12,
    ENTRY_SIZE = 16,
    NAME_SIZE = 12,
    OFFSET_POSITION = 12,
    KEY_SIZE = 4,
    ASCII_SPACE = 32,
    ASCII_LAST = 126
};

static int encode_name(const char *name, uint8_t *encoded) {
    static const uint8_t key[KEY_SIZE] = {0xad, 0xde, 0xed, 0xac};
    size_t length = 0;
    for (; name[length] != '\0'; ++length) {
        const unsigned char character = (unsigned char)name[length];
        if (length == NAME_SIZE || character <= ASCII_SPACE || character > ASCII_LAST ||
            character == '/' || character == '\\' || character == ':') {
            return -1;
        }
        /* Original 625e/6266 DOS folding, including its punctuation mapping. */
        encoded[length] = character >= '`' ? (uint8_t)(character - ('a' - 'A')) : character;
    }
    if (length == 0) {
        return -1;
    }
    for (; length < NAME_SIZE; ++length) {
        encoded[length] = 0;
    }
    for (size_t index = 0; index < NAME_SIZE; ++index) {
        encoded[index] ^= key[index % KEY_SIZE];
    }
    return 0;
}

int fist_resource_find(const uint8_t *data, size_t size, const char *name, fist_asset_view *out) {
    if (data == NULL || name == NULL || out == NULL || size < HEADER_SIZE + ENTRY_SIZE ||
        memcmp(data, "RESOURCE1\r\n\x1a", RESOURCE_TAG_SIZE) != 0) {
        return -1;
    }
    uint8_t encoded[NAME_SIZE] = {0};
    if (encode_name(name, encoded) != 0) {
        return -1;
    }
    const size_t count = fist_read_u32le(data + COUNT_OFFSET);
    if (count > ((size - HEADER_SIZE) / ENTRY_SIZE) - 1) {
        return -1;
    }
    const size_t directory_end = HEADER_SIZE + ((count + 1) * ENTRY_SIZE);
    size_t previous = directory_end;
    for (size_t index = 0; index <= count; ++index) {
        const size_t offset =
            fist_read_u32le(data + HEADER_SIZE + (index * ENTRY_SIZE) + OFFSET_POSITION);
        if (offset < previous || offset > size || (index == 0 && offset != directory_end)) {
            return -1;
        }
        previous = offset;
    }
    if (previous != size) {
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        const uint8_t *entry = data + HEADER_SIZE + (index * ENTRY_SIZE);
        if (memcmp(entry, encoded, NAME_SIZE) == 0) {
            const size_t offset = fist_read_u32le(entry + OFFSET_POSITION);
            const size_t end = fist_read_u32le(entry + ENTRY_SIZE + OFFSET_POSITION);
            *out = (fist_asset_view){data + offset, end - offset};
            return 0;
        }
    }
    return -1;
}
