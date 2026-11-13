// Graph.cpp
#include "Graph.h"
#include "Renderer.h"
#include <deque>
#include <algorithm>
#include <cmath>

namespace {
    const int   MAX_SAMPLES = 60;
    const float SAMPLE_SEC  = 0.5f;

    std::deque<float> allowedS, blockedS;
    float acc = 0;
    int lastAllowed = 0, lastBlocked = 0;

    // ---- Cohen-Sutherland line clipping ----
    const int INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;

    int outCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
        int c = INSIDE;
        if (x < xmin) c |= LEFT; else if (x > xmax) c |= RIGHT;
        if (y < ymin) c |= BOTTOM; else if (y > ymax) c |= TOP;
        return c;
    }

    bool clipLine(double& x1, double& y1, double& x2, double& y2,
                  double xmin, double ymin, double xmax, double ymax) {
        int c1 = outCode(x1, y1, xmin, ymin, xmax, ymax);
        int c2 = outCode(x2, y2, xmin, ymin, xmax, ymax);
        while (true) {
            if (!(c1 | c2)) return true;     // both inside
            if (c1 & c2)    return false;    // both outside, same side
            int co = c1 ? c1 : c2;
            double x = 0, y = 0;
            if (co & TOP)         { x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1); y = ymax; }
            else if (co & BOTTOM) { x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1); y = ymin; }
            else if (co & RIGHT)  { y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1); x = xmax; }
            else                  { y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1); x = xmin; }
            if (co == c1) { x1 = x; y1 = y; c1 = outCode(x1, y1, xmin, ymin, xmax, ymax); }
            else          { x2 = x; y2 = y; c2 = outCode(x2, y2, xmin, ymin, xmax, ymax); }
        }
    }

    void clippedLine(double x1, double y1, double x2, double y2,
                     double xmin, double ymin, double xmax, double ymax) {
        if (clipLine(x1, y1, x2, y2, xmin, ymin, xmax, ymax))
            Renderer::lineBresenham((int)std::lround(x1), (int)std::lround(y1),
                                    (int)std::lround(x2), (int)std::lround(y2));
    }
}

namespace Graph {

void reset() {
    allowedS.clear(); blockedS.clear();
    acc = 0; lastAllowed = lastBlocked = 0;
}

void update(float dt, const Network& net) {
    acc += dt;
    int blockedNow = net.blockedCount + net.rateLimitedCount;
    if (net.allowedCount < lastAllowed || blockedNow < lastBlocked) {   // counters were reset
        lastAllowed = 0; lastBlocked = 0;
    }
    while (acc >= SAMPLE_SEC) {
        acc -= SAMPLE_SEC;
        allowedS.push_back((float)(net.allowedCount - lastAllowed));
        blockedS.push_back((float)(blockedNow - lastBlocked));
        lastAllowed = net.allowedCount;
        lastBlocked = blockedNow;
        if ((int)allowedS.size() > MAX_SAMPLES) { allowedS.pop_front(); blockedS.pop_front(); }
    }
}

void draw(int x, int y, int w, int h) {
    // panel
    Renderer::setColor(0.05f, 0.06f, 0.11f);
    Renderer::rectFilled(x + w / 2, y + h / 2, w / 2, h / 2);
    Renderer::setColor(0.3f, 0.35f, 0.5f);
    Renderer::rectOutline(x + w / 2, y + h / 2, w / 2, h / 2);

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(x + 8, y + h - 16, "TRAFFIC");
    Renderer::setColor(0.3f, 1.0f, 0.4f);
    Renderer::drawText(x + 80, y + h - 16, "allowed");
    Renderer::setColor(1.0f, 0.3f, 0.3f);
    Renderer::drawText(x + 140, y + h - 16, "blocked / limited");

    // plot area = clipping window
    double xmin = x + 8, xmax = x + w - 8, ymin = y + 8, ymax = y + h - 26;

    float peak = 6.0f;
    for (float v : allowedS) peak = std::max(peak, v);
    for (float v : blockedS) peak = std::max(peak, v);

    // grid
    Renderer::setColor(0.15f, 0.18f, 0.28f);
    for (int i = 0; i <= 4; i++) {
        double gy = ymin + (ymax - ymin) * i / 4.0;
        clippedLine(xmin, gy, xmax, gy, xmin, ymin, xmax, ymax);
    }

    auto plot = [&](const std::deque<float>& s, float r, float g, float b) {
        Renderer::setColor(r, g, b);
        double step = (xmax - xmin) / (MAX_SAMPLES - 1);
        int n = (int)s.size();
        for (int i = 1; i < n; i++) {
            // newest sample on the right
            double x1 = xmax - (n - i) * step,     y1 = ymin + (ymax - ymin) * s[i - 1] / peak;
            double x2 = xmax - (n - 1 - i) * step, y2 = ymin + (ymax - ymin) * s[i] / peak;
            clippedLine(x1, y1, x2, y2, xmin, ymin, xmax, ymax);
        }
    };
    plot(allowedS, 0.3f, 1.0f, 0.4f);
    plot(blockedS, 1.0f, 0.3f, 0.3f);
}

} // namespace Graph