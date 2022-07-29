#include "rfcommon/FighterState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterState::FighterState(
        TimeStamp timeStampMs,
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
        FighterFlags flags)
    : timeStamp_(timeStampMs)
    , motion_(motion)
    , frameNumber_(frameNumber)
    , framesLeft_(framesLeft)
    , posx_(posx)
    , posy_(posy)
    , damage_(damage)
    , hitstun_(hitstun)
    , shield_(shield)
    , status_(status)
    , hitStatus_(hitStatus)
    , stocks_(stocks)
    , flags_(flags)
{}

// ----------------------------------------------------------------------------
FighterState FighterState::withNewFrameNumber(FrameNumber number)
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

}
