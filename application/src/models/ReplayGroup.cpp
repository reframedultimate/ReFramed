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
    return fileList_;
}

// ----------------------------------------------------------------------------
const QString& ReplayGroup::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
void ReplayGroup::setName(const QString& name)
{
    name_ = name;
}

// ----------------------------------------------------------------------------
void ReplayGroup::addFile(const QFileInfo& absPathToFile)
{
    if (fileList_.contains(absPathToFile))
        return;

    fileList_.insert(absPathToFile);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileAdded, this, absPathToFile);
}

// ----------------------------------------------------------------------------
bool ReplayGroup::removeFile(const QFileInfo& absPathToFile)
{
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
    for (const auto& fileInfo : fileList_)
        dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, fileInfo);
    fileList_.clear();
}

}
