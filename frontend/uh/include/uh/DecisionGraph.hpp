#pragma once

#include <QVector>
#include <QSet>

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
    QSet<uint16_t> states_;
    QVector<Edge> edges_;
};

}
