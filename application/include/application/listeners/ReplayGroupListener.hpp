#pragma once

#include <QString>

namespace rfcommon {
    class ReplayFileParts;
}

namespace rfapp {

class ReplayGroup;

class ReplayGroupListener
{
public:
    virtual void onReplayGroupFileAdded(ReplayGroup* group, const rfcommon::ReplayFileParts& fileName) = 0;
    virtual void onReplayGroupFileRemoved(ReplayGroup* group, const rfcommon::ReplayFileParts& fileName) = 0;
};

}
