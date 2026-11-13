// Effects.cpp
#include "Effects.h"
#include "Renderer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <algorithm>

namespace {
    const float PI = 3.14159265f;
    struct Pt { float x, y; };

    // 2D rotation about (cx, cy)
    Pt rotate(Pt p, float cx, float cy, float ang) {
        float c = std::cos(ang), s = std::sin(ang);
        float dx = p.x - cx, dy = p.y - cy;
        return { cx + dx * c - dy * s, cy + dx * s + dy * c };
    }

    // Scan-line polygon fill (works for any simple polygon). Uses the current glColor.
    void scanlineFill(const std::vector<Pt>& v) {
        int n = (int)v.size();
        float ymin = 1e9f, ymax = -1e9f;
        for (const auto& p : v) { ymin = std::min(ymin, p.y); ymax = std::max(ymax, p.y); }

        for (int y = (int)std::ceil(ymin); y <= (int)std::floor(ymax); y++) {
            std::vector<float> xs;
            for (int i = 0; i < n; i++) {
                Pt a = v[i], b = v[(i + 1) % n];
                if (a.y == b.y) continue;                 // skip horizontal edges
                if (a.y > b.y) std::swap(a, b);
                if (y >= a.y && y < b.y)
                    xs.push_back(a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y));
            }
            std::sort(xs.begin(), xs.end());
            for (size_t k = 0; k + 1 < xs.size(); k += 2)
                Renderer::lineBresenham((int)std::lround(xs[k]), y, (int)std::lround(xs[k + 1]), y);
        }
    }

    void outline(const std::vector<Pt>& v) {
        int n = (int)v.size();
        for (int i = 0; i < n; i++) {
            Pt a = v[i], b = v[(i + 1) % n];
            Renderer::lineBresenham((int)a.x, (int)a.y, (int)b.x, (int)b.y);
        }
    }

    // turn on alpha blending (safe to call every frame)
    void blendOn() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void disc(float cx, float cy, float radius) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 40; i++) {
            float a = 2 * PI * i / 40;
            glVertex2f(cx + radius * std::cos(a), cy + radius * std::sin(a));
        }
        glEnd();
    }
}

namespace Effects {

float time() { return glutGet(GLUT_ELAPSED_TIME) / 1000.0f; }

void init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawBackground(int w, int h) {
    glBegin(GL_QUADS);
        glColor3f(0.02f, 0.03f, 0.06f); glVertex2i(0, 0);  glVertex2i(w, 0);
        glColor3f(0.07f, 0.09f, 0.18f); glVertex2i(w, h);  glVertex2i(0, h);
    glEnd();
}

void drawGlow(float cx, float cy, float radius, float r, float g, float b, float intensity) {
    blendOn();
    for (int i = 1; i <= 5; i++) {              // overlapping discs: brighter near the centre
        glColor4f(r, g, b, 0.06f * intensity);
        disc(cx, cy, radius * i / 5.0f);
    }
}

void drawFirewallBack(float x, float y) {
    blendOn();
    float t = time();
    float R = 95.0f;
    drawGlow(x, y, R, 1.0f, 0.5f, 0.1f, 1.0f);

    // radar rings
    glColor4f(1.0f, 0.6f, 0.2f, 0.25f);
    Renderer::circleMidpoint((int)x, (int)y, (int)R);
    Renderer::circleMidpoint((int)x, (int)y, (int)(R * 0.6f));

    // rotating sweep: fading wedge + bright leading line
    float ang = std::fmod(t * 1.6f, 2 * PI);
    float span = 1.2f;
    int N = 24;
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.6f, 0.2f, 0.0f);
        glVertex2f(x, y);
        for (int i = 0; i <= N; i++) {
            float a = ang - span + span * i / N;
            glColor4f(1.0f, 0.6f, 0.2f, 0.35f * i / N);
            glVertex2f(x + R * std::cos(a), y + R * std::sin(a));
        }
    glEnd();
    glColor4f(1.0f, 0.8f, 0.4f, 0.9f);
    Renderer::lineBresenham((int)x, (int)y, (int)(x + R * std::cos(ang)), (int)(y + R * std::sin(ang)));
}

void drawFirewallShield(float x, float y) {
    blendOn();
    float t = time();
    float s = 20.0f * (1.0f + 0.1f * std::sin(t * 3.0f));   // scaling pulse
    std::vector<Pt> v = {
        {x - 0.8f * s, y + 0.8f * s}, {x, y + 1.0f * s}, {x + 0.8f * s, y + 0.8f * s},
        {x + 0.8f * s, y},            {x, y - 1.0f * s}, {x - 0.8f * s, y}
    };
    glColor4f(1.0f, 0.55f, 0.1f, 0.65f);
    scanlineFill(v);
    glColor4f(1.0f, 0.9f, 0.6f, 1.0f);
    outline(v);
}

void drawWarningTriangle(float x, float y, float size, bool critical) {
    blendOn();
    float t = time();
    float ang = 0.35f * std::sin(t * (critical ? 8.0f : 4.0f));   // wobble (rotation)
    Pt c = {x, y};
    std::vector<Pt> v = {
        rotate({x, y + size},              c.x, c.y, ang),
        rotate({x - 0.9f * size, y - 0.7f * size}, c.x, c.y, ang),
        rotate({x + 0.9f * size, y - 0.7f * size}, c.x, c.y, ang)
    };
    float pulse = 0.65f + 0.3f * std::sin(t * 6.0f);
    if (critical) glColor4f(1.0f, 0.2f, 0.2f, pulse);
    else          glColor4f(1.0f, 0.8f, 0.1f, pulse);
    scanlineFill(v);
    glColor4f(1, 1, 1, 1);
    outline(v);

    // exclamation mark
    glColor4f(0.1f, 0.05f, 0.05f, 1);
    Renderer::rectFilled((int)x, (int)(y + size * 0.05f), 2, (int)(size * 0.3f));
    Renderer::rectFilled((int)x, (int)(y - size * 0.45f), 2, 2);
}

void drawScreenFlash(int w, int h, float r, float g, float b, float strength) {
    blendOn();
    float a = strength * (0.28f + 0.2f * std::sin(time() * 8.0f));
    float T = 45.0f;
    glBegin(GL_QUADS);
        // bottom
        glColor4f(r, g, b, a); glVertex2f(0, 0);  glVertex2f(w, 0);
        glColor4f(r, g, b, 0); glVertex2f(w, T);  glVertex2f(0, T);
        // top
        glColor4f(r, g, b, a); glVertex2f(0, h);  glVertex2f(w, h);
        glColor4f(r, g, b, 0); glVertex2f(w, h - T); glVertex2f(0, h - T);
        // left
        glColor4f(r, g, b, a); glVertex2f(0, 0);  glVertex2f(0, h);
        glColor4f(r, g, b, 0); glVertex2f(T, h);  glVertex2f(T, 0);
        // right
        glColor4f(r, g, b, a); glVertex2f(w, 0);  glVertex2f(w, h);
        glColor4f(r, g, b, 0); glVertex2f(w - T, h); glVertex2f(w - T, 0);
    glEnd();
}

void drawTrail(float x, float y, float dirX, float dirY, float r, float g, float b) {
    blendOn();
    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.001f) return;
    dirX /= len; dirY /= len;
    for (int i = 1; i <= 7; i++) {
        glColor4f(r, g, b, 0.5f * (1.0f - i / 8.0f));
        Renderer::rectFilled((int)(x - dirX * i * 7), (int)(y - dirY * i * 7), std::max(1, 6 - i), std::max(1, 6 - i));
    }
}

} // namespace Effects