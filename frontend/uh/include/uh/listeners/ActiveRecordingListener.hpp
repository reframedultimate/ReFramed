#pragma once

namespace uh {

class PlayerState;

class ActiveRecordingListener
{
public:
    virtual void onActiveRecordingPlayerStateAdded(int player, const PlayerState& state) = 0;
};

}
