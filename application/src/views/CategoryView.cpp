#include "rfcommon/Profiler.hpp"
#include "application/views/CategoryView.hpp"
#include "application/models/CategoryModel.hpp"
#include "application/models/CategoryModel.hpp"
#include "application/models/ReplayManager.hpp"

#include <QMenu>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>

#include <QDebug>

namespace rfapp {

// ----------------------------------------------------------------------------
CategoryView::CategoryView(
        CategoryModel* categoryModel,
        ReplayManager* replayManager,
        QWidget* parent)
    : QTreeWidget(parent)
    , categoryModel_(categoryModel)
    , replayManager_(replayManager)
    , sessionItem_(new QTreeWidgetItem({ "Session" }, static_cast<int>(CategoryType::TOP_LEVEL_SESSION)))
    , replayGroupsItem_(new QTreeWidgetItem({"Replay Groups"}, static_cast<int>(CategoryType::TOP_LEVEL_REPLAY_GROUPS)))
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    addTopLevelItem(sessionItem_);
    addTopLevelItem(replayGroupsItem_);

    // Configure which items accept drag/drop
    setAcceptDrops(true);
    replayGroupsItem_->setFlags(replayGroupsItem_->flags() | Qt::ItemIsDropEnabled);

    // Populate various children based on the data available in the model
    onReplayManagerGroupAdded(replayManager_->allReplayGroup());
    for (int i = 0; i != replayManager_->replayGroupsCount(); ++i)
        onReplayManagerGroupAdded(replayManager_->replayGroup(i));
    for (int i = 0; i != replayManager_->gamePathCount(); ++i)
        onReplayManagerGamePathAdded(replayManager_->gamePathName(i), replayManager_->gamePath(i));
    for (int i = 0; i != replayManager_->videoPathCount(); ++i)
        onReplayManagerVideoPathAdded(replayManager_->videoPathName(i), replayManager_->videoPath(i));

    replayGroupsItem_->setExpanded(true);

