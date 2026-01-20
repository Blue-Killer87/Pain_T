#include <algorithm>

#include "history.h"

History::History(size_t maxHistory) : maxHistory(maxHistory){}

History::Snapshot History::createSnapshot(const Canvas& canvas) const{
    Snapshot snap;
    snap.width  = canvas.getWidth();
    snap.height = canvas.getHeight();

    const auto& pixels = canvas.getPixels();
    snap.data.reserve(pixels.size() * 2);

    // Simple RLE
    size_t i = 0;
    while (i < pixels.size()){
        uint32_t pixel = pixels[i];
        size_t run = 1;
        while (i + run < pixels.size() && pixels[i + run] == pixel)
            ++run;

        snap.data.push_back(pixel);
        snap.data.push_back(static_cast<uint32_t>(run));
        i += run;
    }

    return snap;
}

void 
History::restoreSnapshot(Canvas& canvas, const Snapshot& snap) const{
    //resize canvas properly
    canvas.setSize(snap.width, snap.height);
    auto& pixels = canvas.getPixels();
    pixels.resize(snap.width * snap.height);

    size_t out = 0;
    for (size_t i = 0; i + 1 < snap.data.size() && out < pixels.size(); i += 2){
        uint32_t pixel = snap.data[i];
        uint32_t run   = snap.data[i + 1];
        for (uint32_t r = 0; r < run && out < pixels.size(); ++r)
            pixels[out++] = pixel;
    }
}

void 
History::push(const Canvas& canvas){
    if (undoStack.size() >= maxHistory){
        undoStack.erase(undoStack.begin());
    }

    undoStack.push_back(createSnapshot(canvas));
    redoStack.clear();
}

bool 
History::undo(Canvas& canvas){
    if (undoStack.empty()) return false;

    redoStack.push_back(createSnapshot(canvas));
    Snapshot snap = undoStack.back();
    undoStack.pop_back();
    restoreSnapshot(canvas, snap);
    return true;
}

bool 
History::redo(Canvas& canvas){
    if (redoStack.empty()) return false;

    undoStack.push_back(createSnapshot(canvas));
    Snapshot snap = redoStack.back();
    redoStack.pop_back();
    restoreSnapshot(canvas, snap);
    return true;
}
