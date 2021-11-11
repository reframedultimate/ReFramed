#include "uh/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerState::PlayerState(
        uint64_t timeStampMs,
        Frame frame,
        float posx, float posy,
        float damage,
        float hitstun,
        float shield,
        FighterStatus status,
        FighterMotion motion,
        FighterHitStatus hit_status,
        FighterStocks stocks,
        bool attackConnected,
        bool facingDirection)
    : timeStampMs_(timeStampMs)
    , frame_(frame)
    , posx_(posx)
    , posy_(posy)
    , damage_(damage)
    , hitstun_(hitstun)
    , shield_(shield)
    , motionL_(static_cast<uint32_t>(motion & 0xFFFFFFFF))
    , motionH_(static_cast<uint8_t>((motion >> 32) & 0xFF))
    , hitStatus_(hit_status)
    , stocks_(stocks)
    , flags_(
        (static_cast<uint8_t>(attackConnected) << 0)
      | (static_cast<uint8_t>(facingDirection) << 1))
    , status_(status)
{
}

}
