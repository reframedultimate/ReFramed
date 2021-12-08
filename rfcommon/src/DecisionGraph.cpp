#include "rfcommon/DecisionGraph.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
void DecisionGraph::addNode(uint16_t state)
{
    states_.insertOrGet(state, 0);
}

// ----------------------------------------------------------------------------
void DecisionGraph::addEdge(uint16_t from, uint16_t to, uint32_t frame, float damage)
{
    assert(states_.find(from) != states_.end());
    assert(states_.find(to) != states_.end());
    edges_.push(Edge{from, to, frame, damage});
}

}
