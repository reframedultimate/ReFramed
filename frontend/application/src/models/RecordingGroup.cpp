#include "application/Util.hpp"
#include "application/listeners/RecordingGroupListener.hpp"
#include "application/models/RecordingGroup.hpp"

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
    QString oldName = name_;
    name_ = name;
    dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupNameChanged, this, oldName, name);
}

// ----------------------------------------------------------------------------
void RecordingGroup::addFile(const QFileInfo& absPathToFile)
{
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
            dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, this, absPathToFile);
            return true;
        }
    return false;
}

// ----------------------------------------------------------------------------
void RecordingGroup::removeAllFiles()
{
    fileList_.clear();
}

}
