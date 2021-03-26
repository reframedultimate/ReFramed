#pragma once

#include <cstdint>

namespace uh {

/*!
 * \brief Stores information about a single state of one player. This includes
 * the animation state, frame it occurred on, damage, and stock count.
 */
class PlayerState
{
public:
    PlayerState(uint32_t frame, uint16_t status, float damage, uint8_t stocks);

    uint32_t frame() const { return frame_; }
    uint16_t status() const { return status_; }
    float damage() const { return damage_; }
    uint8_t stocks() const { return stocks_; }

private:
    float damage_;
    uint32_t frame_;
    uint8_t stocks_;
    uint16_t status_;
};

}
