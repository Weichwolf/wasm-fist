#ifndef FIST_RENDER_RENDERER_H
#define FIST_RENDER_RENDERER_H

#include <stdint.h>

typedef struct fist_renderer fist_renderer;

/* One render-thread owner. Pixels are tightly packed RGBA8, bottom row first;
 * the borrowed view remains valid until the next draw or destruction. */
fist_renderer *fist_renderer_create(int width, int height);
void fist_renderer_destroy(fist_renderer *renderer);
const uint8_t *fist_renderer_pixels(fist_renderer *renderer);

/* Diagnostic geometry for dependency/presentation verification, not gameplay. */
int fist_renderer_draw_probe(fist_renderer *renderer);

#endif
