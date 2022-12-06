#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayListSortFilterModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/widgets/ReplaySearchBox.hpp"
#include "application/views/ExportReplayPackDialog.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/views/ReplayEditorDialog.hpp"
#include "application/views/ReplayGroupListView.hpp"
#include "application/views/ReplayListView.hpp"
#include "application/views/ReplayManagerView.hpp"
#include "application/views/VideoAssociatorDialog.hpp"
#include "application/widgets/CollapsibleSplitter.hpp"

#include "rfcommon/Session.hpp"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayManagerView::ReplayManagerView(
        ReplayManager* replayManager,
        PluginManager* pluginManager,
        UserMotionLabelsManager* userMotionLabelsManager,
        rfcommon::Hash40Strings* hash40Strings,
        QWidget* parent)
    : QWidget(parent)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
    , userMotionLabelsManager_(userMotionLabelsManager)
    , hash40Strings_(hash40Strings)
    , replayListModel_(new ReplayListModel(replayManager))
    , replayListSortFilterModel_(new ReplayListSortFilterModel)
    , replayListView_(new ReplayListView)
    , replayGroupListView_(new ReplayGroupListView(replayManager))
    , pluginDockView_(new PluginDockView(replayManager, pluginManager))
{
    replayGroupListView_->addItem(replayManager_->allReplayGroup()->name());
    for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        replayGroupListView_->addItem(replayManager_->replayGroup(i)->name());
    replayGroupListView_->setCurrentItem(replayGroupListView_->item(0));
    replayGroupListView_->setContextMenuPolicy(Qt::CustomContextMenu);

    replayListModel_->setReplayGroup(replayManager_->allReplayGroup());
    replayListSortFilterModel_->setSourceModel(replayListModel_.get());
    replayListSortFilterModel_->setDynamicSortFilter(true);
    replayListSortFilterModel_->sort(0);
    replayListView_->setModel(replayListSortFilterModel_.get());
    replayListView_->setContextMenuPolicy(Qt::CustomContextMenu);

    replayListView_->expandAll();
    for (int i = 0; i != replayListSortFilterModel_->columnCount(); ++i)
        replayListView_->resizeColumnToContents(i);
    replayListView_->collapseAll();

    if (replayListSortFilterModel_->rowCount() > 0)
        replayListView_->expand(replayListSortFilterModel_->index(0, 0));
    if (replayListSortFilterModel_->rowCount() > 1)
        replayListView_->expand(replayListSortFilterModel_->index(1, 0));

    QLabel* searchBoxLabel = new QLabel("Filters:");
    ReplaySearchBox* searchBox = new ReplaySearchBox;
    searchBox->setClearButtonEnabled(true);
    QHBoxLayout* searchBoxLayout = new QHBoxLayout;
    searchBoxLayout->addWidget(searchBoxLabel);
    searchBoxLayout->addWidget(searchBox);
    searchBoxLayout->setContentsMargins(QMargins(0, 6, 0, 6));
    QWidget* searchBoxContainer = new QWidget;
    searchBoxContainer->setLayout(searchBoxLayout);

    QSplitter* vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(replayGroupListView_);
    vSplitter->addWidget(searchBoxContainer);
    vSplitter->addWidget(replayListView_);
    vSplitter->setStretchFactor(0, 0);
    vSplitter->setStretchFactor(1, 0);
    vSplitter->setStretchFactor(2, 1);
    vSplitter->setCollapsible(0, false);
    vSplitter->setCollapsible(1, false);
    vSplitter->setCollapsible(2, false);

    searchBoxContainer->setFixedHeight(searchBoxContainer->sizeHint().height());

    hSplitter_ = new CollapsibleSplitter(Qt::Horizontal);
    hSplitter_->addWidget(vSplitter);
    hSplitter_->addWidget(pluginDockView_);
    hSplitter_->setStretchFactor(0, 0);
    hSplitter_->setStretchFactor(1, 1);
    //hSplitter_->setCollapsible(1, false);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(hSplitter_);

    setLayout(l);

    connect(replayGroupListView_, &QListWidget::currentItemChanged, this, &ReplayManagerView::groupSelected);
    connect(searchBox, &ReplaySearchBox::searchTextChanged, this, &ReplayManagerView::searchTextChanged);
    connect(replayListView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ReplayManagerView::onItemSelectionChanged);
    connect(replayListView_, &QTreeView::customContextMenuRequested, this, &ReplayManagerView::onReplayRightClicked);
    connect(replayGroupListView_, &QListWidget::customContextMenuRequested, this, &ReplayManagerView::onGroupRightClicked);
}

