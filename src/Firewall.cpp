// Firewall.cpp
#include "Firewall.h"
#include <algorithm>
#include <fstream>
#include <sstream>

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
    riskyPorts = {23, 445, 3389};
    signatures = {"MALWARE-X1", "TROJAN-77", "EVIL-PAYLOAD"};
    maxSize = 1500; rateLimitN = 5; windowSec = 1.0f;
    banStrikes = 6; banSeconds = 10.0f; maxFailedLogins = 3;
    sources.clear();
    failedLogins.clear();
}

bool Firewall::isBlacklisted(const std::string& ip) const {
    return std::find(blacklist.begin(), blacklist.end(), ip) != blacklist.end();
}

bool Firewall::isKnownSignature(const std::string& sig) const {
    return std::find(signatures.begin(), signatures.end(), sig) != signatures.end();
}

Decision Firewall::inspect(const Packet& p, float now) {
    // R1: blacklisted source
    if (rules[0].enabled && isBlacklisted(p.srcIP))
        return {ACT_BLOCK, "R1", "Blacklisted host"};

    // R2: risky destination ports
    if (rules[1].enabled && std::find(riskyPorts.begin(), riskyPorts.end(), p.port) != riskyPorts.end())
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

const Rule* Firewall::findRule(const std::string& id) const {
    for (const auto& r : rules) if (r.id == id) return &r;
    return nullptr;
}

// ---------- Week 3: config file ----------
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::vector<std::string> splitList(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) out.push_back(item);
    }
    return out;
}

// Format: key = value, one per line. Lines starting with # are comments.
// Returns false if the file could not be opened (defaults stay in use).
bool Firewall::loadConfig(const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        try {
            if      (key == "maxSize")         maxSize = std::stoi(val);
            else if (key == "rateLimitN")      rateLimitN = std::stoi(val);
            else if (key == "windowSec")       windowSec = std::stof(val);
            else if (key == "banStrikes")      banStrikes = std::stoi(val);
            else if (key == "banSeconds")      banSeconds = std::stof(val);
            else if (key == "maxFailedLogins") maxFailedLogins = std::stoi(val);
            else if (key == "blacklist")       blacklist = splitList(val);
            else if (key == "signatures")      signatures = splitList(val);
            else if (key == "ports") {
                riskyPorts.clear();
                for (const auto& t : splitList(val)) riskyPorts.push_back(std::stoi(t));
            }
            else if (key.size() == 2 && key[0] == 'R') {          // R1=on / R3=off
                for (auto& r : rules)
                    if (r.id == key) r.enabled = (val == "on" || val == "1" || val == "true");
            }
        } catch (...) { /* bad number: ignore that line */ }
    }
    return true;
}