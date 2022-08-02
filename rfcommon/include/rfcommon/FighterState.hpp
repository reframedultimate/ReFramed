#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterFlags.hpp"
#include "rfcommon/FighterHitStatus.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/FighterStocks.hpp"
#include "rfcommon/FrameNumber.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/TimeStamp.hpp"

namespace rfcommon {

/*!
 * \brief Stores information about a single state of one player. This includes
 * the animation state, frame it occurred on, damage, and stock count.
 */
class RFCOMMON_PUBLIC_API FighterState
{
public:
    FighterState(
            TimeStamp timeStamp,
            FrameNumber frameNumber,
            FramesLeft framesLeft,
            float posx, float posy,
            float damage,
            float hitstun,
            float shield,
            FighterStatus status,
            FighterMotion motion,
            FighterHitStatus hitStatus,
            FighterStocks stocks,
            FighterFlags flags);
    ~FighterState();

    FighterState withNewFrameNumber(FrameNumber number) const;
    FighterState withNewFrameCounters(TimeStamp timeStamp, FrameNumber number, FramesLeft framesLeft) const;

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

    bool hasSameDataAs(const FighterState& other) const;

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
