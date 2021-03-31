#include "uh/models/PlayerInfo.hpp"
#include "uh/listeners/PlayerInfoListener.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerInfo::PlayerInfo(const QString& tag, uint8_t fighterID)
    : tag_(tag)
    , fighterID_(fighterID)
{
}

// ----------------------------------------------------------------------------
void PlayerInfo::setTag(const QString &tag)
{
    tag_ = tag;
    dispatcher.dispatch(&PlayerInfoListener::onPlayerInfoTagChanged, *this);
}

}
