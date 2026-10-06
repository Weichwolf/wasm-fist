#ifndef FIST_RENDER_TERRAIN_SCENE_H
#define FIST_RENDER_TERRAIN_SCENE_H

#include "assets/terrain.h"
#include "render/renderer.h"

/* Internal current-context scene implementation. Use the renderer API. */
int fist_draw_terrain_scene(fist_renderer *renderer, const fist_terrain *terrain,
                            const fist_terrain_view *view);

#endif
