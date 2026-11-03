// AttackGenerator.cpp
#include "AttackGenerator.h"
#include "Network.h"
#include <cstdio>
#include <cstdlib>

const char* AttackGenerator::name(AttackType t) {
    switch (t) {
        case ATK_FLOOD:     return "Flood / DDoS";
        case ATK_PORTSCAN:  return "Port scan";
        case ATK_MALWARE:   return "Malware signature";
        case ATK_BRUTE:     return "Brute-force login";
        case ATK_OVERSIZE:  return "Oversized packets";
        case ATK_BLACKLIST: return "Blacklisted host";
        default:            return "none";
    }
}

void AttackGenerator::start(AttackType t, Network& net) {
    current = t;
    src = std::rand() % 6;     // a random client PC is the attacker
    index = 0;
    timer = 0;

    switch (t) {
        case ATK_FLOOD:     remaining = 40; interval = 0.07f; break;
        case ATK_PORTSCAN:  remaining = 14; interval = 0.15f; break;
        case ATK_MALWARE:   remaining = 6;  interval = 0.5f;  break;
        case ATK_BRUTE:     remaining = 10; interval = 0.3f;  break;
        case ATK_OVERSIZE:  remaining = 5;  interval = 0.4f;  break;
        case ATK_BLACKLIST: remaining = 5;  interval = 0.4f;  break;
        default:            remaining = 0;  break;
    }

    char buf[96];
    std::snprintf(buf, sizeof(buf), "ATTACK: %s from %s", name(t), net.nodes[src].name.c_str());
    net.log.add(buf, 1.0f, 0.55f, 0.1f);
}

void AttackGenerator::spawnOne(Network& net) {
    int dest = (current == ATK_BRUTE) ? Network::INTERNAL_IDX : Network::DMZ_IDX;
    Packet p = net.createPacket(src, dest);
    p.isAttack = true;

    switch (current) {
        case ATK_FLOOD:
            p.protocol = "UDP";
            p.port = 80;
            p.size = 64 + std::rand() % 64;
            break;
        case ATK_PORTSCAN: {
            static const int ports[] = {21, 22, 23, 25, 53, 80, 110, 135, 139, 443, 445, 3306, 3389, 8080};
            p.port = ports[index % 14];
            p.size = 60;
            break;
        }
        case ATK_MALWARE:
            p.port = 443;
            p.signature = "MALWARE-X1";
            break;
        case ATK_BRUTE:
            p.port = 22;
            p.size = 120;
            p.failedLogin = true;
            break;
        case ATK_OVERSIZE:
            p.size = 9000;
            break;
        case ATK_BLACKLIST:
            p.srcIP = "66.66.66.66";
            break;
        default:
            break;
    }
    net.spawnPacket(p);
}

void AttackGenerator::update(float dt, Network& net) {
    if (!active()) return;
    timer -= dt;
    while (timer <= 0 && remaining > 0) {
        spawnOne(net);
        remaining--;
        index++;
        timer += interval;
    }
    if (remaining <= 0) current = ATK_NONE;
}
