// Challenge.h - Defender Challenge mode (press C)
// 6 waves of attacks. All rules R1-R6 start OFF; the player must switch on the right ones.
#pragma once
#include "Firewall.h"
#include "Packet.h"
#include "AttackGenerator.h"

class Network;

class Challenge {
public:
    enum State { IDLE, BREAK, WAVE, FINISHED, FAILED };

    State state = IDLE;
    int score = 0;
    int integrity = 100;      // server integrity (%)
    int wave = 0;             // 0-based index of current/next wave
    int stopped = 0;          // attack packets stopped by the firewall
    int leaked = 0;           // attack packets that got through
    int falseBlocks = 0;      // normal packets wrongly blocked / limited

    void start(Network& net, AttackGenerator& atk);
    void stop();
    bool active() const { return state != IDLE; }

    void update(float dt, Network& net, AttackGenerator& atk);
    void onDecision(const Packet& p, FwAction a);   // called by Network when firewall decides
    void draw() const;                              // overlay panel
    char rating() const;

private:
    float timer = 0;
    float trafficTimer = 0;
    AttackType curType = ATK_NONE;

    bool isThreat(const Packet& p) const;
    int  leakDamage() const;
};