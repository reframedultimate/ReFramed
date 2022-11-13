#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/GameNumber.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API ScoreCount
{
public:
    ~ScoreCount();

    static ScoreCount fromGameNumber(GameNumber gameNumber);
    static ScoreCount fromScore(int p1, int p2);

    int left() const { return p1_; }
    int right() const { return p2_; }

    GameNumber gameNumber() const { return gameNumber_; }

    bool operator==(const ScoreCount& rhs) const;
    bool operator!=(const ScoreCount& rhs) const;

private:
    ScoreCount(int p1, int p2, int gameNumber);

private:
    int p1_, p2_;
    GameNumber gameNumber_;
};

}
