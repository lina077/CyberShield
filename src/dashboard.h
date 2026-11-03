// Dashboard.h
// Right-side panel: status, counters, rule list, event log.
// Panel position is set by constants at the top of Dashboard.cpp (window is 1280 x 700).
#pragma once
#include "Network.h"
#include "AttackGenerator.h"

namespace Dashboard {
    void draw(const Network& net, const AttackGenerator& atk);
}
