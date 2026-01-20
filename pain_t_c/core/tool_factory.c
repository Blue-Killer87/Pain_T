#include <stddef.h>
#include "tool_factory.h"
#include "brush.h"
#include "fill.h"

Tool* tool_create(ToolID id, Theme* theme) {
    switch (id) {
        case TOOL_BRUSH:
            return brush_create(theme->foreground, 4);

        case TOOL_ERASER:
            return brush_create(theme->background, 8);

        case TOOL_FILL:
            return fill_create(theme->foreground);

        default:
            return NULL;
    }
}
