#include "application/ui_ReplayGroupView.h"
#include "application/models/ReplayGroup.hpp"
#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/views/ExportReplayPackDialog.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/views/ReplayEditorDialog.hpp"
#include "application/views/ReplayGroupView.hpp"
#include "application/views/ReplayListView.hpp"
#include "application/views/UserMotionLabelsEditor.hpp"
#include "application/views/VideoAssociatorDialog.hpp"

#include "application/views/PluginDockView.hpp"

#include "rfcommon/Session.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Profiler.hpp"

#include <QListWidget>
#include <QListWidgetItem>
#include <QCompleter>
#include <QStringListModel>
#include <QKeySequence>
#include <QShortcut>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include <QDebug>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayGroupView::ReplayGroupView(
        ReplayManager* replayManager,
        PluginManager* pluginManager,
        UserMotionLabelsManager* userMotionLabelsManager,
        rfcommon::Hash40Strings* hash40Strings,
        QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ReplayGroupView)
    , pluginManager_(pluginManager)
    , replayManager_(replayManager)
    , userMotionLabelsManager_(userMotionLabelsManager)
    , hash40Strings_(hash40Strings)
    , replayListModel_(new ReplayListModel)
    , replayListView_(new ReplayListView)
    /*, filterCompleter_(new ReplayNameCompleter)*/
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_recordingList->addWidget(replayListView_);
    ui_->layout_data->addWidget(pluginDockView_);

    replayListView_->setContextMenuPolicy(Qt::CustomContextMenu);
    replayListView_->setModel(replayListModel_.get());
    connect(replayListView_, &QListWidget::customContextMenuRequested, this, &ReplayGroupView::onItemRightClicked);

    /*ui_->lineEdit_filters->setCompleter(filterCompleter_);*/

    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), replayListView_, nullptr, nullptr, Qt::WidgetShortcut);

    replayManager_->dispatcher.addListener(this);

    connect(replayListView_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ReplayGroupView::onItemSelectionChanged);
    connect(ui_->lineEdit_filters, &QLineEdit::textChanged,
            this, &ReplayGroupView::onFiltersTextChanged);
    connect(shortcut, &QShortcut::activated,
            this, &ReplayGroupView::onDeleteKeyPressed);

    // TODO just for testing
    setReplayGroup(replayManager_->allReplayGroup());
}

