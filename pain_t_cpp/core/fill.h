#ifndef FILL_H
#define FILL_H

#include <cstdint>

#include "tool.h"

class Fill : public Tool{
public:
    explicit Fill(uint32_t color);
    void press(Canvas& canvas, int x, int y) override;
    bool usesColor() const override {return true;}
    void setColor(uint32_t c) override {color = c;}
    uint32_t getColor() const override {return color;}

private:
    void floodFill(Canvas& canvas, int x, int y, uint32_t target, uint32_t replacement);
    uint32_t color;
};

#endif
