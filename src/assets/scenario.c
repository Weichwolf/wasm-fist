#include "assets/scenario.h"
#include "assets/bytes.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    TAG_SIZE = 4,
    WORD_SIZE = 2,
    DWORD_SIZE = 4,
    MODE_OFFSET = 2,
    LIMIT_OFFSET = 5,
    POSITIONS_OFFSET = 6,
    CATALOG_VALUE_OFFSET = 4,
    DOS_BASENAME_SIZE = 8,
    DOS_EXTENSION_SIZE = 3,
    DOS_FILENAME_SIZE = DOS_BASENAME_SIZE + 1 + DOS_EXTENSION_SIZE
};

static int chunk_kind(const uint8_t *tag) {
    static const char tags[FIST_SCENARIO_CHUNK_COUNT][TAG_SIZE] = {"SHDR", "DCBS", "PATH", "STMP",
                                                                   "PINF", "BINF", "TERM"};
    for (int index = 0; index < FIST_SCENARIO_CHUNK_COUNT; ++index) {
        if (memcmp(tag, tags[index], TAG_SIZE) == 0) {
            return index;
        }
    }
    return -1;
}

static int read_chunks(fist_asset_view input, fist_scenario *scenario) {
    while (input.size >= FIST_SCENARIO_CHUNK_HEADER_SIZE) {
        const int kind = chunk_kind(input.data);
        const size_t size = fist_read_u16le(input.data + TAG_SIZE);
        if (size > input.size - FIST_SCENARIO_CHUNK_HEADER_SIZE) {
            return -1;
        }
        if (scenario->chunks[FIST_SCENARIO_HEADER].data == NULL && kind != FIST_SCENARIO_HEADER) {
            return -1;
        }
        const uint8_t *payload = input.data + FIST_SCENARIO_CHUNK_HEADER_SIZE;
        if (kind >= 0) {
            if (scenario->chunks[kind].data != NULL) {
                return -1;
            }
            scenario->chunks[kind] = (fist_asset_view){payload, size};
        }
        input.data = payload + size;
        input.size -= FIST_SCENARIO_CHUNK_HEADER_SIZE + size;
        if (kind == FIST_SCENARIO_END) {
            return size == 0 && input.size == 0 ? 0 : -1;
        }
    }
    return -1;
}

static int read_asset_names(fist_scenario *scenario) {
    const uint8_t *info = scenario->chunks[FIST_SCENARIO_BATTLE_INFO].data;
    for (size_t index = 0; index < FIST_SCENARIO_ASSET_COUNT; ++index) {
        const uint8_t *name = info + (index * FIST_SCENARIO_NAME_SIZE);
        const uint8_t *end = memchr(name, '\0', FIST_SCENARIO_NAME_SIZE);
        if (end == NULL) {
            /* INDIA4's sky field fills all 16 bytes with DOS space padding. */
            end = name + FIST_SCENARIO_NAME_SIZE;
        }
        size_t length = 0;
        for (const uint8_t *cursor = name; cursor < end; ++cursor) {
            /* DOS 8.3 names contain space padding, not literal space characters. */
            if (*cursor != ' ') {
                scenario->asset_names[index][length++] = (char)*cursor;
            }
        }
        if (length == 0 || length > DOS_FILENAME_SIZE) {
            return -1;
        }
        scenario->asset_names[index][length] = '\0';
    }
    return 0;
}

fist_scenario_unit_iterator fist_scenario_units_begin(const fist_scenario *scenario) {
    const fist_asset_view units = scenario->chunks[FIST_SCENARIO_UNITS];
    return (fist_scenario_unit_iterator){{units.data + WORD_SIZE, units.size - WORD_SIZE},
                                         scenario->unit_count};
}

int fist_scenario_units_next(fist_scenario_unit_iterator *iterator, fist_scenario_unit *out) {
    if (iterator == NULL || out == NULL) {
        return -1;
    }
    if (iterator->remaining_count == 0) {
        return iterator->remaining.size == 0 ? 0 : -1;
    }
    const fist_asset_view input = iterator->remaining;
    if (input.data == NULL || input.size < FIST_SCENARIO_CHUNK_HEADER_SIZE) {
        return -1;
    }
    const size_t size = fist_read_u16le(input.data);
    if (size < WORD_SIZE || size > input.size - FIST_SCENARIO_CHUNK_HEADER_SIZE) {
        return -1;
    }
    const uint8_t *state = input.data + FIST_SCENARIO_CHUNK_HEADER_SIZE;
    const fist_scenario_unit unit = {fist_read_u16le(input.data + WORD_SIZE),
                                     fist_read_u16le(input.data + CATALOG_VALUE_OFFSET),
                                     {state, size}};
    iterator->remaining =
        (fist_asset_view){state + size, input.size - FIST_SCENARIO_CHUNK_HEADER_SIZE - size};
    --iterator->remaining_count;
    *out = unit;
    return 1;
}

static int decode_metadata(fist_scenario *scenario) {
    for (size_t index = 0; index < FIST_SCENARIO_CHUNK_COUNT; ++index) {
        if (scenario->chunks[index].data == NULL) {
            return -1;
        }
    }
    const fist_asset_view header = scenario->chunks[FIST_SCENARIO_HEADER];
    const fist_asset_view units = scenario->chunks[FIST_SCENARIO_UNITS];
    if (header.size != FIST_SCENARIO_HEADER_SIZE || units.size < WORD_SIZE ||
        scenario->chunks[FIST_SCENARIO_BATTLE_INFO].size != FIST_SCENARIO_INFO_SIZE) {
        return -1;
    }
    scenario->version = fist_read_u16le(header.data);
    scenario->mode = header.data[MODE_OFFSET];
    scenario->limit = header.data[LIMIT_OFFSET];
    for (size_t index = 0; index < FIST_SCENARIO_POSITION_COUNT; ++index) {
        scenario->map_positions[index] =
            fist_read_i32le(header.data + POSITIONS_OFFSET + (index * DWORD_SIZE));
    }
    scenario->unit_count = fist_read_u16le(units.data);
    fist_scenario_unit_iterator iterator = fist_scenario_units_begin(scenario);
    fist_scenario_unit unit = {0};
    int result = 0;
    do {
        result = fist_scenario_units_next(&iterator, &unit);
    } while (result == 1);
    return result == 0 ? read_asset_names(scenario) : -1;
}

int fist_scenario_decode(const uint8_t *data, size_t size, fist_scenario *out) {
    if (data == NULL || out == NULL) {
        return -1;
    }
    fist_scenario scenario = {0};
    if (read_chunks((fist_asset_view){data, size}, &scenario) != 0 ||
        decode_metadata(&scenario) != 0) {
        return -1;
    }
    *out = scenario;
    return 0;
}
