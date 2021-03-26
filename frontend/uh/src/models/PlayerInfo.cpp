#include "uh/models/PlayerInfo.hpp"

namespace uh {

PlayerInfo::PlayerInfo(const QString& tag, uint8_t fighterID, uint8_t entryID)
    : tag_(tag)
    , fighterID_(fighterID)
    , entryID_(entryID)
{
}

}
