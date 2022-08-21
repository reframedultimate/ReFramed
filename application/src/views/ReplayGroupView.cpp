#include "application/ui_ReplayGroupView.h"
#include "application/models/ReplayGroup.hpp"
#include "application/models/ReplayManager.hpp"
#include "application/views/ReplayEditorDialog.hpp"
#include "application/views/ReplayGroupView.hpp"
#include "application/views/ReplayListWidget.hpp"
#include "application/views/ReplayViewer.hpp"
#include "application/views/UserMotionLabelsEditor.hpp"
#include "application/views/VideoAssociatorDialog.hpp"

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

namespace rfapp {

class ReplayNameCompleter : public QCompleter
{
public:
    ReplayNameCompleter(QObject* parent=nullptr);

    void setGroupWeakRef(ReplayGroup* group);
    void groupExpired();

    void setCompleteCategories();
    void setCompleteTags();

    QStringList splitPath(const QString &path) const override;

private:
    ReplayGroup* group_ = nullptr;
    QStringListModel* model_;
};

ReplayNameCompleter::ReplayNameCompleter(QObject* parent)
    : QCompleter(parent)
    , model_(new QStringListModel)
{
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setModel(model_);
}

void ReplayNameCompleter::setGroupWeakRef(ReplayGroup* group)
{
    PROFILE(ReplayNameCompleter, setGroupWeakRef);

    group_ = group;
    setCompleteCategories();
}

void ReplayNameCompleter::groupExpired()
{
    PROFILE(ReplayNameCompleter, groupExpired);

    group_ = nullptr;
    model_->setStringList({});
}

void ReplayNameCompleter::setCompleteCategories()
{
    PROFILE(ReplayNameCompleter, setCompleteCategories);

    QStringList list = {
        "name:",
        "tag:",
        "after:",
        "before:",
        "fighter:",
        "format:",
        "game:",
        "set:",
        "stage:",
        "winner:"
    };

    model_->setStringList(std::move(list));
}

void ReplayNameCompleter::setCompleteTags()
{
    PROFILE(ReplayNameCompleter, setCompleteTags);

    model_->setStringList({"TheComet", "Stino"});
}

QStringList ReplayNameCompleter::splitPath(const QString &path) const
{
    PROFILE(ReplayNameCompleter, splitPath);

    return path.split("[:,]");
}

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
    , replayListWidget_(new ReplayListWidget)
    /*, filterCompleter_(new ReplayNameCompleter)*/
    , replayViewer_(new ReplayViewer(pluginManager))
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_recordingList->addWidget(replayListWidget_);
    ui_->layout_data->addWidget(replayViewer_);

    replayListWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(replayListWidget_, &QListWidget::customContextMenuRequested, this, &ReplayGroupView::onItemRightClicked);

    /*ui_->lineEdit_filters->setCompleter(filterCompleter_);*/

    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), replayListWidget_, nullptr, nullptr, Qt::WidgetShortcut);

    replayManager_->dispatcher.addListener(this);

    connect(replayListWidget_, &QListWidget::itemSelectionChanged,
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

    replayViewer_->clearReplays();

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->absFilePathList())
        replayListWidget_->addReplayFileName(fileName);
    replayListWidget_->sortItems(Qt::DescendingOrder);

    /*filterCompleter_->setRecordingGroupWeakRef(group);*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::clearReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayGroupView, clearReplayGroup);

    assert(currentGroup_ == group && group != nullptr);

    replayViewer_->clearReplays();

    currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;

    replayListWidget_->clear();

    /*filterCompleter_->recordingGroupExpired();*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onItemRightClicked(const QPoint& pos)
{
    PROFILE(ReplayGroupView, onItemRightClicked);

    QPoint item = replayListWidget_->mapToGlobal(pos);

    QMenu menu;
    QAction* editMetaData = nullptr; 
    QAction* associateVideo = nullptr;
    QAction* editLabels = nullptr;
    QAction* a = nullptr;

    const auto selected = replayListWidget_->selectedItems();
    if (selected.size() == 1)
    {
        editMetaData = menu.addAction("Edit Meta Data...");
        associateVideo = menu.addAction("Associate Video...");
        a = menu.exec(item);
    }
    else if (selected.size() > 1)
    {
    }

    if (a == nullptr)
        return;

    if (a == editMetaData)
    {
        for (const auto& fileName : currentGroup_->absFilePathList())
            if (replayListWidget_->itemMatchesReplayFileName(selected[0], fileName))
            {
                QString absFileName = fileName.absoluteFilePath();
                QByteArray ba = absFileName.toUtf8();
                rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(ba.constData());
                if (session)
                {
                    ReplayEditorDialog dialog(replayManager_, session, absFileName);
                    dialog.exec();
                }
                break;
            }
    }
    else if (a == associateVideo)
    {
        for (const auto& fileName : currentGroup_->absFilePathList())
            if (replayListWidget_->itemMatchesReplayFileName(selected[0], fileName))
            {
                QString absFileName = fileName.absoluteFilePath();
                QByteArray ba = absFileName.toUtf8();
                rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(ba.constData());
                if (session)
                {
                    VideoAssociatorDialog dialog(pluginManager_, replayManager_, session, absFileName);
                    dialog.exec();
                }
                break;
            }
    }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onItemSelectionChanged()
{
    PROFILE(ReplayGroupView, onItemSelectionChanged);

    if (currentGroup_ == nullptr)
        return;

    replayViewer_->clearReplays();

    const auto selected = replayListWidget_->selectedItems();
    if (selected.size() == 1)
    {
        for (const auto& fileName : currentGroup_->absFilePathList())
            if (replayListWidget_->itemMatchesReplayFileName(selected[0], fileName))
            {
                replayViewer_->clearReplays();
                replayViewer_->loadGameReplays({fileName.absoluteFilePath()});
                break;
            }
    }
    else if (selected.size() > 1)
    {
        QStringList fileNames;
        for (auto item : selected)
            for (const auto& fileName : currentGroup_->absFilePathList())
                if (replayListWidget_->itemMatchesReplayFileName(item, fileName))
                {
                    fileNames.push_back(fileName.absoluteFilePath());
                    break;
                }


        replayViewer_->loadGameReplays(fileNames);
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

    for (const auto& absFilePath : replayListWidget_->selectedReplayFilePaths())
        currentGroup_->removeFile(absFilePath);
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
void ReplayGroupView::onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    PROFILE(ReplayGroupView, onReplayGroupFileAdded);

    (void)group;
    replayListWidget_->addReplayFileName(absPathToFile);
    replayListWidget_->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    PROFILE(ReplayGroupView, onReplayGroupFileRemoved);

    (void)group;
    replayListWidget_->removeReplayFileName(absPathToFile);
}

}
