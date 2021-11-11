#include "application/Util.hpp"
#include "application/listeners/SavedGameSessionGroupListener.hpp"
#include "application/models/SavedGameSessionGroup.hpp"

#include <QDebug>

namespace uhapp {

// ----------------------------------------------------------------------------
SavedGameSessionGroup::SavedGameSessionGroup(const QString& name)
    : name_(name)
{
}

// ----------------------------------------------------------------------------
const QSet<QFileInfo>& SavedGameSessionGroup::absFilePathList() const
{
    return fileList_;
}

// ----------------------------------------------------------------------------
const QString& SavedGameSessionGroup::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
void SavedGameSessionGroup::setName(const QString& name)
{
    name_ = name;
}

// ----------------------------------------------------------------------------
void SavedGameSessionGroup::addFile(const QFileInfo& absPathToFile)
{
    if (fileList_.contains(absPathToFile))
        return;

    fileList_.insert(absPathToFile);
    dispatcher.dispatch(&SavedGameSessionGroupListener::onSavedGameSessionGroupFileAdded, this, absPathToFile);
}

// ----------------------------------------------------------------------------
bool SavedGameSessionGroup::removeFile(const QFileInfo& absPathToFile)
{
    for (auto it = fileList_.begin(); it != fileList_.end(); ++it)
        if (*it == absPathToFile)
        {
            fileList_.erase(it);
            dispatcher.dispatch(&SavedGameSessionGroupListener::onSavedGameSessionGroupFileRemoved, this, absPathToFile);
            return true;
        }
    return false;
}

// ----------------------------------------------------------------------------
void SavedGameSessionGroup::removeAllFiles()
{
    for (const auto& fileInfo : fileList_)
        dispatcher.dispatch(&SavedGameSessionGroupListener::onSavedGameSessionGroupFileRemoved, this, fileInfo);
    fileList_.clear();
}

}
