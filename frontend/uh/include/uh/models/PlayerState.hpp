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
    uint8_t hitStatus() const { return hitStatus_; }
    uint8_t stocks() const { return stocks_; }
    bool attackConnected() const { return attackConnected_; }
    bool facingDirection() const { return facingDirection_; }

    uint32_t combinedState() const;

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
    uint8_t hitStatus_;
    uint8_t stocks_;
    bool attackConnected_;
    bool facingDirection_;
};

inline bool operator==(const PlayerState& lhs, const PlayerState& rhs)
{
    if (lhs.motion() != rhs.motion()) return false;
    if (lhs.posx() != rhs.posx()) return false;
    if (lhs.posy() != rhs.posy()) return false;
    if (lhs.damage() != rhs.damage()) return false;
    if (lhs.hitstun() != rhs.hitstun()) return false;
    if (lhs.shield() != rhs.shield()) return false;
    if (lhs.status() != rhs.status()) return false;
    if (lhs.hitStatus() != rhs.hitStatus()) return false;
    if (lhs.stocks() != rhs.stocks()) return false;
    if (lhs.attackConnected() != rhs.attackConnected()) return false;
    if (lhs.facingDirection() != rhs.facingDirection()) return false;
    return true;
}
inline bool operator!=(const PlayerState& lhs, const PlayerState& rhs) { return !operator==(lhs, rhs); }

}
