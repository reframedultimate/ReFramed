#include "uh/KnockbackCalculator.hpp"
#include <limits>

namespace uh {

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
    // TODO
    // - hitstun after tumble: ((knockback * 0.4) - (((knockback * 0.4) - 32) * 0.4)) - 1
    // - when does a move tumble?

    const double staleness = stalenessOf(move.id);
    const double shorthop = move.isShorthop ? 0.85 : 1.0;
    percent_ += move.dmg * shorthop * staleness * mul1v1_;

    double knockback = percent_ / 10.0;
    knockback += percent_ * (move.dmg + 0.3 * move.dmg * staleness) / 20.0;  // XXX: This is wrong but I can't figure out a correct formula
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
    for (int i = 0; i != 9; ++i)
        if (id == staleQueue_[i])
            return true;
    return false;
}

// ----------------------------------------------------------------------------
void KnockbackCalculator::addToQueue(int id)
{
    for (int i = 1; i != 9; ++i)
        staleQueue_[i] = staleQueue_[i-1];
    staleQueue_[0] = id;
}

// ----------------------------------------------------------------------------
double KnockbackCalculator::stalenessOf(int id)
{
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
