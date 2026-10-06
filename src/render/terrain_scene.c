#include "render/terrain_scene.h"
#include "assets/klc.h"
#include "assets/palette.h"
#include "assets/terrain.h"
#include "assets/units.h"
#include "render/model_bitmap.h"
#include "render/renderer.h"
#include "sim/world.h"

#include <GL/softgl.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    /* 11bb/11be shift integer map coordinates by 13 before the 32-bit sampler.
     * The repeated domain is therefore 2^(32-13). Render coordinates divide
     * original positions/altitudes by 256; one decoded height byte is one unit. */
    WORLD_SIDE = FIST_MAP_PERIOD / FIST_POSITION_SCALE,
    RGBA_CHANNELS = 4,
    OPAQUE_ALPHA = 255,
    TRIANGLE_INDICES_PER_CELL = 6,
    MATRIX_ELEMENTS = 16,
    TRANSLATION_X = 12,
    TRANSLATION_Y = 13,
    TRANSLATION_Z = 14,
    HOMOGENEOUS_CORNER = 15,
    NORMAL_SAMPLE_SPAN = 2,
    INSPECTION_ALTITUDE = 64
};

typedef struct {
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat texture[2];
} terrain_vertex;

typedef struct {
    terrain_vertex *vertices;
    GLuint *indices;
    GLsizei index_count;
    uint8_t *texture;
} terrain_mesh;

float fist_map_coordinate(int32_t position) {
    return (float)((uint32_t)position % FIST_MAP_PERIOD) / FIST_POSITION_SCALE;
}

float fist_map_y_coordinate(int32_t position) {
    return fist_map_coordinate(-(int32_t)((uint32_t)position % FIST_MAP_PERIOD));
}

int32_t fist_map_delta(int32_t subject, int32_t observer) {
    const int64_t half = FIST_MAP_PERIOD / 2;
    int64_t delta = ((int64_t)subject - observer + half) % FIST_MAP_PERIOD;
    if (delta < 0) {
        delta += FIST_MAP_PERIOD;
    }
    return (int32_t)(delta - half);
}

static float height_at(const fist_klc_image *image, int32_t column, int32_t row) {
    const size_t mask = image->width - 1;
    const size_t offset =
        (((size_t)(uint32_t)row & mask) * image->width) + ((size_t)(uint32_t)column & mask);
    return image->pixels[offset];
}

float fist_terrain_surface(const fist_terrain *terrain, int32_t map_x, int32_t map_y) {
    const float step = (float)WORLD_SIDE / (float)terrain->heightmap.width;
    const float local_x = fist_map_coordinate(map_x) / step;
    const float local_z = fist_map_y_coordinate(map_y) / step;
    const int32_t column = (int32_t)floorf(local_x);
    const int32_t row = (int32_t)floorf(local_z);
    const float fraction_x = local_x - (float)column;
    const float fraction_z = local_z - (float)row;
    const float top_left = height_at(&terrain->heightmap, column, row);
    const float top_right = height_at(&terrain->heightmap, column + 1, row);
    const float bottom_left = height_at(&terrain->heightmap, column, row + 1);
    const float bottom_right = height_at(&terrain->heightmap, column + 1, row + 1);
    if (fraction_x + fraction_z <= 1.0F) {
        return top_left + (fraction_x * (top_right - top_left)) +
               (fraction_z * (bottom_left - top_left));
    }
    return bottom_right + ((1.0F - fraction_x) * (bottom_left - bottom_right)) +
           ((1.0F - fraction_z) * (top_right - bottom_right));
}

static uint8_t display_component(uint8_t component) {
    return (uint8_t)((((unsigned)component * OPAQUE_ALPHA) + (FIST_PALETTE_DAC_MAX / 2)) /
                     FIST_PALETTE_DAC_MAX);
}

static void sky_color(const fist_terrain *terrain, GLfloat *color) {
    const size_t count = (size_t)terrain->sky.width * terrain->sky.height;
    uint64_t sum[FIST_PALETTE_CHANNELS] = {0};
    for (size_t index = 0; index < count; ++index) {
        const size_t mapped = terrain->sky_map.indices[terrain->sky.pixels[index]];
        for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
            sum[channel] += terrain->palette.rgb6[(mapped * FIST_PALETTE_CHANNELS) + channel];
        }
    }
    for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
        color[channel] = (float)sum[channel] / ((float)count * FIST_PALETTE_DAC_MAX);
    }
    color[RGBA_CHANNELS - 1] = 1.0F;
}

