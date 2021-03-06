#include "application/ui_SavedGameSessionGroupView.h"
#include "application/views/SavedGameSessionGroupView.hpp"
#include "application/models/SavedGameSessionManager.hpp"
#include "application/models/SavedGameSessionGroup.hpp"
#include "application/views/SavedGameSessionListWidget.hpp"
#include "application/views/SessionView.hpp"
#include "rfcommon/SavedGameSession.hpp"
#include "rfcommon/PlayerState.hpp"

#include <QListWidget>
#include <QListWidgetItem>
#include <QCompleter>
#include <QStringListModel>
#include <QKeySequence>
#include <QShortcut>

#include <QDebug>

namespace rfapp {

class SavedGameSessionNameCompleter : public QCompleter
{
public:
    SavedGameSessionNameCompleter(QObject* parent=nullptr);

    void setGroupWeakRef(ReplayGroup* group);
    void groupExpired();

    void setCompleteCategories();
    void setCompleteTags();

    QStringList splitPath(const QString &path) const override;

private:
    ReplayGroup* group_ = nullptr;
    QStringListModel* model_;
};

SavedGameSessionNameCompleter::SavedGameSessionNameCompleter(QObject* parent)
    : QCompleter(parent)
    , model_(new QStringListModel)
{
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setModel(model_);
}

void SavedGameSessionNameCompleter::setGroupWeakRef(ReplayGroup* group)
{
    group_ = group;
    setCompleteCategories();
}

void SavedGameSessionNameCompleter::groupExpired()
{
    group_ = nullptr;
    model_->setStringList({});
}

void SavedGameSessionNameCompleter::setCompleteCategories()
{
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

void SavedGameSessionNameCompleter::setCompleteTags()
{
    model_->setStringList({"TheComet", "Stino"});
}

QStringList SavedGameSessionNameCompleter::splitPath(const QString &path) const
{
    return path.split("[:,]");
}

// ----------------------------------------------------------------------------
ReplayGroupView::ReplayGroupView(
        ReplayManager* replayManager,
        PluginManager* pluginManager,
        QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::SavedGameSessionGroupView)
    , replayManager_(replayManager)
    , savedGameSessionListWidget_(new SavedGameSessionListWidget)
    /*, filterCompleter_(new RecordingNameCompleter)*/
    , sessionView_(new SessionView(pluginManager))
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_recordingList->addWidget(savedGameSessionListWidget_);
    ui_->layout_data->addWidget(sessionView_);

    /*ui_->lineEdit_filters->setCompleter(filterCompleter_);*/

    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), savedGameSessionListWidget_, nullptr, nullptr, Qt::WidgetShortcut);

    replayManager_->dispatcher.addListener(this);

    connect(savedGameSessionListWidget_, &QListWidget::currentItemChanged,
            this, &ReplayGroupView::onCurrentItemChanged);
    connect(ui_->lineEdit_filters, &QLineEdit::textChanged,
            this, &ReplayGroupView::onFiltersTextChanged);
    connect(shortcut, &QShortcut::activated,
            this, &ReplayGroupView::onDeleteKeyPressed);
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
void ReplayGroupView::setSavedGameSessionGroup(ReplayGroup* group)
{
    assert(currentGroup_ == nullptr);

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->absFilePathList())
        savedGameSessionListWidget_->addSavedGameSessionFileName(fileName);
    savedGameSessionListWidget_->sortItems(Qt::DescendingOrder);

    /*filterCompleter_->setRecordingGroupWeakRef(group);*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::clearSavedGameSessionGroup(ReplayGroup* group)
{
    assert(currentGroup_ == group && group != nullptr);

    if (currentSession_)
        sessionView_->clearSavedGameSession(currentSession_);
    currentSession_.drop();

    currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;

    savedGameSessionListWidget_->clear();

    /*filterCompleter_->recordingGroupExpired();*/
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (currentGroup_ == nullptr)
        return;

    for (const auto& fileName : currentGroup_->absFilePathList())
        if (savedGameSessionListWidget_->itemMatchesSavedGameSessionFileName(current, fileName))
        {
            if (currentSession_)
                sessionView_->clearSavedGameSession(currentSession_);

            currentSession_ = rfcommon::SavedGameSession::load(fileName.absoluteFilePath().toStdString().c_str());
            if (currentSession_)
                sessionView_->setSavedGameSession(currentSession_);

            break;
        }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onFiltersTextChanged(const QString& text)
{
    QStringList rules = text.split(",");
    for (const auto& rule : rules)
    {

    }
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onDeleteKeyPressed()
{
    // Can't delete stuff in all group
    if (currentGroup_ == nullptr || currentGroup_ == replayManager_->allReplayGroup())
        return;

    for (const auto& absFilePath : savedGameSessionListWidget_->selectedSavedGameSessionFilePaths())
        currentGroup_->removeFile(absFilePath);
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    if (currentGroup_ == group)
        clearSavedGameSessionGroup(group);
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName)
{
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayGroupFileAdded(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    savedGameSessionListWidget_->addSavedGameSessionFileName(absPathToFile);
    savedGameSessionListWidget_->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void ReplayGroupView::onReplayGroupFileRemoved(ReplayGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    savedGameSessionListWidget_->removeSavedGameSessionFileName(absPathToFile);
}

}
