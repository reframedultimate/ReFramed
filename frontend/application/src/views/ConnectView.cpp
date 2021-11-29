#include "application/views/ConnectView.hpp"
#include "application/ui_ConnectView.h"
#include "application/models/Protocol.hpp"
#include <QJsonObject>

namespace uhapp {

// ----------------------------------------------------------------------------
ConnectView::ConnectView(Config* config, Protocol* protocol, Qt::WindowFlags flags)
    : QDialog(nullptr, flags)
    , ConfigAccessor(config)
    , protocol_(protocol)
    , ui_(new Ui::ConnectView)
{
    ui_->setupUi(this);

    QJsonObject& cfg = getConfig();
    if (cfg["connectview"].isObject() == false)
        cfg["connectview"] = QJsonObject();
    auto cfgConnect = cfg["connectview"].toObject();
    if (cfgConnect["lastip"].isString() == false)
        cfgConnect["lastip"] = QString();
    if (cfgConnect["lastport"].isString() == false)
        cfgConnect["lastport"] = "42069";
    cfg["connectview"] = cfgConnect;

    ui_->lineEdit_address->setText(cfgConnect["lastip"].toString());
    ui_->lineEdit_port->setText(cfgConnect["lastport"].toString());
    ui_->label_info->setText("<font color=\"#000000\">Not connected.</font>");

    protocol_->dispatcher.addListener(this);

    connect(ui_->pushButton_connect, &QPushButton::released,
            this, &ConnectView::onConnectButtonReleased);
    connect(ui_->pushButton_cancel, &QPushButton::released,
            this, &ConnectView::onCancelButtonReleased);
}

// ----------------------------------------------------------------------------
ConnectView::~ConnectView()
{
    protocol_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void ConnectView::setConnecting()
{
    ui_->label_info->setText("<font color=\"#000000\">Connecting...</font>");
    repaint();
}

// ----------------------------------------------------------------------------
void ConnectView::setConnectFailed(const QString& error)
{
    ui_->label_info->setText("<font color=\"#ff0000\">Failed to connect: " + error + "\nMake sure the uhrecorder.nro mod is installed and active</font>");
}

// ----------------------------------------------------------------------------
void ConnectView::onConnectButtonReleased()
{
    bool ok = false;
    uint16_t port = ui_->lineEdit_port->text().toUInt(&ok);
    if (!ok)
    {
        ui_->label_info->setText("<font color=\"#ff0000\">Invalid port number</font>");
        return;
    }

    QJsonObject& cfg = getConfig();
    auto cfgConnect = cfg["connectview"].toObject();
    cfgConnect["lastip"] = ui_->lineEdit_address->text();
    cfgConnect["lastport"] = QString::number(port);
    cfg["connectview"] = cfgConnect;

    protocol_->connectToServer(ui_->lineEdit_address->text(), port);
}

// ----------------------------------------------------------------------------
void ConnectView::onCancelButtonReleased()
{
    protocol_->disconnectFromServer();
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    setConnecting();
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    setConnectFailed(errormsg);
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    close();
}

}
