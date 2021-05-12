#include "application/Util.hpp"
#include "application/listeners/RecordingGroupListener.hpp"
#include "application/models/RecordingGroup.hpp"

#include <QDebug>

namespace uhapp {

// ----------------------------------------------------------------------------
RecordingGroup::RecordingGroup(const QString& name)
    : name_(name)
{
}

// ----------------------------------------------------------------------------
const QSet<QFileInfo>& RecordingGroup::absFilePathList() const
{
    return fileList_;
}

// ----------------------------------------------------------------------------
const QString& RecordingGroup::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
void RecordingGroup::setName(const QString& name)
{
    name_ = name;
}

// ----------------------------------------------------------------------------
void RecordingGroup::addFile(const QFileInfo& absPathToFile)
{
    if (fileList_.contains(absPathToFile))
        return;

    fileList_.insert(absPathToFile);
    dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, this, absPathToFile);
}

// ----------------------------------------------------------------------------
bool RecordingGroup::removeFile(const QFileInfo& absPathToFile)
{
    for (auto it = fileList_.begin(); it != fileList_.end(); ++it)
        if (*it == absPathToFile)
        {
            fileList_.erase(it);
            dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileRemoved, this, absPathToFile);
            return true;
        }
    return false;
}

// ----------------------------------------------------------------------------
void RecordingGroup::removeAllFiles()
{
    for (const auto& fileInfo : fileList_)
        dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileRemoved, this, fileInfo);
    fileList_.clear();
}

}
