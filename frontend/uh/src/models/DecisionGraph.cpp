#include "uh/models/DecisionGraph.hpp"

namespace uh {

// ----------------------------------------------------------------------------
void DecisionGraph::addNode(uint16_t state)
{
    states_.insert(state);
}

// ----------------------------------------------------------------------------
void DecisionGraph::addEdge(uint16_t from, uint16_t to, uint32_t frame, float damage)
{
    assert(states_.find(from) != states_.end());
    assert(states_.find(to) != states_.end());
    edges_.push_back(Edge{from, to, frame, damage});
}

}
