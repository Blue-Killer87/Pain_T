#include <stdlib.h>
#include <string.h>
#include <cairo.h>

#include "canvas.h"

Canvas* canvas_create(int width, int height) {
    Canvas* c = malloc(sizeof(Canvas));
    c->width = width;
    c->height = height;
    c->pixels = malloc(width * height * sizeof(uint32_t));
    return c;
}

gboolean canvas_save_png(Canvas* c, const char* path) {
    cairo_surface_t* surface =
        cairo_image_surface_create_for_data(
            (unsigned char*)c->pixels,
            CAIRO_FORMAT_ARGB32,
            c->width,
            c->height,
            c->width * 4
        );

    cairo_status_t status =
        cairo_surface_write_to_png(surface, path);

    cairo_surface_destroy(surface);

    return status == CAIRO_STATUS_SUCCESS;
}


void canvas_destroy(Canvas* c) {
    if (!c) return;
    free(c->pixels);
    free(c);
}

void canvas_clear(Canvas* c, uint32_t color) {
    for (int i = 0; i < c->width * c->height; i++)
        c->pixels[i] = color;
}

void canvas_set_pixel(Canvas* c, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= c->width || y >= c->height)
        return;
    c->pixels[y * c->width + x] = color;
}

uint32_t canvas_get_pixel(Canvas* c, int x, int y) {
    if (x < 0 || y < 0 || x >= c->width || y >= c->height)
        return 0;
    return c->pixels[y * c->width + x];
}