    categoryModel_->dispatcher.addListener(this);
    replayManager_->dispatcher.addListener(this);

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
    replayManager_->dispatcher.removeListener(this);
    categoryModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void CategoryView::dragEnterEvent(QDragEnterEvent* event)
{
    PROFILE(CategoryView, dragEnterEvent);

    if (!event->mimeData()->formats().contains("application/x-ultimate-hindsight-rfr"))
    {
        event->ignore();
        return;
    }

    event->accept();
}

// ----------------------------------------------------------------------------
void CategoryView::dragMoveEvent(QDragMoveEvent* event)
{
    PROFILE(CategoryView, dragMoveEvent);

    QTreeWidgetItem* targetItem = itemAt(event->pos());
    if (targetItem == nullptr)
    {
        event->ignore();
        return;
    }

    switch (static_cast<CategoryType>(targetItem->type()))
    {
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS:
            event->accept();
            break;

        case CategoryType::ITEM_REPLAY_GROUP: {
            // Don't allow dropping on the all group
            ReplayGroup* group = replayManager_->replayGroup(targetItem->text(0));
            assert(group);
            if (group == replayManager_->allReplayGroup())
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
    PROFILE(CategoryView, dropEvent);

    QTreeWidgetItem* targetItem = itemAt(event->pos());
    if (targetItem == nullptr)
    {
        event->ignore();
        return;
    }

    ReplayGroup* group = nullptr;
    switch (static_cast<CategoryType>(targetItem->type()))
    {
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS:
            group = replayManager_->addReplayGroup(replayManager_->findFreeGroupName("Group"));
            [[fallthrough]];
        case CategoryType::ITEM_REPLAY_GROUP:
            group = group ? group : replayManager_->replayGroup(targetItem->text(0));
        {
            assert(group);

            // Don't allow dropping on the all group
            if (group == replayManager_->allReplayGroup())
            {
                event->ignore();
                return;
            }

            QByteArray encodedData = event->mimeData()->data("application/x-ultimate-hindsight-rfr");
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
    PROFILE(CategoryView, onCustomContextMenuRequested);

    QTreeWidgetItem* item = itemAt(pos);
    if (item == nullptr)
        return;

    switch (static_cast<CategoryType>(item->type()))
    {
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS: {
            QMenu menu(this);
            QAction* newGroupAction = menu.addAction("New group");
            menu.addSeparator();
            QAction* newDataSetAction = menu.addAction("New data set");
            QAction* result = menu.exec(mapToGlobal(pos));

            if (result == newGroupAction)
                replayManager_->addReplayGroup(replayManager_->findFreeGroupName("Group"));
            else if (result == newDataSetAction) {}
        } break;

        case CategoryType::ITEM_REPLAY_GROUP: {
            ReplayGroup* group = replayManager_->replayGroup(item->text(0));
            if (group == nullptr)
                return;

            if (group != replayManager_->allReplayGroup())
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
                {
                    replayManager_->addReplayGroup(
                            replayManager_->findFreeGroupName("Group"));
                }
                else if (result == renameGroupAction)
                {
                    editItem(item, 0);
                }
                else if (result == duplicateGroupAction)
                {
                    replayManager_->duplicateReplayGroup(group,
                            replayManager_->findFreeGroupName(group->name()));
                }
                else if (result == deleteGroupAction)
                {
                    replayManager_->removeReplayGroup(group);
                }
                else if (result == clearRecordingsAction)
                {
                    group->removeAllFiles();
                }
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
                {
                    replayManager_->addReplayGroup(
                            replayManager_->findFreeGroupName("Group"));
                }
                else if (result == duplicateGroupAction)
                {
                    replayManager_->duplicateReplayGroup(group,
                            replayManager_->findFreeGroupName(group->name()));
                }
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
    PROFILE(CategoryView, onCurrentItemChanged);

    if (current == nullptr)
        return;

    switch (static_cast<CategoryType>(current->type()))
    {
        case CategoryType::TOP_LEVEL_SESSION        : categoryModel_->selectSessionCategory(); break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : categoryModel_->selectReplayGroupsCategory(); break;

        case CategoryType::ITEM_REPLAY_GROUP        : categoryModel_->selectReplayGroup(current->text(0)); break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onItemChanged(QTreeWidgetItem* item, int column)
{
    PROFILE(CategoryView, onItemChanged);

    switch (static_cast<CategoryType>(item->type()))
    {
        // Have to make sure that the name the user typed in for the item doesn't
        // conflict with any other existing group names. If it does, we add an
        // index at the end and update the item's text to reflect this
        case CategoryType::ITEM_REPLAY_GROUP: {
            int idx = 1;
            auto it = oldGroupNames_.find(item);
            const QString& oldName = it.value();
            ReplayGroup* group = replayManager_->replayGroup(oldName);
            QString newName = item->text(column);
            while (true)
            {
                if (replayManager_->renameReplayGroup(group, newName))
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
void CategoryView::onCategorySelected(CategoryType category)
{
    PROFILE(CategoryView, onCategorySelected);

#define SELECT_ITEM(itemname) { \
        QSignalBlocker block(this); \
        clearSelection(); \
        itemname->setSelected(true); \
        setCurrentItem(itemname); \
    }

    switch (category)
    {
        case CategoryType::TOP_LEVEL_SESSION        : SELECT_ITEM(sessionItem_)          break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : SELECT_ITEM(replayGroupsItem_)     break;

        case CategoryType::ITEM_REPLAY_GROUP:
            break;
    }

#undef SELECT_ITEM
}

// ----------------------------------------------------------------------------
void CategoryView::onCategoryItemSelected(CategoryType category, const QString& name)
{
    PROFILE(CategoryView, onCategoryItemSelected);

#define SELECT_CHILD_OF(itemname) \
    for (int i = 0; i != itemname->childCount(); ++i) \
    { \
        QTreeWidgetItem* child = itemname->child(i); \
        if (itemname->child(i)->text(0) == name) \
        { \
            QSignalBlocker block(this); \
            clearSelection(); \
            child->setSelected(true); \
            setCurrentItem(child); \
            break; \
        } \
    }

    switch (category)
    {
        case CategoryType::TOP_LEVEL_SESSION        : SELECT_CHILD_OF(sessionItem_)          break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : SELECT_CHILD_OF(replayGroupsItem_);    break;

        case CategoryType::ITEM_REPLAY_GROUP:
            break;
    }

#undef SELECT_CHILD_OF
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerDefaultGamePathChanged(const QDir& path)
{
    PROFILE(CategoryView, onReplayManagerDefaultGamePathChanged);

    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGroupAdded(ReplayGroup* group)
{
    PROFILE(CategoryView, onReplayManagerGroupAdded);

    QTreeWidgetItem* item = new QTreeWidgetItem({group->name()}, static_cast<int>(CategoryType::ITEM_REPLAY_GROUP));
    const ReplayGroup* allGroup = replayManager_->allReplayGroup();

    // All groups that are not in the "All" group are editable
    if (group != allGroup)
        item->setFlags(item->flags() | Qt::ItemIsEditable);

    // Have to keep track of the names that were set in order to be able to
    // rename them
    oldGroupNames_.insert(item, group->name());

    // Insert child and enter edit mode so the user can type in a different name
    // if they want to
    replayGroupsItem_->addChild(item);
    if (group != allGroup)
    {
        setCurrentItem(item, 0);
        editItem(item, 0);
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName)
{
    PROFILE(CategoryView, onReplayManagerGroupNameChanged);

    // This will only ever really do anything if the group name is changed outside
    // of this class, in which case we should generate the onItemChange() event
    // as if the item were edited by the user
    for (int i = 0; i != replayGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = replayGroupsItem_->child(i);
        if (item->text(0) == oldName)
        {
            // calls onItemChanged()
            item->setText(0, newName);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    PROFILE(CategoryView, onReplayManagerGroupRemoved);

    for (int i = 0; i != replayGroupsItem_->childCount(); i++)
    {
        QTreeWidgetItem* item = replayGroupsItem_->child(i);
        if (item->text(0) == group->name())
        {
            oldGroupNames_.remove(item);
            replayGroupsItem_->removeChild(item);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGamePathAdded(const QString& name, const QDir& path)
{
    PROFILE(CategoryView, onReplayManagerGamePathAdded);

    //replaySourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::ITEM_REPLAY_SOURCE)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGamePathNameChanged(const QString& oldName, const QString& newName)
{
    PROFILE(CategoryView, onReplayManagerGamePathNameChanged);
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGamePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath)
{
    PROFILE(CategoryView, onReplayManagerGamePathChanged);

    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGamePathRemoved(const QString& name)
{
    PROFILE(CategoryView, onReplayManagerGamePathRemoved);
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoPathAdded(const QString& name, const QDir& path)
{
    PROFILE(CategoryView, onReplayManagerVideoPathAdded);

    //videoSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::ITEM_VIDEO_SOURCE)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoPathNameChanged(const QString& oldName, const QString& newName)
{
    PROFILE(CategoryView, onReplayManagerVideoPathNameChanged);
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoPathChanged(const QString& name, const QDir& oldPath, const QDir& newPath)
{
    PROFILE(CategoryView, onReplayManagerVideoPathChanged);

    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoPathRemoved(const QString& name)
{
    PROFILE(CategoryView, onReplayManagerVideoPathRemoved);
}

}
