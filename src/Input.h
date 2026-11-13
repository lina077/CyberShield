// Input.h - Input Handler module: keyboard, special keys and mouse
#pragma once
#include "Network.h"
#include "AttackGenerator.h"

namespace Input {
    // simulation state controlled by the keyboard (main.cpp reads these)
    inline bool  paused   = false;
    inline bool  showHelp = false;
    inline float speedMul = 1.0f;

    void init(Network* net, AttackGenerator* atk, int winH);

    // GLUT callbacks
    void keyboard(unsigned char key, int x, int y);
    void special(int key, int x, int y);
    void mouse(int button, int state, int mx, int my);
}