// ----------------------------------------------------------------------------
ReplayManagerView::~ReplayManagerView()
{
}

// ----------------------------------------------------------------------------
void ReplayManagerView::toggleSideBar()
{
    hSplitter_->toggleCollapse();
}

// ----------------------------------------------------------------------------
void ReplayManagerView::groupSelected(QListWidgetItem* current, QListWidgetItem* previous)
{
    replayListModel_->clearReplayGroup();

    if (current == nullptr)
        return;

    for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
    {
        ReplayGroup* group = replayManager_->replayGroup(i);
        if (current->text() == group->name())
        {
            replayListModel_->setReplayGroup(group);

            if (replayListSortFilterModel_->rowCount() > 0)
                replayListView_->expand(replayListSortFilterModel_->index(0, 0));
            if (replayListSortFilterModel_->rowCount() > 1)
                replayListView_->expand(replayListSortFilterModel_->index(1, 0));
            return;
        }
    }

    replayListModel_->setReplayGroup(replayManager_->allReplayGroup());

    if (replayListSortFilterModel_->rowCount() > 0)
        replayListView_->expand(replayListSortFilterModel_->index(0, 0));
    if (replayListSortFilterModel_->rowCount() > 1)
        replayListView_->expand(replayListSortFilterModel_->index(1, 0));
}

// ----------------------------------------------------------------------------
void ReplayManagerView::searchTextChanged(int type, const QStringList& text)
{
    if (replayListSortFilterModel_->filtersCleared() && text.size() > 0)
    {
        storeExpandedStates_.clear();
        for (int row = 0; row != replayListSortFilterModel_->rowCount(); ++row)
        {
            const QModelIndex idx = replayListSortFilterModel_->index(row, 0);
            storeExpandedStates_.push_back(replayListView_->isExpanded(idx));
        }
    }

    switch (type)
    {
        case ReplaySearchBox::GENERIC: replayListSortFilterModel_->setGenericSearchTerms(text); break;
    }

    if (replayListSortFilterModel_->filtersCleared())
    {
        for (int row = 0; row != replayListSortFilterModel_->rowCount(); ++row)
        {
            const QModelIndex idx = replayListSortFilterModel_->index(row, 0);
            replayListView_->setExpanded(idx, storeExpandedStates_[row]);
        }
    }
    else
    {
        replayListView_->expandAll();
    }
}

// ----------------------------------------------------------------------------
void ReplayManagerView::onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    pluginDockView_->clearReplays();

    QStringList selectedFileNames;
    auto selectedIdxs = replayListView_->selectionModel()->selectedRows();
    for (const auto& idx : selectedIdxs)
    {
        // NOTE: The selection model is using proxy model indices, but we need
        // source model indices in order to successfully retrieve the filename
        const QModelIndex& srcIdx = replayListSortFilterModel_->mapToSource(idx);
        QString fileName = replayListModel_->indexFileName(srcIdx);
        if (fileName.isEmpty())
            continue;
        selectedFileNames.push_back(std::move(fileName));
    }

    if (selectedFileNames.size() == 1)
    {
        assert(QDir(selectedFileNames[0]).isRelative());

        const auto filePathUtf8 = replayManager_->resolveGameFile(selectedFileNames[0].toUtf8().constData());
        const auto filePath = QString::fromUtf8(filePathUtf8.cStr());
        if (filePath.isEmpty())
            return;
        pluginDockView_->loadGameReplays({ filePath });
    }
    else if (selectedFileNames.size() > 1)
    {
        QStringList filePaths;
        for (const auto& fileName : selectedFileNames)
        {
            const auto filePathUtf8 = replayManager_->resolveGameFile(fileName.toUtf8().constData());
            const auto filePath = QString::fromUtf8(filePathUtf8.cStr());
            if (filePath.isEmpty() == false)
                filePaths.push_back(filePath);
        }

        pluginDockView_->loadGameReplays(filePaths);
    }
}

