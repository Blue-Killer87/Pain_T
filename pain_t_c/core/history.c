#include "history.h"
#include <stdlib.h>
#include <string.h>

#define HISTORY_MAX 64

static Snapshot undo_stack[HISTORY_MAX];
static Snapshot redo_stack[HISTORY_MAX];
static int undo_top = 0;
static int redo_top = 0;

void history_init(void) {
    undo_top = 0;
    redo_top = 0;
}

/* ---------- Snapshot helpers ---------- */

static Snapshot snapshot_create(Canvas *c) {
    Snapshot s;
    size_t count = (size_t)c->width * c->height;

    s.data = malloc(sizeof(uint32_t) * count * 2); // worst case
    s.length = 0;

    size_t i = 0;
    while (i < count) {
        uint32_t pixel = c->pixels[i];
        size_t run = 1;

        while (i + run < count && c->pixels[i + run] == pixel)
            run++;

        s.data[s.length++] = pixel;
        s.data[s.length++] = (uint32_t)run;
        i += run;
    }

    return s;
}

static void snapshot_restore(Canvas *c, Snapshot *s) {
    size_t out = 0;
    size_t total = (size_t)c->width * c->height;

    for (size_t i = 0; i + 1 < s->length && out < total; i += 2) {
        uint32_t pixel = s->data[i];
        uint32_t run   = s->data[i + 1];

        for (uint32_t r = 0; r < run && out < total; r++)
            c->pixels[out++] = pixel;
    }
}

static void snapshot_free(Snapshot *s) {
    free(s->data);
    s->data = NULL;
    s->length = 0;
}

/* ---------- History API ---------- */

void history_push(Canvas *c) {
    if (undo_top == HISTORY_MAX) {
        snapshot_free(&undo_stack[0]);
        memmove(&undo_stack[0], &undo_stack[1],
                sizeof(Snapshot) * (HISTORY_MAX - 1));
        undo_top--;
    }

    undo_stack[undo_top++] = snapshot_create(c);

    for (int i = 0; i < redo_top; i++)
        snapshot_free(&redo_stack[i]);
    redo_top = 0;
}

int history_undo(Canvas *c) {
    if (undo_top == 0)
        return 0;

    redo_stack[redo_top++] = snapshot_create(c);

    Snapshot s = undo_stack[--undo_top];
    snapshot_restore(c, &s);
    snapshot_free(&s);

    return 1;
}

int history_redo(Canvas *c) {
    if (redo_top == 0)
        return 0;

    undo_stack[undo_top++] = snapshot_create(c);

    Snapshot s = redo_stack[--redo_top];
    snapshot_restore(c, &s);
    snapshot_free(&s);

    return 1;
}

void history_free(void) {
    for (int i = 0; i < undo_top; i++)
        snapshot_free(&undo_stack[i]);
    for (int i = 0; i < redo_top; i++)
        snapshot_free(&redo_stack[i]);
}

bool history_can_undo(void) {
    return undo_top > 0;
}

bool history_can_redo(void) {
    return redo_top > 0;
}
