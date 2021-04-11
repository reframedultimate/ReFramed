#include "uh/views/RecordingGroupView.hpp"
#include "uh/ui_RecordingGroupView.h"
#include "uh/models/RecordingGroup.hpp"
#include "uh/views/RecordingView.hpp"

#include <QListWidget>

namespace uh {

// ----------------------------------------------------------------------------
RecordingGroupView::RecordingGroupView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingGroupView)
{
    ui_->setupUi(this);

    ui_->layout_data->addWidget(new RecordingView);
}

// ----------------------------------------------------------------------------
RecordingGroupView::~RecordingGroupView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void RecordingGroupView::setRecordingGroup(RecordingGroup* group)
{
    ui_->listWidget_recordings->clear();

    for (const auto& file : group->fileList())
        ui_->listWidget_recordings->addItem(QFileInfo(file.absolutePath()).baseName());
}

}
