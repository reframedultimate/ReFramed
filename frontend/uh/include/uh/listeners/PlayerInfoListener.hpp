#pragma once

#include <cstdint>

namespace uh {

class PlayerInfo;

class PlayerInfoListener
{
public:
    virtual void onPlayerInfoTagChanged(const PlayerInfo& playerInfo) = 0;
};

}
