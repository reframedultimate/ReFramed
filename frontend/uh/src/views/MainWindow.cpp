#include "uh/ui_MainWindow.h"
#include "uh/views/ConnectView.hpp"
#include "uh/views/MainWindow.hpp"

namespace uh {

// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    connect(ui_->action_Connect, SIGNAL(triggered(bool)), this, SLOT(onConnectActionTriggered()));
}

// ----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MainWindow::onConnectActionTriggered()
{
    ConnectView* c = new ConnectView(this, Qt::Popup | Qt::Dialog);
    c->setWindowModality(Qt::WindowModal);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
}

}
