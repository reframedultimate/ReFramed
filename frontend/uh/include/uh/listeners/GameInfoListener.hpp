#pragma once

#include <cstdint>

namespace uh {

class GameInfo;

class GameInfoListener
{
public:
    virtual void onGameInfoNumberChanged(const GameInfo& gameInfo) = 0;
    virtual void onGameInfoFormatChanged(const GameInfo& gameInfo) = 0;
};

}
