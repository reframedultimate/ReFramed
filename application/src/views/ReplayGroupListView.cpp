#include "rfcommon/Profiler.hpp"
#include "application/views/ReplayGroupListView.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/models/ReplayGroup.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayGroupListView::ReplayGroupListView(ReplayManager* replayManager, QWidget* parent)
    : QListWidget(parent)
    , replayManager_(replayManager)
{
    replayManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ReplayGroupListView::~ReplayGroupListView()
{
    replayManager_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ReplayGroupListView::onReplayManagerGroupAdded(ReplayGroup* group)
{
    PROFILE(ReplayGroupListView, onReplayManagerGroupAdded);

    addItem(group->name());
}

// ----------------------------------------------------------------------------
void ReplayGroupListView::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName)
{
    PROFILE(ReplayGroupListView, onReplayManagerGroupNameChanged);

    for (int i = 0; i != count(); ++i)
    {
        QListWidgetItem* it = item(i);
        if (it->text() == group->name())
        {
            it->setText(newName);
            return;
        }
    }
}

// ----------------------------------------------------------------------------
void ReplayGroupListView::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    PROFILE(ReplayGroupListView, onReplayManagerGroupRemoved);

    for (int i = 0; i != count(); ++i)
    {
        QListWidgetItem* it = item(i);
        if (it->text() == group->name())
        {
            delete it;
            return;
        }
    }
}

}