// ----------------------------------------------------------------------------
void ReplayManagerView::onReplayRightClicked(const QPoint& pos)
{
    QStringList selectedFileNames;
    const auto selectedIdxs = replayListView_->selectionModel()->selectedRows();
    for (const auto& idx : selectedIdxs)
    {
        // NOTE: The selection model is using proxy model indices, but we need
        // source model indices in order to successfully retrieve the filename
        const QModelIndex& srcIdx = replayListSortFilterModel_->mapToSource(idx);
        QString fileName = replayListModel_->indexFileName(srcIdx);
        if (fileName.isEmpty())
            continue;
        selectedFileNames.push_back(std::move(fileName));
    }

    if (selectedFileNames.size() == 0)
        return;

    QListWidgetItem* currentGroupItem = replayGroupListView_->currentItem();
    if (currentGroupItem == nullptr)
        return;

    QMenu groupMenu;
    for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        groupMenu.addAction(replayManager_->replayGroup(i)->name());

    auto isAddToGroupAction = [&groupMenu](QAction* a) -> bool {
        for (const auto& action : groupMenu.actions())
            if (a == action)
                return true;
        return false;
    };

    QMenu menu;
    QAction* editMetaData = menu.addAction("Edit meta data");
    QAction* associateVideo = menu.addAction("Associate video");
    menu.addSeparator();
    QAction* exportPack = menu.addAction("Export as replay pack");
    menu.addSeparator();
    QAction* addToNewGroup = menu.addAction("Add to new group");
    QAction* addToGroup = menu.addAction("Add to group");
    addToGroup->setMenu(&groupMenu);
    QAction* removeFromGroup = menu.addAction("Remove from group");
    menu.addSeparator();
    QAction* deleteReplays = menu.addAction("Delete");

    if (selectedFileNames.size() == 1)
    {
    }
    else
    {
        associateVideo->setEnabled(false);
    }

    if (currentGroupItem->text() == replayManager_->allReplayGroup()->name())
    {
        removeFromGroup->setEnabled(false);
    }
    else
    {
        deleteReplays->setEnabled(false);
    }

    QPoint item = replayListView_->viewport()->mapToGlobal(pos);
    QAction* a = menu.exec(item);
    if (a == nullptr)
        return;

    if (a == editMetaData)
    {
        ReplayEditorDialog dialog(replayManager_, selectedFileNames, this);
        dialog.setGeometry(calculatePopupGeometryActiveScreen(900));
        dialog.exec();
    }
    else if (a == associateVideo)
    {
        const auto filePathUtf8 = replayManager_->resolveGameFile(selectedFileNames[0].toUtf8().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, filePathUtf8.cStr());
        if (session)
        {
            VideoAssociatorDialog dialog(pluginManager_, replayManager_, session, selectedFileNames[0], this);
            dialog.setGeometry(calculatePopupGeometryActiveScreen());
            dialog.exec();
        }
    }
    else if (a == exportPack)
    {
        ExportReplayPackDialog dialog(replayManager_, selectedFileNames, selectedFileNames, this);
        dialog.exec();
    }
    else if (a == addToNewGroup)
    {
        retryAddToNewGroup:
        bool ok;
        QString name = QInputDialog::getText(this, "Create new group", "Enter the name of the new group", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            if (name.isEmpty())
            {
                QMessageBox::critical(this, "Empty name", "The group name must not be empty");
                goto retryAddToNewGroup;
            }

            ReplayGroup* group = replayManager_->addReplayGroup(name);
            if (group == nullptr)
            {
                QMessageBox::critical(this, "Duplicate group", "Group \"" + name + "\" already exists");
                goto retryAddToNewGroup;
            }

            for (const auto& fileName : selectedFileNames)
                group->addFile(fileName);
        }
    }
    else if (isAddToGroupAction(a))
    {
        for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        {
            ReplayGroup* group = replayManager_->replayGroup(i);
            if (a->text() == group->name())
            {
                for (const auto& fileName : selectedFileNames)
                    group->addFile(fileName);
                break;
            }
        }
    }
    else if (a == removeFromGroup)
    {
        for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        {
            ReplayGroup* group = replayManager_->replayGroup(i);
            if (currentGroupItem->text() == group->name())
            {
                for (const auto& fileName : selectedFileNames)
                    group->removeFile(fileName);
                break;
            }
        }
    }
    else if (a == deleteReplays)
    {
        if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete these replays?") == QMessageBox::Yes)
        {
            bool success = true;
            for (const auto& fileName : selectedFileNames)
                if (replayManager_->deleteReplay(fileName) == false)
                    success = false;

            if (success == false)
                QMessageBox::critical(this, "Error deleting file(s)", "Failed to delete some of the selected files because they don't exist. They were probably deleted by an external process.");
        }
    }
}

