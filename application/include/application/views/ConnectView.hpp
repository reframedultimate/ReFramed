#pragma once

#include "application/models/ConfigAccessor.hpp"
#include "uh/ProtocolListener.hpp"
#include <QDialog>

class QTcpSocket;

namespace Ui {
    class ConnectView;
}

namespace uhapp {

class Protocol;

class ConnectView : public QDialog
                  , public ConfigAccessor
                  , public uh::ProtocolListener
{
    Q_OBJECT
public:
    explicit ConnectView(
            Config* config,
            Protocol* protocol,
            Qt::WindowFlags flags=Qt::Popup | Qt::Dialog
    );
    ~ConnectView();

    void setConnecting();
    void setConnectFailed(const QString& error);

private slots:
    void onConnectButtonReleased();
    void onCancelButtonReleased();

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override {}

    void onProtocolTrainingStarted(uh::RunningTrainingSession* session) override { (void)session; }
    void onProtocolTrainingEnded(uh::RunningTrainingSession* session) override { (void)session; }
    void onProtocolMatchStarted(uh::RunningGameSession* session) override { (void)session; }
    void onProtocolMatchEnded(uh::RunningGameSession* session) override { (void)session; }

private:
    Protocol* protocol_;
    Ui::ConnectView* ui_;
};

}
