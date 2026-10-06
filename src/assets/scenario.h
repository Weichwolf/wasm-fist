#ifndef FIST_ASSETS_SCENARIO_H
#define FIST_ASSETS_SCENARIO_H

#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_SCENARIO_CHUNK_HEADER_SIZE = 6,
    FIST_SCENARIO_HEADER_SIZE = 54,
    FIST_SCENARIO_NAME_SIZE = 16,
    FIST_SCENARIO_ASSET_COUNT = 4,
    FIST_SCENARIO_POSITION_COUNT = 8,
    FIST_SCENARIO_INFO_SIZE = 70
};

typedef enum {
    FIST_SCENARIO_HEADER,
    FIST_SCENARIO_UNITS,
    FIST_SCENARIO_PATHS,
    FIST_SCENARIO_STAMPS,
    FIST_SCENARIO_PLAYER_INFO,
    FIST_SCENARIO_BATTLE_INFO,
    FIST_SCENARIO_END,
    FIST_SCENARIO_CHUNK_COUNT
} fist_scenario_chunk;

typedef struct {
    uint16_t catalog_index;
    uint16_t catalog_value;
    fist_asset_view state;
} fist_scenario_unit;

typedef struct {
    fist_asset_view remaining;
    uint16_t remaining_count;
} fist_scenario_unit_iterator;

typedef struct {
    uint16_t version;
    uint8_t mode;
    uint8_t limit;
    int32_t map_positions[FIST_SCENARIO_POSITION_COUNT];
    /* heightmap, colormap, palette, sky; DOS space padding is removed. */
    char asset_names[FIST_SCENARIO_ASSET_COUNT][FIST_SCENARIO_NAME_SIZE + 1];
    uint16_t unit_count;
    fist_asset_view chunks[FIST_SCENARIO_CHUNK_COUNT];
} fist_scenario;

/* Decode the complete FSG envelope and DCBS record framing. Views borrow the
 * immutable input buffer, which must outlive the scenario and unit iterator.
 * Zero means success; invalid/incomplete input returns -1 without changing out.
 * Chunk semantics beyond metadata and record framing remain separate contracts. */
int fist_scenario_decode(const uint8_t *data, size_t size, fist_scenario *out);
/* Begin requires a successfully decoded scenario with its input still alive. */
fist_scenario_unit_iterator fist_scenario_units_begin(const fist_scenario *scenario);
/* Return 1 for a record, 0 for complete exhaustion, -1 for malformed input.
 * Exhaustion and errors leave the iterator and output unchanged. */
int fist_scenario_units_next(fist_scenario_unit_iterator *iterator, fist_scenario_unit *out);

#endif
