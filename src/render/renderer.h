#ifndef FIST_RENDER_RENDERER_H
#define FIST_RENDER_RENDERER_H

#include "assets/terrain.h"
#include "assets/units.h"
#include "render/hud.h"
#include "render/model_bitmap.h"

#include <stdint.h>

typedef struct fist_renderer fist_renderer;

enum { FIST_RENDER_HELPERS = 3, FIST_RENDER_TARGET_SAMPLES = 4 };

typedef struct {
    int width;
    int height;
    int sample_buffers;
    int samples;
    int multisample_enabled;
    int configured_helpers;
    int started_helpers;
    int total_threads;
} fist_render_profile;

typedef struct {
    /* Original integer map coordinates. Altitude is in decoded height units;
     * heading is one unsigned 16-bit turn, pitch is radians (positive up). */
    int32_t map_x;
    int32_t map_y;
    float altitude;
    uint16_t heading;
    float pitch;
} fist_terrain_view;

typedef struct {
    fist_model_bitmap bitmap;
    int32_t map_x;
    int32_t map_y;
    float altitude;
    float texel_width;
    float texel_height;
} fist_scene_vehicle;

/* One render-thread owner. Pixels are tightly packed RGBA8, bottom row first;
 * the borrowed view remains valid until the next draw or destruction. */
fist_renderer *fist_renderer_create(int width, int height);
/* Accept zero samples for diagnostics or four for the target profile. Both
 * require three successfully started helpers before the first draw. */
fist_renderer *fist_renderer_create_multisample(int width, int height, int samples);
/* Observe actual framebuffer/worker state; failure leaves out unchanged. */
int fist_renderer_get_profile(const fist_renderer *renderer, fist_render_profile *out);
void fist_renderer_destroy(fist_renderer *renderer);
const uint8_t *fist_renderer_pixels(fist_renderer *renderer);

/* Diagnostic geometry for dependency/presentation verification, not gameplay. */
int fist_renderer_draw_probe(fist_renderer *renderer);
int fist_renderer_draw_hud(fist_renderer *renderer, const fist_hud *hud);

/* Render a successfully loaded fist_terrain bundle around a finite camera pose.
 * The bundle is borrowed only during the draw; queued work is completed before
 * return. Returns 0 on success or -1 on missing inputs, allocation or GL errors. */
int fist_renderer_draw_terrain(fist_renderer *renderer, const fist_terrain *terrain,
                               const fist_terrain_view *view);
/* Borrow an already composed vehicle during the draw; transparent pixels do
 * not write color/depth. The billboard shares terrain projection and fog. */
int fist_renderer_draw_vehicle(fist_renderer *renderer, const fist_terrain *terrain,
                               const fist_terrain_view *view, const fist_scene_vehicle *vehicle);
/* Deliberate inspection camera: selected ground vehicle's X/Y and heading, altitude
 * 64 render units above the decoded ground sample and a slight downward pitch.
 * This is a preview view, not reconstructed vehicle/cockpit camera behavior. */
int fist_terrain_inspection_view(const fist_terrain *terrain, const fist_unit_definition *vehicle,
                                 fist_terrain_view *out);

#endif
