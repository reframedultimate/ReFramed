#include "stats/models/StatsCalculator.hpp"
#include "stats/listeners/StatsCalculatorListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/LinearMap.hpp"
#include <limits>

// TODO Extract this info from the global mapping info structure, once
// ReFramed's refactoring is done and this is possible.
static const auto FIGHTER_STATUS_KIND_LANDING = rfcommon::FighterStatus::fromValue(22);
static const auto FIGHTER_STATUS_KIND_PASSIVE = rfcommon::FighterStatus::fromValue(103);
static const auto FIGHTER_STATUS_KIND_PASSIVE_FB = rfcommon::FighterStatus::fromValue(104);
static const auto FIGHTER_STATUS_KIND_WAIT = rfcommon::FighterStatus::fromValue(0);
static const auto FIGHTER_STATUS_KIND_GUARD_ON = rfcommon::FighterStatus::fromValue(27);
static const auto FIGHTER_STATUS_KIND_JUMP_SQUAT = rfcommon::FighterStatus::fromValue(10);
static const auto FIGHTER_STATUS_KIND_WALK = rfcommon::FighterStatus::fromValue(1);
static const auto FIGHTER_STATUS_KIND_DASH = rfcommon::FighterStatus::fromValue(3);
static const auto FIGHTER_STATUS_KIND_REBIRTH = rfcommon::FighterStatus::fromValue(182);
static const auto FIGHTER_STATUS_KIND_SHIELD_BREAK_FLY = rfcommon::FighterStatus::fromValue(92);
static const auto FIGHTER_STATUS_KIND_DEAD = rfcommon::FighterStatus::fromValue(181);

// ----------------------------------------------------------------------------
static bool isTouchingGround(const rfcommon::FighterState& state)
{
    const rfcommon::FighterStatus landStates[] = {
        FIGHTER_STATUS_KIND_LANDING,
        FIGHTER_STATUS_KIND_PASSIVE,
        FIGHTER_STATUS_KIND_PASSIVE_FB,
        FIGHTER_STATUS_KIND_WAIT,
        FIGHTER_STATUS_KIND_GUARD_ON,
        FIGHTER_STATUS_KIND_JUMP_SQUAT,
        FIGHTER_STATUS_KIND_WALK,
        FIGHTER_STATUS_KIND_DASH
    };

    for (int i = 0; i != sizeof(landStates) / sizeof(*landStates); ++i)
        if (state.status() == landStates[i])
            return true;

    return false;
}

// ----------------------------------------------------------------------------
StatsCalculator::StatsCalculator()
{
    StatsCalculator::resetStatistics();
}

// ----------------------------------------------------------------------------
void StatsCalculator::resetStatistics()
{
    damageCounters.reset();
    damagesAtDeath.reset();
    firstBlood.reset();
    deaths.reset();
    stageControl.reset();
    stringFinder.reset();

    dispatcher.dispatch(&StatsCalculatorListener::onStatsUpdated);
}

// ----------------------------------------------------------------------------
void StatsCalculator::updateStatsSilent(const rfcommon::Frame<4>& frame)
{
    // We only care about 1v1 for now
    if (frame.count() != 2)
        return;

    damageCounters.update(frame);
    damagesAtDeath.update(frame);
    firstBlood.update(frame);
    deaths.update(frame);
    stageControl.update(frame);
    stringFinder.update(frame);
}

// ----------------------------------------------------------------------------
void StatsCalculator::updateStatistics(const rfcommon::Frame<4>& frame)
{
    updateStatsSilent(frame);
    dispatcher.dispatch(&StatsCalculatorListener::onStatsUpdated);
}

// ----------------------------------------------------------------------------
void StatsCalculator::udpateStatisticsBulk(const rfcommon::FrameData* fdata)
{
    for (int frameIdx = 0; frameIdx != fdata->frameCount(); ++frameIdx)
    {
        rfcommon::Frame<4> frame;
        for (int fighterIdx = 0; fighterIdx != fdata->fighterCount(); ++fighterIdx)
            frame.push(fdata->stateAt(fighterIdx, frameIdx));

        updateStatsSilent(frame);
    }

    dispatcher.dispatch(&StatsCalculatorListener::onStatsUpdated);
}

