#ifndef FIST_VEHICLE_PROBE_IO_H
#define FIST_VEHICLE_PROBE_IO_H

#include "sim/vehicle_state.h"

#include <stddef.h>
#include <stdint.h>

/* Original serialized component start for a supported ground class. */
size_t fist_probe_component_offset(uint16_t type);

void fist_probe_write_vehicle_state(const fist_vehicle_state *state);

#endif
