#include "application/views/CategoryView.hpp"
#include "application/models/RecordingManager.hpp"
#include "application/models/RecordingGroup.hpp"

namespace uh {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(RecordingManager* recordingManager, QWidget* parent)
    : QTreeWidget(parent)
    , recordingManager_(recordingManager)
    , analysisCategoryItem_(new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL_ANALYSIS)))
    , recordingGroupsItem_(new QTreeWidgetItem({"Recording Groups"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_GROUPS)))
    , recordingSourcesItem_(new QTreeWidgetItem({"Recording Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_SOURCES)))
    , videoSourcesItem_(new QTreeWidgetItem({"Video Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_VIDEO_SOURCES)))
    , activeRecordingItem_(new QTreeWidgetItem({"Active Recording"}, static_cast<int>(CategoryType::TOP_LEVEL_ACTIVE_RECORDING)))
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(recordingGroupsItem_);
    addTopLevelItem(recordingSourcesItem_);
    addTopLevelItem(videoSourcesItem_);
    addTopLevelItem(activeRecordingItem_);

    setActiveRecordingViewDisabled(true);

    for (const auto& group : recordingManager->recordingGroups())
        onRecordingManagerGroupAdded(group.second.get());
    const auto& recordingSources = recordingManager->recordingSources();
    for (auto it = recordingSources.begin(); it != recordingSources.end(); ++it)
        onRecordingManagerRecordingSourceAdded(it.key(), it.value());
    const auto& videoSources = recordingManager->videoSources();
    for (auto it = videoSources.begin(); it != videoSources.end(); ++it)
        onRecordingManagerVideoSourceAdded(it.key(), it.value());

    recordingGroupsItem_->setExpanded(true);
    recordingSourcesItem_->setExpanded(true);
    videoSourcesItem_->setExpanded(true);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenuRequested(const QPoint&)));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

// ----------------------------------------------------------------------------
void CategoryView::setActiveRecordingViewDisabled(bool disable)
{
    activeRecordingItem_->setDisabled(disable);
}

// ----------------------------------------------------------------------------
void CategoryView::onCustomContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    switch (categoryOf(item))
    {
        case CategoryType::TOP_LEVEL_RECORDING_GROUPS:
            break;

        default:
            break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onTreeWidgetCategoriesCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
#define EMIT_IF_CHANGED(category) \
    if (categoryOf(current) == category && (previous == nullptr || categoryOf(previous) != category)) \
        emit categoryChanged(category);
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_ANALYSIS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_RECORDING_GROUPS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_RECORDING_GROUPS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_VIDEO_SOURCES)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_ACTIVE_RECORDING)
    EMIT_IF_CHANGED(CategoryType::RECORDING_GROUP_ITEM)
    EMIT_IF_CHANGED(CategoryType::RECORDING_SOURCE_ITEM)
    EMIT_IF_CHANGED(CategoryType::VIDEO_SOURCE_ITEM)
#undef EMIT_IF_CHANGED

    if (current->parent() == recordingGroupsItem_)
    {
        RecordingGroup* group = recordingManager_->recordingGroup(current->text(0));
        if (group)
            emit recordingGroupSelected(group);
    }
}

// ----------------------------------------------------------------------------
CategoryType CategoryView::categoryOf(const QTreeWidgetItem* item) const
{
    if (item == analysisCategoryItem_)
        return CategoryType::TOP_LEVEL_ANALYSIS;
    if (item == recordingGroupsItem_)
        return CategoryType::TOP_LEVEL_RECORDING_GROUPS;
    if (item->parent() == recordingGroupsItem_)
        return CategoryType::RECORDING_GROUP_ITEM;
    if (item == recordingSourcesItem_)
        return CategoryType::TOP_LEVEL_RECORDING_SOURCES;
    if (item->parent() == recordingSourcesItem_)
        return CategoryType::RECORDING_SOURCE_ITEM;
    if (item == videoSourcesItem_)
        return CategoryType::TOP_LEVEL_VIDEO_SOURCES;
    if (item->parent() == videoSourcesItem_)
        return CategoryType::VIDEO_SOURCE_ITEM;
    if (item == activeRecordingItem_)
        return CategoryType::TOP_LEVEL_ACTIVE_RECORDING;
    assert(false);
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerDefaultRecordingLocationChanged(const QDir& path)
{
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupAdded(RecordingGroup* group)
{
    recordingGroupsItem_->addChild(new QTreeWidgetItem({group->name()}, static_cast<int>(CategoryType::RECORDING_GROUP_ITEM)));
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupRemoved(RecordingGroup* group)
{
    for (int i = 0; i != recordingGroupsItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = recordingGroupsItem_->child(i);
        if (item->text(0) == group->name())
            recordingGroupsItem_->removeChild(item);
        else
            i++;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerRecordingSourceAdded(const QString& name, const QDir& path)
{
    recordingSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::RECORDING_SOURCE_ITEM)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerRecordingSourceNameChanged(const QString& oldName, const QString& newName)
{
    for (int i = 0; i != recordingSourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = recordingSourcesItem_->child(i);
        if (item->text(0) == oldName)
        {
            item->setText(0, newName);
            // TODO update tooltip
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerRecordingSourceRemoved(const QString& name)
{
    for (int i = 0; i != recordingSourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = recordingSourcesItem_->child(i);
        if (item->text(0) == name)
        {
            recordingSourcesItem_->removeChild(item);
            // TODO remove tooltip
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerVideoSourceAdded(const QString& name, const QDir& path)
{
    videoSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::VIDEO_SOURCE_ITEM)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerVideoSourceNameChanged(const QString& oldName, const QString& newName)
{
    for (int i = 0; i != videoSourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = videoSourcesItem_->child(i);
        if (item->text(0) == oldName)
        {
            item->setText(0, newName);
            // TODO update tooltip
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerVideoSourceRemoved(const QString& name)
{
    for (int i = 0; i != videoSourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = videoSourcesItem_->child(i);
        if (item->text(0) == name)
        {
            videoSourcesItem_->removeChild(item);
            // TODO remove tooltip
            break;
        }
    }
}

}
