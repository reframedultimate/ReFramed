#pragma once

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ActiveSessionManagerListener
{
public:
    virtual void onActiveSessionManagerGameStarted(rfcommon::Session* game) = 0;
    virtual void onActiveSessionManagerGameEnded(rfcommon::Session* game) = 0;
    virtual void onActiveSessionManagerTrainingStarted(rfcommon::Session* training) = 0;
    virtual void onActiveSessionManagerTrainingEnded(rfcommon::Session* training) = 0;
};

}