static void destroy_mesh(terrain_mesh *mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh->texture);
    *mesh = (terrain_mesh){0};
}

static int build_texture(const fist_terrain *terrain, terrain_mesh *mesh) {
    const size_t side = terrain->colormap.width;
    if (side == 0 || side > INT_MAX || side > SIZE_MAX / side / RGBA_CHANNELS) {
        return -1;
    }
    const size_t count = side * side;
    mesh->texture = malloc(count * RGBA_CHANNELS);
    if (mesh->texture == NULL) {
        return -1;
    }
    for (size_t index = 0; index < count; ++index) {
        const size_t mapped = terrain->colormap_map.indices[terrain->colormap.pixels[index]];
        for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
            mesh->texture[(index * RGBA_CHANNELS) + channel] = display_component(
                terrain->palette.rgb6[(mapped * FIST_PALETTE_CHANNELS) + channel]);
        }
        mesh->texture[(index * RGBA_CHANNELS) + RGBA_CHANNELS - 1] = OPAQUE_ALPHA;
    }
    return 0;
}

static int build_mesh(const fist_terrain *terrain, const fist_terrain_view *view,
                      terrain_mesh *mesh) {
    const size_t side = terrain->heightmap.width;
    if (side == 0 || side >= INT_MAX || side > (size_t)INT_MAX / side / TRIANGLE_INDICES_PER_CELL) {
        return -1;
    }
    const size_t stride = side + 1;
    if (stride > SIZE_MAX / stride / sizeof(terrain_vertex)) {
        return -1;
    }
    const size_t count = side * side * TRIANGLE_INDICES_PER_CELL;
    if (count > SIZE_MAX / sizeof(GLuint)) {
        return -1;
    }
    mesh->vertices = malloc(stride * stride * sizeof(terrain_vertex));
    mesh->indices = malloc(count * sizeof(GLuint));
    mesh->index_count = (GLsizei)count;
    if (mesh->vertices == NULL || mesh->indices == NULL) {
        return -1;
    }
    const float step = (float)WORLD_SIDE / (float)side;
    const int32_t base_x =
        (int32_t)floorf(fist_map_coordinate(view->map_x) / step) - (int32_t)(side / 2);
    const int32_t base_z =
        (int32_t)floorf(fist_map_y_coordinate(view->map_y) / step) - (int32_t)(side / 2);
    for (size_t row = 0; row < stride; ++row) {
        for (size_t column = 0; column < stride; ++column) {
            const int32_t sample_x = base_x + (int32_t)column;
            const int32_t sample_z = base_z + (int32_t)row;
            terrain_vertex *vertex = mesh->vertices + (row * stride) + column;
            vertex->position[0] = (float)sample_x * step;
            vertex->position[1] = height_at(&terrain->heightmap, sample_x, sample_z);
            vertex->position[2] = (float)sample_z * step;
            vertex->normal[0] = height_at(&terrain->heightmap, sample_x - 1, sample_z) -
                                height_at(&terrain->heightmap, sample_x + 1, sample_z);
            vertex->normal[1] = step * NORMAL_SAMPLE_SPAN;
            vertex->normal[2] = height_at(&terrain->heightmap, sample_x, sample_z - 1) -
                                height_at(&terrain->heightmap, sample_x, sample_z + 1);
            vertex->texture[0] = vertex->position[0] / WORLD_SIDE;
            vertex->texture[1] = vertex->position[2] / WORLD_SIDE;
        }
    }
    size_t offset = 0;
    for (size_t row = 0; row < side; ++row) {
        for (size_t column = 0; column < side; ++column) {
            const GLuint top_left = (GLuint)((row * stride) + column);
            const GLuint bottom_left = (GLuint)(((row + 1) * stride) + column);
            mesh->indices[offset++] = top_left;
            mesh->indices[offset++] = bottom_left;
            mesh->indices[offset++] = top_left + 1;
            mesh->indices[offset++] = top_left + 1;
            mesh->indices[offset++] = bottom_left;
            mesh->indices[offset++] = bottom_left + 1;
        }
    }
    return build_texture(terrain, mesh);
}

