#pragma once

#include <QString>
#include <QFileInfo>

namespace rfapp {

class ReplayGroup;

class ReplayGroupListener
{
public:
    virtual void onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile) = 0;
    virtual void onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile) = 0;
};

}
