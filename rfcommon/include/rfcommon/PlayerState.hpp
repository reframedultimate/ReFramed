#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdint>
#include <vector>
#include <cassert>

namespace rfcommon {

class FighterFrame;
extern template class RFCOMMON_TEMPLATE_API Vector<FighterFrame, int32_t>;

/*!
 * \brief Stores information about a single state of one player. This includes
 * the animation state, frame it occurred on, damage, and stock count.
 */
class RFCOMMON_PUBLIC_API FighterFrame
{
public:
    FighterFrame(
            TimeStampMS timeStampMs,
            Frame frame,
            float posx, float posy,
            float damage,
            float hitstun,
            float shield,
            FighterStatus status,
            FighterMotion motion,
            FighterHitStatus hit_status,
            FighterStocks stocks,
            FighterFlags flags);

    TimeStampMS timeStampMs() const { return timeStampMs_; }
    Frame frame() const { return frame_; }
    float posx() const { return posx_; }
    float posy() const { return posy_; }
    float damage() const { return damage_; }
    float hitstun() const { return hitstun_; }
    float shield() const { return shield_; }
    FighterStatus status() const { return status_; }
    FighterMotion motion() const { return motion_; }
    FighterHitStatus hitStatus() const { return hitStatus_; }
    FighterStocks stocks() const { return stocks_; }
    FighterFlags flags() const { return flags_; }

private:
    friend class DataPoint;
    friend class Vector<FighterFrame>;
    FighterFrame() {}

private:
    TimeStampMS timeStampMs_;     // u64
    FighterMotion motion_;        // u64
    Frame frame_;                 // u32
    float posx_;
    float posy_;
    float damage_;
    float hitstun_;
    float shield_;
    FighterStatus status_;        // u16
    FighterHitStatus hitStatus_;  // u8
    FighterStocks stocks_;        // u8
    FighterFlags flags_;          // u8
};

inline bool operator==(const FighterFrame& lhs, const FighterFrame& rhs)
{
    if (lhs.posx() != rhs.posx()) return false;
    if (lhs.posy() != rhs.posy()) return false;
    if (lhs.damage() != rhs.damage()) return false;
    if (lhs.hitstun() != rhs.hitstun()) return false;
    if (lhs.shield() != rhs.shield()) return false;
    if (lhs.status() != rhs.status()) return false;
    if (lhs.motion() != rhs.motion()) return false;
    if (lhs.hitStatus() != rhs.hitStatus()) return false;
    if (lhs.stocks() != rhs.stocks()) return false;
    if (lhs.flags() != rhs.flags()) return false;
    return true;
}
inline bool operator!=(const FighterFrame& lhs, const FighterFrame& rhs) { return !operator==(lhs, rhs); }

}
