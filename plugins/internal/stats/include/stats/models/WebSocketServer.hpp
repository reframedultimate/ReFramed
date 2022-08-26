#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Vector.hpp"
#include <QObject>
#include <QAbstractSocket>
#include <QWebSocketProtocol>

class WebSocketServerListener;

class QSslError;
class QSslPreSharedKeyAuthenticator;
class QWebSocket;
class QWebSocketServer;
class QWebSocketCorsAuthenticator;

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();

    void startServer(const QString& hostName, uint16_t port, bool secureMode);
    void stopServer();
    bool isRunning() const;

    void sendBinaryMessage(const QByteArray& ba) const;

    rfcommon::ListenerDispatcher<WebSocketServerListener> dispatcher;

private slots:
    void onAcceptError(QAbstractSocket::SocketError socketError);
    void onNewConnection();
    void onOriginAuthenticationRequired(QWebSocketCorsAuthenticator* authenticator);
    void onPeerVerifyError(const QSslError& error);
    void onPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator);
    void onServerError(QWebSocketProtocol::CloseCode closeCode);
    void onSslErrors(const QList<QSslError>& errors);

    void onSocketDisconnected(QWebSocket* sock);

private:
    QWebSocketServer* server_;
    rfcommon::Vector<QWebSocket*> clients_;
};
