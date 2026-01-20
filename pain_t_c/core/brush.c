#include "tool.h"
#include <stdlib.h>

/* ---- Forward declarations ---- */
static void draw_circle(Canvas* c, int cx, int cy, int r, uint32_t color);
static void draw_line(Canvas* c,
                      int x0, int y0,
                      int x1, int y1,
                      int r, uint32_t color);

typedef struct {
    int last_x;
    int last_y;
    int has_last;
} BrushState;

typedef struct {
    BrushState state;
    int size;
    uint32_t color;
} Brush;


void brush_set_size(Tool* t, int size) {
    Brush* b = (Brush*)t->data;
    if (size < 1) size = 1;
    if (size > 100) size = 100;
    b->size = size;
}

static int brush_get_size(Tool* t) {
    Brush* b = t->data;
    return b->size;
}

static void brush_set_color(Tool* t, uint32_t color) {
    Brush* b = t->data;
    b->color = color;
}

static uint32_t brush_get_color(Tool* t) {
    Brush* b = t->data;
    return b->color;
}

static void brush_press(Tool* t, Canvas* c, int x, int y) {
    Brush* b = (Brush*)t->data;

    draw_circle(c, x, y, b->size, b->color);

    b->state.last_x = x;
    b->state.last_y = y;
    b->state.has_last = 1;
}


static void brush_drag(Tool* t, Canvas* c, int x, int y) {
    Brush* b = (Brush*)t->data;

    if (!b->state.has_last) {
        brush_press(t, c, x, y);
        return;
    }

    draw_line(c,
              b->state.last_x, b->state.last_y,
              x, y,
              b->size,
              b->color);

    b->state.last_x = x;
    b->state.last_y = y;
}


static void brush_release(Tool* t, Canvas* c, int x, int y) {
    (void)c; (void)x; (void)y;

    Brush* b = (Brush*)t->data;
    b->state.has_last = 0;
}


static void draw_circle(Canvas* c, int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r)
                canvas_set_pixel(c, cx + x, cy + y, color);
        }
    }
}

static void draw_line(Canvas* c,
                      int x0, int y0,
                      int x1, int y1,
                      int r, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_circle(c, x0, y0, r, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}



Tool* brush_create(uint32_t color, int size) {
    Tool* t = malloc(sizeof(Tool));
    Brush* b = malloc(sizeof(Brush));

    b->state.has_last = 0;
    b->size = size;
    b->color = color;

    t->on_press = brush_press;
    t->on_drag = brush_drag;
    t->on_release = brush_release;

    t->uses_color = TRUE;
    t->supports_size = TRUE;
    t->set_size = brush_set_size;
    t->get_size = brush_get_size;
    t->set_color = brush_set_color;
    t->get_color = brush_get_color;


    t->data = b;

    return t;
}

