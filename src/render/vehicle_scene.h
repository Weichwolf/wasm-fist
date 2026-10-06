#ifndef FIST_RENDER_VEHICLE_SCENE_H
#define FIST_RENDER_VEHICLE_SCENE_H

#include "assets/model.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "render/renderer.h"

#include <stdint.h>

/* Explicit preview camera: behind the selected vehicle, above the exact
 * decoded mesh surface. This does not execute original camera installation. */
int fist_vehicle_inspection_view(const fist_terrain *terrain, const fist_unit_definition *vehicle,
                                 uint16_t heading, fist_terrain_view *out);
/* Select/compose the original default C model for the observer-to-object
 * bearing. Place its origin on the preview mesh, ignoring uninitialized FSG
 * altitude. Retain MAL colors directly as an intentional visual improvement.
 * Success owns out.bitmap; failure preserves out. No gameplay state changes. */
int fist_scene_vehicle_prepare(const fist_terrain *terrain, const fist_unit_definition *definition,
                               const fist_model *model, const fist_terrain_view *view,
                               fist_scene_vehicle *out);

#endif
