#include "application/ui_RecordingGroupView.h"
#include "application/views/RecordingGroupView.hpp"
#include "application/models/RecordingGroup.hpp"
#include "application/views/RecordingView.hpp"
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
RecordingGroupView::RecordingGroupView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingGroupView)
    , recordingView_(new RecordingView)
    /*, filterCompleter_(new RecordingNameCompleter)*/
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_data->addWidget(recordingView_);

    /*ui_->lineEdit_filters->setCompleter(filterCompleter_);*/

    connect(ui_->listWidget_recordings, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(ui_->lineEdit_filters, SIGNAL(textChanged(const QString&)),
            this, SLOT(onFiltersTextChanged(const QString&)));
}

// ----------------------------------------------------------------------------
RecordingGroupView::~RecordingGroupView()
{
    recordingGroupExpired();
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingGroupView::setRecordingGroupWeakRef(RecordingGroup* group)
{
    recordingGroupExpired();

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->absFilePathList())
        ui_->listWidget_recordings->addItem(fileName.completeBaseName());
    ui_->listWidget_recordings->sortItems(Qt::DescendingOrder);

    /*filterCompleter_->setRecordingGroupWeakRef(group);*/
}

// ----------------------------------------------------------------------------
void RecordingGroupView::recordingGroupExpired()
{
    if (currentGroup_)
        currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;
    ui_->listWidget_recordings->clear();
    /*filterCompleter_->recordingGroupExpired();*/
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (currentGroup_ == nullptr)
        return;

    for (const auto& fileName : currentGroup_->absFilePathList())
        if (fileName.completeBaseName() == current->text())
        {
            uh::SavedRecording* recording = uh::SavedRecording::load(fileName.absoluteFilePath().toStdString());
            if (recording)
                recordingView_->setRecording(recording);
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
void RecordingGroupView::onRecordingGroupNameChanged(RecordingGroup* group, const QString& oldName, const QString& newName)
{
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileAdded(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    ui_->listWidget_recordings->addItem(absPathToFile.completeBaseName());
    ui_->listWidget_recordings->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileRemoved(RecordingGroup* group, const QFileInfo& absPathToFile)
{
    (void)group;
    for (const auto& item : ui_->listWidget_recordings->findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
        delete item;
}

}
