#ifndef HISTORY_H
#define HISTORY_H

#include "canvas.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t *data;   // RLE: [pixel, run, pixel, run, ...]
    size_t length;    // number of uint32_t entries
} Snapshot;

void history_init(void);
void history_free(void);

void history_push(Canvas *c);
int  history_undo(Canvas *c);
int  history_redo(Canvas *c);

bool history_can_undo(void);
bool history_can_redo(void);

#endif
