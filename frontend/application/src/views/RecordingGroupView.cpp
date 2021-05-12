#include "application/ui_RecordingGroupView.h"
#include "application/views/RecordingGroupView.hpp"
#include "application/models/RecordingManager.hpp"
#include "application/models/RecordingGroup.hpp"
#include "application/views/RecordingView.hpp"
#include "application/views/RecordingListWidget.hpp"
#include "uh/SavedRecording.hpp"
#include "uh/PlayerState.hpp"

#include <QListWidget>
#include <QListWidgetItem>
#include <QCompleter>
#include <QStringListModel>

namespace uhapp {

class RecordingNameCompleter : public QCompleter
{
public:
    RecordingNameCompleter(QObject* parent=nullptr);

    void setRecordingGroupWeakRef(RecordingGroup* group);
    void recordingGroupExpired();

    void setCompleteCategories();
    void setCompleteTags();

    QStringList splitPath(const QString &path) const override;

private:
    RecordingGroup* group_ = nullptr;
    QStringListModel* model_;
};

RecordingNameCompleter::RecordingNameCompleter(QObject* parent)
    : QCompleter(parent)
    , model_(new QStringListModel)
{
    setCaseSensitivity(Qt::CaseInsensitive);
    setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    setModel(model_);
}

void RecordingNameCompleter::setRecordingGroupWeakRef(RecordingGroup* group)
{
    group_ = group;
    setCompleteCategories();
}

void RecordingNameCompleter::recordingGroupExpired()
{
    group_ = nullptr;
    model_->setStringList({});
}

void RecordingNameCompleter::setCompleteCategories()
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

void RecordingNameCompleter::setCompleteTags()
{
    model_->setStringList({"TheComet", "Stino"});
}

QStringList RecordingNameCompleter::splitPath(const QString &path) const
{
    return path.split("[:,]");
}

// ----------------------------------------------------------------------------
RecordingGroupView::RecordingGroupView(RecordingManager* recordingManager, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingGroupView)
    , recordingManager_(recordingManager)
    , recordingListWidget_(new RecordingListWidget)
    , recordingView_(new RecordingView)
    /*, filterCompleter_(new RecordingNameCompleter)*/
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_recordingList->addWidget(recordingListWidget_);
    ui_->layout_data->addWidget(recordingView_);

    /*ui_->lineEdit_filters->setCompleter(filterCompleter_);*/

    recordingManager_->dispatcher.addListener(this);

    connect(recordingListWidget_, &QListWidget::currentItemChanged,
            this, &RecordingGroupView::onCurrentItemChanged);
    connect(ui_->lineEdit_filters, &QLineEdit::textChanged,
            this, &RecordingGroupView::onFiltersTextChanged);
}

// ----------------------------------------------------------------------------
RecordingGroupView::~RecordingGroupView()
{
    if (currentGroup_)
        currentGroup_->dispatcher.removeListener(this);
    recordingManager_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingGroupView::setRecordingGroup(RecordingGroup* group)
{
    clear();

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->absFilePathList())
        recordingListWidget_->addRecordingFileName(fileName);
    recordingListWidget_->sortItems(Qt::DescendingOrder);

    /*filterCompleter_->setRecordingGroupWeakRef(group);*/
}

// ----------------------------------------------------------------------------
void RecordingGroupView::clear()
{
    if (currentGroup_)
        currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;

    recordingListWidget_->clear();
    recordingView_->clear();
    /*filterCompleter_->recordingGroupExpired();*/
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (currentGroup_ == nullptr)
        return;

    for (const auto& fileName : currentGroup_->absFilePathList())
        if (recordingListWidget_->itemMatchesRecordingFileName(current, fileName))
        {
            uh::SavedRecording* recording = uh::SavedRecording::load(fileName.absoluteFilePath().toStdString().c_str());
            if (recording)
                recordingView_->setRecording(recording);
            else
                recordingView_->clear();

            break;
        }
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onFiltersTextChanged(const QString& text)
{
    QStringList rules = text.split(",");
    for (const auto& rule : rules)
    {

    }
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingManagerGroupRemoved(RecordingGroup* group)
{
    if (currentGroup_ == group)
        clear();
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingManagerGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName)
{
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    recordingListWidget_->addRecordingFileName(absPathToFile);
    recordingListWidget_->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    recordingListWidget_->removeRecordingFileName(absPathToFile);
}

}
