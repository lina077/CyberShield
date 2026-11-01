// main.cpp - shared
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include "Renderer.h"
#include "Network.h"

const int WIN_W = 1000, WIN_H = 700;

Network net;
bool paused = false;
float speedMul = 1.0f;
int lastTime = 0;

void drawGrid() {
    Renderer::setColor(0.12f, 0.14f, 0.2f);
    for (int x = 0; x <= WIN_W; x += 50) Renderer::lineDDA(x, 0, x, WIN_H);   // DDA grid
    for (int y = 0; y <= WIN_H; y += 50) Renderer::lineDDA(0, y, WIN_W, y);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    net.draw();

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(10, WIN_H - 20, "CyberShield | N: send packet  Space: pause  +/-: speed  R: reset  Esc: exit");
    glutSwapBuffers();
}

void timer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    if (!paused) net.update(dt * speedMul);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);   // ~60 FPS
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'n': case 'N': net.sendNormalPacket(); break;
        case ' ': paused = !paused; break;
        case '+': case '=': speedMul *= 1.25f; break;
        case '-': case '_': speedMul /= 1.25f; break;
        case 'r': case 'R': net.reset(); break;
        case 27: exit(0);
    }
}

int main(int argc, char** argv) {
    std::srand((unsigned)std::time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);   // double buffering
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("CyberShield");

    glClearColor(0.04f, 0.05f, 0.09f, 1.0f);
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    net.init();
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}