// ----------------------------------------------------------------------------
void StatsCalculator::DamageCounters::reset()
{
    for (int i = 0; i != MAX_FIGHTERS; ++i)
    {
        totalDamageTaken[i] = 0.0;
        totalDamageDealt[i] = 0.0;
        oldDamage_[i] = 0.0;
    }
}
void StatsCalculator::DamageCounters::update(const rfcommon::Frame<4>& frame)
{
    for (int i = 0; i != frame.count(); ++i)
    {
        const double deltaDamage = frame[i].damage() - oldDamage_[i];
        oldDamage_[i] = frame[i].damage();

        // Only care about cases where damage increases. Note that delta can be negative
        // when a player heals or respawns.
        if (deltaDamage > 0.0)
        {
            totalDamageTaken[i] += deltaDamage;

            // Figure out which player dealt the damage. If there are more than 2
            // players we have to rely on attackConnected().
            // XXX There is currently no way to distinguish self-damage from projectile
            // damage. attackConnected() is not true for projectiles.
            for (int j = 0; j != frame.count(); ++j)
            {
                if (i == j)
                    continue;

                if (frame.count() == 2)  // Is 1v1
                {
                    // This ignores self damage but it's close enough
                    totalDamageDealt[j] += deltaDamage;
                }
                else
                {
                    // This ignores projectile damage
                    if (frame[j].flags().attackConnected())
                        totalDamageDealt[j] += deltaDamage;
                }
            }

            // Damage but accounting for heals
            // if (states[i].damage() != oldDamage_[i] && frame[i].status() != FIGHTER_STATUS_KIND_REBIRTH)
            //     damageTaken_[i] += states[i].damage() - oldDamage_[i];
        }
    }
}

// ----------------------------------------------------------------------------
void StatsCalculator::DamagesAtDeath::reset()
{
    for (int i = 0; i != MAX_FIGHTERS; ++i)
    {
        oldStocks_[i] = 0;
        damagesAtDeath[i].clearCompact();
    }
}
void StatsCalculator::DamagesAtDeath::update(const rfcommon::Frame<4>& frame)
{
    for (int i = 0; i != frame.count(); ++i)
    {
        // This is either the first frame, or the player somehow gained a stock (items lol?)
        if (oldStocks_[i] < frame[i].stocks().count())
            oldStocks_[i] = frame[i].stocks().count();

        // Store player damage at death
        if (frame[i].stocks().count() < oldStocks_[i])
        {
            damagesAtDeath[i].push(frame[i].damage());
            oldStocks_[i] = frame[i].stocks().count();
        }
    }
}

// ----------------------------------------------------------------------------
void StatsCalculator::FirstBlood::reset()
{
    firstBloodFighterIdx = -1;
}
void StatsCalculator::FirstBlood::update(const rfcommon::Frame<4>& frame)
{
    if (firstBloodFighterIdx == -1)
    {
        if (frame[0].stocks() > frame[1].stocks())
            firstBloodFighterIdx = 0;
        if (frame[0].stocks() < frame[1].stocks())
            firstBloodFighterIdx = 1;
    }
}

// ----------------------------------------------------------------------------
// SELF DESTRUCTS WIP
void StatsCalculator::Deaths::reset()
{
  initialStocks = -1;
  for (int i = 0; i != MAX_FIGHTERS; ++i)
      numDeaths[i] = 0;
}
void StatsCalculator::Deaths::update(const rfcommon::Frame<4>& frame)
{
    if (initialStocks == -1)
        initialStocks = frame[0].stocks().count();

    for (int i = 0; i != frame.count(); ++i)
    {
        numDeaths[i] = initialStocks - frame[i].stocks().count();
    }
}

