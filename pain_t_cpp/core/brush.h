#ifndef BRUSH_H
#define BRUSH_H

#include <cstdint>

#include "tool.h"

class Brush : public Tool{
public:
    Brush(uint32_t color, int size = 1);

    void press(Canvas& canvas, int x, int y) override;
    void drag(Canvas& canvas, int x, int y) override;
    void release(Canvas& canvas, int x, int y) override;

    bool usesColor() const override{return true;}
    void setColor(uint32_t c) override{color = c;}
    uint32_t getColor() const override{return color;}

    bool supportsSize() const override{return true;}
    void setSize(int s) override;
    int getSize() const override{return size;}

private:
    void drawCircle(Canvas& c, int cx, int cy, int r);
    void drawLine(Canvas& c, int x0, int y0, int x1, int y1);
    
    uint32_t color;
    int size;

    int lastX;
    int lastY;
    bool hasLast;
};

#endif
