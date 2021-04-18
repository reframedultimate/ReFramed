#pragma once

#include <vector>
#include <unordered_set>

namespace uh {

class DecisionGraph
{
public:
    struct Edge
    {
        int from, to;
        uint32_t frame;
        float damage;
    };

public:
    void addNode(uint16_t state);
    void addEdge(uint16_t from, uint16_t to, uint32_t frame, float damage);

private:
    std::unordered_set<uint16_t> states_;
    std::vector<Edge> edges_;
};

}
