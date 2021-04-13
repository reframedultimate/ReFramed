#include "uh/models/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerState::PlayerState(
        uint32_t frame,
        uint8_t stocks,
        float damage,
        float shield,
        uint16_t status,
        uint64_t motion,
        float hitstun,
        bool attack_connected)
    : motion_(motion)
    , frame_(frame)
    , damage_(damage)
    , shield_(shield)
    , hitstun_(hitstun)
    , status_(status)
    , stocks_(stocks)
    , attack_connected_(attack_connected)
{
}

}