// ----------------------------------------------------------------------------
ReplayGroupView::~ReplayGroupView()
{
    if (currentGroup_)
        currentGroup_->dispatcher.removeListener(this);
    replayManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void ReplayGroupView::setReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayGroupView, setReplayGroup);

    assert(currentGroup_ == nullptr);

    pluginDockView_->clearReplays();

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->fileNames())
        replayListModel_->addReplay(QString(fileName).remove(".rfr"), fileName);
    //replayListView_->sortItems(Qt::DescendingOrder);

    /*filterCompleter_->setRecordingGroupWeakRef(group);*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::clearReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayGroupView, clearReplayGroup);

    assert(currentGroup_ == group && group != nullptr);

    pluginDockView_->clearReplays();

    currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;

    replayListModel_->clear();

    /*filterCompleter_->recordingGroupExpired();*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onItemRightClicked(const QPoint& pos)
{
    PROFILE(ReplayGroupView, onItemRightClicked);

    QPoint item = replayListView_->mapToGlobal(pos);

    QMenu menu;
    QAction* editMetaData = nullptr;
    QAction* associateVideo = nullptr;
    QAction* exportPack = nullptr;
    QAction* deleteReplays = nullptr;
    QAction* a = nullptr;

    /*
    const auto selected = replayListView_->selectedItems();
    if (selected.size() == 1)
    {
        editMetaData = menu.addAction("Edit Meta Data");
        associateVideo = menu.addAction("Associate Video");
        menu.addSeparator();
        exportPack = menu.addAction("Export as replay pack");
        menu.addSeparator();
        deleteReplays = menu.addAction("Delete");
        a = menu.exec(item);
    }
    else if (selected.size() > 1)
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
        QString fileName = replayListView_->itemFileName(selected[0]);
        rfcommon::String absFileName = replayManager_->resolveGameFile(fileName.toLocal8Bit().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, absFileName.cStr());
        if (session)
        {
            ReplayEditorDialog dialog(replayManager_, session, fileName);
            dialog.exec();
        }
    }
    else if (a == associateVideo)
    {
        QString fileName = replayListView_->itemFileName(selected[0]);
        rfcommon::String absFileName = replayManager_->resolveGameFile(fileName.toLocal8Bit().constData());
        rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayManager_, absFileName.cStr());
        if (session)
        {
            VideoAssociatorDialog dialog(pluginManager_, replayManager_, session, fileName);
            dialog.exec();
        }
    }
    else if (a == exportPack)
    {
        QStringList replayFileNames;
        QStringList replayNames;
        for (QListWidgetItem* item : selected)
        {
            replayFileNames.push_back(replayListView_->itemFileName(item));
            replayNames.push_back(item->text());
        }

        ExportReplayPackDialog dialog(replayManager_, replayNames, replayFileNames);
        dialog.exec();
    }
    else if (a == deleteReplays)
    {
        if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete these replays?") == QMessageBox::Yes)
        {
            bool success = true;
            for (const auto& fileName : replayListView_->selectedReplayFileNames())
                if (replayManager_->deleteReplay(fileName) == false)
                    success = false;

            if (success == false)
                QMessageBox::critical(this, "Error deleting file(s)", "Failed to delete some of the selected files because they don't exist. They were probably deleted by an external process.");
        }
    }*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onItemSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    PROFILE(ReplayGroupView, onItemSelectionChanged);

    if (currentGroup_ == nullptr)
        return;

    pluginDockView_->clearReplays();

    auto indexes = replayListView_->selectionModel()->selectedRows();
    if (indexes.size() == 1)
    {
        const auto fileName = replayListModel_->fileName(indexes[0]);
        if (fileName == "")
            return;
        assert(QDir(fileName).isRelative());

        const QString absFilePath = QString::fromLocal8Bit(replayManager_->resolveGameFile(fileName.toLocal8Bit().constData()).cStr());
        if (absFilePath.length() == 0)
            return;
        pluginDockView_->loadGameReplays({ absFilePath });
    }
    else if (indexes.size() > 1)
    {
        QStringList absFilePaths;
        for (const auto& index : indexes)
        {
            const QString fileName = replayListModel_->fileName(index);
            if (fileName == "")
                continue;

            const QString absFilePath = QString::fromLocal8Bit(replayManager_->resolveGameFile(fileName.toLocal8Bit().constData()).cStr());
            if (absFilePath.length() > 0)
                absFilePaths.push_back(absFilePath);
        }

        pluginDockView_->loadGameReplays(absFilePaths);
    }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onFiltersTextChanged(const QString& text)
{
    PROFILE(ReplayGroupView, onFiltersTextChanged);

    QStringList rules = text.split(",");
    for (const auto& rule : rules)
    {
    }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onDeleteKeyPressed()
{
    PROFILE(ReplayGroupView, onDeleteKeyPressed);

    // Can't delete stuff in all group
    if (currentGroup_ == nullptr || currentGroup_ == replayManager_->allReplayGroup())
        return;

    for (const auto& index : replayListView_->selectionModel()->selectedRows())
    {
        const QString fileName = replayListModel_->fileName(index);
        if (fileName == "")
            continue;
        currentGroup_->removeFile(fileName);
    }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    PROFILE(ReplayGroupView, onReplayManagerGroupRemoved);

    if (currentGroup_ == group)
        clearReplayGroup(group);
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName)
{
    PROFILE(ReplayGroupView, onReplayManagerGroupNameChanged);

}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName)
{
    PROFILE(ReplayGroupView, onReplayGroupFileAdded);

    (void)group;
    /*
    auto items = replayListView_->selectedItems();

    replayListView_->addReplay(QString(fileName).remove(".rfr"), fileName);
    replayListView_->sortItems(Qt::DescendingOrder);

    if (items.size() > 0)
    {
        for (const auto& item : items)
            replayListView_->setItemSelected(item, true);
        replayListView_->scrollToItem(items[items.size() - 1]);
    }
    else
    {
        for (int i = 0; i < replayListView_->count(); ++i)
        {
            auto item = replayListView_->item(i);
            if (replayListView_->itemFileName(item) == fileName)
            {
                replayListView_->setItemSelected(item, true);
                replayListView_->scrollToItem(item);
                break;
            }
        }
    }*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName)
{
    PROFILE(ReplayGroupView, onReplayGroupFileRemoved);

    (void)group;
    replayListModel_->removeReplay(fileName);
}

}
