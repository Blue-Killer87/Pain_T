#ifndef TOOL_H
#define TOOL_H

#include <gtk/gtk.h>
#include "canvas.h"
#include <stdint.h>

typedef struct Tool Tool;

typedef void (*ToolPressFn)(Tool*, Canvas*, int, int);
typedef void (*ToolDragFn)(Tool*, Canvas*, int, int);
typedef void (*ToolReleaseFn)(Tool*, Canvas*, int, int);
typedef void (*ToolSetSizeFn)(Tool*, int);
typedef int  (*ToolGetSizeFn)(Tool*);
typedef void (*ToolSetColorFn)(Tool*, uint32_t);
typedef uint32_t (*ToolGetColorFn)(Tool*);

struct Tool {
    ToolPressFn   on_press;
    ToolDragFn    on_drag;
    ToolReleaseFn on_release;

    ToolSetSizeFn set_size;
    ToolGetSizeFn get_size;

    ToolSetColorFn set_color;  
    ToolGetColorFn get_color;   

    gboolean uses_color;
    gboolean supports_size;

    void* data;
};


void tool_destroy(Tool* t);
int  tool_get_size(Tool* t);

#endif
