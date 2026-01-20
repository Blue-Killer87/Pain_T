#include <cairo.h>
#include <algorithm>
#include <cmath>
#include <gdk/gdk.h>
#include <gdk/gdkcairo.h>

#include "image_tool.h"
#include "canvas.h"

ImageTool::ImageTool(GdkPixbuf* pix)
    : pixbuf(pix)
    {
    x = 50;
    y = 50;
    w = gdk_pixbuf_get_width(pixbuf);
    h = gdk_pixbuf_get_height(pixbuf);
}

ImageTool::~ImageTool(){
    if (pixbuf)
        g_object_unref(pixbuf);
}

void 
ImageTool::press(Canvas&, int mx, int my){
    dragMode = hitTest(mx, my);
    lastX = mx;
    lastY = my;
}

void 
ImageTool::drag(Canvas&, int mx, int my){
    int dx = mx - lastX;
    int dy = my - lastY;

    switch (dragMode){
        case DragMode::Move:
            x += dx;
            y += dy;
            break;

        case DragMode::ResizeBR:
            w += dx;
            h += dy;
            break;

        case DragMode::ResizeBL:
            x += dx;
            w -= dx;
            h += dy;
            break;

        case DragMode::ResizeTR:
            y += dy;
            h -= dy;
            w += dx;
            break;

        case DragMode::ResizeTL:
            x += dx;
            y += dy;
            w -= dx;
            h -= dy;
            break;

        default:
            break;
    }

    w = std::max(w, 10);
    h = std::max(h, 10);

    lastX = mx;
    lastY = my;
}

void 
ImageTool::release(Canvas&, int, int){
    dragMode = DragMode::None;
}

ImageTool::DragMode ImageTool::hitTest(int mx, int my) const{
    if (std::abs(mx - x) <= HANDLE && std::abs(my - y) <= HANDLE)
        return DragMode::ResizeTL;

    if (std::abs(mx - (x + w)) <= HANDLE && std::abs(my - y) <= HANDLE)
        return DragMode::ResizeTR;

    if (std::abs(mx - x) <= HANDLE && std::abs(my - (y + h)) <= HANDLE)
        return DragMode::ResizeBL;

    if (std::abs(mx - (x + w)) <= HANDLE && std::abs(my - (y + h)) <= HANDLE)
        return DragMode::ResizeBR;

    if (mx >= x && mx <= x + w && my >= y && my <= y + h)
        return DragMode::Move;

    return DragMode::None;
}

void 
ImageTool::drawOverlay(cairo_t* cr){
    if (!pixbuf) return;

    //draw the image scaled to current w/h
    gdk_cairo_set_source_pixbuf(
        cr,
        gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_BILINEAR),
        x,
        y
    );
    cairo_paint(cr);

    //bounding box
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_set_line_width(cr, 2.0);
    cairo_rectangle(cr, x, y, w, h);
    cairo_stroke(cr);

    //handles
    auto handle = [&](int hx, int hy){
        cairo_rectangle(cr, hx - HANDLE/2, hy - HANDLE/2, HANDLE, HANDLE);
        cairo_fill(cr);
    };

    handle(x, y);
    handle(x + w, y);
    handle(x, y + h);
    handle(x + w, y + h);
}

void 
ImageTool::apply(Canvas& canvas){
    if (!pixbuf) return;

    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_BILINEAR);
    int imgW = gdk_pixbuf_get_width(scaled);
    int imgH = gdk_pixbuf_get_height(scaled);
    int nChannels = gdk_pixbuf_get_n_channels(scaled);
    int rowStride = gdk_pixbuf_get_rowstride(scaled);
    guchar* pixData = gdk_pixbuf_get_pixels(scaled);

    std::vector<uint32_t> block(imgW * imgH, 0xFFFFFFFF);

    for (int iy = 0; iy < imgH; ++iy){
        for (int ix = 0; ix < imgW; ++ix) {
            guchar* p = pixData + iy * rowStride + ix * nChannels;
            uint8_t r = p[0];
            uint8_t g = p[1];
            uint8_t b = p[2];
            uint8_t a = (nChannels == 4) ? p[3] : 255;
            block[iy * imgW + ix] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    canvas.setPixelsBlock(x, y, imgW, imgH, block);

    g_object_unref(scaled); //cleanup
}


