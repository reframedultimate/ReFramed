#pragma once

#include <QString>
#include <QDateTime>

namespace uh {

class GameInfo
{
public:
    GameInfo();
    GameInfo(uint16_t stageID, const QDateTime& date);

    uint8_t stageID() const { return stageID_; }
    const QDateTime& timeStarted() const { return date_; }

private:
    QDateTime date_;
    uint8_t stageID_;
};

}
