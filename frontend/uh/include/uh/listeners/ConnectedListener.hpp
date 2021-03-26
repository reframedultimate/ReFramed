#pragma once

#include "uh/platform/tcp_socket.h"

namespace uh {

class ConnectedListener
{
public:
    virtual void transferSocketOwnership(tcp_socket socket) = 0;
};

}
