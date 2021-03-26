#include "uh/models/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
PlayerState::PlayerState(uint32_t frame, uint16_t status, float damage, uint8_t stocks)
    : damage_(damage)
    , frame_(frame)
    , stocks_(stocks)
    , status_(status)
{
}

}
