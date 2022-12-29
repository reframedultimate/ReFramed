#include "rfcommon/ScoreCount.hpp"
#include "rfcommon/Profiler.hpp"

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
    PROFILE(ScoreCount, fromGameNumber);

    return ScoreCount(0, 0, gameNumber.value());
}

// ----------------------------------------------------------------------------
ScoreCount ScoreCount::fromScore(int p1, int p2)
{
    PROFILE(ScoreCount, fromScore);

    return ScoreCount(p1, p2, p1 + p2 + 1);
}

// ----------------------------------------------------------------------------
ScoreCount ScoreCount::fromScoreAndGameNumber(int p1, int p2, GameNumber gameNumber)
{
    PROFILE(ScoreCount, fromScoreAndGameNumber);

    return ScoreCount(p1, p2, gameNumber.value());
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
