#include "render/hud.h"
#include "sim/vehicle_state.h"
#include "sim/weapon_control.h"

#include <GL/softgl.h>
#include <stddef.h>
#include <stdint.h>

enum {
    GLYPH_WIDTH = 5,
    GLYPH_HEIGHT = 7,
    GLYPH_ADVANCE = 6,
    DECIMAL_BASE = 10,
    DIGIT_BUFFER = 5,
    TEXT_BUFFER = 64,
    VIEWPORT_FIELDS = 4,
    MIN_WIDTH = 320,
    MIN_HEIGHT = 200,
    LARGE_WIDTH = 480,
    PANEL_WIDTH = 384,
    PANEL_HEIGHT = 82,
    MARGIN = 12,
    PADDING = 10,
    ROW_HEIGHT = 24,
    STATUS_COLUMN = 13,
    RESERVE_COLUMN = 15
};

static const GLubyte cyan[] = {103, 225, 206};
static const GLubyte amber[] = {241, 189, 83};
static const GLubyte white[] = {230, 237, 241};
static const GLubyte muted[] = {149, 169, 181};
static const GLubyte background[] = {18, 29, 38};

/* Authored 5x7 UI glyphs: digits, then uppercase letters. */
static const uint8_t glyphs[][GLYPH_HEIGHT] = {
    {14, 17, 19, 21, 25, 17, 14}, {4, 12, 4, 4, 4, 4, 14},      {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},     {2, 6, 10, 18, 31, 2, 2},     {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14}, {31, 1, 2, 4, 8, 8, 8},       {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 1, 14},   {14, 17, 17, 31, 17, 17, 17}, {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14}, {30, 17, 17, 17, 17, 17, 30}, {31, 16, 16, 30, 16, 16, 31},
    {31, 16, 16, 30, 16, 16, 16}, {14, 17, 16, 23, 17, 17, 15}, {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14},      {7, 2, 2, 2, 2, 18, 12},      {17, 18, 20, 24, 20, 18, 17},
    {16, 16, 16, 16, 16, 16, 31}, {17, 27, 21, 21, 17, 17, 17}, {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14}, {30, 17, 17, 30, 16, 16, 16}, {14, 17, 17, 17, 21, 18, 13},
    {30, 17, 17, 30, 20, 18, 17}, {15, 16, 16, 14, 1, 1, 30},   {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14}, {17, 17, 17, 17, 17, 10, 4},  {17, 17, 17, 21, 21, 21, 10},
    {17, 17, 10, 4, 10, 17, 17},  {17, 17, 10, 4, 4, 4, 4},     {31, 1, 2, 4, 8, 16, 31}};

typedef struct {
    char bytes[TEXT_BUFFER];
    size_t length;
} hud_text;

typedef struct {
    int left;
    int bottom;
    int width;
    int height;
} hud_box;

static int append(hud_text *text, const char *source) {
    for (; *source != '\0'; ++source) {
        if (text->length >= TEXT_BUFFER - 1) {
            return -1;
        }
        text->bytes[text->length++] = *source;
    }
    text->bytes[text->length] = '\0';
    return 0;
}

static int number(hud_text *text, uint16_t value) {
    char digits[DIGIT_BUFFER + 1] = {0};
    size_t index = DIGIT_BUFFER;
    do {
        digits[--index] = (char)('0' + (value % DECIMAL_BASE));
        value /= DECIMAL_BASE;
    } while (value != 0);
    return append(text, digits + index);
}

static void rectangle(const hud_box *box) {
    glVertex2i(box->left, box->bottom);
    glVertex2i(box->left + box->width, box->bottom);
    glVertex2i(box->left + box->width, box->bottom + box->height);
    glVertex2i(box->left, box->bottom + box->height);
}

static const uint8_t *glyph(char letter) {
    if (letter >= '0' && letter <= '9') {
        return glyphs[(size_t)(letter - '0')];
    }
    if (letter >= 'A' && letter <= 'Z') {
        return glyphs[DECIMAL_BASE + (size_t)(letter - 'A')];
    }
    static const uint8_t dash[GLYPH_HEIGHT] = {0, 0, 0, 31, 0, 0, 0};
    return letter == '-' ? dash : NULL;
}