// ----------------------------------------------------------------------------
void StatsCalculator::StageControl::reset()
{
    for (int i = 0; i != MAX_FIGHTERS; ++i)
    {
        isInNeutralState_[i] = 1;
        neutralStateResetCounter_[i] = 0;
        stageControl[i] = 0;
    }
}
void StatsCalculator::StageControl::update(const rfcommon::Frame<4>& frame)
{
    // Got hit? No longer in neutral state
    for (int i = 0; i != frame.count(); ++i)
        if (frame[i].hitstun() > 0.0 || frame[i].status() == FIGHTER_STATUS_KIND_SHIELD_BREAK_FLY)
        {
            isInNeutralState_[i] = 0;
            neutralStateResetCounter_[i] = 45;  // Some arbitrary value to make sure we don't reset
                                                // back to neutral state too early
        }

    // If player is no longer in hitstun and touches the ground, we put them back
    // into neutral state
    for (int i = 0; i != frame.count(); ++i)
        if (frame[i].hitstun() == 0.0 && isTouchingGround(frame[i]))
        {
            if (neutralStateResetCounter_[i] > 0)
                neutralStateResetCounter_[i]--;
            else
                isInNeutralState_[i] = 1;
        }

    // Figure out which player is in neutral and closest to stage center
    int playerInStageControl = -1;
    double distanceToCenter = std::numeric_limits<double>::max();
    for (int i = 0; i != frame.count(); ++i)
    {
        if (isInNeutralState_[i] && std::abs(frame[i].pos().x()) < distanceToCenter)
        {
            distanceToCenter = std::abs(frame[i].pos().x());
            playerInStageControl = i;
        }
    }

    // Accumulate
    if (playerInStageControl > -1)
        stageControl[playerInStageControl]++;
}

// ----------------------------------------------------------------------------
void StatsCalculator::StringFinder::reset()
{
    for (int i = 0; i != MAX_FIGHTERS; ++i)
    {
        strings[i].clearCompact();
        oldStatus_[i] = 0;
        oldHitstun_[i] = 0.0;
        oldStocks_[i] = 0;
        isInNeutralState_[i] = 1;
        neutralStateResetCounter_[i] = 0;
        beingCombodByIdx_[i] = -1;
        opponentDamageAtOpening_[i] = 0.0;
    }
}
void StatsCalculator::StringFinder::update(const rfcommon::Frame<4>& frame)
{
    // To make things a little clearer, we are looking at this from the
    // perspective of the player dealing the damage ("me"). The player
    // getting combo'd is "them"
    for (int them = 0; them != frame.count(); ++them)
    {
        // Detect if a move hit by seeing if the damage increased, and if the player enters
        // hitstun. The player can receive damage from the blastzone, this is why we check
        // hitstun as well.
        if ((frame[them].hitstun() > oldHitstun_[them]) ||
            (frame[them].status() == FIGHTER_STATUS_KIND_SHIELD_BREAK_FLY && oldStatus_[them] != FIGHTER_STATUS_KIND_SHIELD_BREAK_FLY.value()))
        {
            // This is the first time they got hit in neutral. We set up some counters so we can
            // follow the string and see how much damage it does/whether it kills
            if (isInNeutralState_[them])
            {
                // Figure out which player started the combo.
                // XXX: Currently we only support 1v1
                if (frame.count() != 2)
                    return;
                int me = beingCombodByIdx_[them] = 1 - them;  // Simply the other player

                strings[me].emplace();

                // Store the opening move that started the combo
                strings[me].back().moves.push(frame[me].motion());
                strings[me].back().damage = frame[them].damage() - oldDamage_[them];

                // Start a new damage counter for this string
                opponentDamageAtOpening_[me] = oldDamage_[them];  // Store damage before the hit
            }
            // The string is being continued
            else if (beingCombodByIdx_[them] >= 0)
            {
                int me = beingCombodByIdx_[them];

                // Add move to list
                strings[me].back().moves.push(frame[me].motion());
                strings[me].back().damage = frame[them].damage() - opponentDamageAtOpening_[me];
            }

            isInNeutralState_[them] = 0;
            neutralStateResetCounter_[them] = 45;  // Some arbitrary value to make sure we don't reset
                                                   // back to neutral state too early
        }
    }

    // If the player dies, mark that the string killed
    for (int i = 0; i != frame.count(); ++i)
    {
        if (beingCombodByIdx_[i] > -1 && frame[i].status() == FIGHTER_STATUS_KIND_DEAD && oldStatus_[i] != FIGHTER_STATUS_KIND_DEAD.value())
        {
            int me = beingCombodByIdx_[i];
            strings[me].back().damage = opponentDamageAtOpening_[me] - frame[i].damage();
            strings[me].back().killed = true;
            beingCombodByIdx_[i] = -1;
            neutralStateResetCounter_[i] = 0;
            isInNeutralState_[i] = 1;
        }
    }

    // If player is no longer in hitstun and touches the ground, we put them back
    // into neutral state
    for (int i = 0; i != frame.count(); ++i)
        if (frame[i].hitstun() == 0.0 && isTouchingGround(frame[i]))
        {
            if (neutralStateResetCounter_[i] > 0)
            {
                neutralStateResetCounter_[i]--;
                if (neutralStateResetCounter_[i] == 0)
                {
                    isInNeutralState_[i] = 1;
                    beingCombodByIdx_[i] = -1;
                }
            }
        }

    // Have to update old vars
    for (int i = 0; i != frame.count(); ++i)
    {
        oldHitstun_[i] = frame[i].hitstun();
        oldDamage_[i] = frame[i].damage();
        oldStatus_[i] = frame[i].status().value();
    }
}

