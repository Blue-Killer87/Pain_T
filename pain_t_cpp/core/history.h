#ifndef HISTORY_H
#define HISTORY_H

#include <vector>
#include <cstdint>

#include "canvas.h"

class History{
public:
    History(size_t maxHistory = 64);

    void push(const Canvas& canvas);
    bool undo(Canvas& canvas);
    bool redo(Canvas& canvas);

    bool canUndo() const {return !undoStack.empty();}
    bool canRedo() const {return !redoStack.empty();}

private:
    struct Snapshot{
        std::vector<uint32_t> data;
        int width;
        int height;
    };

    Snapshot createSnapshot(const Canvas& canvas) const;
    void restoreSnapshot(Canvas& canvas, const Snapshot& snap) const;

    size_t maxHistory;
    std::vector<Snapshot> undoStack;
    std::vector<Snapshot> redoStack;
};

#endif
