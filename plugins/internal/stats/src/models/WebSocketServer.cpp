#include "stats/models/WebSocketServer.hpp"
#include "stats/listeners/WebSocketServerListener.hpp"

#include <QWebSocket>
#include <QWebSocketServer>

// ----------------------------------------------------------------------------
WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
    , server_(nullptr)
{}

// ----------------------------------------------------------------------------
WebSocketServer::~WebSocketServer()
{
    while (clients_.count())
        clients_.back()->close();

    stopServer();
}

// ----------------------------------------------------------------------------
void WebSocketServer::startServer(const QString& hostName, uint16_t port, bool secureMode)
{
    stopServer();

    dispatcher.dispatch(&WebSocketServerListener::onWSServerStarting);

    server_ = new QWebSocketServer("ReFramed Statistics Server", secureMode ? QWebSocketServer::SecureMode : QWebSocketServer::NonSecureMode);
    if (server_->listen(hostName == "" ? QHostAddress::Any : QHostAddress(hostName), port))
    {
        connect(server_, &QWebSocketServer::acceptError, this, &WebSocketServer::onAcceptError);
        connect(server_, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
        //connect(server_, &QWebSocketServer::originAuthenticationRequired, this, &WebSocketServer::onOriginAuthenticationRequired);
        connect(server_, &QWebSocketServer::peerVerifyError, this, &WebSocketServer::onPeerVerifyError);
        //connect(server_, &QWebSocketServer::preSharedKeyAuthenticationRequired, this, &WebSocketServer::onPreSharedKeyAuthenticationRequired);
        connect(server_, &QWebSocketServer::serverError, this, &WebSocketServer::onServerError);
        connect(server_, &QWebSocketServer::sslErrors, this, &WebSocketServer::onSslErrors);

        dispatcher.dispatch(&WebSocketServerListener::onWSServerStarted, server_->serverAddress().toString(), server_->serverPort());
    }
    else
    {
        dispatcher.dispatch(&WebSocketServerListener::onWSServerFailedToStart, server_->errorString());
        delete server_;
        server_ = nullptr;
    }
}

// ----------------------------------------------------------------------------
void WebSocketServer::stopServer()
{
    if (server_ == nullptr)
        return;

    delete server_;
    server_ = nullptr;

    dispatcher.dispatch(&WebSocketServerListener::onWSServerStopped);
}

// ----------------------------------------------------------------------------
bool WebSocketServer::isRunning() const
{
    return server_ != nullptr;
}

// ----------------------------------------------------------------------------
void WebSocketServer::sendBinaryMessage(const QByteArray& ba) const
{
    for (auto client : clients_)
        client->sendBinaryMessage(ba);
}

// ----------------------------------------------------------------------------
void WebSocketServer::onAcceptError(QAbstractSocket::SocketError socketError)
{
    dispatcher.dispatch(&WebSocketServerListener::onWSClientError, "Failed to accept client connection: " + QString(socketError));
}

// ----------------------------------------------------------------------------
void WebSocketServer::onNewConnection()
{
    QWebSocket* sock = server_->nextPendingConnection();
    if (sock == nullptr)
        return;

    connect(sock, &QWebSocket::disconnected, [this, sock] {
        onSocketDisconnected(sock);
    });
    clients_.push(sock);

    dispatcher.dispatch(&WebSocketServerListener::onWSClientConnected, sock->peerAddress().toString(), sock->peerPort());
}

// ----------------------------------------------------------------------------
void WebSocketServer::onOriginAuthenticationRequired(QWebSocketCorsAuthenticator* authenticator)
{
}

// ----------------------------------------------------------------------------
void WebSocketServer::onPeerVerifyError(const QSslError& error)
{
    dispatcher.dispatch(&WebSocketServerListener::onWSClientError, "Peer verification error: " + error.errorString());
}

// ----------------------------------------------------------------------------
void WebSocketServer::onPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator)
{

}

// ----------------------------------------------------------------------------
void WebSocketServer::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
    dispatcher.dispatch(&WebSocketServerListener::onWSClientError, "Server error: " + QString(closeCode));
}

// ----------------------------------------------------------------------------
void WebSocketServer::onSslErrors(const QList<QSslError>& errors)
{
    QString errorStr;
    for (int i = 0; i != errors.size(); ++i)
    {
        if (i != 0)
            errorStr += ", ";
        errorStr += errors[i].errorString();
    }

    dispatcher.dispatch(&WebSocketServerListener::onWSClientError, "SSL errors: " + errorStr);
}

// ----------------------------------------------------------------------------
void WebSocketServer::onSocketDisconnected(QWebSocket* sock)
{
    for (int i = 0; i != clients_.count(); ++i)
        if (clients_[i] == sock)
        {
            clients_.erase(i);
            sock->deleteLater();
            break;
        }
}
