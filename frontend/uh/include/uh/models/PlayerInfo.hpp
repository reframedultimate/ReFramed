#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include <QString>

namespace uh {

class PlayerInfoListener;

class PlayerInfo
{
public:
    PlayerInfo(const QString& tag, uint8_t fighterID);

    const QString& tag() const { return tag_; }
    uint8_t fighterID() const { return fighterID_; }

    void setTag(const QString& tag);

    ListenerDispatcher<PlayerInfoListener> dispatcher;

private:
    QString tag_;
    uint8_t fighterID_;
};

}
