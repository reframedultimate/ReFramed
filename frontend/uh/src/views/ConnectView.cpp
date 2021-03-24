#include "uh/views/ConnectView.hpp"
#include "uh/ui_ConnectView.h"

namespace uh {

// ----------------------------------------------------------------------------
ConnectView::ConnectView(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , ui_(new Ui::ConnectView)
{
    ui_->setupUi(this);
}

// ----------------------------------------------------------------------------
ConnectView::~ConnectView()
{
    delete ui_;
}

}
