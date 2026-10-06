#include "app/driving_view.h"
#include "app/driving.h"
#include "render/hud.h"
#include "render/model_bitmap.h"
#include "render/renderer.h"
#include "render/vehicle_scene.h"
#include "sim/weapon_control.h"
#include <stddef.h>
int fist_driving_draw(const fist_driving *driving, fist_renderer *renderer) {
    if (driving == NULL || renderer == NULL) {
        return -1;
    }
    fist_terrain_view view = {0};
    fist_scene_vehicle vehicle = {0};
    const fist_scene_actor actor = {&driving->player, &driving->visual, &driving->model};
    fist_hud hud = {.paused = driving->paused};
    if (fist_weapon_inspect(&driving->player, &hud.weapon) != 0 ||
        fist_vehicle_follow_view(&driving->terrain, &driving->player, &view) != 0 ||
        fist_scene_actor_prepare(&driving->terrain, &actor, &view, &vehicle) != 0) {
        return -1;
    }
    const int result = fist_renderer_draw_vehicle(renderer, &driving->terrain, &view, &vehicle);
    fist_model_bitmap_destroy(&vehicle.bitmap);
    return result == 0 ? fist_renderer_draw_hud(renderer, &hud) : result;
}
