#ifndef FIST_ASSETS_UNITS_H
#define FIST_ASSETS_UNITS_H

#include "assets/scenario.h"
#include "assets/view.h"

#include <stddef.h>
#include <stdint.h>

enum {
    FIST_UNIT_TYPE_COUNT = 28,
    FIST_UNIT_REGISTRY_COUNT = 182,
    FIST_UNIT_PLATOON_COUNT = 8,
    FIST_UNIT_MEMBERS_PER_PLATOON = 4,
    FIST_UNIT_ROSTER_COUNT = FIST_UNIT_PLATOON_COUNT * FIST_UNIT_MEMBERS_PER_PLATOON,
    FIST_UNIT_ROSTER_PLACEHOLDER = 23,
    FIST_UNIT_SHORT_SIZE = 55,
    FIST_UNIT_EXTENDED_SIZE = 251,
    FIST_UNIT_NO_DEFINITION = UINT16_MAX
};

typedef struct {
    uint16_t type;
    uint16_t registry_index;
    uint16_t generation;
    /* Saved allocation slot: the original loader replaces it after allocation. */
    uint16_t saved_pool_index;
    int32_t map_x;
    int32_t map_y;
    int32_t altitude;
    uint16_t heading;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t platoon;
    uint8_t member;
    /* Complete immutable snapshot, borrowed from the owning definition set. */
    fist_asset_view snapshot;
} fist_unit_definition;

typedef struct {
    fist_unit_definition *definitions;
    size_t count;
    uint8_t *storage;
    /* Definition indices, or FIST_UNIT_NO_DEFINITION; last assignment wins. */
    uint16_t registry[FIST_UNIT_REGISTRY_COUNT];
    uint16_t roster[FIST_UNIT_ROSTER_COUNT];
} fist_units;

/* Requires a decoded scenario with live input. Copies all DCBS bytes; no input
 * views survive. Decodes definitions and normal-side roster without executing
 * vehicle initialization. Return 0 on success, -1 on invalid data/allocation.
 * Failure leaves out unchanged. On success out must not already own storage. */
int fist_units_decode(const fist_scenario *scenario, fist_units *out);
/* Returns NULL for missing/invalid roster slots, including a destroyed set. */
const fist_unit_definition *fist_units_roster_get(const fist_units *units, size_t slot);
void fist_units_destroy(fist_units *units);

#endif
