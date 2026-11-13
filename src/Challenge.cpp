// Challenge.cpp
#include "Challenge.h"
#include "Network.h"
#include "Renderer.h"
#include <cstdio>
#include <algorithm>

namespace {
    struct WaveInfo { AttackType type; const char* hint; };
    const WaveInfo WAVES[] = {
        {ATK_FLOOD,     "One host is sending far too many packets per second"},
        {ATK_BLACKLIST, "Traffic is coming from a known bad host"},
        {ATK_OVERSIZE,  "Abnormally large packets are arriving"},
        {ATK_MALWARE,   "Packets carry a malicious payload signature"},
        {ATK_PORTSCAN,  "Someone is probing risky ports (telnet, SMB, RDP)"},
        {ATK_BRUTE,     "Repeated failed logins to the internal server"},
    };
    const int NUM_WAVES = 6;
    const float BREAK_SECONDS = 5.0f;
}

void Challenge::start(Network& net, AttackGenerator& atk) {
    net.reset();                 // clean world (this also calls stop())
    atk.reset();

    state = BREAK;
    wave = 0;
    timer = BREAK_SECONDS;
    trafficTimer = 0;
    score = 0; integrity = 100; stopped = leaked = falseBlocks = 0;
    curType = ATK_NONE;

    for (int i = 0; i < 6; i++) net.firewall.rules[i].enabled = false;   // R1..R6 OFF
    net.log.add("CHALLENGE: all rules are OFF - enable the right ones!", 0.4f, 0.8f, 1.0f);
}

void Challenge::stop() {
    state = IDLE;
    score = 0; integrity = 100; wave = 0;
    stopped = leaked = falseBlocks = 0;
    timer = trafficTimer = 0;
    curType = ATK_NONE;
}

void Challenge::update(float dt, Network& net, AttackGenerator& atk) {
    if (state == IDLE || state == FINISHED || state == FAILED) return;

    // background normal traffic
    trafficTimer -= dt;
    if (trafficTimer <= 0) { net.sendNormalPacket(); trafficTimer = 1.2f; }

    if (state == BREAK) {
        timer -= dt;
        if (timer <= 0) {
            curType = WAVES[wave].type;
            atk.start(curType, net);
            state = WAVE;
        }
    } else if (state == WAVE) {
        if (!atk.active()) {                       // all attack packets have been sent
            bool inFlight = false;
            for (const auto& p : net.packets) if (p.isAttack) { inFlight = true; break; }
            if (!inFlight) {
                wave++;
                if (wave >= NUM_WAVES) {
                    state = FINISHED;
                    net.log.add("CHALLENGE COMPLETE", 0.4f, 1.0f, 0.5f);
                } else {
                    state = BREAK;
                    timer = BREAK_SECONDS;
                }
            }
        }
    }

    if (integrity <= 0) {
        integrity = 0;
        state = FAILED;
        atk.reset();
        net.log.add("CHALLENGE FAILED: server compromised", 1.0f, 0.3f, 0.3f);
    }
}

bool Challenge::isThreat(const Packet& p) const {
    if (curType == ATK_PORTSCAN)               // only probes of risky ports matter
        return p.port == 23 || p.port == 445 || p.port == 3389;
    return true;
}

int Challenge::leakDamage() const {
    switch (curType) {
        case ATK_FLOOD:    return 2;
        case ATK_BRUTE:    return 3;
        case ATK_MALWARE:  return 10;
        default:           return 5;
    }
}

void Challenge::onDecision(const Packet& p, FwAction a) {
    if (state != WAVE && state != BREAK) return;
    bool stoppedIt = (a != ACT_ALLOW);

    if (p.isAttack) {
        if (!isThreat(p)) return;
        if (stoppedIt) { stopped++; score += 10; }
        else           { leaked++;  score -= 10; integrity -= leakDamage(); }
    } else {
        if (stoppedIt) { falseBlocks++; score -= 5; }
        else           { score += 1; }
    }
}

char Challenge::rating() const {
    if (integrity >= 85) return 'A';
    if (integrity >= 60) return 'B';
    if (integrity >= 30) return 'C';
    return 'D';
}

void Challenge::draw() const {
    if (!active()) return;

    const int cx = 550, cy = 610, hw = 230, hh = 40;
    Renderer::setColor(0.06f, 0.08f, 0.15f);
    Renderer::rectFilled(cx, cy, hw, hh);
    Renderer::setColor(0.3f, 0.7f, 1.0f);
    Renderer::rectOutline(cx, cy, hw, hh);

    char buf[160];
    float x = cx - hw + 12;

    int shown = std::min(wave + 1, NUM_WAVES);
    std::snprintf(buf, sizeof(buf), "DEFENDER CHALLENGE   Wave %d/%d", shown, NUM_WAVES);
    Renderer::setColor(0.4f, 0.8f, 1.0f);
    Renderer::drawText(x, cy + 24, buf);

    if (state == BREAK) {
        std::snprintf(buf, sizeof(buf), "Next wave in %.0fs - switch on the right rules!", timer);
        Renderer::setColor(1, 1, 1);
    } else if (state == WAVE) {
        std::snprintf(buf, sizeof(buf), "INCOMING: %s", WAVES[wave].hint);
        Renderer::setColor(1.0f, 0.6f, 0.2f);
    } else if (state == FINISHED) {
        std::snprintf(buf, sizeof(buf), "COMPLETE!  Rating: %c   (press C to restart)", rating());
        Renderer::setColor(0.4f, 1.0f, 0.5f);
    } else {
        std::snprintf(buf, sizeof(buf), "SERVER COMPROMISED   Rating: %c   (press C to retry)", rating());
        Renderer::setColor(1.0f, 0.3f, 0.3f);
    }
    Renderer::drawText(x, cy + 6, buf);

    std::snprintf(buf, sizeof(buf), "Score %d   Stopped %d   Leaked %d   False blocks %d",
                  score, stopped, leaked, falseBlocks);
    Renderer::setColor(0.85f, 0.85f, 0.85f);
    Renderer::drawText(x, cy - 12, buf);

    // integrity bar
    int barW = 200;
    int filled = barW * std::max(0, integrity) / 100;
    Renderer::setColor(0.2f, 0.2f, 0.25f);
    Renderer::rectFilled(cx - hw + 12 + 100 + barW / 2, cy - 28, barW / 2, 4);
    if (integrity > 60)      Renderer::setColor(0.2f, 0.9f, 0.3f);
    else if (integrity > 30) Renderer::setColor(1.0f, 0.8f, 0.1f);
    else                     Renderer::setColor(1.0f, 0.2f, 0.2f);
    if (filled > 0) Renderer::rectFilled(cx - hw + 12 + 100 + filled / 2, cy - 28, filled / 2, 4);
    Renderer::setColor(0.85f, 0.85f, 0.85f);
    Renderer::drawText(x, cy - 33, "Server integrity");
}