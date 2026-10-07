#ifndef FIST_MISSION_PROBE_IO_H
#define FIST_MISSION_PROBE_IO_H

#include "sim/mission_world.h"

void fist_probe_write_tree(const fist_tree *tree);
int fist_probe_write_mission_world(const fist_mission_world *world);

#endif
