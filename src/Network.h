// Network.h - UPDATED for Week 2
#pragma once
#include <vector>
#include <utility>
#include "Node.h"
#include "Packet.h"
#include "Firewall.h"
#include "Logger.h"

enum SecurityStatus { STATUS_SAFE, STATUS_WARNING, STATUS_CRITICAL };

class Network {
public:
    // node index layout: 0-5 clients, 6 firewall, 7 DMZ web server, 8 internal server
    static constexpr int FIREWALL_IDX = 6;
    static constexpr int DMZ_IDX      = 7;
    static constexpr int INTERNAL_IDX = 8;

    std::vector<Node> nodes;
    std::vector<std::pair<int,int>> links;
    std::vector<Packet> packets;

    Firewall firewall;
    EventLog log;

    // statistics (counted when a packet reaches the firewall)
    int totalPackets = 0;
    int allowedCount = 0;
    int blockedCount = 0;
    int rateLimitedCount = 0;

    float simTime = 0;
    float threatLevel = 0;      // rises on block/rate-limit, decays over time

    void init();
    void reset();
    void draw() const;
    void update(float dt);      // dt in seconds

    Packet createPacket(int client, int dest);   // normal-looking packet
    void spawnPacket(const Packet& p);
    void sendNormalPacket();

    SecurityStatus status() const;

private:
    void drawNode(const Node& n) const;
    void drawPacket(const Packet& p) const;
    void onReachFirewall(Packet& p);
};
