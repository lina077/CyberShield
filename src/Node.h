// Node.h - Person B
#pragma once
#include <string>

enum NodeType { CLIENT, FIREWALL, DMZ_SERVER, INTERNAL_SERVER };

struct Node {
    float x, y;
    NodeType type;
    std::string name;
};