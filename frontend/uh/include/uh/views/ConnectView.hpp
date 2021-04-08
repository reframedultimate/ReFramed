#pragma once

#include <QDialog>

class QTcpSocket;

namespace Ui {
    class ConnectView;
}

namespace uh {

class Settings;

class ConnectView : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectView(Settings* settings, Qt::WindowFlags flags=Qt::Popup | Qt::Dialog);
    ~ConnectView();

    void setConnecting();

signals:
    void connectRequest(const QString& ipAddress, uint16_t port);

public slots:
    void setConnectFailed();

private slots:
    void onConnectButtonReleased();

private:
    Ui::ConnectView* ui_;
    Settings* settings_;
};

}
