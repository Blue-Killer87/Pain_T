#ifndef TOOL_H
#define TOOL_H

#include <cstdint>
#include <cairo.h>

#include "canvas.h"

class Tool{
public:
    virtual ~Tool() = default;

    virtual void press(Canvas& canvas, int x, int y) = 0;
    virtual void drag(Canvas& canvas, int x, int y){(void)canvas; (void)x; (void)y;}
    virtual void release(Canvas& canvas, int x, int y){(void)canvas; (void)x; (void)y;}

    virtual bool usesColor() const{return false;}
    virtual void setColor(uint32_t c){(void)c;}
    virtual uint32_t getColor() const{return 0xFF000000;}
    virtual void drawOverlay(cairo_t*){}
    virtual void apply(Canvas& canvas){(void)canvas;}

    virtual bool supportsSize() const{return false;}
    virtual void setSize(int s){(void)s;}
    virtual int getSize() const{return 1;}
};

#endif
