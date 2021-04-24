#include "uh/DataPoint.hpp"
#include "uh/Recording.hpp"

namespace uh {

// ----------------------------------------------------------------------------
DataPoint::DataPoint(const PlayerState& state, Recording* recording, int player)
    : recording_(recording)
    , state_(state)
    , playerIdx_(player)
{
}

// ----------------------------------------------------------------------------
const std::string& DataPoint::playerName() const
{
    return recording_->playerName(playerIdx_);
}

// ----------------------------------------------------------------------------
uint32_t DataPoint::combinedState() const
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
