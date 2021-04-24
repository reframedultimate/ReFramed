#include "uh/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerState::PlayerState(
        uint64_t timeStampMs,
        uint32_t frame,
        float posx, float posy,
        float damage,
        float hitstun,
        float shield,
        uint16_t status,
        uint64_t motion,
        uint8_t hit_status,
        uint8_t stocks,
        bool attackConnected,
        bool facingDirection)
    : timeStampMs_(timeStampMs)
    , frame_(frame)
    , posx_(posx)
    , posy_(posy)
    , damage_(damage)
    , hitstun_(hitstun)
    , shield_(shield)
    , hitStatus_(hit_status)
    , stocks_(stocks)
    , flags_(
        (static_cast<uint8_t>(attackConnected) << 0)
      | (static_cast<uint8_t>(facingDirection) << 1))
    , motionL_(static_cast<uint32_t>(motion & 0xFFFFFFFF))
    , motionH_(static_cast<uint8_t>((motion >> 32) & 0xFF))
    , status_(status)
{
}

}
