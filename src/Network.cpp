// Network.cpp - UPDATED for Week 4 (Week 3 code kept as is; new lines marked NEW)
#include "Network.h"
#include "Renderer.h"
#include "Effects.h"          // NEW
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>

void Network::init() {
    nodes.clear();
    links.clear();
    packets.clear();
    log.clear();
    firewall.init();
    challenge.stop();         // NEW
    configLoaded = firewall.loadConfig("config/firewall.cfg");   // run the exe from the project folder
    nextId = 1;
    hasSelected = selectedGone = false;

    totalPackets = allowedCount = blockedCount = rateLimitedCount = 0;
    simTime = 0;
    threatLevel = 0;

    for (int i = 0; i < 6; i++) {
        nodes.push_back({100.0f, 120.0f + i * 90.0f, CLIENT, "PC-" + std::to_string(i + 1)});
    }
    nodes.push_back({450.0f, 350.0f, FIREWALL, "Firewall"});             // 6
    nodes.push_back({800.0f, 520.0f, DMZ_SERVER, "DMZ Web Server"});     // 7
    nodes.push_back({800.0f, 180.0f, INTERNAL_SERVER, "Internal DB"});   // 8

    for (int i = 0; i < 6; i++) links.push_back({i, FIREWALL_IDX});
    links.push_back({FIREWALL_IDX, DMZ_IDX});
    links.push_back({FIREWALL_IDX, INTERNAL_IDX});

    log.add("Simulation started", 0.7f, 0.7f, 0.7f);
    if (configLoaded) log.add("Config loaded: config/firewall.cfg", 0.7f, 0.7f, 0.7f);
    else              log.add("No config file, using defaults", 0.7f, 0.7f, 0.5f);
}

void Network::reset() { init(); }

Packet Network::createPacket(int client, int dest) {
    Packet p;
    bool toDMZ = (dest == DMZ_IDX);

    p.srcIP = "192.168.1." + std::to_string(10 + client);
    p.dstIP = toDMZ ? "10.0.0.80" : "10.0.1.5";
    p.port = toDMZ ? ((std::rand() % 2) ? 80 : 443) : 3306;
    p.protocol = "TCP";
    p.size = 200 + std::rand() % 800;
    p.signature = "";
    p.x = nodes[client].x;
    p.y = nodes[client].y;
    p.speed = 220.0f;
    p.path = {client, FIREWALL_IDX, dest};
    p.pathIdx = 0;
    p.state = MOVING;
    return p;
}

void Network::spawnPacket(const Packet& p) {
    Packet q = p;
    q.id = nextId++;
    packets.push_back(q);
}

// click -> pick the closest packet within 15 px (or clear the selection)
void Network::selectAt(float x, float y) {
    int best = -1;
    float bestD = 15.0f * 15.0f;
    for (size_t i = 0; i < packets.size(); i++) {
        float dx = packets[i].x - x, dy = packets[i].y - y;
        float d = dx * dx + dy * dy;
        if (d <= bestD) { bestD = d; best = (int)i; }
    }
    if (best >= 0) {
        selected = packets[best];
        hasSelected = true;
        selectedGone = false;
    } else {
        hasSelected = false;
    }
}

void Network::toggleRule(int i) {
    if (i < 0 || i >= 6) return;          // R7 (default allow) cannot be switched off
    Rule& r = firewall.rules[i];
    r.enabled = !r.enabled;
    char buf[96];
    std::snprintf(buf, sizeof(buf), "RULE %s %s: %s", r.id.c_str(),
                  r.enabled ? "ENABLED" : "DISABLED", r.description.c_str());
    log.add(buf, 0.6f, 0.8f, 1.0f);
}

void Network::sendNormalPacket() {
    int c = std::rand() % 6;
    int dest = (std::rand() % 2 == 0) ? DMZ_IDX : INTERNAL_IDX;
    spawnPacket(createPacket(c, dest));
}

SecurityStatus Network::status() const {
    if (threatLevel >= 8.0f) return STATUS_CRITICAL;
    if (threatLevel >= 2.5f) return STATUS_WARNING;
    return STATUS_SAFE;
}

// Firewall check: runs once when a packet reaches the firewall node
void Network::onReachFirewall(Packet& p) {
    Decision d = firewall.inspect(p, simTime);
    p.matchedRule = d.ruleId;
    p.threat = d.threat;
    totalPackets++;

    const char* word = "ALLOW";
    float r = 0.3f, g = 1.0f, b = 0.4f;

    switch (d.action) {
        case ACT_ALLOW:
            p.state = ALLOWED;
            allowedCount++;
            break;
        case ACT_BLOCK:
            p.state = BLOCKED;
            p.timer = 0.8f;          // stays visible for a moment, then disappears
            blockedCount++;
            threatLevel += 1.0f;
            word = "BLOCK"; r = 1.0f; g = 0.25f; b = 0.25f;
            break;
        case ACT_RATE_LIMIT:
            p.state = RATE_LIMITED;
            p.speed *= 0.35f;        // throttled: moves slowly
            rateLimitedCount++;
            threatLevel += 1.0f;
            word = "LIMIT"; r = 1.0f; g = 0.9f; b = 0.1f;
            break;
    }

    char buf[128];
    std::snprintf(buf, sizeof(buf), "[%4.1fs] %s %s:%d %s %s",
                  simTime, word, p.srcIP.c_str(), p.port,
                  d.ruleId.c_str(), d.threat.c_str());
    log.add(buf, r, g, b);

    challenge.onDecision(p, d.action);     // NEW: Defender Challenge scoring
}

