#pragma once

#include <QString>

namespace uh {

class PlayerInfo
{
public:
    PlayerInfo(const QString& tag, uint8_t fighterID);

    const QString& tag() const { return tag_; }
    uint8_t fighterID() const { return fighterID_; }

private:
    QString tag_;
    uint8_t fighterID_;
};

}
