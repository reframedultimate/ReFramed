#include "stats/models/WebSocketServer.hpp"

#include <QWebSocketServer>

// ----------------------------------------------------------------------------
WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
{}

// ----------------------------------------------------------------------------
WebSocketServer::~WebSocketServer()
{}
