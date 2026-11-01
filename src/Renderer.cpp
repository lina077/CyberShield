// Renderer.cpp - Person A
#include "Renderer.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace Renderer {

void setColor(float r, float g, float b) { glColor3f(r, g, b); }

// plot a single pixel (own glBegin/glEnd)
void putPixel(int x, int y) {
    glBegin(GL_POINTS);
        glVertex2i(x, y);
    glEnd();
}

// internal: plot inside an already-open glBegin(GL_POINTS)
static inline void plot(int x, int y) { glVertex2i(x, y); }

// ---------------- DDA line ----------------
void lineDDA(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1, dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));

    glBegin(GL_POINTS);
    if (steps == 0) {
        plot(x1, y1);
    } else {
        float xInc = (float)dx / steps;
        float yInc = (float)dy / steps;
        float x = (float)x1, y = (float)y1;
        for (int i = 0; i <= steps; i++) {
            plot((int)std::lround(x), (int)std::lround(y));
            x += xInc;
            y += yInc;
        }
    }
    glEnd();
}

// ---------------- Bresenham line (all octants) ----------------
void lineBresenham(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    glBegin(GL_POINTS);
    while (true) {
        plot(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
    glEnd();
}

// ---------------- Midpoint circle ----------------
static void plot8(int cx, int cy, int x, int y) {
    plot(cx + x, cy + y); plot(cx - x, cy + y);
    plot(cx + x, cy - y); plot(cx - x, cy - y);
    plot(cx + y, cy + x); plot(cx - y, cy + x);
    plot(cx + y, cy - x); plot(cx - y, cy - x);
}

void circleMidpoint(int cx, int cy, int r) {
    int x = 0, y = r;
    int p = 1 - r;

    glBegin(GL_POINTS);
    plot8(cx, cy, x, y);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plot8(cx, cy, x, y);
    }
    glEnd();
}

// ---------------- helpers ----------------
void rectOutline(int cx, int cy, int hw, int hh) {
    int l = cx - hw, r = cx + hw, b = cy - hh, t = cy + hh;
    lineBresenham(l, b, r, b);
    lineBresenham(r, b, r, t);
    lineBresenham(r, t, l, t);
    lineBresenham(l, t, l, b);
}

void rectFilled(int cx, int cy, int hw, int hh) {
    glBegin(GL_QUADS);
        glVertex2i(cx - hw, cy - hh);
        glVertex2i(cx + hw, cy - hh);
        glVertex2i(cx + hw, cy + hh);
        glVertex2i(cx - hw, cy + hh);
    glEnd();
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)text);
}

} // namespace Renderer