#include <vector>
#include <cstdint>

#include "fill.h"
#include "canvas.h"

Fill::Fill(uint32_t color) : color(color){}

void 
Fill::press(Canvas& canvas, int x, int y){
    uint32_t target = canvas.getPixel(x, y);
    floodFill(canvas, x, y, target, color);
}

void 
Fill::floodFill(Canvas& canvas, int x, int y, uint32_t target, uint32_t replacement){
    if (target == replacement) return;

    int w = canvas.getWidth();
    int h = canvas.getHeight();
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    if (canvas.getPixel(x, y) != target) return;

    struct Point {int x, y;};
    std::vector<bool> visited(w*h, false);
    std::vector<Point> stack;
    stack.push_back({x, y});
    visited[y*w + x] = true;

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!stack.empty()){
        Point p = stack.back(); stack.pop_back();
        canvas.setPixel(p.x, p.y, replacement);

        for (int i = 0; i < 4; ++i){
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
            int idx = ny*w + nx;
            if (visited[idx]) continue;
            if (canvas.getPixel(nx, ny) != target) continue;
            visited[idx] = true;
            stack.push_back({nx, ny});
        }
    }
}
