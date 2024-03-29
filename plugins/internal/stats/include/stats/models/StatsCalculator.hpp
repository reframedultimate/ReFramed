#pragma once

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {
    class Session;
    class FighterState;
    class FrameData;
    template <int N> class Frame;
}

class StatsCalculatorListener;

class StatsCalculator
{
public:
    static const int MAX_FIGHTERS = 2;

    struct TimeInterval
    {
        rfcommon::FrameIndex start = rfcommon::FrameIndex::fromValue(0);
        rfcommon::FrameIndex end = rfcommon::FrameIndex::fromValue(0);
    };

    StatsCalculator();

    /*!
     * \brief Resets all of the stats to their default value. Call this
     * once before calling updateStatistics().
     */
    void resetStatistics();

    /*!
     * \brief Incrementally updates all of the statistics from a single
     * frame of data.
     */
    void updateStatistics(const rfcommon::Frame<4>& frame);

    /*!
     * \brief Calculates statistics for all frames in fdata and adds them
     * (incrementally) to the current stats.
     * \param fdata Frame data
     */
    void udpateStatisticsBulk(const rfcommon::FrameData* fdata);

    double avgDeathPercentAfterHit(int fighterIdx) const;
    double avgDeathPercentBeforeHit(int fighterIdx) const;
    double earliestDeathPercentAfterHit(int fighterIdx) const;
    double earliestDeathPercentBeforeHit(int fighterIdx) const;
    double latestDeathPercentAfterHit(int fighterIdx) const;
    double latestDeathPercentBeforeHit(int fighterIdx) const;
    int numNeutralWins(int fighterIdx) const;
    int numNeutralLosses(int fighterIdx) const;
    int numNonKillingNeutralWins(int fighterIdx) const;
    int numStocksTaken(int fighterIdx) const;
    int numStocks(int fighterIdx) const;
    int numSelfDestructs(int fighterIdx) const;
    double neutralWinPercent(int fighterIdx) const;
    double avgDamagePerOpening(int fighterIdx) const;
    double openingsPerKill(int fighterIdx) const;
    double stageControlPercent(int fighterIdx) const;

    /*!
     * \brief Total amount of damage (percent) a player dealt.
     *
     * In 1v1, this is the total amount of damage the opponent received.
     * This metric currently counts self damage as well (in 1v1) as there is
     * no way to distinguish self damage from projectile damage.
     *
     * With 3 or more players, this statistic doesn't include projectile damage
     * because it's not possible to distinguish self damage from projectile damage.
     */
    double totalDamageDealt(int fighterIdx) const;

    /*!
     * \brief Total amount of damage (percent) received.
     */
    double totalDamageTaken(int fighterIdx) const;

    rfcommon::FighterMotion mostCommonNeutralOpeningMove(int fighterIdx) const;
    rfcommon::FighterMotion mostCommonKillMove(int fighterIdx) const;
    rfcommon::FighterMotion mostCommonNeutralOpenerIntoKillMove(int fighterIdx) const;

    rfcommon::Vector<TimeInterval> advantageStateTimeIntervals(int fighterIdx) const;
    rfcommon::Vector<TimeInterval> ledgeTimeIntervals(int fighterIdx) const;

    rfcommon::ListenerDispatcher<StatsCalculatorListener> dispatcher;

private:
    void updateStatsSilent(const rfcommon::Frame<4>& frame);

private:
    // Helper, tracks when players enter and leave neutral state
    struct Helpers {
        void reset();
        void update(const rfcommon::Frame<4>& frame);

        int wasInNeutralState[MAX_FIGHTERS];
        int isInNeutralState[MAX_FIGHTERS];
        int neutralStateResetCounter_[MAX_FIGHTERS];
    } helpers;

    // Variables for tracking damage taken/dealt
    struct DamageCounters {
        void reset();
        void update(const rfcommon::Frame<4>& frame);

        double totalDamageTaken[MAX_FIGHTERS];
        double totalDamageDealt[MAX_FIGHTERS];
        double oldDamage_[MAX_FIGHTERS];
    } damageCounters;

    // Variables for tracking at what percents players die at
    struct DamagesAtDeath {
        void reset();
        void update(const rfcommon::Frame<4>& frame);

        int oldStocks_[MAX_FIGHTERS];
        rfcommon::SmallVector<double, 4> damagesAtDeath[MAX_FIGHTERS];
    } damagesAtDeath;

    // Stores the player that got the first kill
    struct FirstBlood {
        void reset();
        void update(const rfcommon::Frame<4>& frame);

        int firstBloodFighterIdx;
    } firstBlood;

    // Stores the initial value of the game's stock count
    struct Deaths {
        void reset();
        void update(const rfcommon::Frame<4>& frame);

        int initialStocks;
        int numDeaths[MAX_FIGHTERS];
    } deaths;

    // Variables for tracking stage control
    struct StageControl {
        void reset();
        void update(const Helpers& helpers, const rfcommon::Frame<4>& frame);

        int stageControl[MAX_FIGHTERS];
    } stageControl;

    // Variables for tracking neutral openers, how much damage
    // the resulting string does, and whether it kills or not
    struct StringFinder {
        void reset();
        void update(const Helpers& helpers, const rfcommon::Frame<4>& frame);

        struct String {
            rfcommon::Vector<rfcommon::FighterMotion> moves;  // List of all moves in the string/combo
            TimeInterval timeInterval;   // Start and end frame of the string
            double damageAtStart = 0.0;  // Damage before the first hit connected
            double damageAtEnd = 0.0;    // Damage after the last hit connected
            bool killed = false;  // Whether the string/combo killed
        };

        rfcommon::Vector<String> strings[MAX_FIGHTERS];

        double oldDamage_[MAX_FIGHTERS];
        int beingCombodByIdx_[MAX_FIGHTERS];  // Stores the index of the fighter that's doing the combo on me
        int hitstunCounter_[MAX_FIGHTERS];    // We use some leniency between hits. This is used to find the end of the string.
    } stringFinder;
};
