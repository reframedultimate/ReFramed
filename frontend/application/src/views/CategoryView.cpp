#include "application/views/CategoryView.hpp"
#include "application/models/RecordingManager.hpp"
#include "application/models/RecordingGroup.hpp"

#include <QMenu>

namespace uhapp {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(RecordingManager* recordingManager, QWidget* parent)
    : QTreeWidget(parent)
    , recordingManager_(recordingManager)
    , dataSetsItem_(new QTreeWidgetItem({"Data Sets"}, static_cast<int>(CategoryType::TOP_LEVEL_DATA_SETS)))
    , analysisCategoryItem_(new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL_ANALYSIS)))
    , recordingGroupsItem_(new QTreeWidgetItem({"Recording Groups"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_GROUPS)))
    , recordingSourcesItem_(new QTreeWidgetItem({"Recording Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_SOURCES)))
    , videoSourcesItem_(new QTreeWidgetItem({"Video Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_VIDEO_SOURCES)))
    , activeRecordingItem_(new QTreeWidgetItem({"Active Recording"}, static_cast<int>(CategoryType::TOP_LEVEL_ACTIVE_RECORDING)))
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    addTopLevelItem(dataSetsItem_);
    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(recordingGroupsItem_);
    addTopLevelItem(recordingSourcesItem_);
    addTopLevelItem(videoSourcesItem_);
    addTopLevelItem(activeRecordingItem_);

    // We're not connected by default
    setActiveRecordingViewDisabled(true);

    // Populate various children based on the data available in the recording manager
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

    recordingManager_->dispatcher.addListener(this);

    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &CategoryView::onCustomContextMenuRequested);
    connect(this, &QTreeWidget::itemChanged,
            this, &CategoryView::onItemChanged);
    connect(this, &QTreeWidget::currentItemChanged,
            this, &CategoryView::onCurrentItemChanged);
}

// ----------------------------------------------------------------------------
CategoryView::~CategoryView()
{
    recordingManager_->dispatcher.removeListener(this);
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
    if (item == nullptr)
        return;

    auto newGroup = [this](){
        int idx = 1;
        RecordingGroup* group;
        while (true)
        {
            QString name = QString("Group %1").arg(idx);
            if (recordingManager_->addRecordingGroup(name))
            {
                group = recordingManager_->recordingGroup(name);
                break;
            }
            idx++;
        }

        emit recordingGroupSelected(group);
    };

    auto deleteGroup = [this](RecordingGroup* group){
        recordingManager_->removeRecordingGroup(group->name());
        emit categoryChanged(CategoryType::TOP_LEVEL_RECORDING_GROUPS);
    };

    auto renameGroup = [this](QTreeWidgetItem* item){
        editItem(item, 0);
    };

    auto duplicateGroup = [this](RecordingGroup* otherGroup){
        int idx = 1;
        RecordingGroup* group;
        while (true)
        {
            QString name = QString("Group %1").arg(idx);
            if (recordingManager_->addRecordingGroup(name))
            {
                group = recordingManager_->recordingGroup(name);
                break;
            }
            idx++;
        }

        for (const auto& fileInfo : otherGroup->absFilePathList())
            group->addFile(fileInfo);

        emit recordingGroupSelected(group);
    };

    switch (categoryOf(item))
    {
        case CategoryType::TOP_LEVEL_RECORDING_GROUPS: {
            QMenu menu(this);
            QAction* newGroupAction = menu.addAction("New group");
            menu.addSeparator();
            QAction* newDataSetAction = menu.addAction("New data set from group");
            QAction* result = menu.exec(mapToGlobal(pos));

            if (result == newGroupAction)
                newGroup();
            else if (result == newDataSetAction) {}
        } break;

        case CategoryType::RECORDING_GROUP_ITEM: {
            RecordingGroup* group = recordingManager_->recordingGroup(item->text(0));
            if (group == nullptr)
                return;

            QMenu menu(this);
            QAction* newGroupAction = menu.addAction("New group");
            QAction* deleteGroupAction = nullptr;
            QAction* clearRecordingsAction = nullptr;
            QAction* renameGroupAction = nullptr;
            if (group != recordingManager_->allRecordingGroup())
            {
                deleteGroupAction = menu.addAction("Delete group");
                clearRecordingsAction = menu.addAction("Remove all recordings");
                renameGroupAction = menu.addAction("Rename group");
            }
            QAction* duplicateGroupAction = menu.addAction("Duplicate group");
            menu.addSeparator();
            QAction* newDataSetAction = menu.addAction("New data set from group");
            QAction* result = menu.exec(mapToGlobal(pos));

            if (result == newGroupAction)
                newGroup();
            else if (result == deleteGroupAction)
                deleteGroup(group);
            else if (result == renameGroupAction)
                renameGroup(item);
            else if (result == clearRecordingsAction)
                group->removeAllFiles();
            else if (result == duplicateGroupAction)
                duplicateGroup(group);
            else if (result == newDataSetAction) {}
        }

        default:
            break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
#define EMIT_IF_CHANGED(category) \
    if (categoryOf(current) == category && (previous == nullptr || categoryOf(previous) != category)) \
        emit categoryChanged(category);
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_RECORDING_GROUPS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_DATA_SETS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_ANALYSIS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_RECORDING_GROUPS)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_VIDEO_SOURCES)
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_ACTIVE_RECORDING)
    EMIT_IF_CHANGED(CategoryType::RECORDING_GROUP_ITEM)
    EMIT_IF_CHANGED(CategoryType::DATA_SETS_ITEM)
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
void CategoryView::onItemChanged(QTreeWidgetItem* item, int column)
{
    switch (categoryOf(item))
    {
        // Have to make sure that the name the user typed in for the item doesn't
        // conflict with any other existing group names. If it does, we add an
        // index at the end and update the item's text to reflect this
        case CategoryType::RECORDING_GROUP_ITEM: {
            int idx = 1;
            auto it = oldGroupNames_.find(item);
            const QString& oldName = it.value();
            QString newName = item->text(column);
            while (true)
            {
                if (recordingManager_->renameRecordingGroup(oldName, newName))
                    break;
                newName = item->text(column) + QString(" %1").arg(idx);
                idx++;
            }

            QSignalBlocker blocker(this);
            item->setText(column, newName);
            it.value() = newName;
        } break;

        default: break;
    }
}

// ----------------------------------------------------------------------------
CategoryType CategoryView::categoryOf(const QTreeWidgetItem* item) const
{
    if (item == recordingGroupsItem_)
        return CategoryType::TOP_LEVEL_RECORDING_GROUPS;
    if (item->parent() == recordingGroupsItem_)
        return CategoryType::RECORDING_GROUP_ITEM;
    if (item == dataSetsItem_)
        return CategoryType::TOP_LEVEL_DATA_SETS;
    if (item->parent() == dataSetsItem_)
        return CategoryType::DATA_SETS_ITEM;
    if (item == analysisCategoryItem_)
        return CategoryType::TOP_LEVEL_ANALYSIS;
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
    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupAdded(RecordingGroup* group)
{
    QTreeWidgetItem* item = new QTreeWidgetItem({group->name()}, static_cast<int>(CategoryType::RECORDING_GROUP_ITEM));
    const RecordingGroup* allGroup = recordingManager_->allRecordingGroup();

    // All groups that are not in the "All" group are editable
    if (group != allGroup)
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    // Have to keep track of the names that were set in order to be able to
    // rename them
    oldGroupNames_.insert(item, group->name());

    // Insert child and enter edit mode so the user can type in a different name
    // if they want to
    recordingGroupsItem_->addChild(item);
    if (group != allGroup)
    {
        setCurrentItem(item, 0);
        editItem(item, 0);
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName)
{
    // This will only ever really do anything if the group name is changed outside
    // of this class, in which case we should generate the onItemChange() event
    // as if the item were edited by the user
    for (int i = 0; i != recordingGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = recordingGroupsItem_->child(i);
        if (item->text(0) == oldName)
        {
            // calls onItemChanged()
            item->setText(0, newName);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onRecordingManagerGroupRemoved(RecordingGroup* group)
{
    for (int i = 0; i != recordingGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = recordingGroupsItem_->child(i);
        if (item->text(0) == group->name())
        {
            oldGroupNames_.remove(item);
            recordingGroupsItem_->removeChild(item);
            break;
        }
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
