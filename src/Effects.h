// Effects.h - visual polish (glow, radar, shield, warning, flash, trails)
#pragma once

namespace Effects {
    void  init();                                   // call once after glutCreateWindow
    float time();                                   // seconds since start

    void drawBackground(int w, int h);              // dark gradient
    void drawGlow(float cx, float cy, float radius, float r, float g, float b, float intensity);
    void drawFirewallBack(float x, float y);        // glow + rotating radar (draw BEFORE the node)
    void drawFirewallShield(float x, float y);      // pulsing scan-line-filled shield (draw AFTER the node)
    void drawWarningTriangle(float x, float y, float size, bool critical);
    void drawScreenFlash(int w, int h, float r, float g, float b, float strength);
    void drawTrail(float x, float y, float dirX, float dirY, float r, float g, float b);
}