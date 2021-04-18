#include "application/views/ConnectView.hpp"
#include "application/ui_ConnectView.h"
#include <QJsonObject>

namespace uh {

// ----------------------------------------------------------------------------
ConnectView::ConnectView(Config* config, Qt::WindowFlags flags)
    : QDialog(nullptr, flags)
    , ConfigAccessor(config)
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

    setConnecting();
    emit connectRequest(ui_->lineEdit_address->text(), port);
}

}
