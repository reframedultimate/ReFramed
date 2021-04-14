#include "uh/models/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerState::PlayerState(
        uint32_t frame,
        float posx, float posy,
        float damage,
        float hitstun,
        float shield,
        uint16_t status,
        uint64_t motion,
        uint8_t hit_status,
        uint8_t stocks,
        bool attack_connected,
        bool facing_direction)
    : motion_(motion)
    , frame_(frame)
    , posx_(posx)
    , posy_(posy)
    , damage_(damage)
    , hitstun_(hitstun)
    , shield_(shield)
    , status_(status)
    , hit_status_(hit_status)
    , stocks_(stocks)
    , attack_connected_(attack_connected)
    , facing_direction_(facing_direction)
{
}

}
