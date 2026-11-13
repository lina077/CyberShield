// main.cpp - window, main loop and drawing (keyboard/mouse now live in Input.cpp)
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include "Renderer.h"
#include "Network.h"
#include "AttackGenerator.h"
#include "Dashboard.h"
#include "Effects.h"
#include "Graph.h"
#include "Help.h"
#include "Input.h"

const int WIN_W = 1280, WIN_H = 700;

Network net;
AttackGenerator attack;
int lastTime = 0;

void drawGrid() {
    glColor4f(0.35f, 0.45f, 0.8f, 0.10f);     // faint translucent grid
    for (int x = 0; x <= WIN_W; x += 50) Renderer::lineDDA(x, 0, x, WIN_H);
    for (int y = 0; y <= WIN_H; y += 50) Renderer::lineDDA(0, y, WIN_W, y);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    Effects::drawBackground(WIN_W, WIN_H);
    drawGrid();
    net.draw();
    net.challenge.draw();
    Dashboard::draw(net, attack);

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(10, WIN_H - 20,
        "N: normal | 1 Flood  2 PortScan  3 Malware  4 Brute  5 Oversize  6 Blacklist | C: challenge  H: help  Space: pause  +/-: speed  R: reset  Esc: exit");
    Renderer::drawText(10, WIN_H - 40,
        "Click packet: inspect | Click rule or F1-F7: toggle rule | R: reset + reload config");
    if (Input::paused) {
        Renderer::setColor(1, 0.8f, 0.1f);
        Renderer::drawText(10, WIN_H - 60, "PAUSED");
    }

    // screen-edge flash when under attack
    if (net.status() == STATUS_CRITICAL)
        Effects::drawScreenFlash(WIN_W, WIN_H, 1.0f, 0.1f, 0.1f, 1.0f);
    else if (net.status() == STATUS_WARNING)
        Effects::drawScreenFlash(WIN_W, WIN_H, 1.0f, 0.7f, 0.1f, 0.5f);

    if (Input::showHelp) Help::draw(WIN_W, WIN_H);   // on top of everything
    glutSwapBuffers();
}

void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;
    if (dt > 0.1f) dt = 0.1f;          // avoid big jumps (e.g. window dragged)

    if (!Input::paused) {
        float sdt = dt * Input::speedMul;
        net.challenge.update(sdt, net, attack);
        attack.update(sdt, net);
        net.update(sdt);
        Graph::update(sdt, net);
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    std::srand((unsigned)std::time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("CyberShield");

    glClearColor(0.04f, 0.05f, 0.09f, 1.0f);
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    Effects::init();                           // enables alpha blending

    net.init();
    Input::init(&net, &attack, WIN_H);
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutKeyboardFunc(Input::keyboard);
    glutMouseFunc(Input::mouse);
    glutSpecialFunc(Input::special);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}