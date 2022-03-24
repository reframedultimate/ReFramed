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
        FighterHitStatus hit_status,
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
    , hitStatus_(hit_status)
    , stocks_(stocks)
    , flags_(flags)
{
}

}
