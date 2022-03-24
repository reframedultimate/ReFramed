#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdint>
#include <vector>
#include <cassert>

namespace rfcommon {

/*!
 * \brief Stores information about a single state of one player. This includes
 * the animation state, frame it occurred on, damage, and stock count.
 */
class RFCOMMON_PUBLIC_API FighterState
{
public:
    FighterState(
            TimeStamp timeStampMs,
            FrameNumber frameNumber,
            FramesLeft framesLeft,
            float posx, float posy,
            float damage,
            float hitstun,
            float shield,
            FighterStatus status,
            FighterMotion motion,
            FighterHitStatus hit_status,
            FighterStocks stocks,
            FighterFlags flags);

    FighterState withNewFrameNumber(FrameNumber number)
    {
        return FighterState(
                    timeStamp_,
                    number,
                    framesLeft_,
                    posx_, posy_,
                    damage_,
                    hitstun_,
                    shield_,
                    status_,
                    motion_,
                    hitStatus_,
                    stocks_,
                    flags_);
    }

    TimeStamp timeStamp() const { return timeStamp_; }
    FrameNumber frameNumber() const { return frameNumber_; }
    FramesLeft framesLeft() const { return framesLeft_; }
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

    bool hasSameDataAs(const FighterState& other) const
    {
        // Time stamp is allowed to be different
        if (motion() != other.motion()) return false;
        // Frame number is allowed to be different
        // Frames left is allowed to be different
        if (posx() != other.posx()) return false;
        if (posy() != other.posy()) return false;
        if (damage() != other.damage()) return false;
        if (hitstun() != other.hitstun()) return false;
        if (shield() != other.shield()) return false;
        if (status() != other.status()) return false;
        if (hitStatus() != other.hitStatus()) return false;
        if (stocks() != other.stocks()) return false;
        if (flags() != other.flags()) return false;
        return true;
    }

private:
    TimeStamp timeStamp_;         // u64
    FighterMotion motion_;        // u64
    FrameNumber frameNumber_;     // u32
    FramesLeft framesLeft_;       // u32
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

}
