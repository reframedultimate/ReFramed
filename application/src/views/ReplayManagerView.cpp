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

#include "rfcommon/Session.hpp"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

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
    , replayGroupListView_(new ReplayGroupListView)
    , pluginDockView_(new PluginDockView(replayManager, pluginManager))
{
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

    ReplaySearchBox* searchBox = new ReplaySearchBox;
    QLabel* searchBoxLabel = new QLabel("Filters:");
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

    QSplitter* hSplitter = new QSplitter(Qt::Horizontal);
    hSplitter->addWidget(vSplitter);
    hSplitter->addWidget(pluginDockView_);
    hSplitter->setStretchFactor(0, 0);
    hSplitter->setStretchFactor(1, 1);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(hSplitter);

    setLayout(l);

    connect(searchBox, &ReplaySearchBox::searchTextChanged, this, &ReplayManagerView::searchTextChanged);
    connect(replayListView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ReplayManagerView::onItemSelectionChanged);
    connect(replayListView_, &QTreeView::customContextMenuRequested, this, &ReplayManagerView::onReplayRightClicked);
}

// ----------------------------------------------------------------------------
ReplayManagerView::~ReplayManagerView()
{
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

    QMenu menu;
    QAction* editMetaData = nullptr;
    QAction* associateVideo = nullptr;
    QAction* exportPack = nullptr;
    QAction* deleteReplays = nullptr;
    QAction* a = nullptr;

    QPoint item = replayListView_->mapToGlobal(pos);
    if (selectedFileNames.size() == 1)
    {
        editMetaData = menu.addAction("Edit Meta Data");
        associateVideo = menu.addAction("Associate Video");
        menu.addSeparator();
        exportPack = menu.addAction("Export as replay pack");
        menu.addSeparator();
        deleteReplays = menu.addAction("Delete");
        a = menu.exec(item);
    }
    else if (selectedFileNames.size() > 1)
    {
        exportPack = menu.addAction("Export as replay pack");
        menu.addSeparator();
        deleteReplays = menu.addAction("Delete");
        a = menu.exec(item);
    }

    if (a == nullptr)
        return;

    if (a == editMetaData)
    {
        const auto filePathUtf8 = replayManager_->resolveGameFile(selectedFileNames[0].toUtf8().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, filePathUtf8.cStr());
        if (session)
        {
            ReplayEditorDialog dialog(replayManager_, session, selectedFileNames[0]);
            dialog.exec();
        }
    }
    else if (a == associateVideo)
    {
        const auto filePathUtf8 = replayManager_->resolveGameFile(selectedFileNames[0].toUtf8().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, filePathUtf8.cStr());
        if (session)
        {
            VideoAssociatorDialog dialog(pluginManager_, replayManager_, session, selectedFileNames[0]);
            dialog.exec();
        }
    }
    else if (a == exportPack)
    {
        ExportReplayPackDialog dialog(replayManager_, selectedFileNames, selectedFileNames);
        dialog.exec();
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

}
