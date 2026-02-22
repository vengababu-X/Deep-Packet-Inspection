#ifndef FLOW_TRACKER_H
#define FLOW_TRACKER_H

#include <string>
#include <map>

struct FlowKey {
    std::string src, dst;
    int sport, dport;
    int proto;

    bool operator<(const FlowKey& other) const {
        return std::tie(src, dst, sport, dport, proto) <
               std::tie(other.src, other.dst, other.sport, other.dport, other.proto);
    }
};

struct FlowData {
    int packets = 0;
    int bytes = 0;
    std::string app;
};

class FlowTracker {
public:
    void update(const FlowKey& key, int bytes, const std::string& app);
    void print();
private:
    std::map<FlowKey, FlowData> flows;
};

#endif