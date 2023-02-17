#pragma once

#include <QString>

namespace rfapp {

class ReplayGroup;

class ReplayGroupListener
{
public:
    //! A new file was added to the group. This will never be a path, only filename.
    virtual void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) = 0;
    virtual void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) = 0;
};

}