static void camera_matrix(const fist_terrain_view *view, GLfloat *matrix) {
    static const float full_turn = 6.2831853071795864769F;
    const float angle = (float)view->heading * full_turn / FIST_TURN_SIZE;
    const float sine = sinf(angle);
    const float cosine = cosf(angle);
    const float pitch_sine = sinf(view->pitch);
    const float pitch_cosine = cosf(view->pitch);
    const float right[] = {cosine, 0.0F, sine};
    const float up[] = {-sine * pitch_sine, pitch_cosine, cosine * pitch_sine};
    const float forward[] = {sine * pitch_cosine, pitch_sine, -cosine * pitch_cosine};
    const float position[] = {fist_map_coordinate(view->map_x), view->altitude,
                              fist_map_y_coordinate(view->map_y)};
    for (size_t axis = 0; axis < 3; ++axis) {
        matrix[axis * RGBA_CHANNELS] = right[axis];
        matrix[(axis * RGBA_CHANNELS) + 1] = up[axis];
        matrix[(axis * RGBA_CHANNELS) + 2] = -forward[axis];
        matrix[(axis * RGBA_CHANNELS) + 3] = 0.0F;
        matrix[TRANSLATION_X] -= right[axis] * position[axis];
        matrix[TRANSLATION_Y] -= up[axis] * position[axis];
        matrix[TRANSLATION_Z] += forward[axis] * position[axis];
    }
    matrix[HOMOGENEOUS_CORNER] = 1.0F;
}

static uint8_t *vehicle_texture(const fist_model_bitmap *bitmap) {
    const size_t count = (size_t)bitmap->width * bitmap->height;
    uint8_t *rgba = malloc(count * RGBA_CHANNELS);
    if (rgba == NULL) {
        return NULL;
    }
    for (size_t index = 0; index < count; ++index) {
        const size_t palette_index = bitmap->indices[index];
        for (size_t channel = 0; channel < FIST_PALETTE_CHANNELS; ++channel) {
            rgba[(index * RGBA_CHANNELS) + channel] = display_component(
                bitmap->palette.rgb6[(palette_index * FIST_PALETTE_CHANNELS) + channel]);
        }
        rgba[(index * RGBA_CHANNELS) + RGBA_CHANNELS - 1] = palette_index == 0 ? 0 : OPAQUE_ALPHA;
    }
    return rgba;
}

static void vehicle_vertices(const fist_terrain_view *view, const fist_scene_vehicle *vehicle) {
    static const GLfloat corners[4][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    GLfloat matrix[MATRIX_ELEMENTS] = {0};
    camera_matrix(view, matrix);
    const float center[] = {
        fist_map_coordinate(view->map_x) +
            ((float)fist_map_delta(vehicle->map_x, view->map_x) / FIST_POSITION_SCALE),
        vehicle->altitude,
        fist_map_y_coordinate(view->map_y) -
            ((float)fist_map_delta(vehicle->map_y, view->map_y) / FIST_POSITION_SCALE)};
    glBegin(GL_QUADS);
    for (size_t corner = 0; corner < RGBA_CHANNELS; ++corner) {
        const GLfloat *texture = corners[corner];
        const float local_x =
            ((float)vehicle->bitmap.left + (texture[0] * (float)vehicle->bitmap.width)) *
            vehicle->texel_width;
        const float local_y =
            ((float)vehicle->bitmap.bottom + (texture[1] * (float)vehicle->bitmap.height)) *
            vehicle->texel_height;
        GLfloat position[3] = {0};
        for (size_t axis = 0; axis < 3; ++axis) {
            position[axis] = center[axis] + (matrix[axis * RGBA_CHANNELS] * local_x) +
                             (matrix[(axis * RGBA_CHANNELS) + 1] * local_y);
        }
        glTexCoord2fv(texture);
        glVertex3fv(position);
    }
    glEnd();
}

static int draw_vehicle(const fist_terrain_view *view, const fist_scene_vehicle *vehicle) {
    uint8_t *rgba = vehicle_texture(&vehicle->bitmap);
    if (rgba == NULL) {
        return -1;
    }
    /* Reuse depth, fog and camera state. A zero texel neither covers terrain
     * nor occludes later objects. Nearest filtering preserves authored edges. */
    glDisable(GL_LIGHTING);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vehicle->bitmap.width, vehicle->bitmap.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glColor4f(1, 1, 1, 1);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);
    vehicle_vertices(view, vehicle);
    const GLenum error = glGetError();
    glDisable(GL_ALPHA_TEST);
    glDeleteTextures(1, &texture);
    free(rgba);
    return error == GL_NO_ERROR ? 0 : -1;
}

