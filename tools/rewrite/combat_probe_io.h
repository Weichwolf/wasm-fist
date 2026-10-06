#ifndef FIST_COMBAT_PROBE_IO_H
#define FIST_COMBAT_PROBE_IO_H

#include "sim/other_damage.h"
#include "sim/projectile_flight.h"
#include "sim/smoke.h"

void fist_probe_write_other_actor(const fist_other_actor *actor);
void fist_probe_write_explosion(const fist_explosion *effect);
void fist_probe_write_smoke(const fist_drifting_smoke *smoke);

#endif
