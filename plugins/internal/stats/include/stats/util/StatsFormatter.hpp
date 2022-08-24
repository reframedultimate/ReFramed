#pragma once

#include "stats/StatType.hpp"
#include <QString>

class PlayerMeta;
class StatsCalculator;
class UserLabelsModel;

class StatsFormatter
{
public:
    StatsFormatter(
        const StatsCalculator* stats,
        const PlayerMeta* playerMeta);

    QString playerStatAsString(int fighterIdx, StatType type) const;

private:
    const StatsCalculator* stats_;
    const PlayerMeta* playerMeta_;
};
