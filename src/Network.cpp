// Network.cpp - Person B
#include "Network.h"
#include "Renderer.h"
#include <cmath>
#include <cstdlib>
#include <string>
#include <algorithm>

void Network::init() {
    nodes.clear();
    links.clear();
    packets.clear();
    totalPackets = 0;

    // 6 clients on the left
    for (int i = 0; i < 6; i++) {
        nodes.push_back({100.0f, 120.0f + i * 90.0f, CLIENT, "PC-" + std::to_string(i + 1)});
    }
    nodes.push_back({450.0f, 350.0f, FIREWALL, "Firewall"});         // 6
    nodes.push_back({800.0f, 520.0f, DMZ_SERVER, "DMZ Web Server"}); // 7
    nodes.push_back({800.0f, 180.0f, INTERNAL_SERVER, "Internal DB"}); // 8

    for (int i = 0; i < 6; i++) links.push_back({i, FIREWALL_IDX});
    links.push_back({FIREWALL_IDX, DMZ_IDX});
    links.push_back({FIREWALL_IDX, INTERNAL_IDX});
}

void Network::reset() { init(); }

void Network::sendNormalPacket() {
    Packet p;
    int c = std::rand() % 6;
    bool toDMZ = (std::rand() % 2 == 0);
    int dest = toDMZ ? DMZ_IDX : INTERNAL_IDX;

    p.srcIP = "192.168.1." + std::to_string(10 + c);
    p.dstIP = toDMZ ? "10.0.0.80" : "10.0.1.5";
    p.port = toDMZ ? ((std::rand() % 2) ? 80 : 443) : 3306;
    p.protocol = "TCP";
    p.size = 200 + std::rand() % 800;
    p.signature = "";
    p.x = nodes[c].x;
    p.y = nodes[c].y;
    p.speed = 220.0f;
    p.path = {c, FIREWALL_IDX, dest};
    p.pathIdx = 0;
    p.state = MOVING;

    packets.push_back(p);
    totalPackets++;
}

// Week 2: decide ALLOW / BLOCK / RATE_LIMIT here.
void Network::onReachFirewall(Packet& p) {
    (void)p; // for now every packet is allowed
}

void Network::update(float dt) {
    for (auto& p : packets) {
        if (p.state != MOVING) continue;

        const Node& target = nodes[p.path[p.pathIdx + 1]];
        float dx = target.x - p.x;
        float dy = target.y - p.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        float step = p.speed * dt;

        if (dist <= step) {
            // reached the next node
            p.x = target.x;
            p.y = target.y;
            p.pathIdx++;

            if (p.path[p.pathIdx] == FIREWALL_IDX) onReachFirewall(p);
            if (p.pathIdx == (int)p.path.size() - 1) p.state = DONE;
        } else {
            // translation towards the target
            p.x += dx / dist * step;
            p.y += dy / dist * step;
        }
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

    Renderer::setColor(r * 0.25f, g * 0.25f, b * 0.25f);
    Renderer::rectFilled(x, y, hw, hh);

    Renderer::setColor(r, g, b);
    Renderer::rectOutline(x, y, hw, hh);
    Renderer::circleMidpoint(x, y, std::max(hw, hh) + 10);

    Renderer::setColor(1, 1, 1);
    Renderer::drawText(n.x - 25, n.y - std::max(hw, hh) - 28, n.name.c_str());
}

void Network::drawPacket(const Packet& p) const {
    Renderer::setColor(0.2f, 1.0f, 0.3f);
    Renderer::rectFilled((int)p.x, (int)p.y, 6, 6);
}

void Network::draw() const {
    // connections (DDA/Bresenham)
    Renderer::setColor(0.35f, 0.4f, 0.5f);
    for (const auto& l : links) {
        Renderer::lineBresenham((int)nodes[l.first].x, (int)nodes[l.first].y,
                                (int)nodes[l.second].x, (int)nodes[l.second].y);
    }
    for (const auto& n : nodes) drawNode(n);
    for (const auto& p : packets) drawPacket(p);
}