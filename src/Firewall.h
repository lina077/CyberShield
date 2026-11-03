// Firewall.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include "Packet.h"

enum FwAction { ACT_ALLOW, ACT_BLOCK, ACT_RATE_LIMIT };

struct Decision {
    FwAction action;
    std::string ruleId;
    std::string threat;
};

struct Rule {
    std::string id;
    std::string description;
    bool enabled;       // Week 3: toggle at runtime
};

class Firewall {
public:
    std::vector<Rule> rules;               // R1..R7, checked in this order
    std::vector<std::string> blacklist;
    std::vector<int> riskyPorts;            // R2
    std::vector<std::string> signatures;    // R5

    int   maxSize         = 1500;   // R3
    int   rateLimitN      = 5;      // R4: packets per window per source
    float windowSec       = 1.0f;
    int   banStrikes      = 6;      // rate-limited this many times -> blocked
    float banSeconds      = 10.0f;
    int   maxFailedLogins = 3;      // R6

    void init();
    bool loadConfig(const std::string& path);   // Week 3: read config/firewall.cfg
    const Rule* findRule(const std::string& id) const;
    Decision inspect(const Packet& p, float now);

private:
    struct SrcInfo {
        int count = 0;
        float windowStart = 0;
        int strikes = 0;
        float banUntil = 0;
    };
    std::map<std::string, SrcInfo> sources;
    std::map<std::string, int> failedLogins;

    bool isBlacklisted(const std::string& ip) const;
    bool isKnownSignature(const std::string& sig) const;
};