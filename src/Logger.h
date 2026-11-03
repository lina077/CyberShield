// Logger.h - event log (header only)
#pragma once
#include <deque>
#include <string>

struct LogEntry {
    std::string text;
    float r, g, b;
};

class EventLog {
public:
    std::deque<LogEntry> entries;

    void add(const std::string& text, float r, float g, float b) {
        entries.push_back({text, r, g, b});
        if (entries.size() > 200) entries.pop_front();
    }
    void clear() { entries.clear(); }
};
