#include "sim/object_pool.h"
#include "assets/units.h"

#include <stddef.h>
#include <stdint.h>

_Static_assert(FIST_POOL_SHORT_SLOTS + FIST_POOL_EXTENDED_SLOTS == FIST_UNIT_REGISTRY_COUNT,
               "The original object arenas and registry must agree");

static int extended(uint16_t type) {
    return fist_unit_state_size(type) == FIST_UNIT_EXTENDED_SIZE;
}

int fist_object_pool_is_valid(const fist_object_pool *pool) {
    if (pool == NULL || pool->short_count > FIST_POOL_SHORT_SLOTS ||
        pool->extended_count > FIST_POOL_EXTENDED_SLOTS) {
        return 0;
    }
    unsigned short_count = 0;
    unsigned extended_count = 0;
    uint8_t owners[FIST_UNIT_REGISTRY_COUNT] = {0};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const fist_pool_slot *slot = &pool->slots[index];
        if (slot->used > 1 ||
            (slot->used != 0 && (fist_unit_state_size(slot->type) == 0 ||
                                 extended(slot->type) != (index >= FIST_POOL_SHORT_SLOTS)))) {
            return 0;
        }
        if (index < FIST_POOL_SHORT_SLOTS) {
            short_count += slot->used;
        } else {
            extended_count += slot->used;
        }
        const uint16_t target = pool->registry[index].slot;
        if (target != FIST_POOL_NO_SLOT && (target >= FIST_UNIT_REGISTRY_COUNT ||
                                            pool->slots[target].used == 0 || owners[target] != 0)) {
            return 0;
        }
        if (target != FIST_POOL_NO_SLOT) {
            owners[target] = 1;
        }
    }
    return short_count == pool->short_count && extended_count == pool->extended_count;
}

void fist_object_pool_reset(fist_object_pool *pool) {
    if (pool == NULL) {
        return;
    }
    *pool = (fist_object_pool){0};
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        pool->registry[index].slot = FIST_POOL_NO_SLOT;
    }
}

static uint16_t free_slot(const fist_object_pool *pool, uint16_t type) {
    const size_t begin = extended(type) != 0 ? FIST_POOL_SHORT_SLOTS : 0;
    const size_t end = extended(type) != 0 ? FIST_UNIT_REGISTRY_COUNT : FIST_POOL_SHORT_SLOTS;
    for (size_t index = begin; index < end; ++index) {
        if (pool->slots[index].used == 0) {
            return (uint16_t)index;
        }
    }
    return FIST_POOL_NO_SLOT;
}

static int bind_slot(fist_object_pool *pool, fist_pool_import request, fist_pool_allocation *out) {
    const uint16_t slot = free_slot(pool, request.type);
    if (slot == FIST_POOL_NO_SLOT) {
        return FIST_POOL_UNAVAILABLE;
    }
    pool->slots[slot] = (fist_pool_slot){request.type, 1};
    pool->registry[request.registry_index] = (fist_pool_entry){slot, request.value};
    if (extended(request.type) != 0) {
        ++pool->extended_count;
    } else {
        ++pool->short_count;
    }
    *out = (fist_pool_allocation){request.type, slot, request.registry_index, request.value};
    return FIST_POOL_OK;
}

int fist_object_pool_import(fist_object_pool *pool, fist_pool_import request,
                            fist_pool_allocation *out) {
    if (!fist_object_pool_is_valid(pool) || out == NULL ||
        fist_unit_state_size(request.type) == 0 ||
        request.registry_index >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    return bind_slot(pool, request, out);
}

int fist_object_pool_allocate(fist_object_pool *pool, fist_pool_request request,
                              fist_pool_allocation *out) {
    if (!fist_object_pool_is_valid(pool) || out == NULL ||
        fist_unit_state_size(request.type) == 0 || request.low_priority > 1) {
        return -1;
    }
    if (request.low_priority != 0 && pool->short_count >= FIST_POOL_LOW_PRIORITY_LIMIT) {
        return FIST_POOL_UNAVAILABLE;
    }
    for (size_t index = 0; index < FIST_UNIT_REGISTRY_COUNT; ++index) {
        const fist_pool_entry entry = pool->registry[index];
        if (entry.slot == FIST_POOL_NO_SLOT && entry.value == 0) {
            const fist_pool_import binding = {request.type, (uint16_t)index, 1};
            return bind_slot(pool, binding, out);
        }
    }
    return FIST_POOL_UNAVAILABLE;
}

int fist_object_pool_release(fist_object_pool *pool, uint16_t registry_index,
                             fist_pool_allocation *out) {
    if (!fist_object_pool_is_valid(pool) || out == NULL ||
        registry_index >= FIST_UNIT_REGISTRY_COUNT) {
        return -1;
    }
    fist_pool_entry *entry = &pool->registry[registry_index];
    if (entry->slot == FIST_POOL_NO_SLOT) {
        return FIST_POOL_UNAVAILABLE;
    }
    fist_pool_slot *slot = &pool->slots[entry->slot];
    *out = (fist_pool_allocation){slot->type, entry->slot, registry_index, entry->value};
    if (extended(slot->type) != 0) {
        --pool->extended_count;
    } else {
        --pool->short_count;
    }
    *slot = (fist_pool_slot){0};
    entry->slot = FIST_POOL_NO_SLOT;
    entry->value = (uint16_t)(entry->value - 1);
    return FIST_POOL_OK;
}
