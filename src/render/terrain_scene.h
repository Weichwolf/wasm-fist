#ifndef FIST_RENDER_TERRAIN_SCENE_H
#define FIST_RENDER_TERRAIN_SCENE_H

#include "assets/terrain.h"
#include "render/renderer.h"

/* Internal current-context scene implementation. Use the renderer API. */
int fist_draw_terrain_scene(fist_renderer *renderer, const fist_terrain *terrain,
                            const fist_terrain_view *view, const fist_scene_vehicle *vehicle);

enum { FIST_MAP_PERIOD = 524288, FIST_POSITION_SCALE = 256, FIST_TURN_SIZE = 65536 };
float fist_map_coordinate(int32_t position);
float fist_map_y_coordinate(int32_t position);
int32_t fist_map_delta(int32_t subject, int32_t observer);
/* Interpolate the exact two terrain triangles at an original map position. */
float fist_terrain_surface(const fist_terrain *terrain, int32_t map_x, int32_t map_y);

#endif
