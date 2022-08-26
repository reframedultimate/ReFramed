#pragma once

#include <QObject>

class QWebSocketServer;

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();

private:
    QWebSocketServer* server_;
};
