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
    , hitStatus_(hit_status)
    , stocks_(stocks)
    , attackConnected_(attack_connected)
    , facingDirection_(facing_direction)
{
}

// ----------------------------------------------------------------------------
uint32_t PlayerState::combinedState() const
{
    /*
     * Type      | Count | Bits | Comment
     * ----------+-------+------+------------------------------------------------
     * hitstun   | bool  | 1    | Hitstun is not part of motion or status
     * connected | bool  | 1    | Hitlag (attack connecting) is not part of motion or status
     * status    | 872   | 11   | Highest value I could find is kirby (872), add another bit for safety
     * motion    | 66988 | 17   | (2021-04-14) There are 66988 unique hashes
     * ----------+-------+------+------------------------------------------------
     *                   30
     */
    return 0;
}

}
