#ifndef THEME_H
#define THEME_H

#include <cstdint>

struct Theme{
    uint32_t background;
    uint32_t foreground;
};

inline Theme THEME_LIGHT = {0xFFFFFFFF, 0xFF000000};
inline Theme THEME_DARK  = {0xFF000000, 0xFFFFFFFF};

#endif
