#include "rfcommon/KnockbackCalculator.hpp"
#include "rfcommon/Profiler.hpp"
#include <limits>

namespace rfcommon {

double KnockbackCalculator::staleTable[9] = {
    0.09,
    0.08545,
    0.07635,
    0.0679,
    0.05945,
    0.05035,
    0.04255,
    0.03345,
    0.025
};

// ----------------------------------------------------------------------------
KnockbackCalculator::KnockbackCalculator(
        double youPercent,
        double opponentPercent,
        double opponentWeight,
        bool is1v1)
    : rage_(1 + (youPercent - 35)/115 * 0.1)
    , percent_(opponentPercent)
    , weight_(opponentWeight)
    , mul1v1_(is1v1 ? 1.2 : 1.0)
{
    if (rage_ > 1.1) rage_ = 1.1;
    if (rage_ < 1.0) rage_ = 1.0;

    for (int i = 0; i != 9; ++i)
        staleQueue_[i] = std::numeric_limits<int>::max();
}

// ----------------------------------------------------------------------------
double KnockbackCalculator::addMove(const Move& move)
{
    PROFILE(KnockbackCalculator, addMove);

    // TODO
    // - hitstun after tumble: ((knockback * 0.4) - (((knockback * 0.4) - 32) * 0.4)) - 1
    // - when does a move tumble?

    const double staleness = stalenessOf(move.id);
    const double shorthop = move.isShorthop ? 0.85 : 1.0;
    const double dmgDealt = move.dmg * shorthop * staleness * mul1v1_;
    const double dmgDealt_KB = move.dmg * shorthop * staleness;
    const double percent_KB = percent_ + dmgDealt_KB;
    percent_ += dmgDealt;

    double knockback = percent_KB / 10.0;
    knockback += percent_KB * move.dmg * (1.0 - (1.0 - staleness) * 0.3) / 20.0;
    knockback *= 200.0 / (weight_+100.0) * 1.4;
    knockback += 18;
    knockback *= move.kbg / 100.0;
    knockback += move.bkb;
    knockback *= rage_;

    addToQueue(move.id);

    return knockback;
}

// ----------------------------------------------------------------------------
bool KnockbackCalculator::isInQueue(int id)
{
    PROFILE(KnockbackCalculator, isInQueue);

    for (int i = 0; i != 9; ++i)
        if (id == staleQueue_[i])
            return true;
    return false;
}

// ----------------------------------------------------------------------------
void KnockbackCalculator::addToQueue(int id)
{
    PROFILE(KnockbackCalculator, addToQueue);

    for (int i = 8; i > 0; --i)
        staleQueue_[i] = staleQueue_[i-1];
    staleQueue_[0] = id;
}

// ----------------------------------------------------------------------------
double KnockbackCalculator::stalenessOf(int id)
{
    PROFILE(KnockbackCalculator, stalenessOf);

    double staleness = 1.0;
    for (int i = 0; i != 9; ++i)
        if (staleQueue_[i] == id)
            staleness -= staleTable[i];

    // Freshness bonus
    if (staleness == 1.0)
        staleness = 1.05;

    return staleness;
}

}
