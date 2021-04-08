#include "uh/models/Settings.hpp"
#include "uh/views/ConnectView.hpp"
#include "uh/ui_ConnectView.h"

namespace uh {

// ----------------------------------------------------------------------------
ConnectView::ConnectView(Settings* settings, Qt::WindowFlags flags)
    : QDialog(nullptr, flags)
    , ui_(new Ui::ConnectView)
    , settings_(settings)
{
    ui_->setupUi(this);
    ui_->lineEdit_address->setText(settings_->connectIPAddress);
    ui_->lineEdit_port->setText(QString::number(settings_->connectPort));
    ui_->label_info->setText("<font color=\"#000000\">Not connected.</font>");

    connect(ui_->pushButton_connect, SIGNAL(released()), this, SLOT(onConnectButtonReleased()));
}

// ----------------------------------------------------------------------------
ConnectView::~ConnectView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ConnectView::setConnecting()
{
    ui_->label_info->setText("<font color=\"#000000\">Connecting...</font>");
    repaint();
}

// ----------------------------------------------------------------------------
void ConnectView::setConnectFailed()
{
    ui_->label_info->setText("<font color=\"#ff0000\">Failed to connect. Make sure the uhrecorder.nro mod is installed and active</font>");
}

// ----------------------------------------------------------------------------
void ConnectView::onConnectButtonReleased()
{
    bool ok = false;
    uint16_t port = ui_->lineEdit_port->text().toUInt(&ok);
    if (ok)
        settings_->connectPort = port;
    settings_->connectIPAddress = ui_->lineEdit_address->text();

    setConnecting();
    emit connectRequest(settings_->connectIPAddress, settings_->connectPort);
}

}
