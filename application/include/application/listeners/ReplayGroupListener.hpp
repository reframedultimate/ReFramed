#pragma once

#include <QString>
#include <QFileInfo>

namespace rfapp {

class ReplayGroup;

class ReplayGroupListener
{
public:
    virtual void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) = 0;
    virtual void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) = 0;
};

}
