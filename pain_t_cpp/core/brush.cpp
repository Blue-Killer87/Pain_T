#include <cmath>
#include <algorithm>

#include "brush.h"
#include "canvas.h"

Brush::Brush(uint32_t color, int size)
    : color(color), size(size), lastX(0), lastY(0), hasLast(false)
{
    setSize(size);
}

void 
Brush::setSize(int s){
    size = std::clamp(s, 1, 100);
}

void 
Brush::press(Canvas& c, int x, int y){
    drawCircle(c, x, y, size);
    lastX = x;
    lastY = y;
    hasLast = true;
}

void 
Brush::drag(Canvas& c, int x, int y){
    if (!hasLast) {
        press(c, x, y);
        return;
    }
    drawLine(c, lastX, lastY, x, y);
    lastX = x;
    lastY = y;
}

void 
Brush::release(Canvas& c, int x, int y){
    (void)c; (void)x; (void)y;
    hasLast = false;
}

void 
Brush::drawCircle(Canvas& c, int cx, int cy, int r){
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r)
                c.setPixel(cx + x, cy + y, color);
        }
    }
}

void 
Brush::drawLine(Canvas& c, int x0, int y0, int x1, int y1){
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        drawCircle(c, x0, y0, size);
        if (x0==x1 && y0==y1) break;

        int e2 = 2 * err;
        if (e2>-dy) {err-=dy; x0+=sx;}
        if (e2<dx) {err+=dx; y0+=sy;}
    }
}
