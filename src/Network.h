// Network.h - Person B
#pragma once
#include <vector>
#include <utility>
#include "Node.h"
#include "Packet.h"

class Network {
public:
    // node index layout: 0-5 clients, 6 firewall, 7 DMZ web server, 8 internal server
    static constexpr int FIREWALL_IDX = 6;
    static constexpr int DMZ_IDX      = 7;
    static constexpr int INTERNAL_IDX = 8;
    
    std::vector<Node> nodes;
    std::vector<std::pair<int,int>> links;
    std::vector<Packet> packets;

    int totalPackets = 0;

    void init();
    void reset();
    void draw() const;
    void update(float dt);          // dt in seconds
    void sendNormalPacket();

private:
    void drawNode(const Node& n) const;
    void drawPacket(const Packet& p) const;
    void onReachFirewall(Packet& p); // Week 2: firewall rules go here
};