void Network::update(float dt) {
    simTime += dt;
    threatLevel = std::max(0.0f, threatLevel - dt * 1.0f);

    for (auto& p : packets) {
        if (p.state == BLOCKED) {            // blocked packets stop at the firewall
            p.timer -= dt;
            if (p.timer <= 0) p.state = DONE;
            continue;
        }
        if (p.state == DONE) continue;

        const Node& target = nodes[p.path[p.pathIdx + 1]];
        float dx = target.x - p.x;
        float dy = target.y - p.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float step = p.speed * dt;

        if (dist <= step) {
            p.x = target.x;
            p.y = target.y;
            p.pathIdx++;

            if (p.path[p.pathIdx] == FIREWALL_IDX) onReachFirewall(p);
            if (p.state != BLOCKED && p.pathIdx == (int)p.path.size() - 1) p.state = DONE;
        } else {
            p.x += dx / dist * step;
            p.y += dy / dist * step;
        }
    }

    // keep the inspector's copy in sync with the live packet
    if (hasSelected && !selectedGone) {
        bool found = false;
        for (const auto& p : packets) {
            if (p.id == selected.id) {
                selected = p;
                found = true;
                if (p.state == DONE) selectedGone = true;
                break;
            }
        }
        if (!found) selectedGone = true;
    }

    packets.erase(std::remove_if(packets.begin(), packets.end(),
                  [](const Packet& p) { return p.state == DONE; }),
                  packets.end());
}

void Network::drawNode(const Node& n) const {
    float r = 0.3f, g = 0.6f, b = 1.0f;
    int hw = 22, hh = 22;
    switch (n.type) {
        case CLIENT:          r = 0.3f; g = 0.6f; b = 1.0f; break;
        case FIREWALL:        r = 1.0f; g = 0.5f; b = 0.1f; hw = 26; hh = 40; break;
        case DMZ_SERVER:      r = 0.2f; g = 0.9f; b = 0.9f; hw = 30; hh = 24; break;
        case INTERNAL_SERVER: r = 0.7f; g = 0.4f; b = 1.0f; hw = 30; hh = 24; break;
    }
    int x = (int)n.x, y = (int)n.y;

    Effects::drawGlow(n.x, n.y, (float)(std::max(hw, hh) + 35), r, g, b, 0.7f);   // NEW

    Renderer::setColor(r * 0.25f, g * 0.25f, b * 0.25f);
    Renderer::rectFilled(x, y, hw, hh);

    Renderer::setColor(r, g, b);
    Renderer::rectOutline(x, y, hw, hh);
    Renderer::circleMidpoint(x, y, std::max(hw, hh) + 10);

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(n.x - 25, n.y - std::max(hw, hh) - 28, n.name.c_str());
}

// colour coding: normal = blue/green, suspicious = orange, rate-limited = yellow, blocked = red
void Network::drawPacket(const Packet& p) const {
    float r, g, b;
    int half = 6;
    switch (p.state) {
        case BLOCKED:      r = 1.0f; g = 0.15f; b = 0.15f; half = 9; break;
        case RATE_LIMITED: r = 1.0f; g = 0.9f;  b = 0.1f;  break;
        case ALLOWED:      r = 0.2f; g = 1.0f;  b = 0.3f;  break;
        default:
            if (p.isAttack) { r = 1.0f; g = 0.55f; b = 0.1f; }   // suspicious
            else            { r = 0.3f; g = 0.7f;  b = 1.0f; }   // normal
    }

    // NEW: fading trail behind a moving packet
    if (p.state != BLOCKED && p.pathIdx + 1 < (int)p.path.size()) {
        const Node& tgt = nodes[p.path[p.pathIdx + 1]];
        Effects::drawTrail(p.x, p.y, tgt.x - p.x, tgt.y - p.y, r, g, b);
    }

    Renderer::setColor(r, g, b);
    Renderer::rectFilled((int)p.x, (int)p.y, half, half);

    if (p.state == BLOCKED) {   // draw an X with Bresenham
        int x = (int)p.x, y = (int)p.y;
        Renderer::setColor(1, 1, 1);
        Renderer::lineBresenham(x - half, y - half, x + half, y + half);
        Renderer::lineBresenham(x - half, y + half, x + half, y - half);
    }
}

void Network::draw() const {
    const Node& fw = nodes[FIREWALL_IDX];
    Effects::drawFirewallBack(fw.x, fw.y);          // NEW: glow + rotating radar

    Renderer::setColor(0.35f, 0.4f, 0.5f);
    for (const auto& l : links) {
        Renderer::lineBresenham((int)nodes[l.first].x, (int)nodes[l.first].y,
                                (int)nodes[l.second].x, (int)nodes[l.second].y);
    }
    for (const auto& n : nodes) drawNode(n);

    Effects::drawFirewallShield(fw.x, fw.y);        // NEW: pulsing shield on the firewall
    if (status() != STATUS_SAFE)                    // NEW: warning triangle
        Effects::drawWarningTriangle(fw.x, fw.y + 95, 22, status() == STATUS_CRITICAL);

    for (const auto& p : packets) drawPacket(p);

    if (hasSelected && !selectedGone) {     // selection ring
        Renderer::setColor(1, 1, 1);
        Renderer::circleMidpoint((int)selected.x, (int)selected.y, 14);
    }
}