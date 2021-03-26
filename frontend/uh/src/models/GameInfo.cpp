#include "uh/models/GameInfo.hpp"

namespace uh {

// ----------------------------------------------------------------------------
GameInfo::GameInfo()
    : date_(QDateTime::currentDateTime())
    , stageID_(-1)
{
}

// ----------------------------------------------------------------------------
GameInfo::GameInfo(uint16_t stageID, const QDateTime& date)
    : date_(date)
    , stageID_(stageID)
{
}

}
