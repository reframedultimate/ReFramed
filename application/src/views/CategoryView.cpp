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
    , dataSetsItem_(new QTreeWidgetItem({"Data Sets"}, static_cast<int>(CategoryType::TOP_LEVEL_DATA_SETS)))
    , analysisCategoryItem_(new QTreeWidgetItem({"Analysis"}, static_cast<int>(CategoryType::TOP_LEVEL_ANALYSIS)))
    , replayGroupsItem_(new QTreeWidgetItem({"Replay Groups"}, static_cast<int>(CategoryType::TOP_LEVEL_REPLAY_GROUPS)))
    , replaySourcesItem_(new QTreeWidgetItem({"Replay Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_REPLAY_SOURCES)))
    , videoSourcesItem_(new QTreeWidgetItem({"Video Sources"}, static_cast<int>(CategoryType::TOP_LEVEL_VIDEO_SOURCES)))
    , sessionItem_(new QTreeWidgetItem({"Session"}, static_cast<int>(CategoryType::TOP_LEVEL_SESSION)))
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    addTopLevelItem(dataSetsItem_);
    addTopLevelItem(analysisCategoryItem_);
    addTopLevelItem(replayGroupsItem_);
    addTopLevelItem(replaySourcesItem_);
    addTopLevelItem(videoSourcesItem_);
    addTopLevelItem(sessionItem_);

    // Configure which items accept drag/drop
    setAcceptDrops(true);
    replayGroupsItem_->setFlags(replayGroupsItem_->flags() | Qt::ItemIsDropEnabled);

    // Populate various children based on the data available in the model
    for (int i = 0; i != replayManager_->replayGroupsCount(); ++i)
        onReplayManagerGroupAdded(replayManager_->replayGroup(i));
    for (int i = 0; i != replayManager_->replaySourcesCount(); ++i)
        onReplayManagerReplaySourceAdded(replayManager_->replaySourceName(i), replayManager_->replaySourcePath(i));
    for (int i = 0; i != replayManager_->videoSourcesCount(); ++i)
        onReplayManagerVideoSourceAdded(replayManager_->videoSourceName(i), replayManager_->videoSourcePath(i));

    replayGroupsItem_->setExpanded(true);
    replaySourcesItem_->setExpanded(true);
    videoSourcesItem_->setExpanded(true);

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
    if (current == nullptr)
        return;

    switch (static_cast<CategoryType>(current->type()))
    {
        case CategoryType::TOP_LEVEL_DATA_SETS      : categoryModel_->selectDataSetsCategory(); break;
        case CategoryType::TOP_LEVEL_ANALYSIS       : categoryModel_->selectAnalysisCategory(); break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : categoryModel_->selectReplayGroupsCategory(); break;
        case CategoryType::TOP_LEVEL_REPLAY_SOURCES : categoryModel_->selectReplaySourcesCategory(); break;
        case CategoryType::TOP_LEVEL_VIDEO_SOURCES  : categoryModel_->selectVideoSourcesCategory(); break;
        case CategoryType::TOP_LEVEL_SESSION        : categoryModel_->selectSessionCategory(); break;

        case CategoryType::ITEM_DATA_SET            : /* TODO */ break;
        case CategoryType::ITEM_ANALYSIS            : /* TODO */ break;
        case CategoryType::ITEM_REPLAY_GROUP        : categoryModel_->selectReplayGroup(current->text(0)); break;
        case CategoryType::ITEM_REPLAY_SOURCE       : categoryModel_->selectReplaySource(current->text(0)); break;
        case CategoryType::ITEM_VIDEO_SOURCE        : categoryModel_->selectVideoSource(current->text(0)); break;
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onItemChanged(QTreeWidgetItem* item, int column)
{
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
#define SELECT_ITEM(itemname) { \
        QSignalBlocker block(this); \
        clearSelection(); \
        itemname->setSelected(true); \
        setCurrentItem(itemname); \
    }

    switch (category)
    {
        case CategoryType::TOP_LEVEL_ANALYSIS       : SELECT_ITEM(analysisCategoryItem_) break;
        case CategoryType::TOP_LEVEL_DATA_SETS      : SELECT_ITEM(dataSetsItem_)         break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : SELECT_ITEM(replayGroupsItem_)     break;
        case CategoryType::TOP_LEVEL_REPLAY_SOURCES : SELECT_ITEM(replaySourcesItem_)    break;
        case CategoryType::TOP_LEVEL_VIDEO_SOURCES  : SELECT_ITEM(videoSourcesItem_)     break;
        case CategoryType::TOP_LEVEL_SESSION        : SELECT_ITEM(sessionItem_)          break;

        case CategoryType::ITEM_ANALYSIS:
        case CategoryType::ITEM_DATA_SET:
        case CategoryType::ITEM_REPLAY_GROUP:
        case CategoryType::ITEM_REPLAY_SOURCE:
        case CategoryType::ITEM_VIDEO_SOURCE:
            break;
    }

#undef SELECT_ITEM
}

// ----------------------------------------------------------------------------
void CategoryView::onCategoryItemSelected(CategoryType category, const QString& name)
{
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
        case CategoryType::TOP_LEVEL_ANALYSIS       : SELECT_CHILD_OF(analysisCategoryItem_) break;
        case CategoryType::TOP_LEVEL_DATA_SETS      : SELECT_CHILD_OF(dataSetsItem_);        break;
        case CategoryType::TOP_LEVEL_REPLAY_GROUPS  : SELECT_CHILD_OF(replayGroupsItem_);    break;
        case CategoryType::TOP_LEVEL_REPLAY_SOURCES : SELECT_CHILD_OF(replaySourcesItem_);   break;
        case CategoryType::TOP_LEVEL_VIDEO_SOURCES  : SELECT_CHILD_OF(videoSourcesItem_)     break;
        case CategoryType::TOP_LEVEL_SESSION        : SELECT_CHILD_OF(sessionItem_)          break;

        case CategoryType::ITEM_ANALYSIS:
        case CategoryType::ITEM_DATA_SET:
        case CategoryType::ITEM_REPLAY_GROUP:
        case CategoryType::ITEM_REPLAY_SOURCE:
        case CategoryType::ITEM_VIDEO_SOURCE:
            break;
    }

#undef SELECT_CHILD_OF
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerDefaultReplaySaveLocationChanged(const QDir& path)
{
    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerGroupAdded(ReplayGroup* group)
{
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
void CategoryView::onReplayManagerReplaySourceAdded(const QString& name, const QDir& path)
{
    replaySourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::ITEM_REPLAY_SOURCE)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerReplaySourceNameChanged(const QString& oldName, const QString& newName)
{
    for (int i = 0; i != replaySourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = replaySourcesItem_->child(i);
        if (item->text(0) == oldName)
        {
            item->setText(0, newName);
            // TODO update tooltip
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerReplaySourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath)
{
    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerReplaySourceRemoved(const QString& name)
{
    for (int i = 0; i != replaySourcesItem_->childCount(); ++i)
    {
        QTreeWidgetItem* item = replaySourcesItem_->child(i);
        if (item->text(0) == name)
        {
            replaySourcesItem_->removeChild(item);
            // TODO remove tooltip
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoSourceAdded(const QString& name, const QDir& path)
{
    videoSourcesItem_->addChild(new QTreeWidgetItem({name}, static_cast<int>(CategoryType::ITEM_VIDEO_SOURCE)));
    // TODO tooltip for path
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoSourceNameChanged(const QString& oldName, const QString& newName)
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
void CategoryView::onReplayManagerVideoSourcePathChanged(const QString& name, const QDir& oldPath, const QDir& newPath)
{
    // TODO update tooltip
}

// ----------------------------------------------------------------------------
void CategoryView::onReplayManagerVideoSourceRemoved(const QString& name)
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
