#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterFrame::FighterFrame(
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
        FighterFlags flags)
    : timeStampMs_(timeStampMs)
    , motion_(motion)
    , frame_(frame)
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
