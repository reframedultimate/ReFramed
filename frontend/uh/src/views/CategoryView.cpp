#include "uh/views/CategoryView.hpp"
#include "uh/models/RecordingManager.hpp"
#include "uh/models/RecordingGroup.hpp"

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
    auto categoryOf = [this](const QTreeWidgetItem* item) -> CategoryType {
        if (item == analysisCategoryItem_)
            return CategoryType::ANALYSIS;
        if (item == recordingGroupsCategoryItem_ || item->parent() == recordingGroupsCategoryItem_)
            return CategoryType::RECORDING_GROUPS;
        if (item == recordingSourcesCategoryItem_)
            return CategoryType::RECORDING_SOURCES;
        if (item == videoSourcesCategoryItem_)
            return CategoryType::VIDEO_SOURCES;
        if (item == activeRecordingCategoryItem_)
            return CategoryType::ACTIVE_RECORDING;
        assert(false);
    };

#define EMIT_IF_CHANGED(category) \
    if (categoryOf(current) == category && (previous == nullptr || categoryOf(previous) != category)) \
        emit categoryChanged(category);
    EMIT_IF_CHANGED(CategoryType::ANALYSIS)
    EMIT_IF_CHANGED(CategoryType::RECORDING_GROUPS)
    EMIT_IF_CHANGED(CategoryType::RECORDING_SOURCES)
    EMIT_IF_CHANGED(CategoryType::VIDEO_SOURCES)
    EMIT_IF_CHANGED(CategoryType::ACTIVE_RECORDING)
#undef EMIT_IF_CHANGED

    if (current->parent() == recordingGroupsCategoryItem_)
    {
        RecordingGroup* group = recordingManager_->recordingGroup(current->text(0));
        if (group)
            emit recordingGroupSelected(group);
    }
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
