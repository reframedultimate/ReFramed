#include "uh/listeners/ConnectedListener.hpp"
#include "uh/models/Settings.hpp"
#include "uh/views/ConnectView.hpp"
#include "uh/ui_ConnectView.h"
#include "uh/platform/tcp_socket.h"

namespace uh {

// ----------------------------------------------------------------------------
ConnectView::ConnectView(Settings* settings, ConnectedListener* listener, Qt::WindowFlags flags)
    : QWidget(nullptr, flags)
    , ui_(new Ui::ConnectView)
    , listener_(listener)
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
void ConnectView::onConnectButtonReleased()
{
    bool ok = false;
    uint16_t port = ui_->lineEdit_port->text().toUInt(&ok);
    if (ok)
        settings_->connectPort = port;
    settings_->connectIPAddress = ui_->lineEdit_address->text();

    ui_->label_info->setText("<font color=\"#000000\">Connecting...</font>");
    repaint();

    tcp_socket sock;
    QByteArray ba = settings_->connectIPAddress.toLocal8Bit();
    if (tcp_socket_connect_to_host(&sock, ba.data(), settings_->connectPort) == 0)
    {
        listener_->transferSocketOwnership(sock);
        close();
    }
    else
    {
        ui_->label_info->setText("<font color=\"#ff0000\">Failed to connect. Make sure the uhrecorder.nro mod is installed and active</font>");
    }
}

}
