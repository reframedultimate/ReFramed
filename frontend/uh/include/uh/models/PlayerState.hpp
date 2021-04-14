#pragma once

#include <cstdint>
#include <QVector>

namespace uh {

/*!
 * \brief Stores information about a single state of one player. This includes
 * the animation state, frame it occurred on, damage, and stock count.
 */
class PlayerState
{
public:
    PlayerState(
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
            bool facing_direction);

    uint32_t frame() const { return frame_; }
    float posx() const { return posx_; }
    float posy() const { return posy_; }
    float damage() const { return damage_; }
    float hitstun() const { return hitstun_; }
    float shield() const { return shield_; }
    uint16_t status() const { return status_; }
    uint64_t motion() const { return motion_; }
    uint8_t hit_status() const { return hit_status_; }
    uint8_t stocks() const { return stocks_; }
    bool attack_connected() const { return attack_connected_; }
    bool facing_direction() const { return facing_direction_; }

private:
    PlayerState() {}
    friend class QVector<PlayerState>;

private:
    uint64_t motion_;
    uint32_t frame_;
    float posx_;
    float posy_;
    float damage_;
    float hitstun_;
    float shield_;
    uint16_t status_;
    uint8_t hit_status_;
    uint8_t stocks_;
    bool attack_connected_;
    bool facing_direction_;
};

}
