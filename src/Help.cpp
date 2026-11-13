// Help.cpp
#include "Help.h"
#include "Renderer.h"
#include <GL/freeglut.h>

namespace {
    void line(float x, float& y, const char* key, const char* what) {
        Renderer::setColor(1.0f, 0.8f, 0.3f);
        Renderer::drawText(x, y, key);
        Renderer::setColor(0.9f, 0.9f, 0.9f);
        Renderer::drawText(x + 90, y, what);
        y -= 20;
    }
    void legend(float x, float& y, float r, float g, float b, const char* what) {
        Renderer::setColor(r, g, b);
        Renderer::rectFilled((int)x + 5, (int)y + 4, 5, 5);
        Renderer::setColor(0.9f, 0.9f, 0.9f);
        Renderer::drawText(x + 20, y, what);
        y -= 18;
    }
}

void Help::draw(int w, int h) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // dim the whole screen
    glColor4f(0, 0, 0, 0.75f);
    glBegin(GL_QUADS);
        glVertex2i(0, 0); glVertex2i(w, 0); glVertex2i(w, h); glVertex2i(0, h);
    glEnd();

    int cx = w / 2, cy = h / 2, hw = 340, hh = 270;
    Renderer::setColor(0.07f, 0.09f, 0.16f);
    Renderer::rectFilled(cx, cy, hw, hh);
    Renderer::setColor(0.3f, 0.7f, 1.0f);
    Renderer::rectOutline(cx, cy, hw, hh);

    float x = cx - hw + 25;
    float y = cy + hh - 35;
    Renderer::setColor(1, 1, 1);
    Renderer::drawText(x, y, "CYBERSHIELD - HELP   (press H to close)");
    y -= 32;

    line(x, y, "N",        "Send a normal packet");
    line(x, y, "1 - 6",    "Flood / Port scan / Malware / Brute force / Oversized / Blacklisted host");
    line(x, y, "C",        "Start or stop the Defender Challenge");
    line(x, y, "Space",    "Pause / resume");
    line(x, y, "+ / -",    "Change simulation speed");
    line(x, y, "Click",    "Inspect a packet, or toggle a firewall rule");
    line(x, y, "F1 - F7",  "Toggle firewall rule R1 - R7");
    line(x, y, "R",        "Reset the simulation");
    line(x, y, "H",        "Show / hide this help");
    line(x, y, "Esc",      "Exit");

    y -= 14;
    Renderer::setColor(1, 1, 1);
    Renderer::drawText(x, y, "PACKET COLOURS");
    y -= 22;
    legend(x, y, 0.3f, 0.7f, 1.0f, "Blue: normal traffic, on its way to the firewall");
    legend(x, y, 1.0f, 0.55f, 0.1f, "Orange: suspicious (attack) traffic");
    legend(x, y, 0.2f, 1.0f, 0.3f, "Green: allowed by the firewall");
    legend(x, y, 1.0f, 0.9f, 0.1f, "Yellow: rate-limited (slowed down)");
    legend(x, y, 1.0f, 0.15f, 0.15f, "Red with X: blocked");
}