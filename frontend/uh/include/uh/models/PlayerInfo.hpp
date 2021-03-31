#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include <QString>
#include <QVector>

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
    PlayerInfo() {}
    friend class QVector<PlayerInfo>;

    QString tag_;
    uint8_t fighterID_;
};

}
