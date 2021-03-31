#pragma once

#include <QDialog>

class QTcpSocket;

namespace Ui {
    class ConnectView;
}

namespace uh {

class Settings;
class ConnectedListener;

class ConnectView : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectView(Settings* settings, ConnectedListener* listener, Qt::WindowFlags flags=Qt::Popup | Qt::Dialog);
    ~ConnectView();

private slots:
    void onConnectButtonReleased();

private:
    Ui::ConnectView* ui_;
    ConnectedListener* listener_;
    QSharedDataPointer<Settings> settings_;
};

}
