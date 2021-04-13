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
            uint8_t stocks,
            float damage,
            float shield,
            uint16_t status,
            uint64_t motion,
            float hitstun,
            bool attack_connected);

    uint32_t frame() const { return frame_; }
    uint8_t stocks() const { return stocks_; }
    float damage() const { return damage_; }
    float shield() const { return shield_; }
    uint16_t status() const { return status_; }
    uint64_t motion() const { return motion_; }
    float hitstun() const { return hitstun_; }
    bool attack_connected() const { return attack_connected_; }



private:
    PlayerState() {}
    friend class QVector<PlayerState>;

private:
    uint64_t motion_;
    uint32_t frame_;
    float damage_;
    float shield_;
    float hitstun_;
    uint16_t status_;
    uint8_t stocks_;
    bool attack_connected_;
};

}