// ----------------------------------------------------------------------------
double StatsCalculator::avgDeathPercent(int fighterIdx) const
{
    if (damagesAtDeath.damagesAtDeath[fighterIdx].count() == 0)
        return 0.0;

    double sum = 0.0;
    for (double percent : damagesAtDeath.damagesAtDeath[fighterIdx])
        sum += percent;
    sum /= damagesAtDeath.damagesAtDeath[fighterIdx].count();
    return sum;
}

// ----------------------------------------------------------------------------
double StatsCalculator::earliestDeathPercent(int fighterIdx) const
{
    if (damagesAtDeath.damagesAtDeath[fighterIdx].count() == 0)
        return 0.0;

    double low = std::numeric_limits<double>::max();
    for (double percent : damagesAtDeath.damagesAtDeath[fighterIdx])
        if (low > percent)
            low = percent;
    return low;
}

// ----------------------------------------------------------------------------
double StatsCalculator::latestDeathPercent(int fighterIdx) const
{
    if (damagesAtDeath.damagesAtDeath[fighterIdx].count() == 0)
        return 0.0;

    double high = 0.0;
    for (double percent : damagesAtDeath.damagesAtDeath[fighterIdx])
        if (high < percent)
            high = percent;
    return high;
}

// ----------------------------------------------------------------------------
int StatsCalculator::numNeutralWins(int fighterIdx) const
{
    return stringFinder.strings[fighterIdx].count();
}

// ----------------------------------------------------------------------------
int StatsCalculator::numNeutralLosses(int fighterIdx) const
{
    if (fighterIdx > 1)  // Only support 1v1 for now
        return 0;

    const int opponentIdx = 1 - fighterIdx;
    return stringFinder.strings[opponentIdx].count();
}

// ----------------------------------------------------------------------------
int StatsCalculator::numNonKillingNeutralWins(int fighterIdx) const
{
    int count = 0;
    for (const auto& string : stringFinder.strings[fighterIdx])
        if (string.killed == false)
            count++;
    return count;
}

// ----------------------------------------------------------------------------
int StatsCalculator::numStocksTaken(int fighterIdx) const
{
    int count = 0;
    for (const auto& string : stringFinder.strings[fighterIdx])
        if (string.killed)
            count++;
    return count;
}

// ----------------------------------------------------------------------------
// SELF DESTRUCTS WIP
int StatsCalculator::numStocks(int fighterIdx) const
{
    if (fighterIdx > 1)  // Only support 1v1 for now
        return 0;

    int opponentIdx = 1 - fighterIdx;

    return deaths.numDeaths[opponentIdx];
}

// ----------------------------------------------------------------------------
// SELF DESTRUCTS WIP
int StatsCalculator::numSelfDestructs(int fighterIdx) const
{
    if (fighterIdx > 1)  // Only support 1v1 for now
        return 0;

    int opponentIdx = 1 - fighterIdx;

    return numStocks(opponentIdx) - numStocksTaken(opponentIdx);
}

// ----------------------------------------------------------------------------
double StatsCalculator::neutralWinPercent(int fighterIdx) const
{
    const double wins = static_cast<double>(numNeutralWins(fighterIdx));
    const double losses = static_cast<double>(numNeutralLosses(fighterIdx));

    if (wins + losses == 0.0)
        return 0.0;

    return wins / (wins + losses) * 100.0;
}

// ----------------------------------------------------------------------------
double StatsCalculator::avgDamagePerOpening(int fighterIdx) const
{
    const int neutralWins = numNeutralWins(fighterIdx);
    if (neutralWins == 0)
        return 0.0;
    return totalDamageDealt(fighterIdx) / numNeutralWins(fighterIdx);
}

