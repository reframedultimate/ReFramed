#include "uh/ui_MainWindow.h"
#include "uh/views/MainWindow.hpp"

namespace uh {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui_;
}

}

