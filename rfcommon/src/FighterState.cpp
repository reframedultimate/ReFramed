#include "rfcommon/FighterState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterState::FighterState(
        TimeStamp timeStamp,
        FrameIndex frameIndex,
        FramesLeft framesLeft,
        const Vec2& position,
        float damage,
        float hitstun,
        float shield,
        FighterStatus status,
        FighterMotion motion,
        FighterHitStatus hitStatus,
        FighterStocks stocks,
        FighterFlags flags)
    : timeStamp_(timeStamp)
    , motion_(motion)
    , frameIndex_(frameIndex)
    , framesLeft_(framesLeft)
    , pos_(position)
    , damage_(damage)
    , hitstun_(hitstun)
    , shield_(shield)
    , status_(status)
    , hitStatus_(hitStatus)
    , stocks_(stocks)
    , flags_(flags)
{}

// ----------------------------------------------------------------------------
FighterState FighterState::withNewFrameIndex(FrameIndex index) const
{
    return FighterState(
        timeStamp_,
        index,
        framesLeft_,
        pos_,
        damage_,
        hitstun_,
        shield_,
        status_,
        motion_,
        hitStatus_,
        stocks_,
        flags_);
}

// ----------------------------------------------------------------------------
FighterState FighterState::withNewFrameCounters(TimeStamp timeStamp, FrameIndex index, FramesLeft framesLeft) const
{
    return FighterState(
        timeStamp,
        index,
        framesLeft,
        pos_,
        damage_,
        hitstun_,
        shield_,
        status_,
        motion_,
        hitStatus_,
        stocks_,
        flags_);
}

// ----------------------------------------------------------------------------
FighterState::~FighterState()
{}

// ----------------------------------------------------------------------------
bool FighterState::hasSameDataAs(const FighterState& other) const
{
    // Time stamp is allowed to be different
    if (motion() != other.motion()) return false;
    // Frame number is allowed to be different
    // Frames left is allowed to be different
    if (pos() != other.pos()) return false;
    if (damage() != other.damage()) return false;
    if (hitstun() != other.hitstun()) return false;
    if (shield() != other.shield()) return false;
    if (status() != other.status()) return false;
    if (hitStatus() != other.hitStatus()) return false;
    if (stocks() != other.stocks()) return false;
    if (flags() != other.flags()) return false;
    return true;
}

}
