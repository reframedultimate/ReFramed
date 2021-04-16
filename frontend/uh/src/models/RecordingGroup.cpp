#include "uh/Util.hpp"
#include "uh/listeners/RecordingGroupListener.hpp"
#include "uh/models/RecordingGroup.hpp"

namespace uh {

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
    dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupNameChanged, name);
}

// ----------------------------------------------------------------------------
void RecordingGroup::addFile(const QFileInfo& absPathToFile)
{
    fileList_.insert(absPathToFile);
    dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, absPathToFile);
}

// ----------------------------------------------------------------------------
bool RecordingGroup::removeFile(const QFileInfo& absPathToFile)
{
    for (auto it = fileList_.begin(); it != fileList_.end(); ++it)
        if (*it == absPathToFile)
        {
            fileList_.erase(it);
            dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, absPathToFile);
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
