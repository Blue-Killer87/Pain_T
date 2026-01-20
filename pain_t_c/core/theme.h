#ifndef THEME_H
#define THEME_H

#include <stdint.h>

typedef struct {
    uint32_t background;
    uint32_t foreground;
} Theme;

/* Declarations only */
extern Theme THEME_LIGHT;
extern Theme THEME_DARK;

#endif
