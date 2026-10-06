#ifndef FIST_RENDER_TERRAIN_SCENE_H
#define FIST_RENDER_TERRAIN_SCENE_H

#include "assets/terrain.h"
#include "render/renderer.h"

/* Internal current-context scene implementation. Use the renderer API. */
int fist_draw_terrain_scene(fist_renderer *renderer, const fist_terrain *terrain,
                            const fist_terrain_view *view, const fist_scene_vehicle *vehicle);

float fist_map_coordinate(int32_t position);
float fist_map_y_coordinate(int32_t position);
int32_t fist_map_delta(int32_t subject, int32_t observer);
/* Interpolate the exact two terrain triangles at an original map position. */
float fist_terrain_surface(const fist_terrain *terrain, int32_t map_x, int32_t map_y);

#endif
