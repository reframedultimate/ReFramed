#include "uh/views/CategoryView.hpp"
#include "uh/models/RecordingManager.hpp"
#include "uh/models/RecordingGroup.hpp".hpp"

namespace uh {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(RecordingManager* recordingManager, QWidget* parent)
    : QTreeWidget(parent)
    , recordingManager_(recordingManager)
    , analysisCategoryItem_(new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL)))
    , recordingGroupsCategoryItem_(new QTreeWidgetItem({"Recording Groups"}, static_cast<int>(CategoryType::TOP_LEVEL)))
    , recordingSourcesCategoryItem_(new QTreeWidgetItem({"Recording Sources"}, static_cast<int>(CategoryType::TOP_LEVEL)))
    , videoSourcesCategoryItem_(new QTreeWidgetItem({"Video Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL)))
    , activeRecordingCategoryItem_(new QTreeWidgetItem({"Active Recording"}, static_cast<int>(CategoryType::TOP_LEVEL)))
{
    setHeaderHidden(true);

    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(recordingGroupsCategoryItem_);
    addTopLevelItem(recordingSourcesCategoryItem_);
    addTopLevelItem(videoSourcesCategoryItem_);
    addTopLevelItem(activeRecordingCategoryItem_);

    setActiveRecordingViewDisabled(true);

    for (const auto& group : recordingManager->recordingGroups())
        onRecordingManagerGroupAdded(group.second.get());

    recordingGroupsCategoryItem_->setExpanded(true);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

// ----------------------------------------------------------------------------
void CategoryView::setActiveRecordingViewDisabled(bool disable)
{
    activeRecordingCategoryItem_->setDisabled(disable);
}

// ----------------------------------------------------------------------------
void CategoryView::onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current == analysisCategoryItem_)
        emit categoryChanged(CategoryType::ANALYSIS);
    else if (current == recordingGroupsCategoryItem_)
        emit categoryChanged(CategoryType::RECORDING_GROUPS);
    else if (current == recordingSourcesCategoryItem_)
        emit categoryChanged(CategoryType::RECORDING_SOURCES);
    else if (current == videoSourcesCategoryItem_)
        emit categoryChanged(CategoryType::VIDEO_SOURCES);
    else if (current == activeRecordingCategoryItem_)
        emit categoryChanged(CategoryType::ACTIVE_RECORDING);
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerDefaultRecordingLocationChanged(const QDir& path)
{
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupAdded(RecordingGroup* group)
{
    recordingGroupsCategoryItem_->addChild(new QTreeWidgetItem({group->name()}, static_cast<int>(CategoryType::RECORDING_GROUPS)));
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupRemoved(RecordingGroup* group)
{
    for (int i = 0; i != recordingGroupsCategoryItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = recordingGroupsCategoryItem_->child(i);
        if (item->text(0) == group->name())
            recordingGroupsCategoryItem_->removeChild(item);
        else
            i++;
    }
}

}
