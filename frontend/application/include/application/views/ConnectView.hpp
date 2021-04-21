#pragma once

#include "application/models/ConfigAccessor.hpp"
#include <QDialog>

class QTcpSocket;

namespace Ui {
    class ConnectView;
}

namespace uhapp {

class ConnectView : public QDialog
                  , public ConfigAccessor
{
    Q_OBJECT
public:
    explicit ConnectView(Config* config, Qt::WindowFlags flags=Qt::Popup | Qt::Dialog);
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
};

}
