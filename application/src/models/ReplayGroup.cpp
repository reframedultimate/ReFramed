#include "application/Util.hpp"
#include "application/listeners/ReplayGroupListener.hpp"
#include "application/models/ReplayGroup.hpp"

#include "rfcommon/Profiler.hpp"

#include <QDir>
#include <cassert>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayGroup::ReplayGroup(const QString& name)
    : name_(name)
{
}

// ----------------------------------------------------------------------------
const QSet<QString>& ReplayGroup::fileNames() const
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
void ReplayGroup::addFile(const QString& fileName)
{
    PROFILE(ReplayGroup, addFile);
    assert(QDir(fileName).isRelative());

    if (fileList_.contains(fileName))
        return;

    fileList_.insert(fileName);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileAdded, this, fileName);
}

// ----------------------------------------------------------------------------
bool ReplayGroup::removeFile(const QString& fileName)
{
    PROFILE(ReplayGroup, removeFile);
    assert(QDir(fileName).isRelative());

    auto it = fileList_.find(fileName);
    if (it == fileList_.end())
        return false;

    fileList_.erase(it);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, fileName);
    return true;
}

// ----------------------------------------------------------------------------
void ReplayGroup::removeAllFiles()
{
    PROFILE(ReplayGroup, removeAllFiles);

    for (const auto& fileInfo : fileList_)
        dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, fileInfo);
    fileList_.clear();
}

// ----------------------------------------------------------------------------
bool ReplayGroup::isInGroup(const QString& fileName) const
{
    assert(QDir(fileName).isRelative());
    return fileList_.contains(fileName);
}

}
