#pragma once

#include <cstdint>

class QString;

class WebSocketServerListener
{
public:
    virtual void onWSServerStarting() = 0;
    virtual void onWSServerFailedToStart(const QString& error) = 0;
    virtual void onWSServerStarted(const QString& host, uint16_t port) = 0;
    virtual void onWSServerStopped() = 0;

    virtual void onWSClientConnected(const QString& address, uint16_t port) = 0;
    virtual void onWSClientError(const QString& error) = 0;
};
