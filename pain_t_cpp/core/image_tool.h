#ifndef IMAGE_TOOL_H
#define IMAGE_TOOL_H

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>

#include "tool.h"

class ImageTool : public Tool{
public:
    explicit ImageTool(GdkPixbuf* pix);
    ~ImageTool() override;

    void press(Canvas& canvas, int x, int y) override;
    void drag(Canvas& canvas, int x, int y) override;
    void release(Canvas& canvas, int x, int y) override;

    void drawOverlay(cairo_t* cr) override;
    void apply(Canvas& canvas) override;

private:
    enum class DragMode{
        None,
        Move,
        ResizeTL,
        ResizeTR,
        ResizeBL,
        ResizeBR
    };

    DragMode hitTest(int mx, int my) const;

    GdkPixbuf* pixbuf = nullptr;

    int x = 0, y = 0;
    int w = 0, h = 0;

    int lastX = 0, lastY = 0;
    DragMode dragMode = DragMode::None;

    static constexpr int HANDLE = 8;
};

#endif
