// Renderer.h - Person A
// Self-implemented graphics algorithms (DDA, Bresenham, Midpoint circle)
#pragma once
#include <GL/freeglut.h>

namespace Renderer {
    void setColor(float r, float g, float b);
    void putPixel(int x, int y);

    void lineDDA(int x1, int y1, int x2, int y2);
    void lineBresenham(int x1, int y1, int x2, int y2);
    void circleMidpoint(int cx, int cy, int r);

    // helpers (use the algorithms above / OpenGL primitives)
    void rectOutline(int cx, int cy, int halfW, int halfH);  // uses Bresenham
    void rectFilled(int cx, int cy, int halfW, int halfH);   // GL_QUADS
    void drawText(float x, float y, const char* text);
}