// Graph.h - live traffic graph (allowed vs blocked), clipped with Cohen-Sutherland
#pragma once
#include "Network.h"

namespace Graph {
    void reset();
    void update(float dt, const Network& net);       // call every frame (when not paused)
    void draw(int x, int y, int w, int h);           // bottom-left corner + size
}