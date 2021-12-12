#pragma once

#include "application/models/ConfigAccessor.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include <QDialog>

class QTcpSocket;

namespace Ui {
    class ConnectView;
}

namespace rfapp {

class Protocol;

class ConnectView : public QDialog
                  , public ConfigAccessor
                  , public rfcommon::ProtocolListener
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

    void onProtocolTrainingStarted(rfcommon::RunningTrainingSession* session) override { (void)session; }
    void onProtocolTrainingResumed(rfcommon::RunningTrainingSession* session) override { (void)session; }
    void onProtocolTrainingEnded(rfcommon::RunningTrainingSession* session) override { (void)session; }
    void onProtocolMatchStarted(rfcommon::RunningGameSession* session) override { (void)session; }
    void onProtocolMatchResumed(rfcommon::RunningGameSession* session) override { (void)session; }
    void onProtocolMatchEnded(rfcommon::RunningGameSession* session) override { (void)session; }

private:
    Protocol* protocol_;
    Ui::ConnectView* ui_;
};

}
