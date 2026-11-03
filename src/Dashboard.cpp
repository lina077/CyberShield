// Dashboard.cpp
#include "Dashboard.h"
#include "Renderer.h"
#include <cstdio>
#include <string>

namespace {
    const int PX = 910, PY = 20, PW = 360, PH = 660;   // panel rectangle

    void text(float x, float y, float r, float g, float b, const std::string& s) {
        Renderer::setColor(r, g, b);
        Renderer::drawText(x, y, s.c_str());
    }
}

void Dashboard::draw(const Network& net, const AttackGenerator& atk) {
    // panel background + border
    Renderer::setColor(0.07f, 0.08f, 0.13f);
    Renderer::rectFilled(PX + PW / 2, PY + PH / 2, PW / 2, PH / 2);
    Renderer::setColor(0.3f, 0.35f, 0.5f);
    Renderer::rectOutline(PX + PW / 2, PY + PH / 2, PW / 2, PH / 2);

    float x = PX + 15;
    float y = PY + PH - 30;
    char buf[128];

    text(x, y, 1, 1, 1, "SECURITY DASHBOARD");
    y -= 30;

    // status
    const char* st = "SAFE";
    float sr = 0.2f, sg = 1.0f, sb = 0.3f;
    if (net.status() == STATUS_WARNING)  { st = "WARNING";  sr = 1.0f; sg = 0.8f; sb = 0.1f; }
    if (net.status() == STATUS_CRITICAL) { st = "CRITICAL"; sr = 1.0f; sg = 0.2f; sb = 0.2f; }
    Renderer::setColor(sr, sg, sb);
    Renderer::rectFilled((int)x + 6, (int)y + 4, 6, 6);
    std::snprintf(buf, sizeof(buf), "STATUS: %s", st);
    text(x + 20, y, sr, sg, sb, buf);
    y -= 28;

    // counters
    std::snprintf(buf, sizeof(buf), "Total inspected : %d", net.totalPackets);
    text(x, y, 1, 1, 1, buf);                   y -= 18;
    std::snprintf(buf, sizeof(buf), "Allowed         : %d", net.allowedCount);
    text(x, y, 0.3f, 1, 0.4f, buf);             y -= 18;
    std::snprintf(buf, sizeof(buf), "Blocked         : %d", net.blockedCount);
    text(x, y, 1, 0.3f, 0.3f, buf);             y -= 18;
    std::snprintf(buf, sizeof(buf), "Rate-limited    : %d", net.rateLimitedCount);
    text(x, y, 1, 0.9f, 0.1f, buf);             y -= 24;

    // active attack
    std::snprintf(buf, sizeof(buf), "Attack: %s", AttackGenerator::name(atk.current));
    text(x, y, 1, 0.55f, 0.1f, buf);
    y -= 28;

    // firewall rules
    text(x, y, 1, 1, 1, "FIREWALL RULES");
    y -= 18;
    for (const auto& r : net.firewall.rules) {
        std::snprintf(buf, sizeof(buf), "%s %-3s %s", r.id.c_str(), r.enabled ? "ON" : "OFF", r.description.c_str());
        if (r.enabled) text(x, y, 0.75f, 0.85f, 1.0f, buf);
        else           text(x, y, 0.4f, 0.4f, 0.4f, buf);
        y -= 16;
    }
    y -= 14;

    // event log (newest first)
    text(x, y, 1, 1, 1, "EVENT LOG");
    y -= 18;
    for (auto it = net.log.entries.rbegin(); it != net.log.entries.rend(); ++it) {
        if (y < PY + 10) break;
        std::string s = it->text;
        if (s.size() > 50) s = s.substr(0, 50);
        text(x, y, it->r, it->g, it->b, s);
        y -= 16;
    }
}
