// Packet.h - UPDATED for Week 3
#pragma once
#include <string>
#include <vector>

enum PacketState { MOVING, ALLOWED, BLOCKED, RATE_LIMITED, DONE };

struct Packet {
    int id = 0;                // unique id (shown in the inspector)

    // header info
    std::string srcIP, dstIP;
    int port = 0;
    std::string protocol;      // "TCP", "UDP", "ICMP"
    int size = 0;              // bytes
    std::string signature;     // payload signature, "" = clean
    bool failedLogin = false;  // simulated failed login attempt (brute force)
    bool isAttack = false;     // true = packet comes from an attack scenario

    // firewall result (used by the Week 3 packet inspector)
    std::string matchedRule;   // "R1".."R7"
    std::string threat;        // threat label

    // animation
    float x = 0, y = 0;
    float speed = 0;           // pixels per second
    std::vector<int> path;     // node indices, e.g. {client, firewall, server}
    int pathIdx = 0;
    PacketState state = MOVING;
    float timer = 0;           // fade-out timer for blocked packets
};