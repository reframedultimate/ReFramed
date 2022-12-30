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
const QSet<QString>& ReplayGroup::files() const
{
    PROFILE(ReplayGroup, files);

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
bool ReplayGroup::addFile(const QString& file)
{
    PROFILE(ReplayGroup, addFile);

    assert(QDir(file).isRelative());

    if (files_.contains(file))
        return false;

    files_.insert(file);
    dispatcher.dispatch(&ReplayGroupListener::onReplayGroupFileAdded, this, file);
    return true;
}

// ----------------------------------------------------------------------------
bool ReplayGroup::removeFile(const QString& file)
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
bool ReplayGroup::isInGroup(const QString& file) const
{
    PROFILE(ReplayGroup, isInGroup);

    return files_.contains(file);
}

}
