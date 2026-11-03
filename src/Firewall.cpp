// Firewall.cpp
#include "Firewall.h"
#include <algorithm>

void Firewall::init() {
    rules = {
        {"R1", "Source IP in blacklist",         true},
        {"R2", "Dest port in {23,445,3389}",     true},
        {"R3", "Packet size above limit",        true},
        {"R4", "Rate limit per source",          true},
        {"R5", "Payload signature match",        true},
        {"R6", "Repeated failed logins",         true},
        {"R7", "Everything else: ALLOW",         true},
    };
    blacklist = {"66.66.66.66"};
    sources.clear();
    failedLogins.clear();
}

bool Firewall::isBlacklisted(const std::string& ip) const {
    return std::find(blacklist.begin(), blacklist.end(), ip) != blacklist.end();
}

bool Firewall::isKnownSignature(const std::string& sig) const {
    return sig == "MALWARE-X1" || sig == "TROJAN-77" || sig == "EVIL-PAYLOAD";
}

Decision Firewall::inspect(const Packet& p, float now) {
    // R1: blacklisted source
    if (rules[0].enabled && isBlacklisted(p.srcIP))
        return {ACT_BLOCK, "R1", "Blacklisted host"};

    // R2: risky destination ports
    if (rules[1].enabled && (p.port == 23 || p.port == 445 || p.port == 3389))
        return {ACT_BLOCK, "R2", "Port scan / risky service"};

    // R3: oversized packet
    if (rules[2].enabled && p.size > maxSize)
        return {ACT_BLOCK, "R3", "Oversized / flood"};

    // R4: rate limit per source, then auto-block
    if (rules[3].enabled) {
        SrcInfo& s = sources[p.srcIP];
        if (s.banUntil > 0) {
            if (now < s.banUntil)
                return {ACT_BLOCK, "R4", "DDoS flood (source banned)"};
            s.banUntil = 0; s.strikes = 0; s.count = 0; s.windowStart = now;  // ban expired
        }
        if (now - s.windowStart > windowSec) { s.windowStart = now; s.count = 0; }
        s.count++;
        if (s.count > rateLimitN) {
            s.strikes++;
            if (s.strikes >= banStrikes) {
                s.banUntil = now + banSeconds;
                return {ACT_BLOCK, "R4", "DDoS flood (source banned)"};
            }
            return {ACT_RATE_LIMIT, "R4", "DDoS flood"};
        }
    }

    // R5: malware signature
    if (rules[4].enabled && !p.signature.empty() && isKnownSignature(p.signature))
        return {ACT_BLOCK, "R5", "Malware"};

    // R6: repeated failed logins
    if (rules[5].enabled && p.failedLogin) {
        int& f = failedLogins[p.srcIP];
        f++;
        if (f > maxFailedLogins)
            return {ACT_BLOCK, "R6", "Brute force"};
    }

    // R7: default
    return {ACT_ALLOW, "R7", "Normal traffic"};
}
