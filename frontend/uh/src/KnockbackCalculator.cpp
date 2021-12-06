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

double KnockbackCalculator::addMove(const Move& move)
{
    double kb = knockbackOf(move);
    addToQueue(move.id);
    return kb;
}

double KnockbackCalculator::knockbackOf(const Move& move)
{
    double effDamage = move.dmg * stalenessOf(move.id);
    double percentAfter = percent_ + effDamage;
    double effDamage = move.dmg *

    double a = percentAfter / 10.0 + percentAfter * move.dmg / 20.0;

    return (((((percent/10+percent*damage/20) * 200/(WEIGHT+100)*1.4) + 18) * kbg/100) + bkb) * rage;
}

bool KnockbackCalculator::isInQueue(int id)
{
    for (int i = 0; i != 9; ++i)
        if (id == staleQueue_[i])
            return true;
}

void KnockbackCalculator::addToQueue(int id)
{
    for (int i = 1; i != 9; ++i)
        staleQueue_[i] = staleQueue_[i-1];
    staleQueue_[0] = id;
}

double KnockbackCalculator::stalenessOf(int id)
{
    double staleness = 1.0;
    for (int i = 0; i != 9; ++i)
        if (staleQueue_[i] == id)
            staleness -= staleTable[i];
    return staleness;
}

}