// ----------------------------------------------------------------------------
double StatsCalculator::openingsPerKill(int fighterIdx) const
{
    // See issue #10 - Used to be numNeutralWins() / numStocksTaken(),
    // but the problem is this assumes that the last few neutral openings
    // lead to a kill.
    //
    // suppose:
    //   you win neutral 2 times to take the first stock
    //   you win neutral 2 more times to take the second stock
    //   you win neutral 100 times but don't take the third stock, then you lose
    //   numNeutralWins = 104
    //   numKillingNeutralWins = 2
    //   we want to compute neutralWinsBeforeLastStockYouTook = 4

    const auto& fighterStrings = stringFinder.strings[fighterIdx];
    int neutralWinsBeforeLastStockTaken;
    for (int i = fighterStrings.count() - 1; i >= 0; --i)
        if (fighterStrings[i].killed)  // This is the last stock that this fighter took
        {
            neutralWinsBeforeLastStockTaken = i + 1;  // Since we index from 0, not 1
            break;
        }

    const int stocksTaken = numStocksTaken(fighterIdx);
    if (stocksTaken == 0)
        return 0.0;
    return static_cast<double>(neutralWinsBeforeLastStockTaken) / stocksTaken;
}

// ----------------------------------------------------------------------------
double StatsCalculator::stageControlPercent(int fighterIdx) const
{
    int totalStageControl = 0;
    for (int stageControl : stageControl.stageControl)
        totalStageControl += stageControl;

    if (totalStageControl == 0)
        return 0.0;

    return static_cast<double>(stageControl.stageControl[fighterIdx]) * 100.0 / totalStageControl;
}

// ----------------------------------------------------------------------------
double StatsCalculator::totalDamageDealt(int fighterIdx) const
{
    return damageCounters.totalDamageDealt[fighterIdx];
}

// ----------------------------------------------------------------------------
double StatsCalculator::totalDamageTaken(int fighterIdx) const
{
    return damageCounters.totalDamageTaken[fighterIdx];
}

// ----------------------------------------------------------------------------
rfcommon::FighterMotion StatsCalculator::mostCommonNeutralOpeningMove(int fighterIdx) const
{
    rfcommon::SmallLinearMap<rfcommon::FighterMotion, int, 8> candidates;
    for (const auto& string : stringFinder.strings[fighterIdx])
        candidates.insertOrGet(string.moves.front(), 0)->value()++;

    auto mostUsed = rfcommon::FighterMotion::makeInvalid();
    int timesUsed = 0;
    for (auto candidate : candidates)
        if (timesUsed < candidate.value())
        {
            timesUsed = candidate.value();
            mostUsed = candidate.key();
        }

    return mostUsed;
}

// ----------------------------------------------------------------------------
rfcommon::FighterMotion StatsCalculator::mostCommonKillMove(int fighterIdx) const
{
    rfcommon::SmallLinearMap<rfcommon::FighterMotion, int, 8> candidates;
    for (const auto& string : stringFinder.strings[fighterIdx])
        if (string.killed)
            candidates.insertOrGet(string.moves.back(), 0)->value()++;

    auto mostUsed = rfcommon::FighterMotion::makeInvalid();
    int timesUsed = 0;
    for (auto candidate : candidates)
        if (timesUsed < candidate.value())
        {
            timesUsed = candidate.value();
            mostUsed = candidate.key();
        }

    return mostUsed;
}

// ----------------------------------------------------------------------------
rfcommon::FighterMotion StatsCalculator::mostCommonNeutralOpenerIntoKillMove(int fighterIdx) const
{
    rfcommon::SmallLinearMap<rfcommon::FighterMotion, int, 8> candidates;
    for (const auto& string : stringFinder.strings[fighterIdx])
        if (string.killed)
            candidates.insertOrGet(string.moves.front(), 0)->value()++;

    auto mostUsed = rfcommon::FighterMotion::makeInvalid();
    int timesUsed = 0;
    for (auto candidate : candidates)
        if (timesUsed < candidate.value())
        {
            timesUsed = candidate.value();
            mostUsed = candidate.key();
        }

    return mostUsed;
}
