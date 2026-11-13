// Input.cpp - Input Handler: all keyboard and mouse actions
#include "Input.h"
#include "Dashboard.h"
#include "Graph.h"
#include <GL/freeglut.h>
#include <cstdlib>

namespace {
    Network*         gNet    = nullptr;
    AttackGenerator* gAttack = nullptr;
    int              gWinH   = 700;
}

namespace Input {

void init(Network* net, AttackGenerator* atk, int winH) {
    gNet = net;
    gAttack = atk;
    gWinH = winH;
}

// Keyboard
//   N normal packet | 1-6 attacks | C challenge | H help | Space pause
//   + / - speed | R reset | Esc exit
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'n': case 'N': gNet->sendNormalPacket(); break;

        case '1': gAttack->start(ATK_FLOOD, *gNet);     break;
        case '2': gAttack->start(ATK_PORTSCAN, *gNet);  break;
        case '3': gAttack->start(ATK_MALWARE, *gNet);   break;
        case '4': gAttack->start(ATK_BRUTE, *gNet);     break;
        case '5': gAttack->start(ATK_OVERSIZE, *gNet);  break;
        case '6': gAttack->start(ATK_BLACKLIST, *gNet); break;

        case ' ': paused = !paused; break;
        case '+': case '=': speedMul *= 1.25f; break;
        case '-': case '_': speedMul /= 1.25f; break;

        case 'r': case 'R':
            gNet->reset();
            gAttack->reset();
            Graph::reset();
            break;

        case 'c': case 'C':
            if (gNet->challenge.active()) gNet->challenge.stop();
            else gNet->challenge.start(*gNet, *gAttack);
            Graph::reset();
            break;

        case 'h': case 'H': showHelp = !showHelp; break;

        case 27: std::exit(0);
    }
}

// F1..F7 toggle firewall rules R1..R7
void special(int key, int, int) {
    if (key >= GLUT_KEY_F1 && key <= GLUT_KEY_F7)
        gNet->toggleRule(key - GLUT_KEY_F1);
}

// Left click: dashboard = toggle a rule, simulation area = select a packet
void mouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int gy = gWinH - my;                  // GLUT y starts at the top, OpenGL y at the bottom
    if (Dashboard::inPanel(mx)) {
        int r = Dashboard::ruleAt(mx, gy);
        if (r >= 0) gNet->toggleRule(r);
    } else {
        gNet->selectAt((float)mx, (float)gy);
    }
}

} // namespace Input