typedef struct {
    int left;
    int top;
    int scale;
} text_position;

static void draw_text(const char *text, text_position position) {
    for (; *text != '\0'; ++text) {
        const uint8_t *shape = glyph(*text);
        if (shape != NULL) {
            for (unsigned row = 0; row < GLYPH_HEIGHT; ++row) {
                for (unsigned column = 0; column < GLYPH_WIDTH; ++column) {
                    if ((shape[row] & (1U << (GLYPH_WIDTH - column - 1))) != 0) {
                        const hud_box pixel = {position.left + ((int)column * position.scale),
                                               position.top - (((int)row + 1) * position.scale),
                                               position.scale, position.scale};
                        rectangle(&pixel);
                    }
                }
            }
        }
        position.left += GLYPH_ADVANCE * position.scale;
    }
}

static int content(const fist_hud *hud, const hud_box *panel, int scale) {
    hud_text title = {0};
    hud_text ammo = {0};
    hud_text reserve = {0};
    hud_text controls = {0};
    if (append(&title, "WEAPON ") != 0 ||
        number(&title, (uint16_t)((hud->weapon.selected / 2) + 1)) != 0 ||
        append(&ammo, "AMMO ") != 0 || number(&ammo, hud->weapon.ammunition) != 0 ||
        append(&reserve, "RESERVE ") != 0 || number(&reserve, hud->weapon.reserve) != 0 ||
        append(&controls, "1-") != 0 || number(&controls, hud->weapon.station_count) != 0 ||
        append(&controls, " SELECT   TAB NEXT") != 0) {
        return -1;
    }
    const text_position first = {panel->left + PADDING, panel->bottom + panel->height - PADDING,
                                 scale};
    glColor3ubv(cyan);
    draw_text(title.bytes, first);
    const text_position status = {first.left + (STATUS_COLUMN * GLYPH_ADVANCE * scale), first.top,
                                  scale};
    glColor3ubv(amber);
    const char *label = hud->weapon.ammunition == 0 ? "EMPTY" : "SELECTED";
    if (hud->weapon.ammunition != 0 && hud->weapon.continuous == 0 && hud->weapon.countdown != 0) {
        label = "RELOADING";
    }
    draw_text(hud->paused != 0 ? "PAUSED" : label, status);
    const text_position second = {first.left, first.top - ROW_HEIGHT, scale};
    glColor3ubv(white);
    draw_text(ammo.bytes, second);
    if (hud->weapon.has_reserve != 0) {
        const text_position stock = {first.left + (RESERVE_COLUMN * GLYPH_ADVANCE * scale),
                                     second.top, scale};
        draw_text(reserve.bytes, stock);
    }
    glColor3ubv(muted);
    draw_text(controls.bytes, (text_position){first.left, second.top - ROW_HEIGHT, scale});
    return 0;
}

int fist_draw_hud(const fist_hud *hud) {
    GLint viewport[VIEWPORT_FIELDS] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (hud == NULL || viewport[2] < MIN_WIDTH || viewport[3] < MIN_HEIGHT || hud->paused > 1 ||
        hud->weapon.station_count < FIST_VEHICLE_WEAPON_SLOTS ||
        hud->weapon.station_count > FIST_WEAPON_MAX_STATIONS || hud->weapon.selected % 2 != 0 ||
        hud->weapon.selected / 2 >= hud->weapon.station_count) {
        return -1;
    }
    const int scale = viewport[2] >= LARGE_WIDTH ? 2 : 1;
    const int available_width = viewport[2] - (MARGIN * 2);
    const int width = available_width < PANEL_WIDTH ? available_width : PANEL_WIDTH;
    const hud_box panel = {MARGIN, MARGIN, width, PANEL_HEIGHT};
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
    glColor3ubv(background);
    rectangle(&panel);
    glColor3ubv(cyan);
    const hud_box border = {panel.left, panel.bottom + panel.height - 2, panel.width, 2};
    rectangle(&border);
    const int result = content(hud, &panel, scale);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    return result == 0 && glGetError() == GL_NO_ERROR ? 0 : -1;
}
