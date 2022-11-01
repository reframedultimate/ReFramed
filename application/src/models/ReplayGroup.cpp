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
const QVector<rfcommon::ReplayFileParts>& ReplayGroup::files() const
{
    PROFILE(ReplayGroup, fileNames);

    return files_;
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
bool ReplayGroup::addFile(const rfcommon::ReplayFileParts& file)
{
    PROFILE(ReplayGroup, addFile);

    if (files_.contains(file))
        return false;

    files_.push_back(file);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileAdded, this, file);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayGroup::removeFile(const rfcommon::ReplayFileParts& file)
{
    PROFILE(ReplayGroup, removeFile);

    for (auto it = files_.begin(); it != files_.end(); ++it)
        if (*it == file)
        {
            files_.erase(it);
            dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, file);
            return true;
        }

    return false;
}

// ----------------------------------------------------------------------------
void ReplayGroup::removeAllFiles()
{
    PROFILE(ReplayGroup, removeAllFiles);

    for (const auto& file : files_)
        dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileRemoved, this, file);
    files_.clear();
}

// ----------------------------------------------------------------------------
bool ReplayGroup::isInGroup(const rfcommon::ReplayFileParts& file) const
{
    return files_.contains(file);
}

}
