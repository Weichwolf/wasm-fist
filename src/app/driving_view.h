#ifndef FIST_APP_DRIVING_VIEW_H
#define FIST_APP_DRIVING_VIEW_H
#include "app/driving.h"
#include "render/renderer.h"
/* Draw current state through the common follow camera and sprite compositor.
 * Temporary bitmaps are released after synchronous rendering. */
int fist_driving_draw(const fist_driving *driving, fist_renderer *renderer);
#endif
