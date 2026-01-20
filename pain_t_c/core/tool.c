#include "tool.h"
#include <stdlib.h>

void tool_destroy(Tool* t) {
    if (!t) return;
    free(t->data);
    free(t);
}

int tool_get_size(Tool* t) {
    if (!t || !t->supports_size)
        return 1;

    if (t->get_size)
        return t->get_size(t);

    return 1;
}
