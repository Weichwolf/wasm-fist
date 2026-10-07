#ifndef FIST_SIM_OBJECT_POOL_H
#define FIST_SIM_OBJECT_POOL_H

#include "assets/units.h"

#include <stdint.h>

enum {
    FIST_POOL_SHORT_SLOTS = 150,
    FIST_POOL_EXTENDED_SLOTS = 32,
    FIST_POOL_LOW_PRIORITY_LIMIT = 120,
    FIST_POOL_NO_SLOT = UINT16_MAX,
    FIST_POOL_OK = 0,
    FIST_POOL_UNAVAILABLE = 1
};

typedef struct {
    uint16_t type;
    uint8_t used;
} fist_pool_slot;

typedef struct {
    /* Flat slot: short 0..149, extended 150..181, or NO_SLOT. */
    uint16_t slot;
    /* Original saved registry word. It is not a monotonic generation ID. */
    uint16_t value;
} fist_pool_entry;

typedef struct {
    fist_pool_slot slots[FIST_UNIT_REGISTRY_COUNT];
    fist_pool_entry registry[FIST_UNIT_REGISTRY_COUNT];
    /* Opaque physical allocation lifetimes, independent of saved registry values. */
    uint64_t lifetimes[FIST_UNIT_REGISTRY_COUNT];
    uint16_t short_count;
    uint16_t extended_count;
} fist_object_pool;

typedef struct {
    uint16_t type;
    uint8_t low_priority;
} fist_pool_request;

typedef struct {
    uint16_t type;
    uint16_t registry_index;
    uint16_t value;
} fist_pool_import;

typedef struct {
    uint16_t type;
    uint16_t slot;
    uint16_t registry_index;
    uint16_t value;
} fist_pool_allocation;

typedef struct {
    uint64_t lifetime;
    uint16_t slot;
} fist_object_reference;

/* Check reference representation only. Empty is exactly {0}; a nonempty
 * reference can be well formed while its allocation has already disappeared. */
int fist_object_reference_is_valid(fist_object_reference reference);

/* Capture a live physical object, including an orphan. Empty is {0}.
 * Release/reset invalidate references; reuse and reload cannot revive them.
 * In-place retype preserves the reference. IDs are process-local, never saved
 * gameplay data; simulation/pool mutations have one writer. Failure preserves out. */
int fist_object_pool_reference(const fist_object_pool *pool, uint16_t slot,
                               fist_object_reference *out);
int fist_object_pool_reference_is_live(const fist_object_pool *pool,
                                       fist_object_reference reference);

/* Metadata/identity owner only; typed object payloads belong to their simulation
 * owners. Reset invalidates every allocation, including orphaned imports. */
void fist_object_pool_reset(fist_object_pool *pool);

/* Read-only metadata validation, including arena occupancy and live bindings.
 * Orphaned physical allocations from duplicate snapshot imports are valid. */
int fist_object_pool_is_valid(const fist_object_pool *pool);

/* Resolve the current registry binding of a physical slot (original b354).
 * Orphans/unused slots return UNAVAILABLE; failure preserves output. */
int fist_object_pool_find(const fist_object_pool *pool, uint16_t slot, fist_pool_allocation *out);

/* Validate current saved-format binding metadata, without changing it.
 * This tuple can repeat after release/reuse; use references for retained targets. */
int fist_object_pool_is_current(const fist_object_pool *pool, fist_pool_allocation allocation);

/* Original in-place type-19 conversion keeps its arena and binding occupied.
 * Only types in the same storage class may replace a current allocation.
 * Return 0, or -1 preserving pool/output on invalid input. */
int fist_object_pool_retype(fist_object_pool *pool, fist_pool_allocation allocation, uint16_t type,
                            fist_pool_allocation *out);

/* Allocate the first physical slot of the original type class and first registry
 * vacancy whose pointer and saved word are both empty. Low priority uses the
 * original 120-short-object admission gate. Returns OK, UNAVAILABLE or -1 for
 * invalid input/state. Failure preserves the complete pool and output. */
int fist_object_pool_allocate(fist_object_pool *pool, fist_pool_request request,
                              fist_pool_allocation *out);

/* Snapshot loader's explicit registry binding. A duplicate overwrites the
 * registry entry but retains its older physical allocation until reset. */
int fist_object_pool_import(fist_object_pool *pool, fist_pool_import request,
                            fist_pool_allocation *out);

/* Detach a live registry entry, free its physical slot and decrement its saved
 * word modulo 65536. UNAVAILABLE is an absent entry. The returned allocation
 * identifies the payload whose owner must apply its deletion/flag semantics. */
int fist_object_pool_release(fist_object_pool *pool, uint16_t registry_index,
                             fist_pool_allocation *out);

#endif
