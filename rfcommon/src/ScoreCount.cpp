#include "rfcommon/ScoreCount.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
ScoreCount::ScoreCount(int p1, int p2, int gameNumber)
    : p1_(p1)
    , p2_(p2)
    , gameNumber_(GameNumber::fromValue(gameNumber))
{}

// ----------------------------------------------------------------------------
ScoreCount::~ScoreCount()
{}

// ----------------------------------------------------------------------------
ScoreCount ScoreCount::fromGameNumber(GameNumber gameNumber)
{
    return ScoreCount(0, 0, gameNumber.value());
}

// ----------------------------------------------------------------------------
ScoreCount ScoreCount::fromScore(int p1, int p2)
{
    return ScoreCount(p1, p2, p1 + p2 + 1);
}

// ----------------------------------------------------------------------------
bool ScoreCount::operator==(const ScoreCount& rhs) const
{
    return p1_ == rhs.p1_ && p2_ == rhs.p2_;
}
bool ScoreCount::operator!=(const ScoreCount& rhs) const
{
    return !operator==(rhs);
}

}
