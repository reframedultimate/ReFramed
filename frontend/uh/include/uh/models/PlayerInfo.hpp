#pragma once

#include <QString>

namespace uh {

class PlayerInfo
{
public:
    PlayerInfo(const QString& tag, uint8_t fighterID, uint8_t entryID);

    const QString& tag() const { return tag_; }
    uint8_t fighterID() const { return fighterID_; }
    uint8_t entryID() const { return entryID_; }

private:
    QString tag_;
    uint8_t fighterID_;
    uint8_t entryID_;
};

}
