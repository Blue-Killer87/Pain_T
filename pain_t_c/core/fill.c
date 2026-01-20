#include "fill.h"
#include "canvas.h"
#include <stdlib.h>


typedef struct {
    int x, y;
} Point;

typedef struct {
    uint32_t color;
} Fill;


static void fill_set_color(Tool* t, uint32_t color) {
    Fill* f = t->data;
    f->color = color;
}

static uint32_t fill_get_color(Tool* t) {
    Fill* f = t->data;
    return f->color;
}


static void flood_fill(Canvas* c, int x, int y,
                       uint32_t target, uint32_t replacement)
{
    if (target == replacement)
        return;

    int w = c->width;
    int h = c->height;

    if (x < 0 || y < 0 || x >= w || y >= h)
        return;

    if (canvas_get_pixel(c, x, y) != target)
        return;

    typedef struct { int x, y; } Point;

    /* visited bitmap: 1 byte per pixel */
    uint8_t* visited = calloc(w * h, 1);
    Point* stack = malloc(sizeof(Point) * w * h);

    int sp = 0;
    stack[sp++] = (Point){ x, y };
    visited[y * w + x] = 1;

    while (sp > 0) {
        Point p = stack[--sp];

        canvas_set_pixel(c, p.x, p.y, replacement);

        const int dx[4] = { 1, -1, 0, 0 };
        const int dy[4] = { 0, 0, 1, -1 };

        for (int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            if (nx < 0 || ny < 0 || nx >= w || ny >= h)
                continue;

            int idx = ny * w + nx;

            if (visited[idx])
                continue;

            if (canvas_get_pixel(c, nx, ny) != target)
                continue;

            visited[idx] = 1;
            stack[sp++] = (Point){ nx, ny };
        }
    }

    free(stack);
    free(visited);
}



static void fill_press(Tool* t, Canvas* c, int x, int y) {
    uint32_t target = canvas_get_pixel(c, x, y);
    Fill* f = (Fill*)t->data;
    flood_fill(c, x, y, target, f->color);

}

static void fill_drag(Tool* t, Canvas* c, int x, int y) {
    (void)t; (void)c; (void)x; (void)y;
}


static void fill_release(Tool* t, Canvas* c, int x, int y) {
    (void)t; (void)c; (void)x; (void)y;
}

Tool* fill_create(uint32_t color) {
    Tool* t = malloc(sizeof(Tool));
    Fill* f = malloc(sizeof(Fill));

    f->color = color;

    t->data = f;

    t->on_press   = fill_press;
    t->on_drag    = fill_drag;
    t->on_release = fill_release;

    t->uses_color    = TRUE;
    t->supports_size = FALSE;

    t->set_size  = NULL;
    t->get_size  = NULL;

    t->set_color = fill_set_color; 
    t->get_color = fill_get_color; 

    return t;
}

