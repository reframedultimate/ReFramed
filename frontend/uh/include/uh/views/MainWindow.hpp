#pragma once

#include <QMainWindow>
#include "uh/listeners/ConnectedListener.hpp"

namespace Ui {
    class MainWindow;
}

namespace uh {

class ActiveRecordingView;
class Protocol;
class Settings;

class MainWindow : public QMainWindow, public ConnectedListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void transferSocketOwnership(tcp_socket socket) override;

private slots:
    void onConnectActionTriggered();

private:
    Ui::MainWindow* ui_ = nullptr;
    ActiveRecordingView* activeRecordingView_ = nullptr;
    QSharedDataPointer<Settings> settings_;
    QScopedPointer<Protocol> protocol_;
};

}

