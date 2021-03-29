#pragma once

#include <QString>
#include <QDateTime>

namespace uh {

class GameInfo
{
public:
    enum Format
    {
        FRIENDLIES,
        PRACTICE,
        BO3,
        BO5,
        BO7,
        FT5,
        FT10,
        OTHER
    };

    GameInfo();
    GameInfo(uint16_t stageID, const QDateTime& timeStarted);

    uint16_t stageID() const { return stageID_; }
    uint8_t gameNumber() const { return gameNumber_; }
    const QDateTime& timeStarted() const { return timeStarted_; }
    Format format() const { return format_; }
    QString formatDesc() const;

    void setGameNumber(uint8_t number);
    void setFormat(Format format, const QString& formatDesc="");

private:
    QDateTime timeStarted_;
    QString formatDesc_;
    Format format_;
    uint16_t stageID_;
    uint8_t gameNumber_;
};

}
