#include "uh/models/GameInfo.hpp"

namespace uh {

// ----------------------------------------------------------------------------
GameInfo::GameInfo()
    : timeStarted_(QDateTime::currentDateTime())
    , formatDesc_("")
    , format_(OTHER)
    , stageID_(-1)
    , gameNumber_(1)
{
}

// ----------------------------------------------------------------------------
GameInfo::GameInfo(uint16_t stageID, const QDateTime& timeStarted)
    : timeStarted_(timeStarted)
    , formatDesc_("")
    , format_(OTHER)
    , stageID_(stageID)
    , gameNumber_(1)
{
}

// ----------------------------------------------------------------------------
QString GameInfo::formatDesc() const
{
    switch (format_)
    {
        case FRIENDLIES : return "Friendlies";
        case PRACTICE   : return "Practice";
        case BO3        : return "Bo3";
        case BO5        : return "Bo5";
        case BO7        : return "Bo7";
        case FT5        : return "FT5";
        case FT10       : return "FT10";
        case OTHER      : return formatDesc_;
    }
}

// ----------------------------------------------------------------------------
void GameInfo::setGameNumber(uint8_t number)
{
    gameNumber_ = number;
}

// ----------------------------------------------------------------------------
void GameInfo::setFormat(Format format, const QString& formatDesc)
{
    format_ = format;
    if (format == OTHER)
        formatDesc_ = formatDesc;
}

}
