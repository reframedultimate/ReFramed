#pragma once

#include "uh/config.hpp"
#include "uh/Vector.hpp"
#include "uh/HashMap.hpp"

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
    HashMap<uint16_t, char> states_;
    Vector<Edge> edges_;
};

}
