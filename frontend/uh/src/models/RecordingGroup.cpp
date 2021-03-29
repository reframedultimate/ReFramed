#include "uh/listeners/RecordingGroupListener.hpp"
#include "uh/models/RecordingGroup.hpp"

namespace uh {

// ----------------------------------------------------------------------------
RecordingGroup::RecordingGroup(const QString& name)
    : name_(name)
{
}

// ----------------------------------------------------------------------------
const QVector<QDir>& RecordingGroup::fileList() const
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
void RecordingGroup::addFile(const QDir& pathToFile)
{
    fileList_.push_back(pathToFile);
    dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, pathToFile);
}

// ----------------------------------------------------------------------------
bool RecordingGroup::removeFile(const QDir& pathToFile)
{
    for (auto it = fileList_.begin(); it != fileList_.end(); ++it)
        if (*it == pathToFile)
        {
            fileList_.erase(it);
            dispatcher.dispatch(&RecordingGroupListener::onRecordingGroupFileAdded, pathToFile);
            return true;
        }
    return false;
}

}
