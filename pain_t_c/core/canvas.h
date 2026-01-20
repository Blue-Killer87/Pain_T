#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>
#include <glib.h>  

typedef struct {
    int width;
    int height;
    uint32_t *pixels;   // ARGB32
} Canvas;

Canvas* canvas_create(int width, int height);
gboolean canvas_save_png(Canvas* c, const char* path);
void canvas_destroy(Canvas* c);

uint32_t canvas_get_pixel(Canvas* c, int x, int y);
void canvas_set_pixel(Canvas* c, int x, int y, uint32_t color);

void canvas_clear(Canvas* c, uint32_t color);

#endif
