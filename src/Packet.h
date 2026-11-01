// Packet.h - Person B
#pragma once
#include <string>
#include <vector>

enum PacketState { MOVING, ALLOWED, BLOCKED, RATE_LIMITED, DONE };

struct Packet {
    // header info (used by firewall in Week 2)
    std::string srcIP, dstIP;
    int port;
    std::string protocol;     // "TCP", "UDP", "ICMP"
    int size;                 // bytes
    std::string signature;    // payload signature, "" = clean

    // animation
    float x, y;
    float speed;              // pixels per second
    std::vector<int> path;    // node indices, e.g. {client, firewall, server}
    int pathIdx;              // index of the node we just left/are at
    PacketState state;
};