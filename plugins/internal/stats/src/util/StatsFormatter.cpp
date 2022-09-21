#include "stats/util/StatsFormatter.hpp"
#include "stats/models/PlayerMeta.hpp"
#include "stats/models/StatsCalculator.hpp"
#include "rfcommon/MetaData.hpp"
#include <utility>

// ----------------------------------------------------------------------------
StatsFormatter::StatsFormatter(
        const StatsCalculator* stats,
        const PlayerMeta* playerMeta)
    : stats_(stats)
    , playerMeta_(playerMeta)
{
}

// ----------------------------------------------------------------------------
QString StatsFormatter::playerStatAsString(int fighterIdx, StatType type) const
{
    switch (type)
    {
    case STAT_AVERAGE_DEATH_PERCENT:
            return QString::number(stats_->avgDeathPercentBeforeHit(fighterIdx), 'f', 1) + "% / " +
                    QString::number(stats_->avgDeathPercentAfterHit(fighterIdx), 'f', 1) + "%";
    case STAT_EARLIEST_DEATH:
            return QString::number(stats_->earliestDeathPercentBeforeHit(fighterIdx), 'f', 1) + "% / " +
                    QString::number(stats_->earliestDeathPercentAfterHit(fighterIdx), 'f', 1) + "%";
    case STAT_LATEST_DEATH:
            return QString::number(stats_->latestDeathPercentBeforeHit(fighterIdx), 'f', 1) + "% / " +
                    QString::number(stats_->latestDeathPercentAfterHit(fighterIdx), 'f', 1) + "%";
    case STAT_NEUTRAL_WINS: return QString::number(stats_->numNeutralWins(fighterIdx));
    case STAT_NEUTRAL_LOSSES: return QString::number(stats_->numNeutralLosses(fighterIdx));
    case STAT_NON_KILLING_NEUTRAL_WINS: return QString::number(stats_->numNonKillingNeutralWins(fighterIdx));
    case STAT_STOCKS_TAKEN: return QString::number(stats_->numStocksTaken(fighterIdx));
    case STAT_STOCKS: return QString::number(stats_->numStocks(fighterIdx));
    case STAT_SELF_DESTRUCTS: return QString::number(stats_->numSelfDestructs(fighterIdx));
    case STAT_NEUTRAL_WIN_PERCENT: return QString::number(stats_->neutralWinPercent(fighterIdx), 'f', 1) + "%";
    case STAT_AVERAGE_DAMAGE_PER_OPENING: return QString::number(stats_->avgDamagePerOpening(fighterIdx), 'f', 1) + "%";
    case STAT_OPENINGS_PER_KILL: return QString::number(stats_->openingsPerKill(fighterIdx), 'f', 1);
    case STAT_STAGE_CONTROL_PERCENT: return QString::number(stats_->stageControlPercent(fighterIdx), 'f', 1) + "%";
    case STAT_TOTAL_DAMAGE_DEALT: return QString::number(stats_->totalDamageDealt(fighterIdx), 'f', 1) + "%";
    case STAT_TOTAL_DAMAGE_RECEIVED: return QString::number(stats_->totalDamageTaken(fighterIdx), 'f', 1) + "%";
    case STAT_MOST_COMMON_NEUTRAL_OPENING_MOVE: {
        const rfcommon::FighterMotion motion = stats_->mostCommonNeutralOpeningMove(fighterIdx);
        return playerMeta_->moveName(fighterIdx, motion);
    } break;
    case STAT_MOST_COMMON_KILL_MOVE: {
        const rfcommon::FighterMotion motion = stats_->mostCommonKillMove(fighterIdx);
        return playerMeta_->moveName(fighterIdx, motion);
    } break;
    case STAT_MOST_COMMON_NEUTRAL_OPENING_MOVE_INTO_KILL: {
        const rfcommon::FighterMotion motion = stats_->mostCommonNeutralOpenerIntoKillMove(fighterIdx);
        return playerMeta_->moveName(fighterIdx, motion);
    }break;

    case STAT_COUNT: break;
    }

    return "";
}
