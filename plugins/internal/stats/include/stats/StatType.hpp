#pragma once

#define STAT_TYPES_LIST \
    X(STAGE_CONTROL_PERCENT, "Stage Control", "#7a8ce9") \
    X(OPENINGS_PER_KILL, "Openings / Kill", "#7adbe9") \
    X(MOST_COMMON_NEUTRAL_OPENING_MOVE, "Most Common Neutral Opener", "#8DF77B") \
    X(MOST_COMMON_KILL_MOVE, "Most Common Kill Move", "#8DF77B") \
    X(MOST_COMMON_NEUTRAL_OPENING_MOVE_INTO_KILL, "Most Common Neutral Opener into Kill", "#8DF77B") \
    X(AVERAGE_DAMAGE_PER_OPENING, "Average Damage / Opening", "#F7C46F") \
    X(TOTAL_DAMAGE_DEALT, "Total Damage Dealt", "#F7C46F") \
    X(TOTAL_DAMAGE_RECEIVED, "Total Damage Received", "#F7C46F") \
    X(AVERAGE_DEATH_PERCENT, "Average Death%", "#FF9090") \
    X(EARLIEST_DEATH, "Earliest Death", "#FF9090") \
    X(LATEST_DEATH, "Latest Death", "#FF9090") \
    X(NEUTRAL_WINS, "Neutral Wins", "#be93f1") \
    X(NEUTRAL_LOSSES, "Neutral Losses", "#be93f1") \
    X(NON_KILLING_NEUTRAL_WINS, "Non-Killing Neutral Wins", "#be93f1") \
    X(NEUTRAL_WIN_PERCENT, "Neutral Win %", "#be93f1") \
    X(SELF_DESTRUCTS, "Self Destructs", "#F7F760") \
    X(STOCKS, "Stocks", "#F7F760") \
    X(STOCKS_TAKEN, "Stocks Taken", "#F7F760")

enum StatType
{
#define X(type, str, colorcode) STAT_##type,
    STAT_TYPES_LIST
#undef X
    STAT_COUNT
};

const char* statTypeToString(StatType type);
StatType stringToStatType(const char* s);
