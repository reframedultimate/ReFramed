#include "uh/views/RecordingView.hpp"
#include "uh/ui_RecordingView.h"

namespace uh {

// ----------------------------------------------------------------------------
RecordingView::RecordingView(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::RecordingView)
{
    ui_->setupUi(this);
}

// ----------------------------------------------------------------------------
RecordingView::~RecordingView()
{
    delete ui_;
}

}
