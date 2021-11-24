#include "application/views/CategoryView.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "application/models/SavedGameSessionGroup.hpp"
#include "application/models/TrainingModeModel.hpp"

#include <QMenu>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

#include <QDebug>

namespace uhapp {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(SavedGameSessionManager* manager, TrainingModeModel* trainingMode, QWidget* parent)
    : QTreeWidget(parent)
    , savedGameSessionManager_(manager)
    , trainingMode_(trainingMode)
    , dataSetsItem_(new QTreeWidgetItem({"Data Sets"}, static_cast<int>(CategoryType::TOP_LEVEL_DATA_SETS)))
    , analysisCategoryItem_(new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL_ANALYSIS)))
    , savedGameSessionGroupsItem_(new QTreeWidgetItem({"Recording Groups"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_GROUPS)))
    , recordingSourcesItem_(new QTreeWidgetItem({"Recording Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_RECORDING_SOURCES)))
    , videoSourcesItem_(new QTreeWidgetItem({"Video Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_VIDEO_SOURCES)))
    , activeRecordingItem_(new QTreeWidgetItem({"Active Recording"}, static_cast<int>(CategoryType::TOP_LEVEL_ACTIVE_RECORDING)))
    , trainingModeItem_(new QTreeWidgetItem({"Training Mode"}, static_cast<int>(CategoryType::TOP_LEVEL_TRAINING_MODE)))
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    addTopLevelItem(dataSetsItem_);
    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(savedGameSessionGroupsItem_);
    addTopLevelItem(recordingSourcesItem_);
    addTopLevelItem(videoSourcesItem_);
    addTopLevelItem(activeRecordingItem_);
    addTopLevelItem(trainingModeItem_);

    setAcceptDrops(true);

    savedGameSessionGroupsItem_->setFlags(savedGameSessionGroupsItem_->flags() | Qt::ItemIsDropEnabled);

    // We're not connected by default
    setRunningGameSessionViewDisabled(true);

    // Populate various children based on the data available in the recording manager
    for (const auto& group : manager->savedGameSessionGroups())
        onSavedGameSessionManagerGroupAdded(group.second.get());
    const auto& recordingSources = manager->savedGameSessionSources();
    for (auto it = recordingSources.begin(); it != recordingSources.end(); ++it)
        onSavedGameSessionManagerGameSessionSourceAdded(it.key(), it.value());
    const auto& videoSources = manager->videoSources();
    for (auto it = videoSources.begin(); it != videoSources.end(); ++it)
        onSavedGameSessionManagerVideoSourceAdded(it.key(), it.value());

    savedGameSessionGroupsItem_->setExpanded(true);
    recordingSourcesItem_->setExpanded(true);
    videoSourcesItem_->setExpanded(true);

    savedGameSessionManager_->dispatcher.addListener(this);
    trainingMode_->dispatcher.addListener(this);

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
    trainingMode_->dispatcher.removeListener(this);
    savedGameSessionManager_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void CategoryView::setRunningGameSessionViewDisabled(bool disable)
{
    activeRecordingItem_->setDisabled(disable);
}

// ----------------------------------------------------------------------------
void CategoryView::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->formats().contains("application/x-ultimate-hindsight-uhr"))
    {
        event->ignore();
        return;
    }

    event->accept();
}

// ----------------------------------------------------------------------------
void CategoryView::dragMoveEvent(QDragMoveEvent* event)
{
    QTreeWidgetItem* targetItem = itemAt(event->pos());
    if (targetItem == nullptr)
    {
        event->ignore();
        return;
    }

    switch (categoryOf(targetItem))
    {
        case CategoryType::TOP_LEVEL_RECORDING_GROUPS:
            event->accept();
            break;

        case CategoryType::RECORDING_GROUP_ITEM: {
            // Don't allow dropping on the all group
            SavedGameSessionGroup* group = savedGameSessionManager_->savedGameSessionGroup(targetItem->text(0));
            assert(group);
            if (group == savedGameSessionManager_->allSavedGameSessionGroup())
            {
                event->ignore();
                return;
            }

            event->accept();
        } break;

        default:
            event->ignore();
            break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::dropEvent(QDropEvent* event)
{
    QTreeWidgetItem* targetItem = itemAt(event->pos());
    if (targetItem == nullptr)
    {
        event->ignore();
        return;
    }

    SavedGameSessionGroup* group = nullptr;
    switch (categoryOf(targetItem))
    {
        case CategoryType::TOP_LEVEL_RECORDING_GROUPS:
            group = newGroup();
            [[fallthrough]];
        case CategoryType::RECORDING_GROUP_ITEM:
            group = group ? group : savedGameSessionManager_->savedGameSessionGroup(targetItem->text(0));
        {
            assert(group);

            // Don't allow dropping on the all group
            if (group == savedGameSessionManager_->allSavedGameSessionGroup())
            {
                event->ignore();
                return;
            }

            QByteArray encodedData = event->mimeData()->data("application/x-ultimate-hindsight-uhr");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            while (!stream.atEnd())
            {
                QString s; stream >> s;
                group->addFile(s);
            }

            event->accept();
        } break;

        default:
            event->ignore();
            break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onCustomContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    if (item == nullptr)
        return;

    switch (categoryOf(item))
    {
        case CategoryType::TOP_LEVEL_RECORDING_GROUPS: {
            QMenu menu(this);
            QAction* newGroupAction = menu.addAction("New group");
            menu.addSeparator();
            QAction* newDataSetAction = menu.addAction("New data set");
            QAction* result = menu.exec(mapToGlobal(pos));

            if (result == newGroupAction)
                newGroup();
            else if (result == newDataSetAction) {}
        } break;

        case CategoryType::RECORDING_GROUP_ITEM: {
            SavedGameSessionGroup* group = savedGameSessionManager_->savedGameSessionGroup(item->text(0));
            if (group == nullptr)
                return;

            if (group != savedGameSessionManager_->allSavedGameSessionGroup())
            {
                QMenu menu(this);
                QAction* newGroupAction = menu.addAction("New group");
                QAction* renameGroupAction = menu.addAction("Rename group");
                QAction* duplicateGroupAction = menu.addAction("Duplicate group");
                menu.addSeparator();
                QAction* deleteGroupAction = menu.addAction("Delete group");
                QAction* clearRecordingsAction = menu.addAction("Remove all recordings");
                menu.addSeparator();
                QAction* newDataSetAction = menu.addAction("New data set from group");

                QAction* result = menu.exec(mapToGlobal(pos));
                if (result == newGroupAction)
                    newGroup();
                else if (result == renameGroupAction)
                    editItem(item, 0);
                else if (result == duplicateGroupAction)
                    duplicateGroup(group);
                else if (result == deleteGroupAction)
                    deleteGroup(group);
                else if (result == clearRecordingsAction)
                    group->removeAllFiles();
                else if (result == newDataSetAction) {}
            }
            else
            {
                QMenu menu(this);
                QAction* newGroupAction = menu.addAction("New group");
                QAction* duplicateGroupAction = menu.addAction("Duplicate group");
                menu.addSeparator();
                QAction* newDataSetAction = menu.addAction("New data set from group");

                QAction* result = menu.exec(mapToGlobal(pos));
                if (result == newGroupAction)
                    newGroup();
                else if (result == duplicateGroupAction)
                    duplicateGroup(group);
                else if (result == newDataSetAction) {}
            }
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
    EMIT_IF_CHANGED(CategoryType::TOP_LEVEL_TRAINING_MODE)
    EMIT_IF_CHANGED(CategoryType::RECORDING_GROUP_ITEM)
    EMIT_IF_CHANGED(CategoryType::DATA_SETS_ITEM)
    EMIT_IF_CHANGED(CategoryType::RECORDING_SOURCE_ITEM)
    EMIT_IF_CHANGED(CategoryType::VIDEO_SOURCE_ITEM)
    EMIT_IF_CHANGED(CategoryType::TRAINING_MODE_ITEM)
#undef EMIT_IF_CHANGED

    if (current->parent() == savedGameSessionGroupsItem_)
    {
        SavedGameSessionGroup* group = savedGameSessionManager_->savedGameSessionGroup(current->text(0));
        if (group)
            emit savedGameSessionGroupSelected(group);
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
                if (savedGameSessionManager_->renameSavedGameSessionGroup(oldName, newName))
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
    if (item == savedGameSessionGroupsItem_)
        return CategoryType::TOP_LEVEL_RECORDING_GROUPS;
    if (item->parent() == savedGameSessionGroupsItem_)
        return CategoryType::RECORDING_GROUP_ITEM;
    if (item == dataSetsItem_)
        return CategoryType::TOP_LEVEL_DATA_SETS;
    if (item->parent() == dataSetsItem_)
        return CategoryType::DATA_SETS_ITEM;
    if (item == analysisCategoryItem_)
        return CategoryType::TOP_LEVEL_ANALYSIS;
    if (item == recordingSourcesItem_)
        return CategoryType::TOP_LEVEL_RECORDING_SOURCES;
    if (item == trainingModeItem_)
        return CategoryType::TOP_LEVEL_TRAINING_MODE;
    if (item->parent() == recordingSourcesItem_)
        return CategoryType::RECORDING_SOURCE_ITEM;
    if (item == videoSourcesItem_)
        return CategoryType::TOP_LEVEL_VIDEO_SOURCES;
    if (item->parent() == videoSourcesItem_)
        return CategoryType::VIDEO_SOURCE_ITEM;
    if (item == activeRecordingItem_)
        return CategoryType::TOP_LEVEL_ACTIVE_RECORDING;
    if (item->parent() == trainingModeItem_)
        return CategoryType::TRAINING_MODE_ITEM;
    assert(false);
}

// ----------------------------------------------------------------------------
SavedGameSessionGroup* CategoryView::newGroup()
{
    int idx = 1;
    SavedGameSessionGroup* group;
    while (true)
    {
        QString name = QString("Group %1").arg(idx);
        if (savedGameSessionManager_->addSavedGameSessionGroup(name))
        {
            group = savedGameSessionManager_->savedGameSessionGroup(name);
            break;
        }
        idx++;
    }

    emit savedGameSessionGroupSelected(group);
    return group;
}

// ----------------------------------------------------------------------------
SavedGameSessionGroup* CategoryView::duplicateGroup(SavedGameSessionGroup* otherGroup)
{
    int idx = 1;
    SavedGameSessionGroup* group;
    while (true)
    {
        QString name = QString("Group %1").arg(idx);
        if (savedGameSessionManager_->addSavedGameSessionGroup(name))
        {
            group = savedGameSessionManager_->savedGameSessionGroup(name);
            break;
        }
        idx++;
    }

    for (const auto& fileInfo : otherGroup->absFilePathList())
        group->addFile(fileInfo);

    emit savedGameSessionGroupSelected(group);
    return group;
}

// ----------------------------------------------------------------------------
void CategoryView::deleteGroup(SavedGameSessionGroup* group)
{
    savedGameSessionManager_->removeSavedGameSessionGroup(group->name());
    emit categoryChanged(CategoryType::TOP_LEVEL_RECORDING_GROUPS);
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerDefaultGameSessionSaveLocationChanged(const QDir& path)
{
    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerGroupAdded(SavedGameSessionGroup* group)
{
    QTreeWidgetItem* item = new QTreeWidgetItem({group->name()}, static_cast<int>(CategoryType::RECORDING_GROUP_ITEM));
    const SavedGameSessionGroup* allGroup = savedGameSessionManager_->allSavedGameSessionGroup();

    // All groups that are not in the "All" group are editable
    if (group != allGroup)
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    // Have to keep track of the names that were set in order to be able to
    // rename them
    oldGroupNames_.insert(item, group->name());

    // Insert child and enter edit mode so the user can type in a different name
    // if they want to
    savedGameSessionGroupsItem_->addChild(item);
    if (group != allGroup)
    {
        setCurrentItem(item, 0);
        editItem(item, 0);
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerGroupNameChanged(SavedGameSessionGroup* group, const QString& oldName, const QString& newName)
{
    // This will only ever really do anything if the group name is changed outside
    // of this class, in which case we should generate the onItemChange() event
    // as if the item were edited by the user
    for (int i = 0; i != savedGameSessionGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = savedGameSessionGroupsItem_->child(i);
        if (item->text(0) == oldName)
        {
            // calls onItemChanged()
            item->setText(0, newName);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerGroupRemoved(SavedGameSessionGroup* group)
{
    for (int i = 0; i != savedGameSessionGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = savedGameSessionGroupsItem_->child(i);
        if (item->text(0) == group->name())
        {
            oldGroupNames_.remove(item);
            savedGameSessionGroupsItem_->removeChild(item);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerGameSessionSourceAdded(const QString& name, const QDir& path)
{
    recordingSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::RECORDING_SOURCE_ITEM)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerGameSessionSourceNameChanged(const QString& oldName, const QString& newName)
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
void CategoryView::onSavedGameSessionManagerGameSessionSourceRemoved(const QString& name)
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
void CategoryView::onSavedGameSessionManagerVideoSourceAdded(const QString& name, const QDir& path)
{
    videoSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::VIDEO_SOURCE_ITEM)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onSavedGameSessionManagerVideoSourceNameChanged(const QString& oldName, const QString& newName)
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
void CategoryView::onSavedGameSessionManagerVideoSourceRemoved(const QString& name)
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

// ----------------------------------------------------------------------------
void CategoryView::onTrainingModePluginLaunched(const QString& name, uh::RealtimePlugin* plugin)
{
    // TODO tooltip containing plugin description
    trainingModeItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::TRAINING_MODE_ITEM)));
}

// ----------------------------------------------------------------------------
void CategoryView::onTrainingModePluginStopped(const QString& name, uh::RealtimePlugin* plugin)
{
    for (int i = 0; i != trainingModeItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = trainingModeItem_->child(i);
        if (item->text(0) == name)
        {
            trainingModeItem_->removeChild(item);
            // TODO remove tooltip
            break;
        }
    }
}

}