// ----------------------------------------------------------------------------
void ReplayManagerView::onGroupRightClicked(const QPoint& pos)
{
    QMenu menu;
    QAction* createGroup = menu.addAction("Create new group");
    QAction* duplicateGroup = menu.addAction("Duplicate group");
    QAction* deleteGroup = menu.addAction("Delete group");

    QPoint item = replayGroupListView_->mapToGlobal(pos);
    QAction* a = menu.exec(item);
    if (a == nullptr)
        return;

    if (a == createGroup)
    {
        retryAddToNewGroup:
        bool ok;
        QString name = QInputDialog::getText(this, "Create new group", "Enter the name of the new group", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            if (name.isEmpty())
            {
                QMessageBox::critical(this, "Empty name", "The group name must not be empty");
                goto retryAddToNewGroup;
            }

            ReplayGroup* group = replayManager_->addReplayGroup(name);
            if (group == nullptr)
            {
                QMessageBox::critical(this, "Duplicate group", "Group \"" + name + "\" already exists");
                goto retryAddToNewGroup;
            }
        }
    }
    else if (a == duplicateGroup)
    {
        retryDupToNewGroup:
        bool ok;
        QString name = QInputDialog::getText(this, "Duplicate group", "Enter the name of the new group", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            if (name.isEmpty())
            {
                QMessageBox::critical(this, "Empty name", "The group name must not be empty");
                goto retryDupToNewGroup;
            }

            ReplayGroup* group = replayManager_->addReplayGroup(name);
            if (group == nullptr)
            {
                QMessageBox::critical(this, "Duplicate group", "Group \"" + name + "\" already exists");
                goto retryDupToNewGroup;
            }

            QListWidgetItem* item = replayGroupListView_->currentItem();
            for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
            {
                ReplayGroup* oldGroup = replayManager_->replayGroup(i);
                if (item->text() == oldGroup->name())
                {
                    for (const auto& fileName : oldGroup->files())
                        group->addFile(fileName);
                    break;
                }
            }
        }
    }
    else if (a == deleteGroup)
    {
        QListWidgetItem* item = replayGroupListView_->currentItem();
        for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        {
            ReplayGroup* group = replayManager_->replayGroup(i);
            if (item->text() == group->name())
            {
                replayManager_->removeReplayGroup(group);
                break;
            }
        }
    }
}

}