/* Context selection is handled by renderer.c; this internal scene owner uses
 * the current softgl context and returns only after queued workers complete. */
int fist_draw_terrain_scene(fist_renderer *renderer, const fist_terrain *terrain,
                            const fist_terrain_view *view, const fist_scene_vehicle *vehicle) {
    if (terrain == NULL || view == NULL || terrain->heightmap.pixels == NULL ||
        terrain->colormap.pixels == NULL || terrain->sky.pixels == NULL) {
        return -1;
    }
    terrain_mesh mesh = {0};
    if (build_mesh(terrain, view, &mesh) != 0) {
        destroy_mesh(&mesh);
        return -1;
    }
    GLfloat background[RGBA_CHANNELS] = {0};
    sky_color(terrain, background);
    static const GLfloat ambient[] = {0.35F, 0.35F, 0.35F, 1.0F};
    static const GLfloat diffuse[] = {0.75F, 0.75F, 0.75F, 1.0F};
    static const GLfloat white[] = {1.0F, 1.0F, 1.0F, 1.0F};
    static const GLfloat sunlight[] = {-0.5F, 1.0F, 0.3F, 0.0F};
    static const GLfloat fog_start = 384.0F;
    static const GLfloat far_distance = 768.0F;
    static const GLfloat frustum_height = 0.57735026919F; /* 60 degree vertical field of view. */
    GLint viewport[RGBA_CHANNELS] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const double aspect = (double)viewport[2] / viewport[3];
    glClearColor(background[0], background[1], background[2], background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-(double)frustum_height * aspect, (double)frustum_height * aspect, -frustum_height,
              frustum_height, 1.0, far_distance);
    GLfloat matrix[MATRIX_ELEMENTS] = {0};
    camera_matrix(view, matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(matrix);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, sunlight);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, background);
    glFogf(GL_FOG_START, fog_start);
    glFogf(GL_FOG_END, far_distance);
    glEnable(GL_FOG);
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)terrain->colormap.width,
                 (GLsizei)terrain->colormap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mesh.texture);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(terrain_vertex), mesh.vertices[0].position);
    glNormalPointer(GL_FLOAT, sizeof(terrain_vertex), mesh.vertices[0].normal);
    glTexCoordPointer(2, GL_FLOAT, sizeof(terrain_vertex), mesh.vertices[0].texture);
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, mesh.indices);
    const int vehicle_result = vehicle == NULL ? 0 : draw_vehicle(view, vehicle);
    const uint8_t *frame = fist_renderer_pixels(renderer);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
    glDisable(GL_FOG);
    glDisable(GL_DEPTH_TEST);
    glDeleteTextures(1, &texture);
    destroy_mesh(&mesh);
    return vehicle_result == 0 && frame != NULL && glGetError() == GL_NO_ERROR ? 0 : -1;
}

int fist_terrain_inspection_view(const fist_terrain *terrain, const fist_unit_definition *vehicle,
                                 fist_terrain_view *out) {
    if (terrain == NULL || vehicle == NULL || out == NULL || terrain->heightmap.pixels == NULL ||
        terrain->heightmap.width == 0 || vehicle->type >= FIST_UNIT_GROUND_VEHICLE_COUNT) {
        return -1;
    }
    static const float downward_pitch = -0.25F;
    const float step = (float)WORLD_SIDE / (float)terrain->heightmap.width;
    const int32_t map_x = vehicle->map_x;
    const int32_t map_y = vehicle->map_y;
    const int32_t column = (int32_t)floorf(fist_map_coordinate(map_x) / step);
    const int32_t row = (int32_t)floorf(fist_map_y_coordinate(map_y) / step);
    *out = (fist_terrain_view){map_x, map_y,
                               height_at(&terrain->heightmap, column, row) + INSPECTION_ALTITUDE,
                               vehicle->heading, downward_pitch};
    return 0;
}
