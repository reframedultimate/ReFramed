#pragma once

#include <cstdint>

namespace uh {

class PlayerState;

class RecordingListener
{
public:
    virtual void onRecordingPlayerStateAdded(int playerID, const PlayerState& state) = 0;
};

}
