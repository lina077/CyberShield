// main.cpp - UPDATED for Week 3
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include "Renderer.h"
#include "Network.h"
#include "AttackGenerator.h"
#include "Dashboard.h"

const int WIN_W = 1280, WIN_H = 700;

Network net;
AttackGenerator attack;
bool paused = false;
float speedMul = 1.0f;
int lastTime = 0;

void drawGrid() {
    Renderer::setColor(0.12f, 0.14f, 0.2f);
    for (int x = 0; x <= WIN_W; x += 50) Renderer::lineDDA(x, 0, x, WIN_H);
    for (int y = 0; y <= WIN_H; y += 50) Renderer::lineDDA(0, y, WIN_W, y);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    net.draw();
    Dashboard::draw(net, attack);

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(10, WIN_H - 20,
        "N: normal | 1 Flood  2 PortScan  3 Malware  4 Brute  5 Oversize  6 Blacklist | Space: pause  +/-: speed  R: reset  Esc: exit");
    Renderer::drawText(10, WIN_H - 40,
        "Click packet: inspect | Click rule or F1-F7: toggle rule | R: reset + reload config");
    if (paused) {
        Renderer::setColor(1, 0.8f, 0.1f);
        Renderer::drawText(10, WIN_H - 60, "PAUSED");
    }
    glutSwapBuffers();
}

void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;          // avoid big jumps (e.g. window dragged)

    if (!paused) {
        attack.update(dt * speedMul, net);
        net.update(dt * speedMul);
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'n': case 'N': net.sendNormalPacket(); break;
        case '1': attack.start(ATK_FLOOD, net);     break;
        case '2': attack.start(ATK_PORTSCAN, net);  break;
        case '3': attack.start(ATK_MALWARE, net);   break;
        case '4': attack.start(ATK_BRUTE, net);     break;
        case '5': attack.start(ATK_OVERSIZE, net);  break;
        case '6': attack.start(ATK_BLACKLIST, net); break;
        case ' ': paused = !paused; break;
        case '+': case '=': speedMul *= 1.25f; break;
        case '-': case '_': speedMul /= 1.25f; break;
        case 'r': case 'R': net.reset(); attack.reset(); break;
        case 27: exit(0);
    }
}

// Week 3: mouse click = select packet (simulation area) or toggle rule (dashboard)
void mouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    int gy = WIN_H - my;                       // GLUT y starts at the top, OpenGL y at the bottom
    if (Dashboard::inPanel(mx)) {
        int r = Dashboard::ruleAt(mx, gy);
        if (r >= 0) net.toggleRule(r);
    } else {
        net.selectAt((float)mx, (float)gy);
    }
}

// Week 3: F1..F7 toggle R1..R7
void special(int key, int, int) {
    if (key >= GLUT_KEY_F1 && key <= GLUT_KEY_F7) net.toggleRule(key - GLUT_KEY_F1);
}

int main(int argc, char** argv) {
    std::srand((unsigned)std::time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("CyberShield");

    glClearColor(0.04f, 0.05f, 0.09f, 1.0f);
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    net.init();
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutSpecialFunc(special);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}