#include <cairo.h>
#include <algorithm>

#include "canvas.h"

Canvas::Canvas(int w, int h) : width(w), height(h), pixels(w * h, 0xFFFFFFFF){}

uint32_t 
Canvas::getPixel(int x, int y) const{
    if (x < 0 || y < 0 || x >= width || y >= height)
        return 0;
    return pixels[y * width + x];
}

void 
Canvas::setPixel(int x, int y, uint32_t color){
    if (x < 0 || y < 0 || x >= width || y >= height)
        return;
    pixels[y * width + x] = color;
}

void 
Canvas::clear(uint32_t color){
    std::fill(pixels.begin(), pixels.end(), color);
}

bool 
Canvas::savePNG(const std::string& path) const{
    cairo_surface_t* surface = cairo_image_surface_create_for_data(
        reinterpret_cast<unsigned char*>(const_cast<uint32_t*>(pixels.data())), 
        CAIRO_FORMAT_ARGB32,
        width,
        height,
        width * 4
    );

    cairo_status_t status = cairo_surface_write_to_png(surface, path.c_str());
    cairo_surface_destroy(surface);

    return status == CAIRO_STATUS_SUCCESS;
}

