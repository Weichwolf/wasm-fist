#ifndef FIST_RENDER_HUD_H
#define FIST_RENDER_HUD_H

#include "sim/weapon_control.h"

#include <stdint.h>

typedef struct {
    fist_weapon_status weapon;
    uint8_t paused;
} fist_hud;

/* Internal owner: draw a readable shared overlay in the current softgl context. */
int fist_draw_hud(const fist_hud *hud);

#endif
