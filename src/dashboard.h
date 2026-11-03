// Dashboard.h
// Right-side panel: status, counters, rule list, event log.
// Panel position is set by constants at the top of Dashboard.cpp (window is 1280 x 700).
#pragma once
#include "Network.h"
#include "AttackGenerator.h"

namespace Dashboard {
    void draw(const Network& net, const AttackGenerator& atk);

    // Week 3
    bool inPanel(int mx);                  // is the click inside the right-side panel?
    int  ruleAt(int mx, int myGL);         // rule row under the mouse (0..6) or -1; myGL = OpenGL y (origin bottom)
}