#pragma once

namespace rfcommon {
    class Session;
}

class FrameDataListListener
{
public:
    virtual void onFrameDataListSessionSet(rfcommon::Session* session) = 0;
    virtual void onFrameDataListSessionCleared(rfcommon::Session* session) = 0;
};
