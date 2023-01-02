#include "application/views/ConnectView.hpp"
#include "application/ui_ConnectView.h"
#include "application/models/Protocol.hpp"

#include "rfcommon/Profiler.hpp"

#include <QJsonObject>

namespace rfapp {

using namespace nlohmann;

// ----------------------------------------------------------------------------
ConnectView::ConnectView(Config* config, Protocol* protocol, QWidget* parent)
    : QDialog(parent)
    , ConfigAccessor(config)
    , protocol_(protocol)
    , ui_(new Ui::ConnectView)
{
    ui_->setupUi(this);

    setWindowTitle("Connect to Nintendo Switch");

    json& cfg = configRoot();
    json& jConnectView = cfg["connectview"];
    json& jLastIP = jConnectView["lastip"];
    json& jLastPort = jConnectView["lastport"];
    if (jLastIP.is_string() == false)
        jLastIP = "";
    if (jLastPort.is_number_unsigned() == false)
        jLastPort = 42069;

    ui_->lineEdit_address->setText(jLastIP.get<std::string>().c_str());
    ui_->lineEdit_port->setText(QString::number(jLastPort.get<uint16_t>()));
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
    PROFILE(ConnectView, setConnecting);

    ui_->label_info->setText("<font color=\"#000000\">Connecting...</font>");
    repaint();
}

// ----------------------------------------------------------------------------
void ConnectView::setConnectFailed(const QString& error)
{
    PROFILE(ConnectView, setConnectFailed);

    ui_->label_info->setText("<font color=\"#ff0000\">Failed to connect: " + error + ". Make sure the libreframed-server.nro mod is installed and active</font>");
}

// ----------------------------------------------------------------------------
void ConnectView::onConnectButtonReleased()
{
    PROFILE(ConnectView, onConnectButtonReleased);

    bool ok = false;
    uint16_t port = ui_->lineEdit_port->text().toUInt(&ok);
    if (!ok)
    {
        ui_->label_info->setText("<font color=\"#ff0000\">Invalid port number</font>");
        return;
    }

    json& cfg = configRoot();
    json& jConnect = cfg["connectview"];
    jConnect["lastip"] = ui_->lineEdit_address->text().toUtf8().constData();
    jConnect["lastport"] = port;

    protocol_->connectToServer(ui_->lineEdit_address->text(), port);
}

// ----------------------------------------------------------------------------
void ConnectView::onCancelButtonReleased()
{
    PROFILE(ConnectView, onCancelButtonReleased);

    protocol_->disconnectFromServer();
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(ConnectView, onProtocolAttemptConnectToServer);

    setConnecting();
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    PROFILE(ConnectView, onProtocolFailedToConnectToServer);

    setConnectFailed(errormsg);
}

// ----------------------------------------------------------------------------
void ConnectView::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(ConnectView, onProtocolConnectedToServer);

    close();
}

}
