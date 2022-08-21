#include "rfcommon/Profiler.hpp"
#include "application/Util.hpp"
#include "application/listeners/ReplayGroupListener.hpp"
#include "application/models/ReplayGroup.hpp"

#include <QDebug>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayGroup::ReplayGroup(const QString& name)
    : name_(name)
{
}

// ----------------------------------------------------------------------------
const QSet<QFileInfo>& ReplayGroup::absFilePathList() const
{
    PROFILE(ReplayGroup, absFilePathList);

    return fileList_;
}

// ----------------------------------------------------------------------------
const QString& ReplayGroup::name() const
{
    PROFILE(ReplayGroup, name);

    return name_;
}

// ----------------------------------------------------------------------------
void ReplayGroup::setName(const QString& name)
{
    PROFILE(ReplayGroup, setName);

    name_ = name;
}

// ----------------------------------------------------------------------------
void ReplayGroup::addFile(const QFileInfo& absPathToFile)
{
    PROFILE(ReplayGroup, addFile);

    if (fileList_.contains(absPathToFile))
        return;

    fileList_.insert(absPathToFile);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileAdded, this, absPathToFile);
}

// ----------------------------------------------------------------------------
bool ReplayGroup::removeFile(const QFileInfo& absPathToFile)
{
    PROFILE(ReplayGroup, removeFile);

    for (auto it = fileList_.begin(); it != fileList_.end(); ++it)
        if (*it == absPathToFile)
        {
            fileList_.erase(it);
            dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, absPathToFile);
            return true;
        }
    return false;
}

// ----------------------------------------------------------------------------
void ReplayGroup::removeAllFiles()
{
    PROFILE(ReplayGroup, removeAllFiles);

    for (const auto& fileInfo : fileList_)
        dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, fileInfo);
    fileList_.clear();
}

}
