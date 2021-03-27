#pragma once

#include <QMainWindow>
#include "uh/listeners/ConnectedListener.hpp"

namespace Ui {
    class MainWindow;
}

namespace uh {

class ActiveRecordingView;
class Protocol;
class Recording;
class Settings;

class MainWindow : public QMainWindow, public ConnectedListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void transferSocketOwnership(tcp_socket socket) override;

    // Changes the UI to reflect connected/disconnected state
    void setStateConnected();
    void setStateDisconnected();

private slots:
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();
    void onConnectionLost();
    void onProtocolFinishedARecording();

private:
    Ui::MainWindow* ui_ = nullptr;
    ActiveRecordingView* activeRecordingView_ = nullptr;
    QSharedDataPointer<Settings> settings_;
    QScopedPointer<Protocol> protocol_;
};

}
