#include "uh/ui_RecordingGroupView.h"
#include "uh/views/RecordingGroupView.hpp"
#include "uh/models/RecordingGroup.hpp"
#include "uh/models/SavedRecording.hpp"
#include "uh/views/RecordingView.hpp"

#include <QListWidget>
#include <QListWidgetItem>

namespace uh {

// ----------------------------------------------------------------------------
RecordingGroupView::RecordingGroupView(QWidget* parent)
    : QWidget(parent)
    , recordingView_(new RecordingView)
    , ui_(new Ui::RecordingGroupView)
{
    ui_->setupUi(this);
    ui_->splitter->setStretchFactor(0, 0);
    ui_->splitter->setStretchFactor(1, 1);
    ui_->splitter->setSizes({600});
    ui_->layout_data->addWidget(recordingView_);

    connect(ui_->listWidget_recordings, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

// ----------------------------------------------------------------------------
RecordingGroupView::~RecordingGroupView()
{
    clear();
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingGroupView::setRecordingGroup(RecordingGroup* group)
{
    clear();

    currentGroup_ = group;
    currentGroup_->dispatcher.addListener(this);

    for (const auto& fileName : group->absFilePathList())
        ui_->listWidget_recordings->addItem(fileName.completeBaseName());
    ui_->listWidget_recordings->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void RecordingGroupView::clear()
{
    if (currentGroup_)
        currentGroup_->dispatcher.removeListener(this);
    currentGroup_ = nullptr;
    ui_->listWidget_recordings->clear();
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (currentGroup_ == nullptr)
        return;

    for (const auto& fileName : currentGroup_->absFilePathList())
        if (fileName.completeBaseName() == current->text())
        {
            SavedRecording* recording = SavedRecording::load(fileName.absoluteFilePath());
            if (recording)
                recordingView_->setRecording(recording);
            break;
        }
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupNameChanged(const QString& name)
{
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileAdded(const QFileInfo& absPathToFile)
{
    ui_->listWidget_recordings->addItem(absPathToFile.completeBaseName());
    ui_->listWidget_recordings->sortItems(Qt::DescendingOrder);
}

// ----------------------------------------------------------------------------
void RecordingGroupView::onRecordingGroupFileRemoved(const QFileInfo& absPathToFile)
{
    for (const auto& item : ui_->listWidget_recordings->findItems(absPathToFile.completeBaseName(), Qt::MatchExactly))
    {
        ui_->listWidget_recordings->removeItemWidget(item);
    }
}

}
